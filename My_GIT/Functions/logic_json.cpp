#define _CRT_SECURE_NO_WARNINGS

#include "logic_json.hpp"
#include "logic_hash.hpp"
#include "nlohmann/json.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <Windows.h>

namespace fs = std::filesystem;
using json = nlohmann::json;

bool write_commit(const Commit& commit) {
	if (!fs::exists(".vcs/commits.json"))
		throw std::runtime_error("No Repository created");

	json commits_json;
	std::ifstream json_file(".vcs/commits.json");
	if (!json_file.is_open())
		throw std::runtime_error("Error. Cant open file");

	json_file >> commits_json;
	json_file.close();

	if (!commits_json.contains("count_commits"))
		commits_json["count_commits"] = 0;
	if (!commits_json.contains("commits"))
		commits_json["commits"] = json::array();

	commits_json["commits"].push_back({
			{"id", commit.commit_id},
			{"create_at", commit.create_at},
			{"ttl", commit.time_to_live},
			{"message", commit.message}
		});

	commits_json["count_commits"] = commits_json["commits"].size();

	std::ofstream out(".vcs/commits.json");
	if (!out.is_open())
		return false;
	out << commits_json;
	out.close();
	return true;
}

bool write_path(const std::string& rep_path) {
	auto full_path = fs::current_path() / rep_path / ".vcs/commits.json";
	if (!fs::exists(full_path))
		throw std::runtime_error("No Repository created");

	json commits_json = {
		{"count_commits", 0},
		{"repository_path", (fs::current_path() / rep_path).c_str()},
		{"commits", json::array()}
	};

	std::ofstream out(full_path);
	if (!out.is_open())
		return false;
	out << commits_json;
	out.close();

	return true;
}

static std::string time_to_string(time_t time) {
	std::tm* tm_info = std::localtime(&time);
	std::stringstream ss;
	ss << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S");
	return ss.str();
}

static std::string get_remaining_time(time_t time) {
	time_t now = std::time(nullptr);
	time_t diff = time - now;

	int hours = diff / 3600;
	int minuts = (diff % 3600) / 60;
	int seconds = diff % 60;

	if (diff <= 0) return "EXPIRED";

	std::stringstream ss;
	if (hours > 0) ss << hours << "h ";
	if (minuts > 0) ss << minuts << "m ";
	ss << seconds << "s ";
	
	return ss.str();
}

std::vector<std::string> get_commit_info() {
	if (!fs::exists(".vcs/commits.json"))
		throw std::runtime_error("No Repository created");

	json commits_json;
	std::ifstream json_file(".vcs/commits.json");
	if (!json_file.is_open())
		throw std::runtime_error("Error. Cant open file");

	json_file >> commits_json;

	if (commits_json.empty())
		return std::vector<std::string>();
	if (commits_json["commits"].is_null())
		return std::vector<std::string>();

	std::vector<std::string> out;
	if (!fs::exists(fs::current_path() / ".vcs\\HEAD"))
		return std::vector<std::string>();
	fs::path head_hash = fs::relative(fs::read_symlink(fs::current_path() / ".vcs\\HEAD"), fs::current_path() / ".vcs\\objects");


	for (auto& commit : commits_json["commits"]) {
		unsigned int id = commit["id"].get<unsigned int>();
		time_t create_at = commit["create_at"].get<time_t>();
		time_t ttl = commit["ttl"].get<time_t>();

		std::string id_str = std::to_string(id) + (get_id_hash(id) == head_hash.string() ? " (HEAD)" : "");
		
		std::string elem =	"ID: " + id_str + "\n" +
							"  Created: " + time_to_string(create_at) + "\n" +
							"  Remain: " + get_remaining_time(create_at + ttl) + "\n" +
							"  Ends at: " + time_to_string(create_at + ttl) + "\n" +
							"  Message: " + commit["message"].get<std::string>() + "\n";

		out.push_back(elem);
	}
	return out;
}

std::string get_rep_path() {
	if (!fs::exists(".vcs/commits.json"))
		return std::string();
	json commits_json;
	std::ifstream json_file(".vcs/commits.json");
	if (!json_file.is_open())
		throw std::runtime_error("Error. Cant open file");

	json_file >> commits_json;

	if (commits_json.empty())
		return std::string();

	if (!commits_json.contains("repository_path"))
		return std::string();
	std::string utf8_path = commits_json["repository_path"];
	int wide_size = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)utf8_path.c_str(), -1, NULL, 0);
	if (!(wide_size > 0))
		return utf8_path;

	std::wstring wpath(wide_size, 0);
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)utf8_path.c_str(), -1, &wpath[0], wide_size);

	int ansi_size = WideCharToMultiByte(CP_ACP, 0, wpath.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (!ansi_size)
		return utf8_path;

	std::string out(ansi_size, 0);
	WideCharToMultiByte(CP_ACP, 0, wpath.c_str(), -1, &out[0], ansi_size, nullptr, nullptr);

	if (!out.empty() && out.back() == '\0')
		out.pop_back();
	return out;
}

bool is_exists_commit_id(const unsigned int& _id) {
	if (!fs::exists(".vcs/commits.json"))
		throw std::runtime_error("No Repository created");

	json commits_json;
	std::ifstream json_file(".vcs/commits.json");
	if (!json_file.is_open())
		throw std::runtime_error("Error. Cant open file");

	json_file >> commits_json;

	if (commits_json.empty())
		return false;
	if (!commits_json.contains("commits"))
		return false;

	for (auto& commit : commits_json["commits"]) {
		if (commit["id"] == _id)
			return true;
	}
	return false;
}