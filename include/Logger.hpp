#pragma once

#include "Utils.hpp"

namespace core
{

enum class LogLevels
{
	FATAL,
	WARN,
	INFO,
	DEBUG,
	TRACE
};

const char *logLevelStr(LogLevels lvl);
const char *logLevelColorStr(LogLevels lvl);

struct SinkInfo
{
	OStream *f;
	bool withCol;
	bool mustClose;

	SinkInfo(OStream *f, bool withCol, bool mustClose);
	// Declaring a copy constructor deletes implicit move constructor required by vector when it
	// grows.
	SinkInfo(SinkInfo &&si) noexcept;
	// No copy constructor.
	SinkInfo(const SinkInfo &SinkInfo) = delete;
	// No copy operator.
	SinkInfo &operator=(const SinkInfo &SinkInfo) = delete;
	~SinkInfo();
};

class Logger
{
	Vector<SinkInfo> sinks;
	LogLevels level;

	void logInternal(LogLevels lvl, StringRef data);

	template<typename... Args> void log(LogLevels lvl, Args &&...args)
	{
		if((int)level < (int)lvl) return;
		String res = utils::toString(std::forward<Args>(args)...);
		logInternal(lvl, res);
	}

public:
	Logger();

	bool addSinkByName(const char *name, bool withCol);

	inline void addSink(OStream *f, bool withCol, bool mustClose)
	{
		sinks.emplace_back(f, withCol, mustClose);
	}

	template<typename... Args> void fatal(Args &&...args)
	{
		log(LogLevels::FATAL, std::forward<Args>(args)...);
	}
	template<typename... Args> void warn(Args &&...args)
	{
		log(LogLevels::WARN, std::forward<Args>(args)...);
	}
	template<typename... Args> void info(Args &&...args)
	{
		log(LogLevels::INFO, std::forward<Args>(args)...);
	}
	template<typename... Args> void debug(Args &&...args)
	{
		log(LogLevels::DEBUG, std::forward<Args>(args)...);
	}
	template<typename... Args> void trace(Args &&...args)
	{
		log(LogLevels::TRACE, std::forward<Args>(args)...);
	}

	inline void setLevel(LogLevels lvl) { level = lvl; }
	inline LogLevels getLevel() { return level; }
};

extern DLL_EXPORT Logger logger;

} // namespace core