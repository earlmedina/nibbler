#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "constants.h"

// Struct modeling response from Input Validator service
struct ValidationResult {
    bool valid = false;
    std::string message; // Message to show to Player when result is invalid
};

// Struct modeling response from Statistics service
struct StatisticsResult {
    int count = 0;
    int sum = 0;
    float mean = 0.0f;
    int median = 0;
    int min = 0;
    int max = 0;
};

ValidationResult ValidateInitials(const std::string &initials);
StatisticsResult GetScoreStats(const std::vector<int> &scores);
std::vector<int> SortDescending(const std::vector<int> &scores);
std::vector<int> GetTopNScores(const std::vector<int> &scores, int n = TOP_SCORES_COUNT);
bool GetStoreValue(const std::string &key, nlohmann::json &value);
bool SetStoreValue(const std::string &key, const nlohmann::json &value);
