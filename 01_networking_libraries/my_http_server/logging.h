#include <iostream>
#include <string>
#include <mutex>
#include <ctime>

enum class LogLevel
{
    Inform,
    Debug,
    Error
};

class Logger
{
public:
    // Singleton instance accessor
    static Logger& getInstance()
    {
        static Logger instance;
        return instance;
    }

    void setLogLevel(LogLevel level)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_logLevel = level;
    }

    void inform(const std::string& message)
    {
        log(LogLevel::Inform, "[INFO] ", message);
    }

    void debug(const std::string& message)
    {
        log(LogLevel::Debug, "[DEBUG] ", message);
    }

    void error(const std::string& message)
    {
        log(LogLevel::Error, "[ERROR] ", message);
    }

private:
    LogLevel m_logLevel = LogLevel::Inform;
    std::mutex m_mutex;

    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(LogLevel level, const std::string& prefix, const std::string& message)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (level >= m_logLevel)
        {
            std::cout << "[" << time(nullptr) << "] " << prefix << message << std::endl;
        }
    }
};

#define LOGI(message) Logger::getInstance().inform(message)
#define LOGD(message) Logger::getInstance().debug(message)
#define LOGE(message) Logger::getInstance().error(message)
