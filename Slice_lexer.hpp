#pragma once

#include <exception>
#include <cstdint>
#include <vector>
#include <string>
#include <cctype>

namespace gtd {
    class SliceLexerError: public std::exception {
    public:
        SliceLexerError(const std::string& _msg) : msg{_msg} {}
        const char* what() const noexcept override {
            return msg.c_str(); 
        }
    private:
        std::string msg{};
    };
}

enum GTD_SliceTokenType {
    GTD_SliceToken_EOF = -1,
    GTD_SliceToken_COLON, GTD_SliceToken_NUMBER
};

namespace gtd {
    class SliceToken {
    public:
        SliceToken() : value{-1ll} {}
        SliceToken(GTD_SliceTokenType _t) : type{_t} {}
        SliceToken(GTD_SliceTokenType _t, int64_t _v) : type{_t}, value{_v} {}
        const GTD_SliceTokenType type{GTD_SliceToken_EOF};
        const int64_t value{};
    };
}

namespace gtd {
    std::vector<SliceToken> slice_lexer(const std::string key) {
        std::vector<SliceToken> tokens{};
        std::string::const_iterator iter = key.begin();
        while(iter != key.end()) {
            if(*iter == ':') {
                tokens.emplace_back(GTD_SliceToken_COLON);
            }
            else if (std::isdigit(*iter) || *iter == '-') {
                std::string num{};
                if(*iter == '-') num += *(iter++);
                while(iter != key.end() && std::isdigit(*iter)) {
                    num += *(iter++);
                }
                if(*(iter--) == '-' || num == "-") throw gtd::SliceLexerError("Unexpected '-' symbol in the key");
                tokens.emplace_back(GTD_SliceToken_NUMBER, static_cast<int64_t>(std::stoll(num)));
            }
            else if(std::isspace(*iter)) {}
            else throw gtd::SliceLexerError("Unexpected symbol in the key");
            ++iter;
        }
        tokens.emplace_back();
        return tokens;
    }
}