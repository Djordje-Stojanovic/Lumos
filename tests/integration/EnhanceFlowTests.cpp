#include "app/EnhancementController.h"
#include "common/Telemetry.h"
#include "engine/CpuStubPipeline.h"
#include "tests/TestHelpers.h"

#include <chrono>
#include <exception>
#include <future>
#include <iostream>

namespace {

void testEndToEndEnhancementAsyncFlow() {
    const auto log_path = lumos::tests::tempOutputPath("enhance_flow_events.jsonl");
    lumos::common::Telemetry telemetry(log_path);
    lumos::engine::CpuStubPipeline pipeline;
    lumos::app::EnhancementController controller(pipeline, telemetry);

    lumos::contracts::EnhancementRequest request;
    request.input_path = lumos::tests::fixturePath("sample_input.ppm").string();
    request.output_path = lumos::tests::tempOutputPath("enhance_flow_out.ppm").string();
    request.scale_factor = 4;
    request.denoise_enabled = true;

    const auto start = std::chrono::steady_clock::now();
    auto future = controller.runEnhancementAsync(request);
    const auto dispatch_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

    lumos::tests::require(dispatch_ms < 100, "async dispatch should return quickly");

    const auto wait_status = future.wait_for(std::chrono::seconds(5));
    lumos::tests::require(wait_status == std::future_status::ready, "future should finish before timeout");

    const auto result = future.get();
    lumos::tests::require(result.ok, "end-to-end enhancement should succeed");
    lumos::tests::require(result.metrics.output_width == 8, "4x scale from 2px source should produce 8px width");
    lumos::tests::require(result.metrics.output_height == 8, "4x scale from 2px source should produce 8px height");
    lumos::tests::requireFileSizePositive(request.output_path, "output file should exist and be non-empty");
    lumos::tests::requireFileSizePositive(log_path, "telemetry output should exist and be non-empty");
}

}  // namespace

int main() {
    try {
        testEndToEndEnhancementAsyncFlow();
        std::cout << "EnhanceFlowTests passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "EnhanceFlowTests failed: " << ex.what() << '\n';
        return 1;
    }
}

