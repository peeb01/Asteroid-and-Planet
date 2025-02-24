/*Normal Angular Position*/

#include "swephexp.h"
#include <iostream>
#include <string>
#include <map>
#include <cmath>
#include <vector>
#include <fstream>
#include <ctime>
#include <sstream> 
#include <locale>
#include <algorithm>
#include <cstring>
// #include <android/asset_manager.h>
// #include <android/asset_manager_jni.h>
// #include "angleUranianHelpler.h"

using namespace std;

extern "C" {
    double calculate_julian_day(int year, int month, int day, double hour) {
            return swe_julday(year, month, day, hour, SE_GREG_CAL);
    }

    double get_angle(double jd, int planet, double latitude, double longitude) {
        char serr[256];
        double xx[6];
        int32 iflag = SEFLG_TOPOCTR | SEFLG_SPEED;   

        swe_set_topo(longitude, latitude, 0);

        if (swe_calc(jd, planet, iflag, xx, serr) < 0) {
            cerr << "Error in swe_calc: " << serr << endl;
            return 0;
        }
        return xx[0];
    }

    double normalize_angle(double angle) {
        if (angle < 0) angle += 360;
        if (angle >= 360) angle -= 360;
        return angle;
    }

    double calculate_midpoint(double angle1, double angle2) {
        double midpoint = (angle1 + angle2) / 2.0;
        if (abs(angle1 - angle2) > 180) {
            midpoint += 180;
        }
        return normalize_angle(midpoint);
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

    // vector<double> angle_file(AAssetManager* assetManager) {
    //     vector<double> tempAngles;

    //     AAsset* file = AAssetManager_open(assetManager, "angleposition.txt", AASSET_MODE_BUFFER);
    //     if (file == nullptr) {
    //         cerr << "Unable to open asset file" << endl;
    //         return tempAngles;
    //     }

    //     size_t fileSize = AAsset_getLength(file);
    //     string fileContent(fileSize, '\0');
    //     AAsset_read(file, fileContent.data(), fileSize);
    //     AAsset_close(file);

    //     istringstream stream(fileContent);
    //     double value;
    //     while (stream >> value) {
    //         tempAngles.push_back(value);
    //     }

    //     return tempAngles;
    // }
    // const vector<double> angle_f = angle_file();

    // init angle can use in Uranian Astrology

    // const vector<double> angle_f = {0, 45, 90, 135, 180};
    
    
    // Get Current time (UTC)
    vector<int> getCurrentTimeUTC() {
        time_t now = std::time(nullptr);
        tm *utc_tm = std::gmtime(&now);

        vector<int> timeComponents = {
            utc_tm->tm_year + 1900,  
            utc_tm->tm_mon + 1,
            utc_tm->tm_mday,         
            utc_tm->tm_hour,         
            utc_tm->tm_min           
        };

        return timeComponents;
    }
    // vector<int> time = getCurrentTimeUTC();

    map<string, double> get_angle_2(
        int year, int month, int day, int hour, int minute,
        double latitude, double longitude, double clatitude, double clongitude,
        const vector<double>& angle_f) {
        
        vector<int> time = getCurrentTimeUTC();
        map<string, double> planet_angle;
        map<int, double> angle_cache;
        map<int, double> angle_cache_john;

        double fractional_hour = hour + minute / 60.0;
        double jd = calculate_julian_day(year, month, day, fractional_hour);
        double jd_john = calculate_julian_day(time[0], time[1], time[2], time[3] + time[4] / 60.0);

        int num_planets = sizeof(planet_ids) / sizeof(planet_ids[0]);

        for (size_t i = 0; i < num_planets; ++i) {
            angle_cache[planet_ids[i]] = normalize_angle(get_angle(jd, planet_ids[i], latitude, longitude));
            angle_cache_john[planet_ids[i]] = normalize_angle(get_angle(jd_john, planet_ids[i], clatitude, clongitude));
        }

        for (size_t i = 0; i < num_planets; ++i) {
            double angleA = angle_cache[planet_ids[i]];
            for (size_t j = 0; j < num_planets; ++j) {
                double angleB = angle_cache_john[planet_ids[j]];
                double dAngle = normalize_angle(fabs(angleA - angleB));

                for (double target_angle : angle_f) {
                    if (fabs(dAngle - target_angle) <= 1.0) {
                        string planet_pair = planet_names[i] + "_" + planet_names[j];
                        planet_angle[planet_pair] = target_angle;
                    }
                }
            }
        }

        return planet_angle;
    }


    map<string, double> get_angle_3(
            int year, int month, int day, int hour, int minute, 
            double latitude, double longitude, double clatitude, double clongitude, const vector<double>& angle_f) {


        vector<int> time = getCurrentTimeUTC();  // {2024, 12, 30, 14, 28};
        map<string, double> planet_triplet_angles;

        double fractional_hour = hour + minute / 60.0;

        double jd = calculate_julian_day(year, month, day, fractional_hour);
        double jd_john = calculate_julian_day(time[0], time[1], time[2], time[3] + time[4] / 60.0);

        int num_planets = sizeof(planet_ids) / sizeof(planet_ids[0]);

        map<int, double> angle_cache;
        map<int, double> angle_cache_john;

        for (int i = 0; i < num_planets; ++i) {
            int planet_id = planet_ids[i];
            angle_cache[planet_id] = normalize_angle(get_angle(jd, planet_id, latitude, longitude));
            angle_cache_john[planet_id] = normalize_angle(get_angle(jd_john, planet_id, clatitude, clongitude));
        }

        // triple angles -> star1, midpoint(star2, star3)
        for (int i = 0; i < num_planets; ++i) {
            int star1_id = planet_ids[i];
            for (int j = i + 1; j < num_planets; ++j) {
                int star2_id = planet_ids[j];
                for (int k = j + 1; k < num_planets; ++k) {
                    int star3_id = planet_ids[k];

                    if (star1_id == star2_id || star1_id == star3_id || star2_id == star3_id) {
                        continue;
                    }

                    double midpoint = calculate_midpoint(angle_cache_john[star2_id], angle_cache_john[star3_id]);
                    double dAngle = abs(midpoint - angle_cache[star1_id]);
                    if (dAngle > 180) dAngle = 360 - dAngle;

                    for (double target_angle : angle_f) {
                        if (fabs(dAngle - target_angle) <= 1.0) {
                            string triplet = planet_names[i] + "_" + planet_names[j] + "_" + planet_names[k];
                            // cout << "Triplet: " << triplet << " dAngle: " << dAngle << endl;
                            planet_triplet_angles[triplet] = dAngle;
                        }
                    }
                }
            }
        }

        return planet_triplet_angles;
    }
    // position angle
    map<string, double> position(
        int year, int month, int day, int hour, int minute,
        double latitude, double longitude ) {
        
        map<string, double> deg;
        double fractional_hour = hour + minute / 60.0;
        double jd = calculate_julian_day(year, month, day, fractional_hour);

        int num_planets = sizeof(planet_ids) / sizeof(planet_ids[0]);

        for (size_t i = 0; i < num_planets; ++i) {
            deg[planet_names[i]] = normalize_angle(get_angle(jd, planet_ids[i], latitude, longitude));
        }
        return deg;
    }

    map<string, double> zodiac_find(int year, int month, int day, int hour, int minute) {
        double fractional_hour = hour + minute / 60.0;
        double jd = calculate_julian_day(year, month, day, fractional_hour);

        double xx[6];
        char serr[256];
        if (swe_calc_ut(jd, SE_SUN, SEFLG_SWIEPH, xx, serr) < 0) {
            cerr << "Error calculating position: " << serr << endl;
            return {};
        }

        // Calculate ecliptic longitude
        double ecliptic_longitude = xx[0];

        // Determine zodiac sign
        vector<string> zodiacs = {"Aries", "Taurus", "Gemini", "Cancer", "Leo", "Virgo",
                                "Libra", "Scorpio", "Sagittarius", "Capricorn", "Aquarius", "Pisces"};

        int zodiac_index = static_cast<int>(ecliptic_longitude / 30.0) % 12;
        string zodiac_name = zodiacs[zodiac_index];

        // Create result map
        map<string, double> result;
        result[zodiac_name] = ecliptic_longitude;
        return result;
    }
    


    string map_to_json(const map<string, double>& angles) {
        string json = "{\n";
        for (auto it = angles.begin(); it != angles.end(); ++it) {
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

    const char* zodiact(int year, int month, int day, int hour, int minute) {
        map<string, double> zodiac_result = zodiac_find(year, month, day, hour, minute);
        if (!zodiac_result.empty()) {
            string result = map_to_json(zodiac_result);
            return strdup(result.c_str());
        } else {
            return "{}";
        }
    }

    const char* position_json(
        int year, int month, int day, int hour, int minute,
        double latitude, double longitude) {
        
        map<string, double> pos = position(year, month, day, hour, minute, latitude, longitude);
        string result_js = map_to_json(pos);

        return strdup(result_js.c_str());
    }

    const char* angle_2_json(
        int year, int month, int day, int hour, int minute,
        double latitude, double longitude, double clatitude, double clongitude,
        const double* angle_f, int angle_count) {
            
        vector<double> angles(angle_f, angle_f + angle_count);
        map<string, double> result = get_angle_2(year, month, day, hour, minute, latitude, longitude, clatitude, clongitude, angles);
        string result_js = map_to_json(result);

        return strdup(result_js.c_str());
    }
    
    const char* angle_3_json(
        int year, int month, int day, int hour, int minute,
        double latitude, double longitude, double clatitude, double clongitude,
        const double* angle_f, int angle_count){

        vector<double> angles(angle_f, angle_f + angle_count);
        map<string, double> result = get_angle_3(year, month, day, hour, minute, latitude, longitude, clatitude, clongitude, angles);
        string result_js = map_to_json(result);

        return strdup(result_js.c_str());
    }

}