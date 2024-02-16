#pragma once

#include <string>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

struct Message {
    int64_t chat_id;

    std::string text;
    int64_t message_id;

    int64_t update_id;
};

struct SendMessage {
    int64_t chat_id;
    std::string text;
    std::string type = "Message";
    std::optional<int64_t> reply_id = std::nullopt;
};

struct BotInfo {
    int64_t id;
    std::string first_name;
    std::string username;
};

struct TelegramApiError : public std::runtime_error {
    TelegramApiError(int http_code, const std::string& details)
        : std::runtime_error{"api error: code=" + std::to_string(http_code) +
                             " details=" + details},
          http_code{http_code},
          details{details} {
    }

    int http_code;
    std::string details;
};

class IApiTelegram {
public:
    virtual ~IApiTelegram() = default;
    virtual std::vector<Message> GetUpdates(int offset = 0, size_t timeout = 0) = 0;
    virtual BotInfo GetMe() = 0;
    virtual void Send(SendMessage send_message) = 0;
};

std::unique_ptr<IApiTelegram> CreateApiTelegram(
    const std::string& api_key, const std::string& api_endpoint = "https://api.telegram.org/bot");
