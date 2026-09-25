#include "ai/MockAIProvider.h"

namespace sanbucraft::ai {

std::string MockAIProvider::name() const { return "mock"; }

ChatResponse MockAIProvider::chat(const ChatRequest& request) {
    return {true, "Mock AI received your question: " + request.userMessage +
                      "\n\nThis response is generated locally from the supplied world context; configure a real provider in a later release for conversational advice.", ""};
}

}  // namespace sanbucraft::ai
