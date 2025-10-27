#pragma once
#include <utility>

enum class VCS_Command {
	INIT_REP,
	PUSH_REP,
	PULL_REP,
	LOG_REP,
	HELP_REP
};

using Command = std::pair<VCS_Command, char*>;

class ConsoleHandler {
public:
	Command parse_argv(int argc, char** argv);
	void execute_command(const Command& cmd);
};

