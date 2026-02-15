#pragma once

#include <chrono>
#include <filesystem>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace lumos::common {

struct TelemetryEvent {
    std::string name;
    std::map<std::string, std::string> fields;
    std::chrono::system_clock::time_point emitted_at;
};

class Telemetry {
  public:
    explicit Telemetry(std::filesystem::path log_path = defaultLogPath());

    void emit(std::string name, std::map<std::string, std::string> fields = {});

    [[nodiscard]] static std::filesystem::path defaultLogPath();
    [[nodiscard]] const std::filesystem::path& logPath() const noexcept;
    [[nodiscard]] const std::vector<TelemetryEvent>& events() const noexcept;

  private:
    static std::string toIso8601(const std::chrono::system_clock::time_point& time_point);
    static std::string jsonEscape(const std::string& value);

    void appendLine(const std::string& line) const;

    std::filesystem::path log_path_;
    std::vector<TelemetryEvent> events_;
    mutable std::mutex mutex_;
};

}  // namespace lumos::common

