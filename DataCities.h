#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

using std::vector;
using std::string;

struct City {
    int index;
    double lat;
    double lon;
};

inline vector<City> Read_cities(const string& file) {
    std::ifstream fin(file);
    vector<City> cities;

    if (!fin.is_open()) {
        std::cerr << "Eroare: Nu s-a putut deschide fisierul " << file << "\n";
        return cities;
    }

    int idx;
    double la, lo;

    while (fin >> idx >> la >> lo) {
        City c;
        c.index = idx;
        c.lat = la;
        c.lon = lo;
        cities.push_back(c);
    }

    fin.close();
    return cities;
}