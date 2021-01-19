void ltrim(std::string& s);
void rtrim(std::string& s);
void trim(std::string& s);

std::shared_ptr<spdlog::logger> CreateLogger(const std::filesystem::path& path, const std::string& id, const std::string& pattern = "[%H:%M:%S %z][%l] %v");