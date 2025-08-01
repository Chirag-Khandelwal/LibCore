#pragma once

#include "Status.hpp"

#if defined(CORE_OS_WINDOWS)
ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp);
ssize_t getline(char **buf, size_t *bufsiz, FILE *fp);
#endif

namespace core::fs
{

StringRef parentDir(StringRef path);

bool exists(StringRef loc);
Status<bool> read(const char *file, String &data);
String absPath(const char *loc);
bool setCWD(const char *path);
String getCWD();
StringRef home();

int copy(StringRef src, StringRef dest, std::error_code &ec);
int mkdir(StringRef dir, std::error_code &ec);
int mklink(StringRef src, StringRef dest, std::error_code &ec);
int rename(StringRef from, StringRef to, std::error_code &ec);
int remove(StringRef path, std::error_code &ec);

} // namespace core::fs