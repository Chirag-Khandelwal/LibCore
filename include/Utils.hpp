#pragma once

#include "Core.hpp"

namespace core
{
class File;
}

namespace core::utils
{

inline bool startsWith(StringRef src, StringRef term) { return src.rfind(term, 0) == 0; }

#if defined(CORE_OS_WINDOWS)
CORE_API WString sToWString(const char *data);
CORE_API String wToString(const wchar_t *data);
#endif

// Output a char `ch`, `count` times.
CORE_API void outputChar(OStream &os, char ch, size_t count);
// Does not include loc. So if loc is a newline, the function won't return loc itself.
CORE_API size_t getNewLineBefore(StringRef data, size_t loc);
// Does not include loc. So if loc is a newline, the function won't return loc itself.
CORE_API size_t getNewLineAfter(StringRef data, size_t loc);
// The range is [0, loc] (inclusive)
CORE_API size_t countNewLinesTill(StringRef data, size_t loc);

// Count number of digits (positive number only)
CORE_API size_t countDigits(size_t num);

// Counts all instances of `c` in `str`.
CORE_API size_t stringCharCount(StringRef str, char ch);

// Replaces all instances of `from` with `to` in `str`.
CORE_API void stringReplace(String &str, StringRef from, StringRef to);

// Also trims the spaces for each split
CORE_API Vector<StringRef> stringDelim(StringRef str, StringRef delim);

// Convert special characters in string (\n, \t, ...) to raw (\\n, \\t, ...)
// and vice versa
CORE_API String toRawString(String &&data);
CORE_API String fromRawString(String &&data);
CORE_API String toRawString(StringRef data);
CORE_API String fromRawString(StringRef data);

CORE_API String vecToStr(Span<StringRef> items);
CORE_API String vecToStr(Span<String> items);

CORE_API void removeBackSlash(String &s);
CORE_API String viewBackSlash(StringRef data);

CORE_API void output(OStream &os, File *src, size_t locStart, size_t locEnd, StringRef data);

inline void appendToString(String &dest) {}

inline void appendToString(String &dest, bool data) { dest += data ? "(true)" : "(false)"; }
inline void appendToString(String &dest, char data) { dest += data; }
inline void appendToString(String &dest, uint8_t data) { dest += std::to_string(data); }
inline void appendToString(String &dest, uint16_t data) { dest += std::to_string(data); }
inline void appendToString(String &dest, int64_t data) { dest += std::to_string(data); }
inline void appendToString(String &dest, size_t data) { dest += std::to_string(data); }
inline void appendToString(String &dest, int data) { dest += std::to_string(data); }
inline void appendToString(String &dest, float data) { dest += std::to_string(data); }
inline void appendToString(String &dest, double data) { dest += std::to_string(data); }
inline void appendToString(String &dest, char *data) { dest += data; }
inline void appendToString(String &dest, const char *data) { dest += data; }
inline void appendToString(String &dest, StringRef data) { dest += data; }
inline void appendToString(String &dest, const String &data) { dest += data; }
#if defined(CORE_OS_WINDOWS)
inline void appendToString(String &dest, const Path &data) { dest += wToString(data.c_str()); }
#else
inline void appendToString(String &dest, const Path &data) { dest += data; }
#endif

template<typename... Args> void appendToString(String &dest, Args... args)
{
    int tmp[] = {(appendToString(dest, args), 0)...};
    static_cast<void>(tmp);
}
template<typename... Args> String toString(Args... args)
{
    String dest;
    appendToString(dest, std::forward<Args>(args)...);
    return dest;
}

} // namespace core::utils