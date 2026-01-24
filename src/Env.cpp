#include "Env.hpp"

#include <cstdlib>

#include "FS.hpp"
#include "Utils.hpp"

#if defined(CORE_OS_WINDOWS)
#include <Windows.h>
#elif defined(CORE_OS_APPLE)
#include <mach-o/dyld.h> // for _NSGetExecutablePath()
#elif defined(CORE_OS_FREEBSD)
#include <sys/sysctl.h> // for sysctl()
#include <sys/types.h>
#else
#include <unistd.h> // for readlink()
#endif

namespace core::env
{

bool set(const char *key, const char *val, bool overwrite)
{
#if defined(CORE_OS_WINDOWS)
    if(!overwrite) {
        size_t envsize = 0;
        int errcode    = getenv_s(&envsize, NULL, 0, key);
        if(!errcode && envsize) return false;
    }
    return _putenv_s(key, val) == 0;
#else
    return setenv(key, val, overwrite) == 0;
#endif
}

String get(const char *key)
{
#if defined(CORE_OS_WINDOWS)
    static char envdata[MAX_ENV_CHARS];
    size_t envsize = 0;
    int errcode    = getenv_s(&envsize, envdata, MAX_ENV_CHARS, key);
    return errcode || !envsize ? "" : envdata;
#else
    const char *env = getenv(key);
    return env ? env : "";
#endif
}

String getProcPath()
{
    char path[MAX_PATH_CHARS];
    memset(path, 0, MAX_PATH_CHARS);
#if defined(CORE_OS_WINDOWS)
    // TODO: Make this return normalized path instead of backslash on Windows
    GetModuleFileNameA(NULL, path, MAX_PATH_CHARS);
#elif defined(CORE_OS_LINUX) || defined(CORE_OS_ANDROID)
    ssize_t count = readlink("/proc/self/exe", path, MAX_PATH_CHARS);
    if(count < 0) return "";
    path[count] = '\0';
#elif defined(CORE_OS_FREEBSD)
    int mib[4];
    mib[0]    = CTL_KERN;
    mib[1]    = KERN_PROC;
    mib[2]    = KERN_PROC_PATHNAME;
    mib[3]    = -1;
    size_t sz = MAX_PATH_CHARS;
    sysctl(mib, 4, path, &sz, NULL, 0);
#elif defined(CORE_OS_NETBSD)
    readlink("/proc/curproc/exe", path, MAX_PATH_CHARS);
#elif defined(CORE_OS_OPENBSD) || defined(CORE_OS_BSDI) || defined(CORE_OS_DRAGONFLYBSD)
    readlink("/proc/curproc/file", path, MAX_PATH_CHARS);
#elif defined(CORE_OS_APPLE)
    uint32_t sz = MAX_PATH_CHARS;
    _NSGetExecutablePath(path, &sz);
#endif
    return fs::normPath(path);
}

String getExeFromPath(const char *exe)
{
    String path = get("PATH");
    if(path.empty()) return path;

#if defined(CORE_OS_WINDOWS)
    Vector<StringRef> paths = utils::stringDelim(path, ";");
#else
    Vector<StringRef> paths = utils::stringDelim(path, ":");
#endif

    String pathstr;
    for(auto &p : paths) {
        pathstr = p;
        pathstr += "/";
        pathstr += exe;
        if(fs::exists(pathstr)) return fs::normPath(pathstr);
    }
    return "";
}

int exec(const char *cmd)
{
    int res = std::system(cmd);
#if !defined(CORE_OS_WINDOWS)
    res = WEXITSTATUS(res);
#endif
    return res;
}

} // namespace core::env