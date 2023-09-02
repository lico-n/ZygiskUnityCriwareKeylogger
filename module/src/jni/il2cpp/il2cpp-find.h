
#ifndef ZYGISK_IL2CPPDUMPER_IL2CPP_DUMP_H
#define ZYGISK_IL2CPPDUMPER_IL2CPP_DUMP_H

#include <cinttypes>
#include <string>
#include <vector>

struct class_method {
    uint64_t absolute_addr;
    uint64_t relative_addr;
    uint32_t parameter_count;
};

uint64_t il2cpp_api_init(void *handle);

std::vector<class_method> il2cpp_find_class_method(std::string const& class_name, std::string const& method_name);

#endif  // ZYGISK_IL2CPPDUMPER_IL2CPP_DUMP_H
