#include "trainingparameters.h"

std::vector<size_t> EnableParameters::inputSkips() {
    std::vector<size_t> _inputSkips;
    for (size_t i = 0; i < totalElements; i++) {
        if (!elementLookup(i)) {
            _inputSkips.push_back(i);
        }
    }
//    if (!loa) { _inputSkips.push_back(0); }
//    if (!latitude) { _inputSkips.push_back(1); }
//    if (!longitude) { _inputSkips.push_back(2); }
//    if (!year) { _inputSkips.push_back(3); }
//    if (!month) { _inputSkips.push_back(4); }
//    if (!day) { _inputSkips.push_back(5); }
//    if (!temp_low) { _inputSkips.push_back(6); }
//    if (!temp_avg) { _inputSkips.push_back(7); }
//    if (!temp_high) { _inputSkips.push_back(8); }
//    if (!dew_low) { _inputSkips.push_back(9); }
//    if (!dew_avg) { _inputSkips.push_back(10); }
//    if (!dew_high) { _inputSkips.push_back(11); }
//    if (!humidity_low) { _inputSkips.push_back(12); }
//    if (!humidity_avg) { _inputSkips.push_back(13); }
//    if (!humidity_high) { _inputSkips.push_back(14); }
//    if (!press_low) { _inputSkips.push_back(15); }
//    if (!press_avg) { _inputSkips.push_back(16); }
//    if (!press_high) { _inputSkips.push_back(17); }
//    if (!visibility_low) { _inputSkips.push_back(18); }
//    if (!visibility_avg) { _inputSkips.push_back(19); }
//    if (!visibility_high) { _inputSkips.push_back(20); }
//    if (!wind_gust) { _inputSkips.push_back(21); }
//    if (!wind_avg) { _inputSkips.push_back(22); }
//    if (!wind_high) { _inputSkips.push_back(23); }
//    if (!precipitation) { _inputSkips.push_back(24); }
//    if (!fog) { _inputSkips.push_back(25); }
//    if (!rain) { _inputSkips.push_back(26); }
//    if (!snow) { _inputSkips.push_back(27); }
//    if (!thunderstorm) { _inputSkips.push_back(28); }
//    if (!population) { _inputSkips.push_back(29); }
    return _inputSkips;
}

void EnableParameters::setSkipsFromVector(const std::vector<size_t> & skips) {
    for (size_t i = 0; i < totalElements; i++) {
        setElementEnabled(i, true);
    }

    for (size_t i = 0; i < skips.size(); i++) {
        setElementEnabled(skips[i], false);
    }
}

bool EnableParameters::elementLookup(const size_t & index) {
    switch(index) {
    case 0:
        return loa;
    case 1:
        return latitude;
    case 2:
        return longitude;
    case 3:
        return year;
    case 4:
        return month;
    case 5:
        return day;
    case 6:
        return temp_low;
    case 7:
        return temp_avg;
    case 8:
        return temp_high;
    case 9:
        return dew_low;
    case 10:
        return dew_avg;
    case 11:
        return dew_high;
    case 12:
        return humidity_low;
    case 13:
        return humidity_avg;
    case 14:
        return humidity_high;
    case 15:
        return press_low;
    case 16:
        return press_avg;
    case 17:
        return press_high;
    case 18:
        return visibility_low;
    case 19:
        return visibility_avg;
    case 20:
        return visibility_high;
    case 21:
        return wind_gust;
    case 22:
        return wind_avg;
    case 23:
        return wind_high;
    case 24:
        return precipitation;
    case 25:
        return fog;
    case 26:
        return rain;
    case 27:
        return snow;
    case 28:
        return thunderstorm;
    case 29:
        return population;
    default:
        return false;
    }
}

void EnableParameters::setElementEnabled(const size_t & index, bool b) {
    switch(index) {
    case 0:
        loa = b;
        break;
    case 1:
        latitude = b;
        break;
    case 2:
        longitude = b;
        break;
    case 3:
        year = b;
        break;
    case 4:
        month = b;
        break;
    case 5:
        day = b;
        break;
    case 6:
        temp_low = b;
        break;
    case 7:
        temp_avg = b;
        break;
    case 8:
        temp_high = b;
        break;
    case 9:
        dew_low = b;
        break;
    case 10:
        dew_avg = b;
        break;
    case 11:
        dew_high = b;
        break;
    case 12:
        humidity_low = b;
        break;
    case 13:
        humidity_avg = b;
        break;
    case 14:
        humidity_high = b;
        break;
    case 15:
        press_low = b;
        break;
    case 16:
        press_avg = b;
        break;
    case 17:
        press_high = b;
        break;
    case 18:
        visibility_low = b;
        break;
    case 19:
        visibility_avg = b;
        break;
    case 20:
        visibility_high = b;
        break;
    case 21:
        wind_gust = b;
        break;
    case 22:
        wind_avg = b;
        break;
    case 23:
        wind_high = b;
        break;
    case 24:
        precipitation = b;
        break;
    case 25:
        fog = b;
        break;
    case 26:
        rain = b;
        break;
    case 27:
        snow = b;
        break;
    case 28:
        thunderstorm = b;
        break;
    case 29:
        population = b;
        break;
    default:
        break;
    }
}
