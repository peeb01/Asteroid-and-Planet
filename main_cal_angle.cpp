#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include "swephexp.h"
#include <sstream>
#include <iomanip>
#include <fstream>

using namespace std;

extern "C" {

    double calculate_julian_day(int year, int month, int day, double hour) {
        return swe_julday(year, month, day, hour, SE_GREG_CAL);
    }

    const double FULL_CIRCLE = 360.0;
    const double MIDPOINT_TOLERANCE = 1.0;

    // use to notmalize angle
    // angle in swisseph can be 0 to 360 : [0, 360)
    double normalize_degrees(double angle) {
        double normalized_angle = fmod(angle, 360.0);
        if (normalized_angle < 0) {
            normalized_angle += 360.0;
        }
        return normalized_angle;
    }

    vector<double> get_planet_position(double jd, int planet, double latitude, double longitude) {
        double xx[6];
        char serr[AS_MAXCH];
        int iflag = SEFLG_SWIEPH | SEFLG_XYZ + SEFLG_TOPOCTR | SEFLG_MOSEPH;

        double altitude = 0.0;
        swe_set_topo(longitude, latitude, altitude);

        int result = swe_calc_ut(jd, planet, iflag, xx, serr);
        if (result < 0) {
            cerr << "Error: " << serr << endl;
            return {};
        }

        return {xx[0], xx[1], xx[2]};
    }
    // calculate midpoint between 2 stars
    // x = (x1 + x2)/2 , y = (y1 + y2)/2 , z = (z1 + z2)/2
    // return vector{x, y, z}
    vector<double> vector_midpoint(double x1, double y1, double z1, double x2, double y2, double z2){
        double x = (x1 + x2) / 2.0;
        double y = (y1 + y2) / 2.0;
        double z = (z1 + z2) / 2.0;

        return {x, y, z};
    }

    double angle_between_stars(double x1, double y1, double z1, double x2, double y2, double z2) {
        double dot_product = x1 * x2 + y1 * y2 + z1 * z2;
        double mag_v1 = sqrt(x1 * x1 + y1 * y1 + z1 * z1);
        double mag_v2 = sqrt(x2 * x2 + y2 * y2 + z2 * z2);

        // Handle zero magnitude case
        if (mag_v1 == 0.0 || mag_v2 == 0.0) {
            return 0.0; // Return NaN for undefined angle
        }

        double cos_theta = dot_product / (mag_v1 * mag_v2);

        // cos_theta to range [-1, 1] to avoid domain errors in acos
        if (cos_theta > 1.0) cos_theta = 1.0;
        if (cos_theta < -1.0) cos_theta = -1.0;

        double angle_rad = acos(cos_theta);
        double angle_deg = angle_rad * (180.0 / M_PI);

        return angle_deg;
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

    map<string, double> get_angle_2(
        int year, int month, int day, int hour, int minute, 
        double latitude, double longitude
    ) {
        map<string, double> plannet_angle;

        double fractional_hour = hour + minute / 60.0;
        double jd = calculate_julian_day(year, month, day, fractional_hour);
        
        int num_planets = sizeof(planet_ids) / sizeof(planet_ids[0]);

        for (int i = 0; i < num_planets; ++i) {
            vector<double> posA = get_planet_position(jd, planet_ids[i], latitude, longitude);

            for (int j = 0; j < num_planets; ++j) {
                vector<double> posB = get_planet_position(jd, planet_ids[j], latitude, longitude);

                double theta;
                if (i == j) {
                    theta = normalize_degrees(angle_between_stars(posA[0], posA[1], posA[2], 0.0, 0.0, 0.0));
                } else {
                    theta = normalize_degrees(angle_between_stars(posA[0], posA[1], posA[2], posB[0], posB[1], posB[2]));
                }

                string planet_pair = planet_names[i] + "_" + planet_names[j];
                plannet_angle[planet_pair] = theta;
            }
        }

        return plannet_angle;
    }
    map<string, double> get_angle_3(
        int year, int month, int day, int hour, int minute, 
        double latitude, double longitude
    ) {
        map<string, double> plannet_angle;

        double fractional_hour = hour + minute / 60.0;
        double jd = calculate_julian_day(year, month, day, fractional_hour);
        
        int num_planets = sizeof(planet_ids) / sizeof(planet_ids[0]);

        for (int i = 0; i < num_planets; ++i) {
            for (int j = i + 1; j < num_planets; ++j) { // Pair star 1 and star 2
                vector<double> posA = get_planet_position(jd, planet_ids[i], latitude, longitude);
                vector<double> posB = get_planet_position(jd, planet_ids[j], latitude, longitude);
                
                // midpoint of star 1 and star 2
                vector<double> midpoint = vector_midpoint(posA[0], posA[1], posA[2], posB[0], posB[1], posB[2]);

                for (int k = 0; k < num_planets; ++k) {
                    if (k != i && k != j) { // Star 3
                        vector<double> posC = get_planet_position(jd, planet_ids[k], latitude, longitude);

                        // angle between midpoint and star 3
                        double theta = normalize_degrees(
                            angle_between_stars(midpoint[0], midpoint[1], midpoint[2], posC[0], posC[1], posC[2])
                        );

                        // unique key for this combination
                        string planet_triplet = planet_names[i] + "_" + planet_names[j] + "_" + planet_names[k];
                        plannet_angle[planet_triplet] = theta;
                    }
                }
            }
        }

        return plannet_angle;
    }

    map<string, double> get_angle_4(
        int year, int month, int day, int hour, int minute, 
        double latitude, double longitude
    ) {
        map<string, double> plannet_angle;

        double fractional_hour = hour + minute / 60.0;
        double jd = calculate_julian_day(year, month, day, fractional_hour);

        int num_planets = sizeof(planet_ids) / sizeof(planet_ids[0]);

        for (int i = 0; i < num_planets; ++i) {
            for (int j = i + 1; j < num_planets; ++j) { // Pair star 1 and star 2
                vector<double> posA = get_planet_position(jd, planet_ids[i], latitude, longitude);
                vector<double> posB = get_planet_position(jd, planet_ids[j], latitude, longitude);
                
                // midpoint of star 1 and star 2
                vector<double> midpoint1 = vector_midpoint(posA[0], posA[1], posA[2], posB[0], posB[1], posB[2]);

                for (int k = 0; k < num_planets; ++k) {
                    if (k == i || k == j) continue; // ensure uniqueness

                    for (int l = k + 1; l < num_planets; ++l) { // Pair star 3 and star 4
                        if (l == i || l == j) continue; // ensure uniqueness
                        
                        vector<double> posC = get_planet_position(jd, planet_ids[k], latitude, longitude);
                        vector<double> posD = get_planet_position(jd, planet_ids[l], latitude, longitude);

                        // midpoint of star 3 and star 4
                        vector<double> midpoint2 = vector_midpoint(posC[0], posC[1], posC[2], posD[0], posD[1], posD[2]);

                        // angle between the two midpoints
                        double theta = normalize_degrees(
                            angle_between_stars(midpoint1[0], midpoint1[1], midpoint1[2], midpoint2[0], midpoint2[1], midpoint2[2])
                        );

                        // unique key for this combination
                        string planet_quadruplet = planet_names[i] + "_" + planet_names[j] + "_" + planet_names[k] + "_" + planet_names[l];
                        plannet_angle[planet_quadruplet] = theta;
                    }
                }
            }
        }

        return plannet_angle;
    }


    string map_to_json(const map<string, double>& angles) {
        string json = "{\n";

        for (auto it = angles.begin(); it != angles.end(); ++it) {
            // Replace `nan` with a string or default value
            double value = it->second;
            json += "  \"" + it->first + "\": ";
            if (isnan(value)) {
                json += "0.000000"; // "0.0"
            } else {
                json += to_string(value);
            }

            if (next(it) != angles.end()) {
                json += ",";
            }
            json += "\n";
        }

        json += "}";
        return json;
    }

    void write_to_file(const string& filename, const string& json_result) {
        ofstream outfile(filename);
        if (outfile.is_open()) {
            outfile << json_result;
            outfile.close();
            cout << "JSON written to " << filename << " successfully." << endl;
        } else {
            cerr << "Error: Could not open file " << filename << " for writing." << endl;
        }
    }
}




int main() {
    int year = 2024, month = 12, day = 24, hour = 10, minute = 30;
    double latitude = 40.7128, longitude = -74.0060;

    map<string, double> planet_angles = get_angle_2(year, month, day, hour, minute, latitude, longitude);
    map<string, double> planet_angles1 = get_angle_3(year, month, day, hour, minute, latitude, longitude);
    // map<string, double> planet_angles2 = get_angle_4(year, month, day, hour, minute, latitude, longitude);

    string json_result = map_to_json(planet_angles);
    string json_result1 = map_to_json(planet_angles1);
    // string json_result2 = map_to_json(planet_angles2);
    // string filename = "planet_angles.json";
    // write_to_file(filename, json_result2);

    // cout << json_result << endl;
    cout << json_result1 << endl;

    return 0;
}


// g++ main_cal_angle.cpp -L. -lswe -o main_cal_angle && ./main_cal_angle