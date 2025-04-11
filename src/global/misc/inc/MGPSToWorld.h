#ifndef GEO_POSE_ECEF_H
#define GEO_POSE_ECEF_H

#include <cmath>
#include "MVector.h"
#include "MRotation.h"

class MGPSToWorld {
public:
    // Inputs
    double lat_deg, lon_deg, alt_m;   // Geodetic position
    double yaw_deg, pitch_deg, roll_deg; // Orientation

    // Outputs
    MVector position_ecef;  // ECEF position (meters)
    MRotation rotation_ecef;  // Rotation matrix (body -> ECEF)

    // WGS-84 constants
    static constexpr double a = 6378137.0;               // Semi-major axis in m
    static constexpr double e2 = 6.69437999014e-3;       // First eccentricity squared
    static constexpr double deg2rad = 0.0174532925199;

    // Alt in m
    MGPSToWorld(double lat, double lon, double alt,
                double yaw, double pitch, double roll)
        : lat_deg(lat), lon_deg(lon), alt_m(alt),
          yaw_deg(yaw), pitch_deg(pitch), roll_deg(roll)
    {
        computeECEF();
    }
    MGPSToWorld();
    ~MGPSToWorld();

    MVector getX_ECEF() const { return rotation_ecef.GetX(); } // Forward
    MVector getY_ECEF() const { return rotation_ecef.GetY(); } // Right
    MVector getZ_ECEF() const { return rotation_ecef.GetZ(); } // Up

private:
    void computeECEF() {
        position_ecef = geodeticToECEF(lat_deg, lon_deg, alt_m);

        MRotation R_body_to_ENU = bodyToENU(yaw_deg, pitch_deg, roll_deg);
        MRotation R_ENU_to_ECEF = enuToECEF(lat_deg, lon_deg);

        rotation_ecef = R_ENU_to_ECEF * R_body_to_ENU;
    }

    MVector geodeticToECEF(double lat, double lon, double alt) {
        double lat_rad = lat * deg2rad;
        double lon_rad = lon * deg2rad;

        double N = a / sqrt(1 - e2 * sin(lat_rad) * sin(lat_rad));

        double x = (N + alt) * cos(lat_rad) * cos(lon_rad);
        double y = (N + alt) * cos(lat_rad) * sin(lon_rad);
        double z = ((1 - e2) * N + alt) * sin(lat_rad);

        return MVector(x, y, z);
    }

    MRotation enuToECEF(double lat, double lon) {
        double lat_rad = lat * deg2rad;
        double lon_rad = lon * deg2rad;

        double sinLat = sin(lat_rad), cosLat = cos(lat_rad);
        double sinLon = sin(lon_rad), cosLon = cos(lon_rad);

        MRotation R;
        R.Set(-sinLon,           cosLon,          0,
              -sinLat * cosLon, -sinLat * sinLon, cosLat,
               cosLat * cosLon,  cosLat * sinLon, sinLat);

        return R;
    }

    MRotation bodyToENU(double yaw, double pitch, double roll) {
        double y = yaw * deg2rad;
        double p = pitch * deg2rad;
        double r = roll * deg2rad;

        MRotation Rz, Ry, Rx;

        Rz.Set(cos(y), -sin(y), 0,
               sin(y),  cos(y), 0,
                    0,       0, 1);

        Ry.Set( cos(p), 0, sin(p),
                     0, 1,      0,
               -sin(p), 0, cos(p));

        Rx.Set(1,      0,       0,
               0, cos(r), -sin(r),
               0, sin(r),  cos(r));

        return Rz * Ry * Rx;
    }
};

#endif // GEO_POSE_ECEF_H
