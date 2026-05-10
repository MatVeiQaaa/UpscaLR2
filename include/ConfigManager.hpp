#pragma once

#include <string>
#include <sstream>
#include <charconv>
#include <filesystem>
#include <vector>
#include <optional>
#include <map>

class ConfigManager {
public:
	ConfigManager(std::string path, bool load = true);

	template <typename T>
	void WriteValue(const std::string& name, T value, bool save) {
		config[name].value = FromVal(value);
		if (save) SaveConfig();
	}

	template <typename T>
	void WriteArray(const std::string& name, const std::vector<T>& array, bool save) {
		config[name].value = ValJoin(array, "<NEXT>");
		if (save) SaveConfig();
	}
	void WriteComment(const std::string& name, const std::string& comment, bool save = false);
	template <typename T>
	T ReadValue(const std::string& name, T def) {
		auto val = ToVal<T>(config[name].value);
		if (!val) {
			WriteValue(name, def, true);
			return def;
		}
		return *val;
	}

	template <typename T>
	void ReadArray(const std::string& name, std::vector<T>& array) {
		array = StrSplit<T>(config[name].value, "<NEXT>");
	}
	bool ValueExists(const std::string& name);

	void SaveConfig();
	void LoadConfig();
private:
	struct Record {
		std::string value;
		std::string comment;
	};

	template <typename T>
	std::optional<T> ToVal(const std::string& val) {
		T value{};
		auto [ptr, ec] = std::from_chars(val.data(), val.data() + val.size(), value);
		if (ec == std::errc()) {
			return value;
		}
		return {};
	}
	template <>
	std::optional<bool> ToVal<bool>(const std::string& val) {
		if (val.empty()) return {};
		return val == "true" ? true : false;
	}
	template<>
	std::optional<std::string> ToVal<std::string>(const std::string& val) {
		if (val.empty()) return {};
		return val;
	}
	template <>
	std::optional<std::filesystem::path> ToVal<std::filesystem::path>(const std::string& val) {
		return ToVal<std::string>(val);
	}

	template <typename T>
	std::string FromVal(const T& val) {
		return std::to_string(val);
	}
	template <>
	std::string FromVal(const bool& val) {
		return val ? "true" : "false";
	}
	template <>
	std::string FromVal(const std::filesystem::path& val) {
		return val.string();
	}
	template <>
	std::string FromVal(const std::string& val) {
		return val;
	}

	template <typename T>
	std::vector<T> StrSplit(std::string str, const std::string& delim) {
		std::vector<T> res;
		std::string token;
		for (size_t offsetFirst = 0, offsetLast = str.find(delim); offsetLast <= str.size() - delim.size() || offsetLast == std::string::npos; offsetLast = str.find(delim, offsetFirst)) {
			token = str.substr(offsetFirst, offsetLast - offsetFirst);
			auto val = ToVal<T>(token);
			if (val) res.push_back(*val);
			if (offsetLast == std::string::npos) break;
			offsetFirst = offsetLast + delim.size();
		}
		return res;
	}

	template <typename T>
	std::string ValJoin(const std::vector<T>& values, const std::string& delim) {
		std::stringstream res;
		for (auto& value : values) {
			res << FromVal<T>(value) << delim;
		}
		return res.str();
	}

	std::string path;
	std::map<std::string, Record> config;
};