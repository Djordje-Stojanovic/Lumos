#pragma once

#include "contracts/IEnhancementPipeline.h"

namespace lumos::engine {

class CpuStubPipeline final : public contracts::IEnhancementPipeline {
  public:
    contracts::EnhancementResult run(const contracts::EnhancementRequest& request) override;
};

}  // namespace lumos::engine

