#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <functional>

#include "VCS.hpp"
#include "../Commit/Commit.hpp"
#include "../Functions/logic_json.hpp"
#include "../Functions/logic_hash.hpp"

namespace fs = std::filesystem;
using path = fs::path;

VCS::VCS() {
	auto path = fs::current_path() / ".vcs";
	is_repositoty = fs::exists(path) ? true : false;
	repository_path = get_rep_path();
}

void VCS::cleaning_expired_commits() {
	try {
		auto commits = get_commit_info();
		std::vector<unsigned int> expired_ids;

		for (const auto& commit : commits) {
			if (commit.find("EXPIRED") != std::string::npos) {
				size_t id = commit.find("ID: ");
				if (id != std::string::npos) {
					size_t end_id = commit.find("\n");
					if (end_id != std::string::npos) {
						std::string id_line = commit.substr(id + 4, end_id - (id + 4));

						size_t head_pos = commit.find(" (HEAD)");
						if (head_pos != std::string::npos) {
							id_line = id_line.substr(0, head_pos);
						}

						try {
							expired_ids.push_back((unsigned int)std::stoul(id_line));
						}
						catch (const std::exception& e) {
							std::cout << "Error parsing commit id: " << e.what() << "\n";
						}
					}
				}
			}
		}

		for (auto& commit_id : expired_ids) {
			path commit_dir = repository_path + "\\.vcs\\objects\\" + get_id_hash(commit_id);
			std::cout << "123\n";
			if (fs::exists(commit_dir)) {
				try {
					if (fs::exists(repository_path + "\\.vcs\\HEAD")) {
						if (fs::equivalent(fs::read_symlink(repository_path + "\\.vcs\\HEAD"), commit_dir)) {
							std::cout << "Skip current commit: " << commit_id << "\n";
							continue;
						}
						fs::remove_all(commit_dir);
						std::cout << "Removing expired commit: " << commit_id << "\n";
					}
				}
				catch (const fs::filesystem_error& e) {
					std::cout << "Error removing commit: " << e.what() << "\n";
				}
			}
		}

	}
	catch (const fs::filesystem_error& e) {
		std::cout << "Error with cleaning commits: " << e.what() << "\n";
	}
}

bool VCS::init(const std::string& directory_path) {
	/*
		Прверяем существует ли уже директроия
		Если да, то выходим
		Иначе создаем ее и вспомогательные файлы
	*/
	try {
		path new_path(directory_path);
		if (fs::exists(new_path))
			throw std::runtime_error("Repositoty is ready");
		if (!fs::create_directories(new_path / ".vcs/objects"))
			return false;
		std::ofstream add_json(new_path / ".vcs/commits.json");
		if (add_json.is_open()) {
			add_json.close();
			write_path(new_path.string());
			std::cout << "Repository is created :)\n";
			return true;
		}
		std::cout << "Problem with creating repository :(\n";
		return false;
	}
	catch (const fs::filesystem_error& e) {
		std::cout << "Error with creating directory: " << e.what() << "\n";
		return false;
	}
	catch (const std::exception& e) {
		std::cout << "Error: " << e.what() << "\n";
		return false;
	}
}

bool VCS::push(const std::string& commit_message) {
	if (!is_repositoty)
		throw std::runtime_error("No repositories creted");

	try {
		cleaning_expired_commits();

		Commit commit(create_uuid(), commit_message);
		auto commit_directory = repository_path + "\\.vcs\\objects\\" + get_id_hash(commit.commit_id);
		if (!fs::create_directory(commit_directory))
			return false;

		if (fs::exists(repository_path + "\\.vcs\\HEAD")) {
			/*
				если у нас были коммиты до этого, то бежим по основной директроии
				сравниваем хеши файлов и добавляем те, которых нет
				остальные не добавляем в коммит
			*/
			auto current_commit_hash = fs::read_symlink(repository_path + "\\.vcs\\HEAD");
			// ----------------- Берем хэш текущего коммита ---------------------------------------
			std::map<std::string, path> commit_hashes;

			std::function<void(const path&)> get_hashes_from_commits = [&](const path& commit_dir) {
				path prev_link = commit_dir / "PREV";
				if (fs::exists(prev_link) && fs::is_symlink(prev_link))
					get_hashes_from_commits(fs::read_symlink(prev_link));

				for (auto& entry : fs::recursive_directory_iterator(commit_dir)) {
					auto path = entry.path();
					if (fs::is_regular_file(path)) {
						std::string file_hash = get_file_hash(path.string());
						commit_hashes[file_hash] = fs::relative(path, commit_dir);
					}
				}
			};
			get_hashes_from_commits(current_commit_hash);
			fs::remove(repository_path + "\\.vcs\\HEAD");
			// ----------------- Бежим по рабочей директории ---------------------------------------
			for (auto& file : fs::directory_iterator(repository_path)) {
				auto file_path = file.path();
				if (fs::equivalent(file_path, repository_path + "\\.vcs"))
					continue;

				auto relative_path = fs::relative(file_path, repository_path);
				auto target_path = commit_directory / relative_path;
				// ----------------- Лямбда для копирования ---------------------------------------
				std::function<void(const path&, const path&)> copy_commit = [&relative_path, &commit_hashes](const path& From, const path& To) mutable {
					std::string working_hash = get_file_hash(From.string());

					auto iter = commit_hashes.find(working_hash);
					if (iter != commit_hashes.end()) {
						if (!fs::equivalent(iter->second, relative_path)) {
							fs::create_directories(To.parent_path());
							fs::copy_file(From, To, fs::copy_options::overwrite_existing);
							std::cout << "File { " << From.filename() << " } added to commit\n";
						}
						commit_hashes.erase(iter);

					}
					else {
						fs::create_directories(To.parent_path());
						fs::copy_file(From, To, fs::copy_options::overwrite_existing);
						std::cout << "File { " << To.filename() << " } added to commit\n";
					}
				};
				// ----------------- Копирование файла ---------------------------------------

				if (fs::is_regular_file(file_path)) {
					copy_commit(file_path, target_path);
				}
				else if (fs::is_directory(file_path)) {
					for (auto& directory_entry : fs::recursive_directory_iterator(file_path)) {
						auto directory_file_path = directory_entry.path();
						relative_path /= directory_file_path.filename();
						copy_commit(directory_file_path, target_path / directory_file_path.filename());
					}
				}
			}
			// ----------- Создаем ссылку на предыдущий коммит --------------------------------------------
			fs::create_directory_symlink(current_commit_hash, commit_directory + "\\PREV");
		}
		else {
			// ----------- Если это наш первый коммит, то бежим по директории и просто копируем файлы ----
			for (auto& file : fs::directory_iterator(repository_path)) {
				auto path = file.path();
				if (fs::equivalent(path, repository_path + "\\.vcs"))
					continue;

				try {
					auto target_path = commit_directory / path.filename();

					if (fs::is_directory(path)) {
						fs::copy(path, target_path, fs::copy_options::recursive
							| fs::copy_options::overwrite_existing);
					}
					else if (fs::is_regular_file(path)) {
						fs::copy_file(path, target_path, fs::copy_options::overwrite_existing);
					}

					std::cout << "File { " << path.filename() << " } added to commit\n";
				}
				catch (const fs::filesystem_error& e) {
					std::cout << "Copying error: " << e.what() << "\n";
				}
			}
		}
		// ------------- Переписываем ссылку на текущий коммит и записываем в джисон --------------
		fs::create_directory_symlink(commit_directory, repository_path + "\\.vcs\\HEAD");
		write_commit(commit);
		return true;
	}
	catch (const fs::filesystem_error& e) {
		std::cout << "Filesystem error in push: " << e.what() << "\n";
		return false;
	}
}

