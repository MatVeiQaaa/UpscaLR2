#include "ConfigManager.hpp"

#include <fstream>
#include <string>

ConfigManager::ConfigManager(std::string path, bool load) {
	ConfigManager::path = path;
	if (load) LoadConfig();
}

void ConfigManager::WriteComment(const std::string& name, const std::string& comment, bool save) {
	config[name].comment = comment;
	if (save) SaveConfig();
}

bool ConfigManager::ValueExists(const std::string& name) {
	return config.contains(name);
}

void ConfigManager::SaveConfig() {
	std::ofstream file(path);
	bool first = true;
	for (auto& [name, entry] : config) {
		if (!entry.comment.empty()) {
			if (first) first = false;
			else file << std::endl;
			std::vector<std::string> lines = StrSplit<std::string>(entry.comment, "\n");
			for (auto& line : lines)
				file << "// " << line << std::endl;
		}
		file << name << " = " << entry.value << std::endl;
	}
}

void ConfigManager::LoadConfig() {
	std::ifstream file(path);
	std::string line;
	std::string comment;
	while (std::getline(file, line))
	{
		if (line.empty()) continue;

		if (line.starts_with("//")) {
			std::string commLine = line.substr(line.find_first_not_of(' ', 2));
			if (!commLine.empty()) {
				if (!comment.empty()) comment += '\n';
				comment += commLine;
			}
		}
		else {
			std::string name = line.substr(0, line.find_first_of('='));
			name.erase(std::remove_if(name.begin(), name.end(), isspace), name.end());

			std::string value = line.substr(line.find_first_of('=') + 1, line.size() - 1);
			if (*value.begin() == ' ') value.erase(value.begin());

			Record& rec = config[name];
			rec.value = value;
			rec.comment = comment;
			comment.clear();
		}
	}
}