#include "client_utils.h"
#include <vector>
#include <nlohmann/json.hpp>
#include <zmq.hpp>

// Unnamed namespace for private data
namespace {
    constexpr int SERVICE_TIMEOUT_MS = 1000; // Max time to wait for a reply before falling back
    constexpr const char *INPUT_VALIDATOR_ADDRESS = "tcp://localhost:5555";

    /**
     * https://zeromq.org/get-started/?language=cpp&library=cppzmq#
     * @param address the address to send the request to
     * @param request reference to request object
     * @param response reference to response object to hold response
     * @return true if request was successful, otherwise false.
     */
    bool SendRequest(const char *address, const nlohmann::json &request, nlohmann::json &response) {
        static zmq::context_t context;

        zmq::socket_t socket(context, zmq::socket_type::req);
        socket.set(zmq::sockopt::rcvtimeo, SERVICE_TIMEOUT_MS); // configure timeout
        socket.set(zmq::sockopt::linger, 0); // Don't hang on close with unset request
        socket.connect(address);

        // Send request
        socket.send(zmq::buffer(request.dump()), zmq::send_flags::none);

        // Wait for response from server
        zmq::message_t message;
        if (!socket.recv(message, zmq::recv_flags::none))
            return false; // If timed out or no response, return false

        try {
            response = nlohmann::json::parse(message.to_string());
        } catch (const nlohmann::json::exception &e) {
            return false;
        }
        return true; // The request was sent successfully;
    }

    /**
     * Local version of initials validator used for fallback if service doesn't respond
     * @param initials reference to initials
     * @return ValidationResult
     */
    ValidationResult LocalValidateInitials(const std::string &initials) {
        if (initials.size() < 2 || initials.size() > 3)
            return {false, "Initials must be 2-3 letters"};
        return {true, ""};
    }
}

ValidationResult ValidateInitials(const std::string &initials) {
    const nlohmann::json request = {
        {"type", "string"},
        {"value", initials},
        {
            "options", {
                {"min_length", 2},
                {"max_length", 3},
                {"allowed", "letters"}
            }
        }
    };

    nlohmann::json reply;
    // Fallback: unsuccessful request
    if (!SendRequest(INPUT_VALIDATOR_ADDRESS, request, reply))
        return LocalValidateInitials(initials);

    // Fallback: malformed request (client-side issue)
    if (reply.value("status", "") != "ok")
        return LocalValidateInitials(initials);

    // Set ValidationResult used by game from service response
    ValidationResult result;
    result.valid = reply.value("valid", false);

    // Show first error if there are any
    const std::vector<std::string> errors = reply.value("errors", std::vector<std::string>{});
    if (!errors.empty())
        result.message = errors[0];

    return result;
}
