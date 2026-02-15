#include "app/EnhancementController.h"
#include "common/Telemetry.h"
#include "engine/CpuStubPipeline.h"
#include "tests/TestHelpers.h"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <iostream>

namespace {

const lumos::common::TelemetryEvent* findEvent(
    const std::vector<lumos::common::TelemetryEvent>& events,
    const std::string& name) {
    const auto it = std::find_if(
        events.begin(),
        events.end(),
        [&name](const auto& event) { return event.name == name; });
    if (it == events.end()) {
        return nullptr;
    }
    return &(*it);
}

void testSuccessPathEmitsRequiredEvents() {
    const auto log_path = lumos::tests::tempOutputPath("telemetry_success.jsonl");
    std::filesystem::remove(log_path);

    lumos::common::Telemetry telemetry(log_path);
    lumos::engine::CpuStubPipeline pipeline;
    lumos::app::EnhancementController controller(pipeline, telemetry);

    lumos::contracts::EnhancementRequest request;
    request.input_path = lumos::tests::fixturePath("sample_input.ppm").string();
    request.output_path = lumos::tests::tempOutputPath("telemetry_success_out.ppm").string();
    request.scale_factor = 2;

    const auto result = controller.runEnhancement(request);
    lumos::tests::require(result.ok, "enhancement should succeed in telemetry success test");

    const auto& events = telemetry.events();
    lumos::tests::require(findEvent(events, "image_imported") != nullptr, "image_imported should be emitted");
    lumos::tests::require(findEvent(events, "enhance_clicked") != nullptr, "enhance_clicked should be emitted");

    const auto* completed = findEvent(events, "enhance_completed");
    lumos::tests::require(completed != nullptr, "enhance_completed should be emitted");
    lumos::tests::require(
        completed->fields.find("duration_ms") != completed->fields.end(),
        "enhance_completed should include duration_ms");

    lumos::tests::requireFileSizePositive(log_path, "telemetry log should be written");
}

void testFailurePathEmitsErrorDetails() {
    const auto log_path = lumos::tests::tempOutputPath("telemetry_failure.jsonl");
    std::filesystem::remove(log_path);

    lumos::common::Telemetry telemetry(log_path);
    lumos::engine::CpuStubPipeline pipeline;
    lumos::app::EnhancementController controller(pipeline, telemetry);

    lumos::contracts::EnhancementRequest request;
    request.input_path = lumos::tests::tempOutputPath("does_not_exist.ppm").string();
    request.output_path = lumos::tests::tempOutputPath("telemetry_failure_out.ppm").string();
    request.scale_factor = 2;

    const auto result = controller.runEnhancement(request);
    lumos::tests::require(!result.ok, "enhancement should fail for missing input");

    const auto* failed = findEvent(telemetry.events(), "enhance_failed");
    lumos::tests::require(failed != nullptr, "enhance_failed should be emitted");
    lumos::tests::require(
        failed->fields.find("error_code") != failed->fields.end(),
        "enhance_failed should include error_code");
    lumos::tests::require(
        failed->fields.find("stage") != failed->fields.end(),
        "enhance_failed should include stage");
}

}  // namespace

int main() {
    try {
        testSuccessPathEmitsRequiredEvents();
        testFailurePathEmitsErrorDetails();
        std::cout << "TelemetryTests passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "TelemetryTests failed: " << ex.what() << '\n';
        return 1;
    }
}

