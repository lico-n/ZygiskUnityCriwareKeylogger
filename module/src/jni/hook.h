#ifndef ZYGISKUNITYCRIWAREKEYEXTRACTOR_HOOK_H
#define ZYGISKUNITYCRIWAREKEYEXTRACTOR_HOOK_H

#include <string>

#include "emulated.h"

void run_hook_thread(std::string const& app_name, std::optional<emulation_module> module);

bool should_hook(std::string const& app_name);

#endif  // ZYGISKUNITYCRIWAREKEYEXTRACTOR_HOOK_H