bool VCS::pull(const unsigned int& commit_id) {
	if (!is_repositoty)
		throw std::runtime_error("No repositories creted");

	if (!is_exists_commit_id(commit_id))
		throw std::runtime_error("There isnt commit with this id");

	try {
		auto check_commit = get_commit_info();
		bool is_expired = false;
		for (const auto& commit : check_commit) {
			if (commit.find("ID: " + std::to_string(commit_id) + "\n") != std::string::npos ||
				commit.find("ID: " + std::to_string(commit_id) + " (HEAD)\n") != std::string::npos) {
				if (commit.find("EXPIRED") != std::string::npos)
					is_expired = true;
				break;
			}
		}
		if (is_expired) {
			throw std::runtime_error("You cant jump on this expired commit");
		}
		cleaning_expired_commits();


		path commit_directory = repository_path + "\\.vcs\\objects\\" + get_id_hash(commit_id);
		if (!fs::exists(commit_directory))
			return false;

		std::map<path, path> file_state;
		// -------- Собираем файлы предыдущих комиитов и записываем в мапу ------------------
		std::function<void(const path&)> apply_commit = [&](const path& commit_dir) {
			path prev_link = commit_dir / "PREV";
			if (fs::exists(prev_link) && fs::is_symlink(prev_link)) {
				apply_commit(fs::read_symlink(prev_link));
			}

			for (auto& entry : fs::recursive_directory_iterator(commit_dir)) {
				auto file_path = entry.path();
				if (file_path.filename() == "PREV" || !fs::is_regular_file(file_path))
					continue;

				file_state[fs::relative(file_path, commit_dir)] = file_path;
			}
		};

		apply_commit(commit_directory);
		// ------- Чистим рабочию директорию ---------------------------
		std::cout << "Cleaning working directory\n";
		for (auto& entry : fs::directory_iterator(repository_path)) {
			auto path = entry.path();
			if (!fs::equivalent(path, repository_path + "\\.vcs")) {
				try {
					fs::remove_all(path);
				}
				catch (const fs::filesystem_error& e) {
					std::cout << "Error removing " << path.filename() << " : " << e.what() << "\n";
				}
			}
		}

		// ------------- Востанавливаем файлы. Из за того, что это мапа и это все рекурсивно происходит, так что измененные файлы перепишутся --------
		std::cout << "Restoring files...\n";
		for (const auto& [relative_path, file_path] : file_state) {
			try {
				auto target_path = repository_path / relative_path;
				fs::create_directories(target_path.parent_path());
				fs::copy_file(file_path, target_path, fs::copy_options::overwrite_existing);
				std::cout << " Restored: " << relative_path << "\n";
			}
			catch (const fs::filesystem_error& e) {
				std::cout << "Error restoring file " << relative_path << " : " << e.what() << "\n";
			}
		}

		fs::remove(repository_path + "\\.vcs\\HEAD");
		fs::create_directory_symlink(commit_directory, repository_path + "\\.vcs\\HEAD");
		return true;
	}
	catch (const fs::filesystem_error& e) {
		std::cout << "Filesystem error in pull: " << e.what() << "\n";
		return false;
	}
}

bool VCS::log() {
	if (!is_repositoty)
		throw std::runtime_error("No repositories creted");
	/*
		get_commit_info возвращает массив строк с нужной инфой
		просто красиво выводим это через "*"
	*/
	try {
		cleaning_expired_commits();

		auto logs = get_commit_info();
		if (logs.empty()) {
			std::cout << "No commits added\n";
			return true;
		}
		for (auto& commit : logs) {
			std::cout << "* " << commit << "\n";
		}
		return true;
	}
	catch (const fs::filesystem_error& e) {
		std::cout << "Filesystem error in log: " << e.what() << "\n";
		return false;
	}
}