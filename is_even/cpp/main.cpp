#include <concepts>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace constraints {

template <typename T>
concept Integral = std::is_integral_v<T>;

template <typename T>
concept Stringlike = std::is_convertible_v<T, std::string_view>;

} // namespace constraints

template <constraints::Integral T> constexpr bool is_even(T val)
{
  return ((val >> 1) << 1) == val;
}

template <constraints::Integral T>
constexpr T strtoint(const constraints::Stringlike auto &str)
{
  try {
    return static_cast<T>(std::stoll(str));
  } catch (const std::exception &) {
    throw std::invalid_argument("Invalid number: " + str);
  }
}

int main([[maybe_unused]] const int argc,
         [[maybe_unused]] const char *const *const argv)
{
  const std::vector<std::string> args(argv, argv + argc);

  try {
    if (args.size() != 2) {
      throw std::invalid_argument("Usage: " + args[0] + " <number>");
    }

    constraints::Integral auto n = strtoint<std::int64_t>(args[1]);
    std::cout << "Number " << n << " is "
              << (is_even(n) ? "Even" : "Odd")
              << std::endl;
  }
  catch (const std::exception &e) {
    std::cerr << "[Error]: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
