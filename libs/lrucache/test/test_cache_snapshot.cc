#include <catch.hpp>

#include "cache/cache_state.hxx"
#include "helpers/utilities.hxx"


TEST_CASE("Cache state snapshot commits", "[cache_state][snapshot][commits]") {
    lrucache::cache_config config = build_default_cache_config();
    config.cache_size = 350;
    lrucache::cache_state state(config);
    std::time_t now = std::time(nullptr);
    std::time_t future = now+99999;
    size_t len = 0;

    // setup snapshot data
    auto item1 = create_item(20, future);
    state.commit_write("key1", item1, now);
    auto item2 = create_item(10, future);
    state.commit_write("key2", item2, now+1);
    auto item3 = create_item(10, future);
    state.commit_write("key3", item3, now+2);
    auto item4 = create_item(10, future);
    state.commit_write("key4", item4, now+3);
    state.begin_snapshot();

    SECTION ( "frozen snapshot data is readable" ) {
        auto result = state.read("key1", len);
        REQUIRE(result != nullptr);
        REQUIRE(len == 20);
    }

    SECTION ( "read commits are possible during snapshot" ) {
        bool result = state.commit_read("key1", now);
        REQUIRE(result);
    }

    SECTION ( "write commits are possible during snapshot" ) {
        auto new_item = create_item(40, future);
    
        state.commit_write("key1", new_item, now);

        auto result = state.read("key1", len);
        REQUIRE(result != nullptr);
        REQUIRE(len == 40);
    }

    SECTION ( "purge commits are ignored during snapshot" ) {
        state.commit_purge_expired(future+99999);

        auto result = state.read("key1", len);
        REQUIRE(result != nullptr);
        REQUIRE(len == 20);
    }

    SECTION ( "commits are applied after snapshot" ) {
        auto new_item = create_item(1, now+10); // will be expired
        state.commit_write("key2", new_item, now+4);
        state.commit_read("key1", now+5);
        state.commit_purge_expired(now+50); // purge key2 -> 3 - 4 - 1
        auto new_item2 = create_item(125, future); // will evict key3
        state.commit_write("key5", new_item2, now+51);
    
        auto result1 = state.read("key1", len);
        auto result2 = state.read("key2", len);
        auto result3 = state.read("key3", len);
        auto result4 = state.read("key4", len);
        auto result5 = state.read("key5", len);

        REQUIRE(result1 != nullptr);
        REQUIRE(result2 != nullptr);
        REQUIRE(result3 != nullptr);
        REQUIRE(result4 != nullptr);
        REQUIRE(result5 != nullptr);

        state.end_snapshot();
    
        auto result6 = state.read("key1", len);
        auto result7 = state.read("key2", len);
        auto result8 = state.read("key3", len);
        auto result9 = state.read("key4", len);
        auto result10 = state.read("key5", len);

        // final cache state contains key 5 -> 1 -> 4
        REQUIRE(result6 != nullptr);
        REQUIRE(result7 == nullptr);
        REQUIRE(result8 == nullptr);
        REQUIRE(result9 != nullptr);
        REQUIRE(result10 != nullptr);
    }
}

TEST_CASE("Cache state snapshot reads", "[cache_state][snapshot][read]") {
    lrucache::cache_config config = build_default_cache_config();
    config.cache_size = 2000;
    lrucache::cache_state state(config);
    std::time_t now = std::time(nullptr);
    std::time_t future = now+99999;
    int item_index = 0;
    size_t read = 0;

    // setup snapshot data
    auto item1 = create_item(6, future);
    memcpy(item1.data.get()->data(), "value1", 6);
    state.commit_write("key1", item1, now);
    auto item2 = create_item(6, future);
    memcpy(item2.data.get()->data(), "value2", 6);
    state.commit_write("key2", item2, now+1);
    auto item3 = create_item(6, future);
    memcpy(item3.data.get()->data(), "value3", 6);
    state.commit_write("key3", item3, now+2);
    auto item4 = create_item(6, future);
    memcpy(item4.data.get()->data(), "value4", 6);
    state.commit_write("key4", item4, now+3);
    state.begin_snapshot();

    SECTION ( "read_snapshot_chunk returns items in order" ) {
        auto data = state.read_snapshot_chunk(9999, item_index, read);
        auto items = read_snapshot_data(data.get(), read);
        REQUIRE(item_index == -1);
        REQUIRE(memcmp(items[0].data.get()->data(), "value4", 6) == 0);
        REQUIRE(memcmp(items[1].data.get()->data(), "value3", 6) == 0);
        REQUIRE(memcmp(items[2].data.get()->data(), "value2", 6) == 0);
        REQUIRE(memcmp(items[3].data.get()->data(), "value1", 6) == 0);
    }

    SECTION ( "return partial snapshot if too much data" ) {
        auto data = state.read_snapshot_chunk(80, item_index, read);
        auto items = read_snapshot_data(data.get(), read);
        REQUIRE(item_index == 2);
        REQUIRE(items.size() == 2);
        auto data2 = state.read_snapshot_chunk(80, item_index, read);
        auto items2 = read_snapshot_data(data2.get(), read);
        REQUIRE(item_index == -1);
        REQUIRE(items2.size() == 2);
        REQUIRE(memcmp(items2[1].data.get()->data(), "value1", 6) == 0);
    }

    SECTION ( "commits don't change frozen snapshot data" ) {
        auto new_item = create_item(100, future);
        state.commit_write("keyX", new_item, now);
        auto data = state.read_snapshot_chunk(9999, item_index, read);
        auto items = read_snapshot_data(data.get(), read);
        REQUIRE(item_index == -1);
        REQUIRE(items.size() == 4);
        REQUIRE(memcmp(items[0].data.get()->data(), "value4", 6) == 0);
    }

}