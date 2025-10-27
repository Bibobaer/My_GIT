#pragma once
#include <string>

class VCS {
private:
	bool is_repositoty;
	std::string repository_path;

	void cleaning_expired_commits();
public:
	VCS();

	bool init(const std::string& directory_path);
	bool push(const std::string& commit_message);

	bool pull(const unsigned int& commit_id);
	bool log();
};

