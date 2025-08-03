#include "File.hpp"

#include "FS.hpp"

namespace core::fs
{

FileLoc::FileLoc() : start(-1), end(-1) {}
FileLoc::FileLoc(uint32_t start, uint32_t end) : start(start), end(end) {}

File::File(StringRef path, bool isVirt, StringRef data) : path(path), data(data), isVirt(isVirt) {}

Result<File, Status<bool>> File::create(const char *path, bool isVirt)
{
	File file(path, isVirt);
	if(!isVirt) {
		Status<bool> res = fs::read(path, file.data);
		if(!res.getCode()) return res;
	}
	if(!file.data.empty()) file.appendLocs.push_back(0);
	return file;
}

bool File::append(StringRef data)
{
	if(!isVirt || data.empty()) return false;
	this->appendLocs.push_back(this->data.size());
	this->data += data;
	return true;
}

StringRef File::getAppendData(uint32_t index) const
{
	if(index >= appendLocs.size()) return "";
	uint32_t start = appendLocs[index];
	uint32_t end   = data.size();
	if(index < appendLocs.size() - 1) end = appendLocs[index + 1];
	return StringRef(data.begin() + appendLocs[index], data.begin() + end);
}

void File::outputChar(OStream &os, char ch, uint32_t count)
{
	for(uint32_t i = 0; i < count / 4; ++i) {
		os << ch << ch << ch << ch;
	}
	for(uint32_t i = 0; i < count % 4; ++i) {
		os << ch;
	}
}

void File::outputString(OStream &os, FileLoc loc, bool iswarn, const String &e)
{
	// Invalid loc, just show the message.
	if(loc.isValid()) {
		uint32_t prevNewLine = getNewLineBefore(loc.getStart());
		uint32_t nextNewLine =
		loc.hasEnd() ? getNewLineAfter(loc.getEnd()) : getNewLineAfter(loc.getStart());

		// + 1 for index -> line number
		uint32_t lineNumber  = countNewLinesTill(prevNewLine) + 1;
		uint32_t columnStart = loc.getStart() - (prevNewLine + 1);
		uint32_t columnEnd = loc.hasEnd() ? loc.getEnd() - (prevNewLine + 1) : columnStart;

		uint32_t prefixSpaceCount = countDigits(lineNumber) + 3; // lineNum + " | "
		StringRef line(data.begin() + (prevNewLine + 1), data.begin() + nextNewLine);
		os << lineNumber << " | " << line << "\n";
		outputChar(os, ' ', prefixSpaceCount + columnStart);
		outputChar(os, '~', columnEnd - columnStart + 1);
		os << "\n";
		outputChar(os, ' ', prefixSpaceCount + columnStart);
		os << "^\n";
		outputChar(os, ' ', prefixSpaceCount);
	}
	if(iswarn) os << "Warning: ";
	else os << "Error: ";
	os << e << "\n";
}

uint32_t File::getNewLineBefore(uint32_t loc)
{
	int64_t locIt = (int64_t)loc - 1;
	while(locIt >= 0) {
		if(data[locIt--] == '\n') return locIt + 1;
	}
	return -1;
}
uint32_t File::getNewLineAfter(uint32_t loc)
{
	int64_t locIt = (int64_t)loc + 1;
	while(locIt < data.size()) {
		if(data[locIt++] == '\n') return locIt - 1;
	}
	return data.size();
}
uint32_t File::countNewLinesTill(uint32_t loc)
{
	if(loc == -1) return 0;
	int64_t end  = loc;
	uint32_t ctr = 0;
	for(int64_t i = 0; i < data.size() && i <= end; ++i) {
		if(data[i] == '\n') ++ctr;
	}
	return ctr;
}

uint32_t File::countDigits(uint32_t num)
{
	// clang-format off
	return  (num < 10 ? 1 :
		(num < 100 ? 2 :
		(num < 1000 ? 3 :
		(num < 10000 ? 4 :
		(num < 100000 ? 5 :
		(num < 1000000 ? 6 :
		(num < 10000000 ? 7 :
		(num < 100000000 ? 8 :
		(num < 1000000000 ? 9 :
		(num < 4294967295 ? 10 : 0))))))))));
	// clang-format on
}

} // namespace core::fs