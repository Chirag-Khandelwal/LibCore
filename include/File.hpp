#pragma once

#include "Result.hpp"

namespace core::fs
{

// To create and manage files (including virtual like `<eval>`)
// as well as provide error messages using file locations.
class File
{
	String path;
	// All file contents are stored in this.
	String data;
	// For virtual files, locations where append() was called.
	// Using this, say, the last appended section of the data string can be retrieved.
	Vector<size_t> appendLocs;
	bool isVirt; // isVirtual

	File(const char *path, bool isVirt);

public:
	static Result<File, bool> create(const char *path, bool isVirt);

	bool set(String &&data);
	// Only for virtual files.
	bool append(StringRef data);
	StringRef getAppendData(size_t index) const;

	inline StringRef getPath() const { return path; }
	inline StringRef getData() const { return data; }
	inline bool isVirtual() const { return isVirt; }
	inline StringRef getLastAppendData() const { return getAppendData(appendLocs.size() - 1); }

	inline bool emptyData() const { return data.empty(); }
	// appendLocs is never empty.
	inline size_t sizeData() const { return data.size(); }
	inline size_t sizeAppendLocs() const { return appendLocs.size(); }
};

} // namespace core::fs