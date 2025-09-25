#include "FS.hpp"

#include <filesystem>

#include "Env.hpp"

#if defined(CORE_OS_WINDOWS)
#include <direct.h>
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace core::fs
{

StringRef parentDir(StringRef path)
{
	auto loc = path.find_last_of("/\\");
	if(loc == String::npos) return "";
	return path.substr(0, loc);
}

Status<bool> read(const char *file, String &data)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen(file, "r");
	if(fp == NULL) return Status(false, "Error: failed to open source file: ", file);

	while((read = getline(&line, &len, fp)) != -1) data += line;

	fclose(fp);
	if(line) free(line);

	return Status(true);
}

StringRef home()
{
	// StringRef works because String _home is static.
#if defined(CORE_OS_WINDOWS)
	static String _home = env::get("USERPROFILE");
#else
	static String _home = env::get("HOME");
#endif
	return _home;
}

int copy(StringRef src, StringRef dest, std::error_code &ec)
{
	auto opts =
	std::filesystem::copy_options::update_existing | std::filesystem::copy_options::recursive;
	std::filesystem::copy(src, dest, opts, ec);
	return ec.value();
}
int mkdir(StringRef dir, std::error_code &ec)
{
	std::filesystem::create_directories(dir, ec);
	return ec.value();
}
int mklink(StringRef src, StringRef dest, std::error_code &ec)
{
	if(exists(dest)) {
		remove(dest, ec);
		if(ec.value()) return ec.value();
	}
	if(exists(src) && std::filesystem::is_directory(src)) {
		std::filesystem::create_directory_symlink(src, dest, ec);
	} else {
		std::filesystem::create_symlink(src, dest, ec);
	}
	return ec.value();
}
int rename(StringRef from, StringRef to, std::error_code &ec)
{
	std::filesystem::rename(from, to, ec);
	return ec.value();
}
int remove(StringRef path, std::error_code &ec)
{
	std::filesystem::remove_all(path, ec);
	return ec.value();
}

bool exists(StringRef loc)
{
	return std::filesystem::symlink_status(loc).type() != std::filesystem::file_type::not_found;
}
String baseName(StringRef path) { return std::filesystem::path(path).filename().string(); }
String absPath(StringRef path) { return std::filesystem::absolute(path).string(); }
void setCWD(StringRef path) { return std::filesystem::current_path(path); }
String getCWD() { return std::filesystem::current_path().string(); }

} // namespace core::fs

#if defined(CORE_OS_WINDOWS)
// getdelim and getline functions from NetBSD libnbcompat:
// https://github.com/archiecobbs/libnbcompat

/*-
 * Copyright (c) 2011 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Christos Zoulas.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp)
{
	char *ptr, *eptr;

	if(*buf == NULL || *bufsiz == 0) {
		*bufsiz = BUFSIZ;
		if((*buf = (char *)malloc(*bufsiz)) == NULL) return -1;
	}

	for(ptr = *buf, eptr = *buf + *bufsiz;;) {
		int c = fgetc(fp);
		if(c == -1) {
			if(feof(fp)) {
				ssize_t diff = (ssize_t)(ptr - *buf);
				if(diff != 0) {
					*ptr = '\0';
					return diff;
				}
			}
			return -1;
		}
		*ptr++ = c;
		if(c == delimiter) {
			*ptr = '\0';
			return ptr - *buf;
		}
		if(ptr + 2 >= eptr) {
			char *nbuf;
			size_t nbufsiz = *bufsiz * 2;
			ssize_t d      = ptr - *buf;
			if((nbuf = (char *)realloc(*buf, nbufsiz)) == NULL) return -1;
			*buf	= nbuf;
			*bufsiz = nbufsiz;
			eptr	= nbuf + nbufsiz;
			ptr	= nbuf + d;
		}
	}
	return 0;
}

ssize_t getline(char **buf, size_t *bufsiz, FILE *fp) { return getdelim(buf, bufsiz, '\n', fp); }
#endif
