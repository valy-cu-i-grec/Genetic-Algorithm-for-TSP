#pragma once
#include <cmath>
#include "DataCities.h" 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum DistanceType 
{
    EUC_2D,
    CEIL_2D,
    GEO
};




inline double GetDistanceEuclidean(const City& c1, const City& c2)
{
    double dx = c1.lat - c2.lat; 
    double dy = c1.lon - c2.lon; 
    return std::sqrt(dx * dx + dy * dy);
}

inline int GetDistanceCeil2D(const City& c1, const City& c2)
{
    return (int)std::ceil(GetDistanceEuclidean(c1, c2));
}

inline double toRadians(double degree)
{
    return degree * (M_PI / 180.0);
}

inline double GetDistanceGeo(const City& c1, const City& c2)
{
    double R = 6371.0; 

    double dLat = toRadians(c2.lat - c1.lat);
    double dLon = toRadians(c2.lon - c1.lon);

    double lat1 = toRadians(c1.lat);
    double lat2 = toRadians(c2.lat);

    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
        std::sin(dLon / 2) * std::sin(dLon / 2) * std::cos(lat1) * std::cos(lat2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return R * c;
}