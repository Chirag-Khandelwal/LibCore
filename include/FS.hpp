#pragma once

#include "Status.hpp"

#if defined(CORE_OS_WINDOWS)
ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp);
ssize_t getline(char **buf, size_t *bufsiz, FILE *fp);
#endif

namespace core::fs
{

StringRef parentDir(StringRef path);

Status<bool> read(const char *file, String &data);
StringRef home();

int copy(StringRef src, StringRef dest, std::error_code &ec);
int mkdir(StringRef dir, std::error_code &ec);
int mklink(StringRef src, StringRef dest, std::error_code &ec);
int rename(StringRef from, StringRef to, std::error_code &ec);
int remove(StringRef path, std::error_code &ec);

bool exists(StringRef loc);
String baseName(StringRef path);
String absPath(StringRef path);
void setCWD(StringRef path);
String getCWD();

} // namespace core::fs