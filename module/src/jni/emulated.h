#ifndef ZYGISKFRIDA_EMULATED_H
#define ZYGISKFRIDA_EMULATED_H

#include <string>
#include <optional>

struct emulation_module {
    void* data;
    size_t size;
};

std::optional<emulation_module> prepare_emulation();

bool reload_emulated(emulation_module module, std::string const& app_name);

#endif  // ZYGISKFRIDA_EMULATED_H
