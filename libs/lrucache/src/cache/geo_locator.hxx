#ifndef LRUCACHE_GEO_LOCATOR_H_
#define LRUCACHE_GEO_LOCATOR_H_

#include <cmath>
#include <vector>
#include <stdexcept>

namespace lrucache {

constexpr double PI = 3.14159265358979323846;

class geo_locator {
public:
    struct location {
        double latitude; // degrees
        double longirude; // degrees
    };

    template <typename T>
    static T find_closest(const location& start, const std::vector<T>& targets) {
        if (targets.empty()) {
            throw std::runtime_error("vector is empty");
        }

        double min_distance = std::numeric_limits<double>::max();
        const T* closest_item = nullptr;

        for (const auto& target : targets) {
            double distance = haversine_distance(start, target.location());
            if (distance < min_distance) {
                min_distance = distance;
                closest_item = &target;
            }
        }

        return *closest_item;
    }
private:
    static double to_radians(double degree) {
        return degree * PI / 180.0;
    }

    static double haversine_distance(const location& start,
                                     const location& target)
    {
        return haversine_distance(
            start.latitude, start.longirude,
            target.latitude, target.longirude);
    }

    static double haversine_distance(
            double lat1, double lon1, double lat2, double lon2)
    {
        // Convert latitude and longitude from degrees to radians
        lat1 = to_radians(lat1);
        lon1 = to_radians(lon1);
        lat2 = to_radians(lat2);
        lon2 = to_radians(lon2);

        // Haversine formula
        double dlat = lat2 - lat1;
        double dlon = lon2 - lon1;
        double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
                   std::cos(lat1) * std::cos(lat2) *
                   std::sin(dlon / 2) * std::sin(dlon / 2);
        double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

        // Earth radius in kilometers
        constexpr double earthRadius = 6371.0;

        // Calculate the distance
        double distance = earthRadius * c;

        return distance;
    }
};

}

#endif // LRUCACHE_GEO_LOCATOR_H_