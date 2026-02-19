#pragma once

#include "Core.hpp"

namespace core::env
{

String get(const char *key);
Path getHome();
Path getProcPath();

bool set(const char *key, const char *val, bool overwrite);

int exec(const char *cmd);

} // namespace core::env