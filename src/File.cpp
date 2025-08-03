#include "File.hpp"

#include "FS.hpp"

namespace core::fs
{

File::File(const char *path, bool isVirt) : path(path), isVirt(isVirt) {}

Result<File, bool> File::create(const char *path, bool isVirt)
{
	File file(path, isVirt);
	if(!isVirt) {
		Status<bool> res = fs::read(path, file.data);
		if(!res.getCode()) return res;
	}
	if(!file.data.empty()) file.appendLocs.push_back(0);
	return file;
}

bool File::set(String &&data)
{
	if(!isVirt) return false;
	this->appendLocs.clear();
	this->appendLocs.push_back(0);
	this->data = std::move(data);
	return true;
}
bool File::append(StringRef data)
{
	if(!isVirt || data.empty()) return false;
	this->appendLocs.push_back(this->data.size());
	this->data += data;
	return true;
}

StringRef File::getAppendData(size_t index) const
{
	if(index >= appendLocs.size()) return "";
	size_t start = appendLocs[index];
	size_t end   = data.size();
	if(index < appendLocs.size() - 1) end = appendLocs[index + 1];
	return StringRef(data.begin() + appendLocs[index], data.begin() + end);
}

} // namespace core::fs