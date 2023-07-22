#pragma once

// https://qiita.com/SaitoAtsushi/items/afb428d0834ca4dda1e5

#include <functional>
#include <type_traits>
#include <utility>

namespace mcview {

class defer_t {
public:
  template <class T, class = typename std::enable_if<std::is_void<decltype((std::declval<T>())())>::value>::type>
  defer_t(T const &f)
      : fDeferred(f) {}

  ~defer_t() {
    fDeferred();
  }

private:
  std::function<void(void)> fDeferred;
};

} // namespace mcview

#define defer_helper2(line) defer_tmp##line
#define defer_helper(line) defer_helper2(line)
#define defer defer_t defer_helper(__LINE__) = [&](void) -> void
