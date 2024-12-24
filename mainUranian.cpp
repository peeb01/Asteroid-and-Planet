#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cmath>
#include "swephexp.h"
#include <algorithm>

using namespace std;

double calculate_julian_day(int year, int month, int day, double hour) {
    return swe_julday(year, month, day, hour, SE_GREG_CAL);
}

// Constants for Uranian Astrology
const double FULL_CIRCLE = 360.0;
const double MIDPOINT_TOLERANCE = 1.0; // Degrees of allowable orb for midpoints

// Normalize degrees to range [0, 360)
double normalize_degrees(double degrees) {
    while (degrees < 0) degrees += FULL_CIRCLE;
    while (degrees >= FULL_CIRCLE) degrees -= FULL_CIRCLE;
    return degrees;
}

// Calculate the midpoint between two positions
double calculate_midpoint(double pos1, double pos2) {
    double midpoint = (pos1 + pos2) / 2.0;
    if (fabs(pos1 - pos2) > 180.0) {
        midpoint = normalize_degrees(midpoint + 180.0);
    }
    return normalize_degrees(midpoint);
}

// Calculate angular distance between two positions
double calculate_angle(double pos1, double pos2) {
    double angle = fabs(pos1 - pos2);
    return normalize_degrees(angle);
}

// Function to calculate planet positions using Swiss Ephemeris
map<string, double> calculate_planet_positions(double jd, double latitude, double longitude) {
    map<string, double> planet_positions;

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

    double xx[6]; // array to hold position data
    char serr[AS_MAXCH]; // error message buffer
    swe_set_topo(longitude, latitude, 0.0); // Set observer location

    for (size_t i = 0; i < sizeof(planet_ids) / sizeof(planet_ids[0]); ++i) {
        int result = swe_calc_ut(jd, planet_ids[i], SEFLG_SWIEPH, xx, serr);
        if (result < 0) {
            cerr << "Error calculating position for " << planet_names[i] << ": " << serr << endl;
        } else {
            planet_positions[planet_names[i]] = normalize_degrees(xx[0]); // Store longitude
        }
    }

    return planet_positions;
}


// Uranian Astrology Analysis Function
// to anlysis plannet pairing significant
map<string, vector<double>> analyze_uranian_astrology(const map<string, double>& planet_positions) {
    map<string, vector<double>> result; // midpoints and angular relationship

    vector<string> planet_names;
    vector<double> planet_angles;

    // extract names and positions from input
    for (const auto& entry : planet_positions) {
        planet_names.push_back(entry.first);
        planet_angles.push_back(entry.second);
    }

    // pair star
    size_t num_planets = planet_names.size();
    for (size_t i = 0; i < num_planets; ++i) {
        for (size_t j = 0; j < num_planets; ++j) {
            string pair_name = planet_names[i] + "_" + planet_names[j];
            double pos1 = planet_angles[i];
            double pos2 = planet_angles[j];

            // midpoint calculation
            double midpoint = calculate_midpoint(pos1, pos2);

            // Check if any other planet is near the midpoint
            vector<double> significant_positions;
            for (size_t k = 0; k < num_planets; ++k) {
                if (k == i || k == j) continue;
                double angle_to_midpoint = calculate_angle(midpoint, planet_angles[k]);
                if (angle_to_midpoint <= MIDPOINT_TOLERANCE) {
                    // Use std::find to check if the position is already in the significant_positions
                    if (std::find(significant_positions.begin(), significant_positions.end(), planet_angles[k]) == significant_positions.end()) {
                        significant_positions.push_back(planet_angles[k]);
                    }
                }
            }

            // Only store the pair if there are significant positions
            if (!significant_positions.empty()) {
                result[pair_name] = {midpoint};
                result[pair_name].insert(result[pair_name].end(), significant_positions.begin(), significant_positions.end());
            }
        }
    }

    return result;
}


int main(){
    double jd = calculate_julian_day(2024, 12, 10, 12.5); 
    map<string, double> planet_positions = calculate_planet_positions(jd, 16.8211, 100.2659); 
    map<string, vector<double>> analysis = analyze_uranian_astrology(planet_positions);

    cout << analysis.size() << endl;

    // int i = 0;
    for (const auto& entry : analysis) {
        cout << "Pair: " << entry.first << ", Midpoint: " << entry.second[0] << ", Significant Positions: ";

        if (entry.second.size() > 1) {
            for (size_t i = 1; i < entry.second.size(); ++i) {
                cout << entry.second[i] << " ";
            }
        } else {
            cout << "None";
        }
        cout << endl;
    }
}


// run code at --> g++ mainUranian.cpp -L. -lswe -o mainUranian && ./mainUranian