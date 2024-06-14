#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "containers.hpp"

int main([[maybe_unused]] const int argc,
		 [[maybe_unused]] const char *const *const argv)
{
	const std::vector<std::string> args(argv, argv + argc);
	containers::circular_buffer<std::string, 5> cb;

	for (const auto& arg : args) {
		cb.push_back(arg);
	}

	for (const auto& item : cb) {
		std::cout << item << '\n';
	}
	std::cout << std::endl;

	cb.push_back("hello world");
	cb.emplace_back("how are you?");
	cb.emplace_front("you're gonna be ok...");
	cb.push_back("just not tonight");

	std::for_each(cb.begin(), cb.end(),
				  [](const auto& item) -> void {
						std::cout << item << '\n';
					});

	return EXIT_SUCCESS;
}
