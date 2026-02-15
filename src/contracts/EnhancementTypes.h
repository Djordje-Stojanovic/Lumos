#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace lumos::contracts {

enum class ErrorCode {
    kNone = 0,
    kInvalidRequest,
    kDecodeFailed,
    kProcessFailed,
    kEncodeFailed,
};

inline std::string_view toString(const ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::kNone:
            return "none";
        case ErrorCode::kInvalidRequest:
            return "invalid_request";
        case ErrorCode::kDecodeFailed:
            return "decode_failed";
        case ErrorCode::kProcessFailed:
            return "process_failed";
        case ErrorCode::kEncodeFailed:
            return "encode_failed";
    }
    return "unknown";
}

struct EnhancementRequest {
    std::string input_path {};
    std::string output_path {};
    int scale_factor {2};
    bool denoise_enabled {false};
    std::string preset_name {"default"};
};

struct EnhancementMetrics {
    int input_width {0};
    int input_height {0};
    int output_width {0};
    int output_height {0};
    std::uint64_t duration_ms {0};
};

struct EnhancementError {
    ErrorCode code {ErrorCode::kNone};
    std::string stage {"none"};
    std::string message {};
};

struct EnhancementResult {
    bool ok {false};
    std::string output_path {};
    EnhancementMetrics metrics {};
    EnhancementError error {};
};

inline bool isValidRequest(const EnhancementRequest& request, std::string* reason = nullptr) {
    if (request.input_path.empty()) {
        if (reason != nullptr) {
            *reason = "input_path must not be empty";
        }
        return false;
    }

    if (request.output_path.empty()) {
        if (reason != nullptr) {
            *reason = "output_path must not be empty";
        }
        return false;
    }

    if (request.scale_factor != 2 && request.scale_factor != 4 && request.scale_factor != 8) {
        if (reason != nullptr) {
            *reason = "scale_factor must be one of: 2, 4, 8";
        }
        return false;
    }

    return true;
}

}  // namespace lumos::contracts

