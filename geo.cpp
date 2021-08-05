#include "geo.h"

namespace transcat::geo {

    double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace std;
        static const double dr = 3.1415926535 / 180.;
        const int radius = 6371000;
        return acos(sin(from.lat * dr) * sin(to.lat * dr)
                    + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
               * radius;
    }

} //namespace transcat::geo