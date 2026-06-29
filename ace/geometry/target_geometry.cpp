#include "ace/geometry/target_geometry.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

namespace ace::geometry {

namespace {

constexpr double kEarthRadiusM = 6371000.0;
constexpr double kDegToRad = M_PI / 180.0;
constexpr double kRadToDeg = 180.0 / M_PI;

}  // namespace

void calculate_target_angles(double device_latitude_deg,
                             double device_longitude_deg,
                             double device_altitude_m,
                             double target_latitude_deg,
                             double target_longitude_deg,
                             double target_altitude_m,
                             float& out_pan_deg,
                             float& out_tilt_deg)
{
    const double lat1_r = device_latitude_deg * kDegToRad;
    const double lon1_r = device_longitude_deg * kDegToRad;
    const double lat2_r = target_latitude_deg * kDegToRad;
    const double lon2_r = target_longitude_deg * kDegToRad;

    const double d_lat = lat2_r - lat1_r;
    const double d_lon = lon2_r - lon1_r;

    const double y = std::sin(d_lon) * std::cos(lat2_r);
    const double x = std::cos(lat1_r) * std::sin(lat2_r) - std::sin(lat1_r) * std::cos(lat2_r) * std::cos(d_lon);
    const double bearing_rad = std::atan2(y, x);
    out_pan_deg = static_cast<float>(bearing_rad * kRadToDeg);

    const double a = std::sin(d_lat / 2.0) * std::sin(d_lat / 2.0) +
                     std::cos(lat1_r) * std::cos(lat2_r) *
                     std::sin(d_lon / 2.0) * std::sin(d_lon / 2.0);
    const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    const double distance_m = kEarthRadiusM * c;

    const double altitude_delta_m = target_altitude_m - device_altitude_m;
    const double elevation_rad = std::atan2(altitude_delta_m, distance_m);
    out_tilt_deg = static_cast<float>(elevation_rad * kRadToDeg);
}

}  // namespace ace::geometry
