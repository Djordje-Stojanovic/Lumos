#pragma once

#include "common/Telemetry.h"
#include "contracts/IEnhancementPipeline.h"

#include <future>
#include <utility>

namespace lumos::app {

class EnhancementController {
  public:
    EnhancementController(contracts::IEnhancementPipeline& pipeline, common::Telemetry& telemetry);

    void trackInputSelected(const std::string& input_path);
    contracts::EnhancementResult runEnhancement(const contracts::EnhancementRequest& request);
    std::future<contracts::EnhancementResult> runEnhancementAsync(contracts::EnhancementRequest request);

  private:
    static std::pair<int, int> inspectPpmDimensions(const std::string& input_path);

    contracts::IEnhancementPipeline& pipeline_;
    common::Telemetry& telemetry_;
};

}  // namespace lumos::app

