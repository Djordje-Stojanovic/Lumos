#include "app/EnhancementController.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <utility>

namespace lumos::app {

EnhancementController::EnhancementController(contracts::IEnhancementPipeline& pipeline, common::Telemetry& telemetry)
    : pipeline_(pipeline), telemetry_(telemetry) {}

contracts::EnhancementResult EnhancementController::runEnhancement(const contracts::EnhancementRequest& request) {
    const auto [width, height] = inspectPpmDimensions(request.input_path);
    if (width > 0 && height > 0) {
        telemetry_.emit(
            "image_imported",
            {
                {"input_path", request.input_path},
                {"input_ext", std::filesystem::path(request.input_path).extension().string()},
                {"input_width", std::to_string(width)},
                {"input_height", std::to_string(height)},
            });
    }

    telemetry_.emit(
        "enhance_clicked",
        {
            {"input_path", request.input_path},
            {"output_path", request.output_path},
            {"scale_factor", std::to_string(request.scale_factor)},
            {"denoise_enabled", request.denoise_enabled ? "true" : "false"},
            {"preset_name", request.preset_name},
        });

    contracts::EnhancementResult result = pipeline_.run(request);
    if (result.ok) {
        telemetry_.emit(
            "enhance_completed",
            {
                {"output_path", result.output_path},
                {"duration_ms", std::to_string(result.metrics.duration_ms)},
                {"output_width", std::to_string(result.metrics.output_width)},
                {"output_height", std::to_string(result.metrics.output_height)},
            });
        return result;
    }

    telemetry_.emit(
        "enhance_failed",
        {
            {"stage", result.error.stage},
            {"error_code", std::string(contracts::toString(result.error.code))},
            {"message", result.error.message},
        });

    return result;
}

std::future<contracts::EnhancementResult> EnhancementController::runEnhancementAsync(contracts::EnhancementRequest request) {
    return std::async(std::launch::async, [this, request = std::move(request)]() { return runEnhancement(request); });
}

std::pair<int, int> EnhancementController::inspectPpmDimensions(const std::string& input_path) {
    std::ifstream input(input_path, std::ios::in);
    if (!input.good()) {
        return {0, 0};
    }

    std::string magic;
    int width = 0;
    int height = 0;
    int max_value = 0;

    input >> magic >> width >> height >> max_value;
    if (magic != "P3" || width <= 0 || height <= 0 || max_value <= 0) {
        return {0, 0};
    }

    return {width, height};
}

}  // namespace lumos::app

