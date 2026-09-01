
#pragma once

#include <string>
#include <vector>

struct ParsedTemplateParameter {
    std::string name;
    std::vector<std::string> allowed_types;
    std::string category;
};
