#pragma once

#include "Core.hpp"

namespace core::env
{

String CORE_API get(const char *key);
Path CORE_API getHome();
Path CORE_API getProcPath();

bool CORE_API set(const char *key, const char *val, bool overwrite);

int CORE_API exec(const char *cmd);

} // namespace core::env