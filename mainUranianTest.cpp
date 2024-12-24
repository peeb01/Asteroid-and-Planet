#include <iostream>
#include <map>
#include <string>
#include "swephexp.h" // Include Swiss Ephemeris header


using namespace std;

map<string, double> generateAngles(double julian_date, double latitude, double longitude) {
    const int planet_ids[] = { SE_MC, SE_ASC, SE_SUN, SE_MOON };
    const string planet_names[] = { "MC", "AS", "SU", "MO" };
    const int num_planets = sizeof(planet_ids) / sizeof(planet_ids[0]);
    map<string, double> angle_map;
    char serr[256];

    // Set ephemeris path
    swe_set_ephe_path("./sweph");

    // Set topocentric observer position
    swe_set_topo(longitude, latitude, 0.0);

    double positions[num_planets][6];
    for (int i = 0; i < num_planets; ++i) {
        // Attempt calculation with Swiss Ephemeris
        int calc_status = swe_calc(julian_date, planet_ids[i], SEFLG_SWIEPH, positions[i], serr);

        // Fallback to Moshier if Swiss Ephemeris fails
        if (calc_status != OK) {
            cerr << "Error for " << planet_names[i] << ": " << (serr[0] != '\0' ? serr : "No specific error")
                 << "\nFalling back to Moshier ephemeris..." << endl;
            calc_status = swe_calc(julian_date, planet_ids[i], SEFLG_MOSEPH, positions[i], serr);
        }

        // Final error handling
        if (calc_status != OK) {
            cerr << "Final error for " << planet_names[i] << ": " << (serr[0] != '\0' ? serr : "No specific error")
                 << endl;
            continue; // Skip this planet
        }
    }

    // Calculate angles between planets
    for (int i = 0; i < num_planets; ++i) {
        for (int j = i + 1; j < num_planets; ++j) {
            double angle = abs(positions[i][0] - positions[j][0]);
            if (angle > 360.0) angle -= 360.0; // Normalize
            angle_map[planet_names[i] + "_" + planet_names[j]] = angle;
        }
    }

    return angle_map;
}

int main() {
    double julian_date = swe_julday(2024, 12, 22, 0.0, SE_GREG_CAL);
    double latitude = 40.0;   // Observer latitude
    double longitude = -74.0; // Observer longitude

    map<string, double> angles = generateAngles(julian_date, latitude, longitude);

    cout << "      Name | Angle" << endl;
    cout << "-----------------------------" << endl;
    for (const auto& pair : angles) {
        cout << pair.first << " | " << pair.second << endl;
    }

    return 0;
}