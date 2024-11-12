#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <unordered_set>
#include <cstdint>
#include <optional>
#include <memory>
#include <sstream>


#include "lexy/callback.hpp"
#include "lexy/dsl.hpp"
#include "lexy/action/parse.hpp"
#include "lexy_ext/report_error.hpp"
#include "lexy/input/string_input.hpp"
#include "lexy/action/parse_as_tree.hpp"

#include "Geometry.hpp"

// 该文件用于符号到几何图形的解析
// 输入：字符串。输出：Geometry或者其子类

namespace GeometryGenerator{
    namespace dsl = lexy::dsl;
    namespace grammar{

    };
    namespace config{

    };
};
