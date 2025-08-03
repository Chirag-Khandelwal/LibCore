#pragma once

#include "Result.hpp"

namespace core::fs
{

class FileLoc
{
	uint32_t start;
	uint32_t end;

public:
	FileLoc();
	FileLoc(uint32_t start, uint32_t end);

	inline void setStart(uint32_t _start) { start = _start; }
	inline void setEnd(uint32_t _end) { end = _end; }

	inline uint32_t getStart() const { return start; }
	inline uint32_t getEnd() const { return end; }

	inline bool hasEnd() const { return end != -1 && start < end; }
	inline bool isValid() const { return start != -1; }
};

// To create and manage files (including virtual like `<eval>`)
// as well as provide error messages using file locations.
class File
{
	String path;
	// All file contents are stored in this.
	String data;
	// For virtual files, locations where append() was called.
	// Using this, say, the last appended section of the data string can be retrieved.
	Vector<uint32_t> appendLocs;
	bool isVirt; // isVirtual

	// Output a char `ch`, `count` times.
	void outputChar(OStream &os, char ch, uint32_t count);
	void outputString(OStream &os, FileLoc loc, bool iswarn, const String &e);
	// Does not include loc. So if loc is a newline, the function won't return loc itself.
	uint32_t getNewLineBefore(uint32_t loc);
	// Does not include loc. So if loc is a newline, the function won't return loc itself.
	uint32_t getNewLineAfter(uint32_t loc);
	// The range is [0, loc] (inclusive)
	uint32_t countNewLinesTill(uint32_t loc);
	// Count number of digits (positive number only)
	uint32_t countDigits(uint32_t num);

	template<typename... Args>
	void output(OStream &os, FileLoc loc, bool iswarn, Args &&...args)
	{
		String res = utils::toString(std::forward<Args>(args)...);
		outputString(os, loc, iswarn, res);
	}

	File(StringRef path, bool isVirt, StringRef data = "");

public:
	static Result<File, Status<bool>> create(const char *path, bool isVirt);

	bool append(StringRef data);
	StringRef getAppendData(uint32_t index) const;

	template<typename... Args> void fail(OStream &os, FileLoc loc, Args &&...args)
	{
		output(os, loc, false, std::forward<Args>(args)...);
	}

	template<typename... Args> void warn(OStream &os, FileLoc loc, Args &&...args)
	{
		output(os, loc, true, std::forward<Args>(args)...);
	}

	inline StringRef getPath() const { return path; }
	inline StringRef getData() const { return data; }
	inline bool isVirtual() const { return isVirt; }
	inline StringRef getLastAppendData() const { return getAppendData(appendLocs.size() - 1); }

	inline bool emptyPath() const { return path.empty(); }
	inline bool emptyData() const { return data.empty(); }
	// appendLocs is never empty.
	inline size_t sizePath() const { return path.size(); }
	inline size_t sizeData() const { return data.size(); }
	inline size_t sizeAppendLocs() const { return appendLocs.size(); }
};

} // namespace core::fs