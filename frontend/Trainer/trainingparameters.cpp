#include "trainingparameters.h"

std::vector<size_t> EnableParameters::inputSkips() {
    std::vector<size_t> _inputSkips;
    if (!loa) { _inputSkips.push_back(0); }
    if (!latitude) { _inputSkips.push_back(1); }
    if (!longitude) { _inputSkips.push_back(2); }
    if (!year) { _inputSkips.push_back(3); }
    if (!month) { _inputSkips.push_back(4); }
    if (!day) { _inputSkips.push_back(5); }
    if (!temp_low) { _inputSkips.push_back(6); }
    if (!temp_avg) { _inputSkips.push_back(7); }
    if (!temp_high) { _inputSkips.push_back(8); }
    if (!dew_low) { _inputSkips.push_back(9); }
    if (!dew_avg) { _inputSkips.push_back(10); }
    if (!dew_high) { _inputSkips.push_back(11); }
    if (!humidity_low) { _inputSkips.push_back(12); }
    if (!humidity_avg) { _inputSkips.push_back(13); }
    if (!humidity_high) { _inputSkips.push_back(14); }
    if (!press_low) { _inputSkips.push_back(15); }
    if (!press_avg) { _inputSkips.push_back(16); }
    if (!press_high) { _inputSkips.push_back(17); }
    if (!visibility_low) { _inputSkips.push_back(18); }
    if (!visibility_avg) { _inputSkips.push_back(19); }
    if (!visibility_high) { _inputSkips.push_back(20); }
    if (!wind_gust) { _inputSkips.push_back(21); }
    if (!wind_avg) { _inputSkips.push_back(22); }
    if (!wind_high) { _inputSkips.push_back(23); }
    if (!precipitation) { _inputSkips.push_back(24); }
    if (!fog) { _inputSkips.push_back(25); }
    if (!rain) { _inputSkips.push_back(26); }
    if (!snow) { _inputSkips.push_back(27); }
    if (!thunderstorm) { _inputSkips.push_back(28); }
    if (!population) { _inputSkips.push_back(29); }
    return _inputSkips;
}
