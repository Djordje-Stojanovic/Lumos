#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

namespace lumos::tests {

inline std::filesystem::path testRoot() {
    return std::filesystem::path(LUMOS_TEST_ROOT);
}

inline std::filesystem::path fixturePath(const std::string_view fixture_name) {
    return testRoot() / "fixtures" / fixture_name;
}

inline std::filesystem::path tempOutputPath(const std::string_view filename) {
    const auto dir = std::filesystem::temp_directory_path() / "lumos_tests_output";
    std::filesystem::create_directories(dir);
    return dir / filename;
}

inline void require(const bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

inline void requireExists(const std::filesystem::path& path, const std::string& message) {
    require(std::filesystem::exists(path), message + ": " + path.string());
}

inline void requireFileSizePositive(const std::filesystem::path& path, const std::string& message) {
    requireExists(path, message);
    require(std::filesystem::file_size(path) > 0, message + " (file is empty): " + path.string());
}

}  // namespace lumos::tests

