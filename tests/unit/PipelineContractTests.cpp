#include "engine/CpuStubPipeline.h"
#include "tests/TestHelpers.h"

#include <exception>
#include <iostream>

namespace {

void testRejectsInvalidRequest() {
    lumos::engine::CpuStubPipeline pipeline;

    lumos::contracts::EnhancementRequest request;
    request.input_path = "";
    request.output_path = lumos::tests::tempOutputPath("invalid_out.ppm").string();
    request.scale_factor = 2;

    const auto result = pipeline.run(request);
    lumos::tests::require(!result.ok, "pipeline should fail for invalid request");
    lumos::tests::require(
        result.error.code == lumos::contracts::ErrorCode::kInvalidRequest,
        "error code should be kInvalidRequest");
}

void testWritesExpectedOutputDimensions() {
    lumos::engine::CpuStubPipeline pipeline;

    lumos::contracts::EnhancementRequest request;
    request.input_path = lumos::tests::fixturePath("sample_input.ppm").string();
    request.output_path = lumos::tests::tempOutputPath("scaled_2x.ppm").string();
    request.scale_factor = 2;
    request.denoise_enabled = false;

    const auto result = pipeline.run(request);
    lumos::tests::require(result.ok, "pipeline should process valid request");
    lumos::tests::require(result.metrics.input_width == 2, "input width should be 2");
    lumos::tests::require(result.metrics.input_height == 2, "input height should be 2");
    lumos::tests::require(result.metrics.output_width == 4, "output width should be 4 for 2x scale");
    lumos::tests::require(result.metrics.output_height == 4, "output height should be 4 for 2x scale");
    lumos::tests::requireFileSizePositive(
        request.output_path,
        "pipeline should produce non-empty output");
}

void testRejectsUnsupportedScaleFactor() {
    lumos::engine::CpuStubPipeline pipeline;

    lumos::contracts::EnhancementRequest request;
    request.input_path = lumos::tests::fixturePath("sample_input.ppm").string();
    request.output_path = lumos::tests::tempOutputPath("invalid_scale.ppm").string();
    request.scale_factor = 3;

    const auto result = pipeline.run(request);
    lumos::tests::require(!result.ok, "pipeline should fail unsupported scale factor");
    lumos::tests::require(
        result.error.code == lumos::contracts::ErrorCode::kInvalidRequest,
        "error code should be kInvalidRequest");
}

}  // namespace

int main() {
    try {
        testRejectsInvalidRequest();
        testWritesExpectedOutputDimensions();
        testRejectsUnsupportedScaleFactor();
        std::cout << "PipelineContractTests passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "PipelineContractTests failed: " << ex.what() << '\n';
        return 1;
    }
}

