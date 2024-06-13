#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "containers.h"

int main([[maybe_unused]] const int argc,
		 [[maybe_unused]] const char *const *const argv)
{
	using namespace containers;

	const std::vector<std::string> args(argv, argv + argc);
	circular_buffer<std::string, memory_model::list> cb(3);

	std::for_each(args.cbegin(), args.cend(),
	  [&](const auto& arg) -> void {
		cb.push(arg);
	});

	while (!cb.is_empty()) {
		std::cout << cb.pop() << ' ';
	}
	std::cout << std::endl;

	std::cout << "cb is full? " << std::boolalpha << cb.is_full() << '\n';
	std::cout << "cb is empty? " << std::boolalpha << cb.is_empty() << '\n';
	std::cout << "cb size is " << cb.size() << '\n';

	return EXIT_SUCCESS;
}
