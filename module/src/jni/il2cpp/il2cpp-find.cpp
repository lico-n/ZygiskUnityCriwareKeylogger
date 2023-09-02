#include "il2cpp-find.h"

#include <cinttypes>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "log.h"
#include "il2cpp-class.h"
#include "xdl.h"

#define DO_API(r, n, p) r (*n) p

#include "il2cpp-api-functions.h"

#undef DO_API

static uint64_t il2cpp_base = 0;

void init_il2cpp_api(void *handle) {
#define DO_API(r, n, p) {                      \
    n = (r(*) p)xdl_sym(handle, #n, nullptr); \
    if (!n) {                                   \
        LOGW("api not found %s", #n);          \
    }                                          \
}

#include "il2cpp-api-functions.h"

#undef DO_API
}

uint64_t il2cpp_api_init(void *handle) {
    LOGI("il2cpp_handle: %p", handle);
    init_il2cpp_api(handle);
    if (il2cpp_domain_get_assemblies) {
        Dl_info dlInfo;
        if (dladdr((void *) il2cpp_domain_get_assemblies, &dlInfo)) {
            il2cpp_base = reinterpret_cast<uint64_t>(dlInfo.dli_fbase);
        }
        LOGI("il2cpp_base: 0x%" PRIx64, il2cpp_base);
    } else {
        LOGE("Failed to initialize il2cpp api.");
        return 0;
    }
    while (!il2cpp_is_vm_thread(nullptr)) {
        LOGI("Waiting for il2cpp_init...");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    auto domain = il2cpp_domain_get();
    il2cpp_thread_attach(domain);

    return il2cpp_base;
}

static std::vector<const Il2CppClass *> find_classes_via_api(std::string const &class_name) {
    size_t size;
    auto domain = il2cpp_domain_get();
    auto assemblies = il2cpp_domain_get_assemblies(domain, &size);

    std::vector<const Il2CppClass *> result;

    for (int i = 0; i < size; ++i) {
        auto image = il2cpp_assembly_get_image(assemblies[i]);

        auto classCount = il2cpp_image_get_class_count(image);
        for (int j = 0; j < classCount; ++j) {
            auto klass = il2cpp_image_get_class(image, j);
            auto name = std::string(il2cpp_class_get_name(klass));
            if (name == class_name) {
                result.push_back(klass);
            }
        }
    }

    return result;
}

static std::vector<const Il2CppClass *> find_classes_via_reflection(std::string const &class_name) {
    size_t size;
    auto domain = il2cpp_domain_get();
    auto assemblies = il2cpp_domain_get_assemblies(domain, &size);
    std::vector<const Il2CppClass *> result;

    auto corlib = il2cpp_get_corlib();
    auto assemblyClass = il2cpp_class_from_name(corlib, "System.Reflection", "Assembly");
    auto assemblyLoad = il2cpp_class_get_method_from_name(assemblyClass, "Load", 1);
    auto assemblyGetTypes = il2cpp_class_get_method_from_name(assemblyClass, "GetTypes", 0);
    if (assemblyLoad && assemblyLoad->methodPointer) {
        LOGI("Assembly::Load: %p", assemblyLoad->methodPointer);
    } else {
        LOGI("miss Assembly::Load");
        return result;
    }
    if (assemblyGetTypes && assemblyGetTypes->methodPointer) {
        LOGI("Assembly::GetTypes: %p", assemblyGetTypes->methodPointer);
    } else {
        LOGI("miss Assembly::GetTypes");
        return result;
    }
    typedef void *(*Assembly_Load_ftn)(void *, Il2CppString *, void *);
    typedef Il2CppArray *(*Assembly_GetTypes_ftn)(void *, void *);
    for (int i = 0; i < size; ++i) {
        auto image = il2cpp_assembly_get_image(assemblies[i]);
        auto image_name = std::string(il2cpp_image_get_name(image));

        auto pos = image_name.rfind('.');
        auto imageNameNoExt = image_name.substr(0, pos);
        auto assemblyFileName = il2cpp_string_new(imageNameNoExt.data());
        auto reflectionAssembly = ((Assembly_Load_ftn) assemblyLoad->methodPointer)(nullptr,
                                                                                    assemblyFileName,
                                                                                    nullptr);
        auto reflectionTypes = ((Assembly_GetTypes_ftn) assemblyGetTypes->methodPointer)(
            reflectionAssembly, nullptr);
        auto items = reflectionTypes->vector;
        for (int j = 0; j < reflectionTypes->max_length; ++j) {
            auto klass = il2cpp_class_from_system_type((Il2CppReflectionType *) items[j]);
            auto name = std::string(il2cpp_class_get_name(klass));
            if (name == class_name) {
                result.push_back(klass);
            }
        }
    }

    return result;
}

static std::vector<const Il2CppClass *> find_classes(std::string const &class_name) {
    if (il2cpp_image_get_class) {
        LOGI("Version greater than 2018.3");
        return find_classes_via_api(class_name);
    }

    LOGI("Version less than 2018.3");
    return find_classes_via_reflection(class_name);
}

static std::vector <class_method> find_methods(const Il2CppClass *klass, std::string const &method_name) {
    std::vector <class_method> result;

    void *iter = nullptr;

    while (auto method = il2cpp_class_get_methods(klass, &iter)) {
        auto name = std::string(il2cpp_method_get_name(method));
        if (name != method_name) {
            continue;
        }

        class_method found_method;
        found_method.absolute_addr = (uint64_t) method->methodPointer;
        found_method.relative_addr = found_method.absolute_addr - il2cpp_base;
        found_method.parameter_count = il2cpp_method_get_param_count(method);

        result.push_back(found_method);
    }

    return result;
}

std::vector <class_method> il2cpp_find_class_method(std::string const &class_name, std::string const &method_name) {
    std::vector <class_method> result;

    auto classes = find_classes(class_name);

    for (auto &klass : classes) {
        auto methods = find_methods(klass, method_name);
        result.insert(result.end(), methods.begin(), methods.end());
    }

    return result;
}
