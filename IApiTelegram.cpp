#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>

#include <Poco/URI.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>


#include "IAPiTelegram.h"

class Log {
public:
    Log(const Message& msg, Poco::URI base_uri) : msg_(msg), base_uri_(base_uri) {
    }
    void Show() {
        std::cout << "Uri : " + base_uri_.getHost() + base_uri_.getPath() + "\n";
        std::cout << "    chat id: " + std::to_string(msg_.chat_id) + "\n";
        std::cout << "    command: " + msg_.text + "\n";
        std::cout << "\n";
    }

private:
    Message msg_;
    Poco::URI base_uri_;
};
class ApiTelegram : public IApiTelegram {
public:
    ApiTelegram(const std::string& api_key, const std::string& api_endpoint)
        : base_uri_(api_endpoint + api_key) {
        is_https_ = api_endpoint.find("https") != std::string::npos;
    }
    ~ApiTelegram() override = default;
    std::vector<Message> GetUpdates(int offset, size_t timeout) override {
        std::string path = "/getUpdates";
        Poco::URI::QueryParameters query_par = {};
        if (offset != 0) {
            query_par.push_back({"offset", std::to_string(offset)});
        }
        if (timeout != 0) {
            query_par.push_back({"timeout", std::to_string(timeout)});
        }
        auto reply = SendRequest(Poco::Net::HTTPRequest::HTTP_GET, path, "", query_par);
        auto result = reply.extract<Poco::JSON::Object::Ptr>()->getArray("result");
        std::vector<Message> messages;
        for (const auto& element : *result) {
            auto token = element.extract<Poco::JSON::Object::Ptr>();
            if (!token->has("message")) {
                continue;
            }
            auto message = token->getObject("message");
            if (!message->has("entities")) {
                continue;
            }
            auto entities = message->getArray("entities");
            if (entities->getObject(0)->getValue<std::string>("type") != "bot_command") {
                continue;
            }
            Log log(Message{message->getObject("chat")->getValue<int64_t>("id"),
                            message->getValue<std::string>("text"),
                            message->getValue<int64_t>("message_id"),
                            token->getValue<int64_t>("update_id")},
                    base_uri_);
            log.Show();
            messages.push_back(Message{message->getObject("chat")->getValue<int64_t>("id"),
                                       message->getValue<std::string>("text"),
                                       message->getValue<int64_t>("message_id"),
                                       token->getValue<int64_t>("update_id")});
        }
        return messages;
    }
    BotInfo GetMe() override {
        auto reply = SendRequest(Poco::Net::HTTPRequest::HTTP_GET, "/getMe");
        auto result = reply.extract<Poco::JSON::Object::Ptr>()->getObject("result");
        if (!result->getValue<bool>("is_bot")) {
            throw TelegramApiError(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR,
                                   "is not bot");
        }
        return BotInfo{result->getValue<int64_t>("id"), result->getValue<std::string>("first_name"),
                       result->getValue<std::string>("username")};
    }
    void Send(SendMessage message) override {
        std::string reply_id;
        if (message.reply_id.has_value()) {
            reply_id = ", \"reply_to_message_id\": " + std::to_string(message.reply_id.value());
        }
        if (message.type == "Message") {
            std::string body = "{\"chat_id\": " + std::to_string(message.chat_id) +
                               ", \"text\": \"" + message.text + "\"" + reply_id + "}";
            auto reply = SendRequest(Poco::Net::HTTPRequest::HTTP_POST, "/sendMessage", body);
        } else if (message.type == "Sticker") {
            std::string body = "{\"chat_id\": " + std::to_string(message.chat_id) +
                               ", \"sticker\": \"" + message.text + "\"" + reply_id + "}";
            auto reply = SendRequest(Poco::Net::HTTPRequest::HTTP_POST, "/sendSticker", body);
        } else {
            std::string body = "{\"chat_id\": " + std::to_string(message.chat_id) +
                               ", \"animation\": \"" + message.text + "\"" + reply_id + "}";
            auto reply = SendRequest(Poco::Net::HTTPRequest::HTTP_POST, "/sendAnimation", body);
        }
    }

private:
    bool is_https_;
    const std::string content_type_ = "application/json";
    Poco::URI base_uri_;
    Poco::Dynamic::Var SendRequest(const std::basic_string<char>& method, const std::string& path,
                                   const std::string& body = "",
                                   const Poco::URI::QueryParameters& query_par = {}) {
        Poco::URI uri = base_uri_;
        uri.setPath(uri.getPath() + path);
        uri.setQueryParameters(query_par);
        std::unique_ptr<Poco::Net::HTTPClientSession> session;
        if (is_https_) {
            session = std::make_unique<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort());
        } else {
            session = std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());
        }

        Poco::Net::HTTPRequest request{method, uri.getPathAndQuery()};
        request.setContentType(content_type_);
        request.setContentLength(body.length());
        auto& in = session->sendRequest(request);
        in << body;
        Poco::Net::HTTPResponse response;
        auto& out = session->receiveResponse(response);

        if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK) {
            throw TelegramApiError(response.getStatus(), response.getReason());
        }

        Poco::JSON::Parser parser;
        const auto reply = parser.parse(out);

        return reply;
    }
};

std::unique_ptr<IApiTelegram> CreateApiTelegram(const std::string& api_key,
                                                const std::string& api_endpoint) {
    return std::make_unique<ApiTelegram>(api_key, api_endpoint);
}
