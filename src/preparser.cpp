
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <fstream>
#include <print>

#include "token.cpp"

struct Phrase {
    const std::vector<std::shared_ptr<Token>> tokens;
};
