#include <map>
#include <random>
#include <stdlib.h>
#include <iostream>
#include "AbstractBot.h"

class MyBot;

class Command {
public:
    virtual SendMessage Execute(Message msg) = 0;
    virtual ~Command() = default;
};

class Random : public Command {
public:
    Random() {
        std::random_device dev;
        rng_.seed(dev());
    }
    SendMessage Execute(Message msg) override {
        return SendMessage{msg.chat_id, std::to_string(dist6_(rng_))};
    }

private:
    std::mt19937 rng_;
    std::uniform_int_distribution<int> dist6_;
};
class Weather : public Command {
    SendMessage Execute(Message msg) override {
        return SendMessage{msg.chat_id, "Winter Is Coming"};
    }
};
class StyleGuide : public Command {
    SendMessage Execute(Message msg) override {
        return SendMessage{
            msg.chat_id,
            "- Что можно сказать и во время секса, и когда проверяешь пулл-реквест?\n"
            "- Я нашел бэкдор.\n"
            "- Хотелось бы обойтись без наследования.\n"};
    }
};
class Stop : public Command {
    SendMessage Execute(Message msg) override {
        return SendMessage({msg.chat_id, "Stop"});
    }
};
class Crash : public Command {
    SendMessage Execute(Message msg) override {
        return SendMessage({msg.chat_id, "Crash"});
    }
};
class Sticker : public Command {
    SendMessage Execute(Message msg) override {
        return SendMessage(
            {msg.chat_id, "CAACAgIAAxkBAANiY6nwEG6pFMiVKCX8djV7UFD6HfIAAg0OAAJtMXlJBLZplaUggi0sBA",
             "Sticker"});
    }
};
class Gif : public Command {
    SendMessage Execute(Message msg) override {
        return SendMessage(
            {msg.chat_id, "CgACAgQAAxkBAAN8Y6n1-v2zNX0ibjxng4H9TfE3AVgAAvECAAK8NA1THCtr5hUI7MQsBA",
             "Animation"});
    }
};
class Unknown : public Command {
    SendMessage Execute(Message msg) override {
        return SendMessage{msg.chat_id, "I do not know this command: " + msg.text};
    }
};

class MyBot : public AbstractBot {
public:
    MyBot(std::unique_ptr<IApiTelegram> api, const std::string& path)
        : AbstractBot(std::move(api), path) {
        map_.emplace("/random", std::make_unique<Random>());
        map_.emplace("/weather", std::make_unique<Weather>());
        map_.emplace("/styleguide", std::make_unique<StyleGuide>());
        map_.emplace("/stop", std::make_unique<Stop>());
        map_.emplace("/crash", std::make_unique<Crash>());
        map_.emplace("/sticker", std::make_unique<Sticker>());
        map_.emplace("/gif", std::make_unique<Gif>());
        map_.emplace("Unknown", std::make_unique<Unknown>());
    }
    void Handle(Message message) override {
        std::string command = (!map_.contains(message.text)) ? "Unknown" : message.text;
        SendMessage smg = map_[command]->Execute(message);
        api_->Send(smg);
        if (message.text == "/stop") {
            exit(0);
        }
        if (message.text == "/crash") {
            abort();
        }
    }

private:
    std::map<std::string, std::unique_ptr<Command>> map_;
};

std::unique_ptr<AbstractBot> CreateBot(std::unique_ptr<IApiTelegram> api, const std::string& path) {
    return std::make_unique<MyBot>(std::move(api), path);
};
