#include "hook.h"

#include <dobby.h>

#include <chrono>
#include <codecvt>
#include <iomanip>
#include <locale>
#include <sstream>
#include <vector>

#include <string>
#include <thread>

#include "emulated.h"
#include "il2cpp/il2cpp-find.h"
#include "log.h"
#include "memory-utils.h"
#include "xdl.h"

std::string uint64_to_hex(uint64_t val) {
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(sizeof(uint64_t) * 2) << std::hex << val << std::endl;
    return ss.str();
}

void print_key(uint64_t key) {
    auto key_string = read_system_string(key);

    LOGI("Extracted Key: %s", key_string.c_str());

    auto parsed_key = std::strtoull(key_string.c_str(), nullptr, 10);

    LOGI("Extracted Key (hex): 0x%s", uint64_to_hex(parsed_key).c_str());
}

pid_t (*original_initialize_4)(
    uint64_t key,
    uint64_t authentication_file,
    uint64_t enableAtomDecryption,
    uint64_t enableManaDecryption
);

pid_t replacement_initialize_4(
    uint64_t key,
    uint64_t authentication_file,
    uint64_t enableAtomDecryption,
    uint64_t enableManaDecryption) {

    LOGI("Initialize (4 params) called");

    print_key(key);

    return original_initialize_4(key, authentication_file, enableAtomDecryption, enableManaDecryption);
}

pid_t (*original_initialize_3)(
    uint64_t key,
    uint64_t enableAtomDecryption,
    uint64_t enableManaDecryption
);

pid_t replacement_initialize_3(
    uint64_t key,
    uint64_t enableAtomDecryption,
    uint64_t enableManaDecryption) {

    LOGI("Initialize (3 params) called");

    print_key(key);

    return original_initialize_3(key, enableAtomDecryption, enableManaDecryption);
}

void install_hook(
    const std::vector<class_method> &methods,
    uint32_t parameter_count,
    void *replacement_func,
    void **orig_func) {

    for (auto &method : methods) {
        if (method.parameter_count != parameter_count) {
            continue;
        }

        DobbyHook(
            reinterpret_cast<void *>(method.absolute_addr),
            replacement_func,
            orig_func);


        LOGI("Installed hook for CriWareDecrypter.Initialize (%d parameters) RVA: 0x%" PRIx64 " VA: 0x%" PRIx64,
             parameter_count, method.relative_addr, method.absolute_addr);
        return;
    }
}

void *get_unity_handle() {
    int max_wait_for_unity_seconds = 10;

    for (int i = 0; i < max_wait_for_unity_seconds * 2; i++) {
        void *handle = xdl_open("libil2cpp.so", 0);
        if (handle) {
            return handle;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return nullptr;
}

void check_and_hook(std::string app_name) {
    void *unity_handle = get_unity_handle();
    if (unity_handle == nullptr) {
        return;
    }

    LOGI("Detected unity game: %s", app_name.c_str());

    auto unity_base = il2cpp_api_init(unity_handle);
    if (unity_base == 0) {
        return;
    }

    auto methods = il2cpp_find_class_method("CriWareDecrypter", "Initialize");

    install_hook(
        methods,
        4,
        reinterpret_cast<void *>(replacement_initialize_4),
        reinterpret_cast<void **>(&original_initialize_4));

    install_hook(
        methods,
        3,
        reinterpret_cast<void *>(replacement_initialize_3),
        reinterpret_cast<void **>(&original_initialize_3));
}

void run_hook_thread(std::string const &app_name, std::optional<emulation_module> emulated_module) {
    std::thread hook_thread(check_and_hook, app_name);
    hook_thread.detach();

#if defined(__i386__) || defined(__x86_64__)
    if (emulated_module.has_value()) {
        std::thread emulated_thread(reload_emulated, emulated_module.value(), app_name);
        emulated_thread.detach();
    }
#endif
}

bool should_hook(std::string const &app_name) {
    std::vector<std::string> black_list = {
        "com.google.",
        "com.android.",
        "android.",
        "webview_zygote",
        "com.ldmnq.",  // ldplayer
        "com.microvirt.",  // memu
        "WebViewLoader",  // memu
    };

    for (auto &do_not_hook : black_list) {
        if (app_name.rfind(do_not_hook, 0) != std::string::npos) {
            LOGI("App skipped: %s", app_name.c_str());
            return false;
        }
    }

    LOGI("App started: %s", app_name.c_str());

    return true;
}
