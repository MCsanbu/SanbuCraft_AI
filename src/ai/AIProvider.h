#pragma once

#include <string>

namespace sanbucraft::ai {

struct ChatRequest {
    std::string systemPrompt;
    std::string userMessage;
};

struct ChatResponse {
    bool success = false;
    std::string content;
    std::string error;
};

class AIProvider {
public:
    virtual ~AIProvider() = default;
    virtual std::string name() const = 0;
    virtual ChatResponse chat(const ChatRequest& request) = 0;
};

}  // namespace sanbucraft::ai
