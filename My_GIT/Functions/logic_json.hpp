#pragma once
#include <vector>
#include <string>

#include "../Commit/Commit.hpp"

bool write_commit(const Commit& commit);
bool write_path(const std::string& rep_path);

std::vector<std::string> get_commit_info();
std::string get_rep_path();

bool is_exists_commit_id(const unsigned int& _id);