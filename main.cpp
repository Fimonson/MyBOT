#include "AbstractBot.h"
#include "IAPiTelegram.h"

int main() {
    std::ifstream in("OffSet.txt");
    bool offset_exists = in.is_open();
    in.close();
    if (!offset_exists) {
        std::ofstream out("OffSet.txt");
        out.close();
    }
    auto api = CreateApiTelegram("5861831735:AAGxYUqU2QPdgNOPsO_p8PDzzXWC6CI0HEo");
    auto bot = CreateBot(std::move(api), "ОffSet.txt");
    bot->Run();
    return 0;
}
