#include <catch.hpp>

#include "cache/cache_state.hxx"
#include "helpers/utilities.hxx"

TEST_CASE("Cache state basic read", "[cache_state][read]") {
    lrucache::cache_config config = build_default_cache_config();
    lrucache::cache_state state(config);
    size_t len = 0;
    unsigned char data[4] = { 'T', 'e', 's', 't' };
    lrucache::cache_item item(data, 4, std::time(nullptr)+1000);

    SECTION( "when cache is empty" ) {
        auto result = state.read("mykey", len);
        REQUIRE(result == nullptr);
        REQUIRE(len == 0);
    }

    SECTION ( "with an existing key" ) {
        state.commit_write("mykey", item, std::time(nullptr));
        auto result = state.read("mykey", len);
        REQUIRE(result != nullptr);
        REQUIRE(memcmp(result.get(), "Test", 4) == 0);
        REQUIRE(len == 4);
    }

    SECTION ( "with the wrong key" ) {
        state.commit_write("mykey", item, std::time(nullptr));
        auto result = state.read("otherkey", len);
        REQUIRE(result == nullptr);
        REQUIRE(len == 0);
    }

    SECTION ( "with an expired item" ) {
        item.expires_at = 0;
        state.commit_write("mykey", item, std::time(nullptr));
        auto result = state.read("mykey", len);
        REQUIRE(result == nullptr);
        REQUIRE(len == 0);
    }

    SECTION ( "read_then with a custom callback" ) {
        state.commit_write("mykey", item, std::time(nullptr));
        state.read_then("mykey", [](unsigned char* data, size_t len) {
            REQUIRE(data != nullptr);
            REQUIRE(memcmp(data, "Test", 4) == 0);
            REQUIRE(len == 4);
        });
    }
}

TEST_CASE("Cache state basic write", "[cache_state][write]") {
    lrucache::cache_config config = build_default_cache_config();
    lrucache::cache_state state(config);
    size_t len = 0;
    auto expiry = std::time(nullptr)+1000;

    SECTION ( "with multiple writes and no eviction" ) {
        auto item1 = create_item(20, expiry);
        auto item2 = create_item(20, expiry);
        state.commit_write("key1", item1, std::time(nullptr));
        state.commit_write("key2", item2, std::time(nullptr));
        auto result = state.read("key1", len);
        REQUIRE(result != nullptr);
        REQUIRE(len == 20);
    }

    SECTION ( "with eviction when cache is full" ) {
        auto item1 = create_item(20, expiry+150);
        auto item2 = create_item(20, expiry+100);
        auto item3 = create_item(20, expiry+200);

        state.commit_write("key1", item1, std::time(nullptr));
        state.commit_write("key2", item2, std::time(nullptr));
        state.commit_write("key3", item3, std::time(nullptr));
        
        auto result1 = state.read("key1", len);
        auto result2 = state.read("key2", len);
        auto result3 = state.read("key3", len);
        REQUIRE(result1 == nullptr);
        REQUIRE(result2 != nullptr);
        REQUIRE(result3 != nullptr);
    }

    SECTION ( "with large item requiring more than 1 eviction" ) {
        auto item1 = create_item(20, expiry);
        auto item2 = create_item(20, expiry);
        auto item3 = create_item(90, expiry);

        state.commit_write("key1", item1, std::time(nullptr));
        state.commit_write("key2", item2, std::time(nullptr));
        state.commit_write("key3", item3, std::time(nullptr));

        auto result1 = state.read("key1", len);
        auto result2 = state.read("key2", len);
        auto result3 = state.read("key3", len);
        REQUIRE(result1 == nullptr);
        REQUIRE(result2 == nullptr);
        REQUIRE(result3 != nullptr);
    }
}
