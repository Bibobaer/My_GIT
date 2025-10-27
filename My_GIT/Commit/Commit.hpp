#pragma once
#include <ctime>
#include <string>

struct Commit {
	unsigned int commit_id;

	time_t create_at;
	time_t time_to_live;

	std::string message;
	Commit(const unsigned int& _id, const std::string& message);
};
