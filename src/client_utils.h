#pragma once

#include <string>

// Struct modeling response from Input Validator service
struct ValidationResult {
    bool valid = false;
    std::string message; // Message to show to Player when result is invalid
};

/**
 * Validate player initials using the Input Validator service,
 * falling back to local rules if the service doesn't respond.
 * @param initials reference to initials
 * @return ValidationResult
 */
ValidationResult ValidateInitials(const std::string &initials);