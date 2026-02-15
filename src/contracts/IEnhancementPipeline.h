#pragma once

#include "contracts/EnhancementTypes.h"

namespace lumos::contracts {

class IEnhancementPipeline {
  public:
    virtual ~IEnhancementPipeline() = default;
    virtual EnhancementResult run(const EnhancementRequest& request) = 0;
};

}  // namespace lumos::contracts

