#pragma once

#include "ai/AIProvider.h"

namespace sanbucraft::ai {

class MockAIProvider final : public AIProvider {
public:
    std::string name() const override;
    ChatResponse chat(const ChatRequest& request) override;
};

}  // namespace sanbucraft::ai
