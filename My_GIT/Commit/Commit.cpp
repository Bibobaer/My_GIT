#include "Commit.hpp"

Commit::Commit(const unsigned int& _id, const std::string& message) : commit_id(_id), message(message) {
	create_at = std::time(nullptr);
	time_to_live = 300;
}