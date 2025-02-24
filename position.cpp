/*Angular Calculate Position*/

#include "swephexp.h"
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <fstream>
#include <ctime>
#include <sstream>
#include <locale>
#include <algorithm>
#include <set>

#define SEFLG_SWIEPH 2 

using namespace std;
extern "C" {
    double calculate_julian_day(int year, int month, int day, double hour) {
        return swe_julday(year, month, day, hour, SE_GREG_CAL);
    }

    double get_angle(double jd, int planet, double latitude, double longitude) {
        char serr[256];
        double xx[6];
        double cusps[13];
        double ascmc[10];

        int32 iflag = SEFLG_TOPOCTR | SEFLG_SPEED;

        swe_set_topo(longitude, latitude, 0);

        if ((planet == SE_MC) || (planet == SE_ASC)) {
            int hsys = 'P'; 
            swe_houses(jd, latitude, longitude, hsys, cusps, ascmc);
            if (planet == SE_MC) {
                return ascmc[1];  // MC
            }
            if (planet == SE_ASC) {
                return ascmc[0];  // ASC
            }
        }

        // Aries Point (AR)
        if (planet == SE_ECL_NUT) {
            if (swe_calc(jd, SE_ECL_NUT, SEFLG_SWIEPH, xx, serr) < 0) {
                cerr << "Error in swe_calc: " << serr << endl;
                return 0;
            }
            return xx[0]; 
        }

        if (swe_calc(jd, planet, iflag, xx, serr) < 0) {
            cerr << "Error in swe_calc: " << serr << endl;
            return 0;
        }

        swe_close();
        return xx[0];
    }

    double normalize_angle(double angle) {
        angle = fmod(angle, 360.0);
        if (angle < 0)
            angle += 360.0;
        return angle;
    }

    double midpoint(double angle1, double angle2) {
        double midpoint = (angle1 + angle2) / 2.0;
        if (abs(angle1 - angle2) > 180) {
            midpoint += 180;
        }
        return normalize_angle(midpoint);
    }

    map<string, double> as_mc(int year, int month, int day, int hour, int minute, double lat, double lon) {
        double hours = hour + minute / 60.0;
        double jds = calculate_julian_day(year, month, day, hours);
        double cusps[13];
        double ascmc[10];

        swe_set_topo(lon, lat, 0);
        int hsys = 'P';
        swe_houses(jds, lat, lon, hsys, cusps, ascmc);
        map<string, double> pos;
        pos["MC"] = normalize_angle(ascmc[1]);                               // Midheaven
        pos["AS"] = normalize_angle(ascmc[0]);                               // Ascendant
        pos["AR"] = normalize_angle(get_angle(jds, SE_ECL_NUT, lat, lon));   // ARMC (Ascensional Midheaven)
        pos["SU"] = normalize_angle(get_angle(jds, SE_SUN, lat, lon));       // Sun
        pos["MO"] = normalize_angle(get_angle(jds, SE_MOON, lat, lon));      // Moon
        pos["NO"] = normalize_angle(get_angle(jds, SE_TRUE_NODE, lat, lon)); // Mean Node
        pos["ME"] = normalize_angle(get_angle(jds, SE_MERCURY, lat, lon));   // Mercury
        pos["VE"] = normalize_angle(get_angle(jds, SE_VENUS, lat, lon));     // Venus
        pos["MA"] = normalize_angle(get_angle(jds, SE_MARS, lat, lon));      // Mars
        pos["JU"] = normalize_angle(get_angle(jds, SE_JUPITER, lat, lon));   // Jupiter
        pos["SA"] = normalize_angle(get_angle(jds, SE_SATURN, lat, lon));    // Saturn
        pos["UR"] = normalize_angle(get_angle(jds, SE_URANUS, lat, lon));    // Uranus
        pos["NE"] = normalize_angle(get_angle(jds, SE_NEPTUNE, lat, lon));   // Neptune
        pos["PL"] = normalize_angle(get_angle(jds, SE_PLUTO, lat, lon));     // Pluto
        pos["CU"] = normalize_angle(get_angle(jds, SE_CUPIDO, lat, lon));    // Cupido
        pos["HA"] = normalize_angle(get_angle(jds, SE_HADES, lat, lon));     // Hades
        pos["ZE"] = normalize_angle(get_angle(jds, SE_ZEUS, lat, lon));      // Zeus
        pos["KR"] = normalize_angle(get_angle(jds, SE_KRONOS, lat, lon));    // Kronos
        pos["AP"] = normalize_angle(get_angle(jds, SE_APOLLON, lat, lon));   // Apollon
        pos["AD"] = normalize_angle(get_angle(jds, SE_ADMETOS, lat, lon));   // Admetos
        pos["VU"] = normalize_angle(get_angle(jds, SE_VULKANUS, lat, lon));  // Vulkanus
        pos["PO"] = normalize_angle(get_angle(jds, SE_POSEIDON, lat, lon));  // Poseidon

        return pos;
    }

    vector<int> getCurrentTimeUTC() {
        time_t now = std::time(nullptr);
        tm *utc_tm = std::gmtime(&now);

        vector<int> timeComponents = {
            utc_tm->tm_year + 1900,
            utc_tm->tm_mon + 1,
            utc_tm->tm_mday,
            utc_tm->tm_hour,
            utc_tm->tm_min};

        return timeComponents;
    }

    map<string, double> angular_midpoint(int year, int month, int day, int hour, int minute, double lat, double lon) {
        map<string, double> pos = as_mc(year, month, day, hour, minute, lat, lon);
        map<string, double> angular;
        set<string> unique_names;

        for (const auto& A : pos) {
            for (const auto& B : pos) {
                if (A.first == B.first) continue;

                vector<string> names = {A.first, B.first};
                sort(names.begin(), names.end());
                string base_name = names[0] + "+" + names[1];

                if (unique_names.count(base_name)) continue;
                unique_names.insert(base_name);

                angular[base_name] = normalize_angle((A.second + B.second) / 2);
            }
        }
        return angular;
    }

    vector<double> get_house(int year, int month, int day, int hour, int minute, double lat, double lon){
        double hours = hour + minute / 60.0;
        double jds = calculate_julian_day(year, month, day, hours);
        double cusps[13];
        double ascmc[10];

        vector<double> house;

        swe_set_topo(lon, lat, 0);
        int hsys = 'P';
        swe_houses(jds, lat, lon, hsys, cusps, ascmc);

        house.push_back(ascmc[0]);
        house.push_back(ascmc[1]);
        house.push_back(ascmc[2]);
        return house;
    }

    map<string, double> angular_3s(int year, int month, int day, int hour, int minute, double lat, double lon) {
        map<string, double> pos = as_mc(year, month, day, hour, minute, lat, lon);
        map<string, double> angular;
        set<string> unique_names;

        for (const auto& A : pos) {
            for (const auto& B : pos) {
                if (A.first == B.first) continue;
                for (const auto& C : pos) {
                    if (A.first == C.first || B.first == C.first) continue;
                    vector<string> names = {A.first, B.first, C.first};
                    sort(names.begin(), names.end());

                    string base_name = names[0] + "-" + names[1] + "-" + names[2];

                    // check name use used?
                    if (unique_names.count(base_name)) continue;
                    unique_names.insert(base_name);

                    angular[base_name] = normalize_angle(A.second + B.second - C.second);
                    angular[names[0] + "/" + names[1] + "/" + names[2]] = normalize_angle(((A.second + B.second) / 2 + C.second) / 2);
                    angular[names[0] + "+" + names[1] + "+" + names[2]] = normalize_angle(A.second + B.second + C.second);
                    angular[names[0] + "-" + names[1] + "-" + names[2]] = normalize_angle(A.second - B.second - C.second);
                }
            }
        }
        return angular;
    }

    map<string, double> merge_maps(const map<string, double>& map1, const map<string, double>& map2) {
        map<string, double> merged = map1; 
        for (const auto& pair : map2) {
            merged[pair.first] = pair.second;
        }
        return merged;
    }


    // MC Jorn
    map<string, double> angularMC(int year, int month, int day, int hour, int minute, double lat, double lon, double cur_lat, double cur_lon) {
        vector<int> cur_time = getCurrentTimeUTC();
        vector<double> house = get_house(cur_time[0], cur_time[1], cur_time[2], cur_time[3], cur_time[4], cur_lat, cur_lon);
        map<string, double> mc_angular;
        map<string, double> pos1 = angular_midpoint(year, month, day, hour, minute, lat, lon);
        map<string, double> pos2 = angular_3s(year, month, day, hour, minute, lat, lon);
        map<string, double> position = merge_maps(pos1, pos2);
        for (const auto& angle : position){
            double mc_ang = fmod(abs(house[1] - angle.second), 45);
            if (mc_ang <= 0.5) {
                mc_angular[angle.first] = angle.second;
            }
        }
        return mc_angular;
    }

    // MC Birth
    map<string, double> jnAngularMC(int year, int month, int day, int hour, int minute, double lat, double lon, double cur_lat, double cur_lon) {
        vector<int> cur_time = getCurrentTimeUTC();
        vector<double> house = get_house(year, month, day, hour, minute, lat, lon);
        map<string, double> mc_angular;
        map<string, double> pos1 = angular_midpoint(cur_time[0], cur_time[1], cur_time[2], cur_time[3], cur_time[4], cur_lat, cur_lon); 
        map<string, double> pos2 = angular_3s(cur_time[0], cur_time[1], cur_time[2], cur_time[3], cur_time[4], cur_lat, cur_lon);
        map<string, double> position = merge_maps(pos1, pos2);
        for (const auto& angle : position){
            double mc_ang = fmod(abs(house[1] - angle.second), 45);
            if (mc_ang <= 0.5) {
                mc_angular[angle.first] = angle.second;
            }
        }
        return mc_angular;
    }

    // transform date time to Julian Day Number (JDN)
    double julian_day(int year, int month, int day, int hour, int minute) {
        double jdn = swe_julday(year, month, day, hour + (minute / 60.0), SE_GREG_CAL);
        return jdn;
    }


    // Solar Arc
    map<string, double> solar_arc(int birth_year, int birth_month, int birth_day, int birth_hour, int birth_minute, double lat, double lon) {
        map<string, double> solar_arc_positions;

        // 1. รับเวลาปัจจุบันจาก getCurrentTimeUTC()
        vector<int> current_time = getCurrentTimeUTC();
        int trans_year = current_time[0];
        int trans_month = current_time[1];
        int trans_day = current_time[2];
        int trans_hour = current_time[3];
        int trans_minute = current_time[4];

        // 2. คำนวณอายุจร (วัน)
        double jd_birth = calculate_julian_day(birth_year, birth_month, birth_day, birth_hour + (birth_minute / 60));
        double jd_trans = calculate_julian_day(trans_year, trans_month, trans_day, trans_hour + (trans_minute / 60));
        double age_days = jd_trans - jd_birth;

        // 3. แปลงอายุจรเป็นปี
        double age_years = age_days / 365.25;

        // 4. คำนวณวันจรสุริยา (วันเกิด + อายุจร (ปี))
        double jd_solar_arc = jd_birth + age_years * 365.25;

        // 5. หา Solar Arc (องศาอาทิตย์)
        double sun_birth, sun_solar_arc;
        char err[256];

        // หาตำแหน่งอาทิตย์ ณ วันเกิด
        double pos[6];
        if (swe_calc_ut(jd_birth, SE_SUN, SEFLG_SWIEPH, pos, err) != ERR) {
            sun_birth = pos[0];
        }

        // หาตำแหน่งอาทิตย์ ณ วันจรสุริยา
        if (swe_calc_ut(jd_solar_arc, SE_SUN, SEFLG_SWIEPH, pos, err) != ERR) {
            sun_solar_arc = pos[0];
        }

        // คำนวณค่า Solar Arc
        double solar_arc_value = sun_solar_arc - sun_birth;

        // 6. คำนวณตำแหน่งดาวกำเนิด
        map<string, double> birth_positions = as_mc(birth_year, birth_month, birth_day, birth_hour, birth_minute, lat, lon);

        // 7. บวก Solar Arc กับดาวกำเนิด
        for (auto const& pair : birth_positions) {
            string star = pair.first;
            double birth_pos = pair.second;
            
            if (fmod(fmod(birth_pos + solar_arc_value, 360.0), 45) <= 1) {
                solar_arc_positions[star] = fmod(birth_pos + solar_arc_value, 360.0);
            }
        }

        return solar_arc_positions;
    }

    string normalize_key(const string& key) {
        vector<string> parts;
        string part;
        stringstream ss(key);
        
        while (getline(ss, part, '-')) {
            parts.push_back(part);
        }
        
        sort(parts.begin(), parts.end());
        
        string normalized_key = parts[0];
        for (size_t i = 1; i < parts.size(); ++i) {
            normalized_key += "-" + parts[i];
        }
        
        return normalized_key;
    }


    map<string, double> summary(
        int year, int month, int day, int hour, int minute,
        double lat, double lon, double cur_lat, double cur_lon){

        map<string, double> angular_mc = angularMC(year, month, day, hour, minute, lat, lon, cur_lat, cur_lon);
        // map<string, double> jnangular = jnAngularMC(year, month, day, hour, minute, lat, lon, lat, lon);
        map<string, double> solar = solar_arc(year, month, day, hour, minute, lat, lon);

        // map<string, double> aj = merge_maps(angular_mc, jnangular);
        map<string, double> all = merge_maps(angular_mc, solar);

        return all;
    }

    string map_to_json(const map<string, double> &angles){
        string json = "{\n";
        for (auto it = angles.begin(); it != angles.end(); ++it) {
            double value = it->second;
            json += "  \"" + it->first + "\": ";
            if (isnan(value)){
                json += "0.000000";
            }
            else {
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
    
    const char* angular_json(int year, int month, int day, int hour, int minute,
        double lat, double lon, double cur_lat, double cur_lon) {
            map<string, double> position = summary(year, month, day, hour, minute, lat, lon, cur_lat, cur_lon);
            string result_js = map_to_json(position);
            
            return strdup(result_js.c_str());
        }
}