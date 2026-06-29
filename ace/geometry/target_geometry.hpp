#pragma once

namespace ace::geometry {

void calculate_target_angles(double device_latitude_deg,
                             double device_longitude_deg,
                             double device_altitude_m,
                             double target_latitude_deg,
                             double target_longitude_deg,
                             double target_altitude_m,
                             float& out_pan_deg,
                             float& out_tilt_deg);

}  // namespace ace::geometry
