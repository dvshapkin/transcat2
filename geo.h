#pragma once

#include <cmath>

namespace transcat::geo {

    struct Coordinates {
        double lat;
        double lng;
    };

    double ComputeDistance(Coordinates from, Coordinates to);

} //namespace transcat::geo
