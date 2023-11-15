#include <catch.hpp>

#include "cache/geo_locator.hxx"

struct target {
    lrucache::geo_locator::location location() const {
        return {longitude, latitude};
    }

    double latitude;
    double longitude;
};

TEST_CASE("Cache geo locator", "[geo_locator][find_closest]") {
    target start = { -133.48285, 43.47921 };

    SECTION ( "find closest target (only one target)" ) {
        // distance manually calculated using https://boulter.com/gps/distance/
        std::vector<target> targets = {
            {77.02360, -24.09066} // 4439.17 km
        };

        auto closest = lrucache::geo_locator::find_closest(
                start.location(), targets);

        REQUIRE(closest.latitude == targets[0].latitude);
        REQUIRE(closest.longitude == targets[0].longitude);
    }

    SECTION ( "find closest target (multiple targets)" ) {
        // distance manually calculated using https://boulter.com/gps/distance/
        std::vector<target> targets = {
            {77.02360, -24.09066},   // 4439.17 km
            {-21.64671, -127.31924}, // 2891.89 km
            {19.86823, -80.48258}    // 9261.31 km
        };

        auto closest = lrucache::geo_locator::find_closest(
                start.location(), targets);
      
        REQUIRE(closest.latitude == targets[1].latitude);
        REQUIRE(closest.longitude == targets[1].longitude);
    }
}