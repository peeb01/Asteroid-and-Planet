#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include "swephexp.h"

using namespace std;

double calculate_julian_day(int year, int month, int day, double hour) {
    return swe_julday(year, month, day, hour, SE_GREG_CAL);
}

vector<double> get_planet_position(double jd, int planet) {
    double xx[6];
    char serr[AS_MAXCH];
    int iflag = SEFLG_SWIEPH | SEFLG_XYZ;

    int result = swe_calc_ut(jd, planet, iflag, xx, serr);
    if (result < 0) {
        cerr << "Error: " << serr << endl;
        return {};
    }

    return {xx[0], xx[1], xx[2]};
}

int main() {
    // string date_str = "2024-12-06 13:31:00";
    int year = 2024, month = 12, day = 6, hour = 13, minute = 31, second = 0;
    double fractional_hour = hour + minute / 60.0 + second / 3600.0;

    double jd = calculate_julian_day(year, month, day, fractional_hour);

    map<int, string> planets = {
        {SE_SUN, "Sun"}, {SE_MOON, "Moon"}, {SE_MERCURY, "Mercury"},
        {SE_VENUS, "Venus"}, {SE_MARS, "Mars"}, {SE_JUPITER, "Jupiter"},
        {SE_SATURN, "Saturn"}, {SE_URANUS, "Uranus"}, {SE_NEPTUNE, "Neptune"}
    };

    swe_set_ephe_path(NULL);

    cout << setw(10) << "Planet"
              << setw(15) << "X (AU)"
              << setw(15) << "Y (AU)"
              << setw(15) << "Z (AU)" << endl;
    cout << string(55, '-') << endl;

    for (const auto& planet : planets) {
        vector<double> position = get_planet_position(jd, planet.first);
        if (!position.empty()) {
            cout << setw(10) << planet.second
                      << setw(15) << position[0]
                      << setw(15) << position[1]
                      << setw(15) << position[2] << endl;
        }
    }

    swe_close();
    return 0;
}


// run code at --> g++ main.cpp -L. -lswe -o main && ./main