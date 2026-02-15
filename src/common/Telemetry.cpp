#include "common/Telemetry.h"

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

namespace lumos::common {

Telemetry::Telemetry(std::filesystem::path log_path) : log_path_(std::move(log_path)) {
    const auto parent = log_path_.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }
}

void Telemetry::track(std::string name, std::map<std::string, std::string> fields) {
    const auto now = std::chrono::system_clock::now();

    TelemetryEvent event {
        .name = std::move(name),
        .fields = std::move(fields),
        .emitted_at = now,
    };

    std::ostringstream json;
    json << "{\"timestamp\":\"" << jsonEscape(toIso8601(event.emitted_at)) << "\","
         << "\"event\":\"" << jsonEscape(event.name) << "\","
         << "\"fields\":{";

    bool first = true;
    for (const auto& [key, value] : event.fields) {
        if (!first) {
            json << ",";
        }
        first = false;
        json << "\"" << jsonEscape(key) << "\":\"" << jsonEscape(value) << "\"";
    }
    json << "}}";

    std::lock_guard<std::mutex> lock(mutex_);
    events_.push_back(event);
    appendLine(json.str());
}

std::filesystem::path Telemetry::defaultLogPath() {
    const char* local_app_data = nullptr;
#if defined(_WIN32)
    char* env_buffer = nullptr;
    std::size_t env_len = 0;
    if (_dupenv_s(&env_buffer, &env_len, "LOCALAPPDATA") == 0 && env_buffer != nullptr) {
        local_app_data = env_buffer;
    }
#else
    local_app_data = std::getenv("LOCALAPPDATA");
#endif

    std::filesystem::path root;
    if (local_app_data != nullptr && local_app_data[0] != '\0') {
        root = std::filesystem::path(local_app_data);
    } else {
        root = std::filesystem::temp_directory_path();
    }

#if defined(_WIN32)
    if (env_buffer != nullptr) {
        free(env_buffer);
    }
#endif

    return root / "Lumos" / "logs" / "events.jsonl";
}

const std::filesystem::path& Telemetry::logPath() const noexcept {
    return log_path_;
}

const std::vector<TelemetryEvent>& Telemetry::events() const noexcept {
    return events_;
}

std::string Telemetry::toIso8601(const std::chrono::system_clock::time_point& time_point) {
    const std::time_t now_time = std::chrono::system_clock::to_time_t(time_point);

    std::tm utc_tm {};
#if defined(_WIN32)
    gmtime_s(&utc_tm, &now_time);
#else
    gmtime_r(&now_time, &utc_tm);
#endif

    std::ostringstream timestamp;
    timestamp << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");
    return timestamp.str();
}

std::string Telemetry::jsonEscape(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());

    for (const char ch : value) {
        switch (ch) {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped.push_back(ch);
                break;
        }
    }

    return escaped;
}

void Telemetry::appendLine(const std::string& line) const {
    std::ofstream output(log_path_, std::ios::out | std::ios::app);
    if (!output.good()) {
        return;
    }
    output << line << '\n';
}

}  // namespace lumos::common
