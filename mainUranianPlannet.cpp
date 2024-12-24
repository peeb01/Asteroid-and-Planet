#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include "swephexp.h"
#include <iomanip>

using namespace std;

double calculate_julian_day(int year, int month, int day, double hour) {
    return swe_julday(year, month, day, hour, SE_GREG_CAL);
}

vector<double> get_planet_position(double jd, int planet, double latitude, double longitude) {
    double xx[6];
    char serr[AS_MAXCH];
    int iflag = SEFLG_SWIEPH | SEFLG_XYZ + SEFLG_TOPOCTR;

    double altitude = 0.0;
    swe_set_topo(longitude, latitude, altitude);

    int result = swe_calc_ut(jd, planet, iflag, xx, serr);
    if (result < 0) {
        cerr << "Error: " << serr << endl;
        return {};
    }

    return {xx[0], xx[1], xx[2]};
}

const int planet_ids[] = {
    SE_MC, SE_ASC, SE_SUN, SE_MOON, SE_MEAN_NODE, SE_MERCURY,
    SE_VENUS, SE_MARS, SE_JUPITER, SE_SATURN, SE_URANUS, SE_NEPTUNE,
    SE_PLUTO, SE_CUPIDO, SE_HADES, SE_ZEUS, SE_KRONOS, SE_APOLLON,
    SE_ADMETOS, SE_VULKANUS, SE_POSEIDON, SE_ARMC
};

const string planet_names[] = {
    "MC", "AS", "SU", "MO", "NO", "ME", "VE", "MA", "JU", "SA", "UR", "NE", 
    "PL", "CU", "HA", "ZE", "KR", "AP", "AD", "VU", "PO", "AR"
};

map<string, vector<double>> get_all_planet_positions(
    int year, int month, int day, int hour, int minute, int second, 
    double latitude, double longitude
) {
    map<string, vector<double>> planet_positions;

    double fractional_hour = hour + minute / 60.0 + second / 3600.0;
    double jd = calculate_julian_day(year, month, day, fractional_hour);
    swe_set_ephe_path(NULL);

    int num_planets = sizeof(planet_ids) / sizeof(planet_ids[0]);

    for (int i = 0; i < num_planets; ++i) {
        int planet_id = planet_ids[i];
        string planet_name = planet_names[i];

        vector<double> position = get_planet_position(jd, planet_id, latitude, longitude);
        if (!position.empty()) {
            planet_positions[planet_name] = position;
        }
    }

    return planet_positions;
}


int main() {
    int year = 2024, month = 12, day = 8, hour = 22, minute = 19, second = 0;
    double longitude = 100.2659;
    double latitude = 16.8211;

    map<string, vector<double>> positions = get_all_planet_positions(
        year, month, day, hour, minute, second, latitude, longitude
    );

    cout << setw(10) << "Planet"
         << setw(15) << "X (AU)"
         << setw(15) << "Y (AU)"
         << setw(15) << "Z (AU)" << endl;
    cout << string(55, '-') << endl;

    int num_planets = sizeof(planet_names) / sizeof(planet_names[0]);

    for (int i = 0; i < num_planets; ++i) {
        string planet_name = planet_names[i];

        if (positions.find(planet_name) != positions.end()) {
            const vector<double>& position = positions[planet_name];
            cout << setw(10) << planet_name
                 << setw(15) << fixed << setprecision(10) << position[0]
                 << setw(15) << fixed << setprecision(10) << position[1]
                 << setw(15) << fixed << setprecision(10) << position[2] << endl;
        }
    }

    return 0;
}


// run code at --> g++ mainUranianPlannet.cpp -L. -lswe -o mainUranianPlannet && ./mainUranianPlannet