#pragma once
#include "IAPiTelegram.h"
#include <fstream>

class AbstractBot {
public:
    void Run() {
        while (flag_) {
            auto msgs = api_->GetUpdates(offset_, kTimeout);
            for (auto m : msgs) {
                if (offset_ != m.update_id) {
                    offset_ = m.update_id;
                }
                ++offset_;
                std::ofstream out;
                out.open(path_);
                if (out.is_open()) {
                    out << offset_ << std::endl;
                }
                out.close();
                Handle(m);
            }
        }
    }
    virtual ~AbstractBot() = default;
    virtual void Handle(Message m) = 0;

protected:
    AbstractBot(std::unique_ptr<IApiTelegram> api, std::string path)
        : api_(std::move(api)), path_(path) {
        std::ifstream in(path_);
        if (in.is_open()) {
            if (!(in >> offset_)) {
                offset_ = 0;
            }
        }
        in.close();
    }
    bool flag_ = true;
    std::unique_ptr<IApiTelegram> api_;
    std::string path_;
    int64_t offset_;
    static const size_t kTimeout = 5;
};
std::unique_ptr<AbstractBot> CreateBot(std::unique_ptr<IApiTelegram> api, const std::string& path);
