#include <iostream>
#include <clocale>

#include "Console/ConsoleHandler.hpp"

int main(int argc, char** argv) {
	std::setlocale(LC_ALL, "Rus");
	try {
		ConsoleHandler handle;
		auto cmd = handle.parse_argv(argc, argv);
		handle.execute_command(cmd);
	}
	catch (const std::runtime_error& e) {
		std::cout << e.what() << "\n";
		return 1;
	}
	return 0;
}