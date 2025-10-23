#include "Utils.hpp"

#include "File.hpp"

#if defined(CORE_OS_WINDOWS)
#include <AtlBase.h>
#include <atlconv.h>
#endif

namespace core::utils
{

void outputChar(OStream &os, char ch, size_t count)
{
    for(size_t i = 0; i < count / 4; ++i) {
        os << ch << ch << ch << ch;
    }
    for(size_t i = 0; i < count % 4; ++i) {
        os << ch;
    }
}
size_t getNewLineBefore(StringRef data, size_t loc)
{
    while(loc != -1 && loc >= 0) {
        if(data[loc--] == '\n') return loc + 1;
    }
    return -1;
}
size_t getNewLineAfter(StringRef data, size_t loc)
{
    while(loc != -1 && loc < data.size()) {
        if(data[loc++] == '\n') return loc - 1;
    }
    return data.size();
}
size_t countNewLinesTill(StringRef data, size_t loc)
{
    if(loc == -1) return 0;
    int64_t end = loc;
    size_t ctr  = 0;
    for(int64_t i = 0; i < data.size() && i <= end; ++i) {
        if(data[i] == '\n') ++ctr;
    }
    return ctr;
}

size_t countDigits(size_t num)
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

size_t stringCharCount(StringRef str, char ch)
{
    size_t count = 0;
    for(auto c : str) {
        if(c == ch) ++count;
    }
    return count;
}

void stringReplace(String &str, StringRef from, StringRef to)
{
    auto &&pos = str.find(from);
    while(pos != String::npos) {
        str.replace(pos, from.length(), to);
        // easy to forget to add to.length()
        pos = str.find(from, pos + to.length());
    }
}

Vector<StringRef> stringDelim(StringRef str, StringRef delim)
{
    Vector<StringRef> res;
    if(str.empty()) return res;

    size_t start = 0;
    size_t end   = str.find(delim);
    while(end != String::npos) {
        res.emplace_back(str.substr(start, end - start));
        start = end + delim.length();
        end   = str.find(delim, start);
    }
    res.emplace_back(str.substr(start, end));

    for(auto &s : res) {
        while(!s.empty() && s.front() == ' ') s = s.substr(0, 1);
        while(!s.empty() && s.back() == ' ') s = s.substr(s.size() - 1);
    }

    return res;
}

String toRawString(StringRef data)
{
    String res(data);
    for(size_t i = 0; i < res.size(); ++i) {
        if(res[i] == '\\') {
            res.insert(i++, "\\");
            continue;
        }
        if(res[i] == '\0') {
            res.erase(res.begin() + i);
            res.insert(i++, "\\0");
            continue;
        }
        if(res[i] == '\a') {
            res.erase(res.begin() + i);
            res.insert(i++, "\\a");
            continue;
        }
        if(res[i] == '\b') {
            res.erase(res.begin() + i);
            res.insert(i++, "\\b");
            continue;
        }
#if !defined(CORE_OS_WINDOWS)
        if(res[i] == '\e') {
            res.erase(res.begin() + i);
            res.insert(i++, "\\e");
            continue;
        }
#endif
        if(res[i] == '\f') {
            res.erase(res.begin() + i);
            res.insert(i++, "\\f");
            continue;
        }
        if(res[i] == '\n') {
            res.erase(res.begin() + i);
            res.insert(i++, "\\n");
            continue;
        }
        if(res[i] == '\r') {
            res.erase(res.begin() + i);
            res.insert(i++, "\\r");
            continue;
        }
        if(res[i] == '\t') {
            res.erase(res.begin() + i);
            res.insert(i++, "\\t");
            continue;
        }
        if(res[i] == '\v') {
            res.erase(res.begin() + i);
            res.insert(i++, "\\v");
            continue;
        }
    }
    return res;
}

String fromRawString(StringRef from)
{
    String data(from);
    for(size_t idx = 0; idx < data.size(); ++idx) {
        if(data[idx] != '\\') continue;
        if(idx + 1 >= data.size()) continue;
        data.erase(data.begin() + idx);
        if(data[idx] == '0') data[idx] = '\0';
        else if(data[idx] == 'a') data[idx] = '\a';
        else if(data[idx] == 'b') data[idx] = '\b';
#if !defined(CORE_OS_WINDOWS)
        else if(data[idx] == 'e') data[idx] = '\e';
#endif
        else if(data[idx] == 'f') data[idx] = '\f';
        else if(data[idx] == 'n') data[idx] = '\n';
        else if(data[idx] == 'r') data[idx] = '\r';
        else if(data[idx] == 't') data[idx] = '\t';
        else if(data[idx] == 'v') data[idx] = '\v';
    }
    return data;
}

String vecToStr(Span<StringRef> items)
{
    String res = "[";
    for(auto i : items) {
        res += i;
        res += ", ";
    }
    if(!items.empty()) {
        res.pop_back();
        res.pop_back();
    }
    res += "]";
    return res;
}

String vecToStr(Span<String> items)
{
    String res = "[";
    for(auto i : items) {
        res += i;
        res += ", ";
    }
    if(!items.empty()) {
        res.pop_back();
        res.pop_back();
    }
    res += "]";
    return res;
}

#if defined(CORE_OS_WINDOWS)
// Windows' string to wstring functions
WString toWString(StringRef data)
{
    size_t wstrLen = std::mbstowcs(nullptr, data.data(), data.size());
    WString wstr(wstrLen, 0);
    std::mbstowcs(wstr.data(), data.data(), data.size());
    return wstr;
}
#endif

void removeBackSlash(String &s)
{
    for(auto it = s.begin(); it != s.end(); ++it) {
        if(*it == '\\') {
            if(it + 1 >= s.end()) continue;
            it = s.erase(it);
            if(*it == '0') *it = '\0';
            else if(*it == 'a') *it = '\a';
            else if(*it == 'b') *it = '\b';
#if !defined(OS_WINDOWS)
            else if(*it == 'e') *it = '\e';
#endif
            else if(*it == 'f') *it = '\f';
            else if(*it == 'n') *it = '\n';
            else if(*it == 'r') *it = '\r';
            else if(*it == 't') *it = '\t';
            else if(*it == 'v') *it = '\v';
        }
    }
}

String viewBackSlash(StringRef data)
{
    String res(data);
    for(auto it = res.begin(); it != res.end(); ++it) {
        if(*it == '\0') {
            it = res.erase(it);
            res.insert(it - res.begin(), "\\0");
            continue;
        }
        if(*it == '\a') {
            it = res.erase(it);
            res.insert(it - res.begin(), "\\a");
            continue;
        }
        if(*it == '\b') {
            it = res.erase(it);
            res.insert(it - res.begin(), "\\b");
            continue;
        }
#if !defined(OS_WINDOWS)
        if(*it == '\e') {
            it = res.erase(it);
            res.insert(it - res.begin(), "\\e");
            continue;
        }
#endif
        if(*it == '\f') {
            it = res.erase(it);
            res.insert(it - res.begin(), "\\f");
            continue;
        }
        if(*it == '\n') {
            it = res.erase(it);
            res.insert(it - res.begin(), "\\n");
            continue;
        }
        if(*it == '\r') {
            it = res.erase(it);
            res.insert(it - res.begin(), "\\r");
            continue;
        }
        if(*it == '\t') {
            it = res.erase(it);
            res.insert(it - res.begin(), "\\t");
            continue;
        }
        if(*it == '\v') {
            it = res.erase(it);
            res.insert(it - res.begin(), "\\v");
            continue;
        }
    }
    return res;
}

void output(OStream &os, fs::File *src, size_t locStart, size_t locEnd, StringRef data)
{
    if(src && locStart != -1) {
        size_t prevNewLine = getNewLineBefore(src->getData(), locStart);
        size_t nextNewLine = locEnd != -1 && locStart < locEnd
                             ? getNewLineAfter(src->getData(), locEnd)
                             : getNewLineAfter(src->getData(), locStart);

        String line(src->getData().begin() + (prevNewLine + 1),
                    src->getData().begin() + nextNewLine);
        size_t tabCount = stringCharCount(line, '\t');
        stringReplace(line, "\t", "    ");

        // + 1 for index -> line number
        size_t lineNumber  = countNewLinesTill(src->getData(), prevNewLine) + 1;
        size_t columnStart = locStart - (prevNewLine + 1) + (tabCount * 3);
        size_t columnEnd   = locEnd != -1 && locStart < locEnd
                             ? locEnd - (prevNewLine + 1) + (tabCount * 3)
                             : columnStart;

        size_t prefixSpaceCount = countDigits(lineNumber) + 3; // lineNum + " | "
        os << "In: " << src->getPath() << "\n";
        os << lineNumber << " | " << line << "\n";
        outputChar(os, ' ', prefixSpaceCount + columnStart);
        outputChar(os, '~', columnEnd - columnStart + 1);
        os << "\n";
        outputChar(os, ' ', prefixSpaceCount + columnStart);
        os << "^\n";
        if(!data.empty()) outputChar(os, ' ', prefixSpaceCount);
    }
    if(!data.empty()) os << data << "\n";
}

} // namespace core::utils
