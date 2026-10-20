#include "profile.h"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include "constants.h"

// Used for to_json and from_json setup: https://stackoverflow.com/questions/75754196/c-question-about-generating-a-simple-json-file-with-library-nlohmann-json
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PlayerProfile, initials,
    highScore, gamesPlayed, soundOn, speedSetting)


/**
 * ProfileStore static method. Loads Profiles from local json file at specified filepath
 * @param filepath - path to json file
 * @return - ProfileStore
 */
ProfileStore ProfileStore::Load(const std::string &filepath) {
    ProfileStore store; // Create store for holding loaded data

    // Try to open json file
    std::ifstream file(filepath);
    if (!file)
        return store; // Return empty store if there's nothing at specified path

    try {
        const nlohmann::json data = nlohmann::json::parse(file); // Parse json from file
        // Try to load profile data, falling back to empty array if there's nothing
        store.profiles = data.value("profiles", std::vector<PlayerProfile>{});
        const std::string lastUsed = data.value("lastUsed", ""); // Try to load lastUsed profile
        for (size_t i = 0; i < store.profiles.size(); i++) {
            PlayerProfile &p = store.profiles[i];
            // High unlikely it won't be 0-2, but clamp speed setting when loading to be safe
            p.speedSetting = std::clamp(p.speedSetting, static_cast<int>(SPEED_SLOW), static_cast<int>(SPEED_FAST));

            // If the lastUsed property matches profile initials, set the ProfileStore current index to that profile.
            if (p.initials == lastUsed)
                store.current = static_cast<int>(i);
        }
    } catch (const nlohmann::json::exception &e) {
        // Using json::exception for general use: https://stackoverflow.com/questions/76171241/which-exception-should-be-caught-for-nlohmannjsonto-cbor
        // More info on json exception handling: https://stackoverflow.com/questions/78707025/how-to-handle-nlohmann-exception-type-error-in-a-readable-way
        return ProfileStore{}; // Return empty store if there's an exception
    }

    // If there's no current (i.e., val is -1) fall back to first profile in store (if there are profiles)
    if (store.current == -1 && !store.profiles.empty())
        store.current = 0;
    return store;
}

/**
 * ProfileStore method. Saves data to local json file for caching/fallback
 * @param filepath - the path to the JSON file for saving data
 */
void ProfileStore::Save(const std::string &filepath) {
    // If there's no profiles to save don't do anything
    if (current < 0)
        return;

    nlohmann::json data; // Init json object to save data
    data["lastUsed"] = profiles[current].initials;
    data["profiles"] = profiles; // Save profiles

    // Save json to specified filepath
    std::ofstream file(filepath);
    if (file)
        file << data.dump(2); // Save indented json
}

/**
    * Selects the profile with matching initials or creates it if it doesn't exist.
 * @param initials - player initials
 * @return - true if profile was created, false if existing profile was used.
 */
bool ProfileStore::SelectOrCreate(const std::string &initials) {
    // Look for profile in store with matching initials and set as current if found
    for (size_t i = 0; i < profiles.size(); i++) {
        if (profiles[i].initials == initials) {
            current = static_cast<int>(i);
            return false;
        }
    }
    // If here, we need to create a profile. Let Player set initials; everything else is default.
    PlayerProfile newProfile;
    newProfile.initials = initials;
    profiles.push_back(newProfile);
    // Set current to last added profile (the new profile)
    current = static_cast<int>(profiles.size()) - 1;
    return true;
}

/**
 * ProfileStore method that checks if player profile can be added to store
 * @return true if number of profiles is less than max (5).
 */
bool ProfileStore::CanAddPlayer() const {
    return static_cast<int>(profiles.size()) < MAX_PROFILES;
}

/**
 * ProfileStore method that retrieves profile store with Read/write access.
 * Should only be used if a profile exists
 * @return ProfileStore with r/w access
 */
PlayerProfile &ProfileStore::Current() {
    return profiles[current];
}

/**
 * ProfileStore method that retrieves profile store with Read-Only access.
 * @return ProfileStore with r access
 */
const PlayerProfile &ProfileStore::Current() const {
    return profiles[current];
}