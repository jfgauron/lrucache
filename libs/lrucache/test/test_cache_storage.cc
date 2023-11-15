#include <catch.hpp>

#include "cache/cache_storage.hxx"
#include "helpers/utilities.hxx"

TEST_CASE("Cache storage read", "[cache_storage][read]") {
    lrucache::cache_config config = build_default_cache_config();
    lrucache::cache_storage storage(config);
    std::time_t now = std::time(nullptr);
    std::time_t future = now+100;
    auto item1 = create_item(20, future);
    storage.commit_write("key1", item1, now);

    SECTION ( "when key does not exist " ) {
        auto result = storage.get_item("anotherkey", now);
        REQUIRE(result == nullptr);
    }

    SECTION ( "when key exist" ) {
        auto result = storage.get_item("key1", now);
        REQUIRE(result != nullptr);
        REQUIRE(result->data_size == 20);
        REQUIRE(result->expires_at == future);
    }

    SECTION ( "when key exist but is expired" ) {
        auto result = storage.get_item("key1", future+1);
        REQUIRE(result == nullptr);
    }

    SECTION ( "after a clear" ) {
        storage.clear();
        auto result = storage.get_item("key1", now);
        REQUIRE(result == nullptr);
    }
}

TEST_CASE("Cache storage evict data", "[cache_storage][evict]") {
    lrucache::cache_config config = build_default_cache_config();
    lrucache::cache_storage storage(config);
    std::time_t now = std::time(nullptr);
    std::time_t future = now+100;
    auto item1 = create_item(20, future);
    auto item2 = create_item(20, future);
    storage.commit_write("key1", item1, now);
    storage.commit_write("key2", item2, now);

    SECTION ( "evict entire cache" ) {
        storage.evict_lru_data("nokey", 99999);
        auto result1 = storage.get_item("key1", now);
        auto result2 = storage.get_item("key2", now);
        REQUIRE(result1 == nullptr);
        REQUIRE(result2 == nullptr);
    }

    SECTION ( "evict only required memory" ) {
        storage.evict_lru_data("nokey", 50);
        auto result1 = storage.get_item("key1", now);
        auto result2 = storage.get_item("key2", now);
        REQUIRE(result1 == nullptr);
        REQUIRE(result2 != nullptr);
    }
}

TEST_CASE("Cache storage update", "[cache_storage][update]") {
    lrucache::cache_config config = build_default_cache_config();
    config.cache_size = 300;
    lrucache::cache_storage storage(config);
    std::time_t now = std::time(nullptr);
    std::time_t future = now+100;
    auto item1 = create_item(40, future);
    auto item2 = create_item(40, future);
    storage.commit_write("key1", item1, now);
    memcpy(item1.data.get()->data(), "otherdata", 10);
    storage.commit_write("key2", item2, now);
    memcpy(item1.data.get()->data(), "olddata", 8);

    SECTION ( "updates item with new data" ) {
        auto new_item = create_item(40, future);
        memcpy(new_item.data.get()->data(), "success", 8);
        storage.commit_write("key1", new_item, now);

        auto result1 = storage.get_item("key1", now);
        REQUIRE(result1 != nullptr);
        REQUIRE(memcmp(result1, "success", 8));
    }

    SECTION ( "move item to most recently used position" ) {
        auto new_item2 = create_item(40, future);
        storage.commit_write("key3", new_item2, now);

        auto new_item = create_item(40, future);
        storage.commit_write("key1", new_item, now);
        storage.evict_lru_data("nokey", 40);

        auto result1 = storage.get_item("key1", now);
        auto result2 = storage.get_item("key2", now);
        auto result3 = storage.get_item("key3", now);
        REQUIRE(result1 != nullptr);
        REQUIRE(result2 == nullptr); // key2 was evicted instead of key1
        REQUIRE(result3 != nullptr);
    }

    SECTION ( "frees up memory if new item is smaller" ) { 
        auto new_item = create_item(40, future);
        storage.commit_write("key3", new_item, now);
     
        auto update_item = create_item(1, future);
        storage.commit_write("key1", update_item, now);
        storage.evict_lru_data("nokey", 50);

        auto result1 = storage.get_item("key1", now);
        auto result2 = storage.get_item("key2", now);
        auto result3 = storage.get_item("key3", now);
        // no item was evicted
        REQUIRE(result1 != nullptr);
        REQUIRE(result2 != nullptr);
        REQUIRE(result3 != nullptr);
    }

    SECTION ( "evicts item if new item is bigger" ) {
        auto update_item = create_item(150, future);
        storage.commit_write("key1", update_item, now);
        storage.evict_lru_data("nokey", 50);

        auto result1 = storage.get_item("key1", now);
        auto result2 = storage.get_item("key2", now);
        // no item was evicted
        REQUIRE(result1 != nullptr);
        REQUIRE(result2 == nullptr); // key2 was evicted
    }

    SECTION ( "updates the item purge buckets" ) {
        auto update_item = create_item(40, future+1000);
        storage.commit_write("key1", update_item, now);
        storage.commit_purge(future+100);

        auto result1 = storage.get_item("key1", now);
        REQUIRE(result1 != nullptr);
    } 
}

TEST_CASE("Cache storage purge expired data", "[cache_storage][purge]") {
    lrucache::cache_config config = build_default_cache_config();
    lrucache::cache_storage storage(config);
    std::time_t now = std::time(nullptr);
    std::time_t future = now+100;
    auto item1 = create_item(20, future+1000);
    auto item2 = create_item(20, future);
    storage.commit_write("key1", item1, now);
    storage.commit_write("key2", item2, now);

    SECTION ( "purge entire cache if all expired" ) {
        storage.commit_purge(future+99999);

        auto result1 = storage.get_item("key1", now);
        auto result2 = storage.get_item("key2", now);
        REQUIRE(result1 == nullptr);
        REQUIRE(result2 == nullptr);
    }

    SECTION ( "only purge expired buckets" ) {
        storage.commit_purge(future+100);

        auto result1 = storage.get_item("key1", now);
        auto result2 = storage.get_item("key2", now);
        REQUIRE(result1 != nullptr); // was not expired
        REQUIRE(result2 == nullptr);
    }
}