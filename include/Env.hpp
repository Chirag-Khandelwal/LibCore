#pragma once

#include "Core.hpp"

namespace core::env
{

String get(const char *key);
String getProcPath();
String getExeFromPath(const char *exe);

bool set(const char *key, const char *val, bool overwrite);

int exec(const char *cmd);

} // namespace core::env