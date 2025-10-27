#include "ConsoleHandler.hpp"
#include <iostream>
#include <filesystem>

#include "../VCS/VCS.hpp"

Command ConsoleHandler::parse_argv(int argc, char** argv) {
	if (argc <= 2)
		return std::make_pair(VCS_Command::HELP_REP, nullptr);

	Command out = {VCS_Command::HELP_REP, nullptr};

	for (int i = 1; i < argc; i++) {
		if (!std::strcmp(argv[i], "init")) {
			if (std::strlen(argv[i + 1]) != 0 && out.second == nullptr) {
				out = { VCS_Command::INIT_REP, argv[i + 1] };
			}
			else {
				if (std::strlen(argv[i + 1]) != 0 && out.second != nullptr) {
					throw std::runtime_error("You cant init two repositories at the same time");
				}
				else {
					throw std::runtime_error("There isnt directory name");
				}
			}
		}
		else if (!std::strcmp(argv[i], "push")) {
			if (std::strlen(argv[i + 1]) != 0 && out.second == nullptr) {
				out = { VCS_Command::PUSH_REP, argv[i + 1] };
			}
			else {
				if (std::strlen(argv[i + 1]) != 0 && out.second != nullptr) {
					throw std::runtime_error("You cant push two commits at the same time");
				}
				else {
					throw std::runtime_error("There isnt message for commit");
				}
			}
		}
		else if (!std::strcmp(argv[i], "pull")) {
			if (std::strlen(argv[i + 1]) != 0 && out.second == nullptr) {
				out = { VCS_Command::PULL_REP, argv[i + 1] };
			}
			else {
				if (std::strlen(argv[i + 1]) != 0 && out.second != nullptr) {
					throw std::runtime_error("You cant pull for two commits at the same time");
				}
				else {
					throw std::runtime_error("There isnt id for commit");
				}
			}
		}
		else if (!std::strcmp(argv[i], "log")) {
			out = { VCS_Command::LOG_REP, argv[i + 1] };
		}
		else if (!std::strcmp(argv[i], "-d")) {
			if (std::strlen(argv[i + 1]) != 0) {
				std::filesystem::current_path(argv[i + 1]);
			}
			else {
				throw std::runtime_error("There isnt directory to edit current directory");
			}
		}
		else if (!std::strcmp(argv[i], "-help")) {
			out = { VCS_Command::HELP_REP, nullptr };
		}
	}

	return out;
}

void ConsoleHandler::execute_command(const Command& cmd) {
	if (cmd.first == VCS_Command::HELP_REP) {
		std::cout << "There are the following commands :\n";
		std::cout << "-help               : displays this message\n";
		std::cout << "-d [folder name]    : changes the current directory to[folder name]\n";
		std::cout << "init [folder name]  : creates a repository with the name[folder name]\n";
		std::cout << "push [message]      : saves a commit\n";
		std::cout << "pull [commit ID]    : switches to the commit with the specified ID\n";
		std::cout << "log                 : displays information about all commits\n";
	}
	else if (cmd.first == VCS_Command::INIT_REP) {
		VCS vcs;
		vcs.init(std::string(cmd.second));
	}
	else if (cmd.first == VCS_Command::PUSH_REP) {
		VCS vcs;
		vcs.push(std::string(cmd.second));
	}
	else if (cmd.first == VCS_Command::PULL_REP) {
		try {
			VCS vcs;
			vcs.pull((unsigned int)std::stoul(cmd.second));
		}
		catch (const std::exception& e) {
			std::cout << "Error with parsing commit id: " << e.what() << "\n";
		}
	}
	else if (cmd.first == VCS_Command::LOG_REP) {
		VCS vcs;
		vcs.log();
	}
	else {
		std::cout << "Undefined command\n";
	}
}