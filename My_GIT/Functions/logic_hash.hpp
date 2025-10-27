#pragma once
#include <string>

unsigned int create_uuid();
std::string get_id_hash(const unsigned int& id);
std::string get_file_hash(const std::string& filename);