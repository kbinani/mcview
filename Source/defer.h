#pragma once

#include <functional>
#include <memory>

using defer_t = std::shared_ptr<void>;

#define defer_helper2(line) defer_tmp ## line
#define defer_helper(line) defer_helper2(line)
#define defer(func_body) defer_t defer_helper(__LINE__) = std::shared_ptr<void>(nullptr, std::bind([&]{func_body}));
