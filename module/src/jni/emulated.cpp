#include "emulated.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <jni.h>
#include <linux/unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/system_properties.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <cstdio>
#include <thread>
#include <optional>

#include "hook.h"
#include "log.h"

static std::string get_native_bridge() {
    auto value = std::array<char, PROP_VALUE_MAX>();
    __system_property_get("ro.dalvik.vm.native.bridge", value.data());
    return {value.data()};
}

struct NativeBridgeCallbacks {
    uint32_t version;
    void *initialize;

    void *(*loadLibrary)(const char *libpath, int flag);

    void *(*getTrampoline)(void *handle, const char *name, const char *shorty, uint32_t len);

    void *isSupported;
    void *getAppEnv;
    void *isCompatibleWith;
    void *getSignalHandler;
    void *unloadLibrary;
    void *getError;
    void *isPathSupported;
    void *initAnonymousNamespace;
    void *createNamespace;
    void *linkNamespaces;

    void *(*loadLibraryExt)(const char *libpath, int flag, void *ns);
};

bool reload_emulated(emulation_module module, std::string const& app_name) {
    sleep(5);

    auto libart = dlopen("libart.so", RTLD_NOW);
    auto JNI_GetCreatedJavaVMs = (jint (*)(JavaVM **, jsize, jsize *)) dlsym(libart,
                                                                             "JNI_GetCreatedJavaVMs");
    JavaVM *vms_buf[1];
    JavaVM *vms;
    jsize num_vms;
    jint status = JNI_GetCreatedJavaVMs(vms_buf, 1, &num_vms);
    if (status == JNI_OK && num_vms > 0) {
        vms = vms_buf[0];
    } else {
        LOGE("GetCreatedJavaVMs error");
        return false;
    }

    auto nb = dlopen("libhoudini.so", RTLD_NOW);
    if (!nb) {
        auto native_bridge = get_native_bridge();
        nb = dlopen(native_bridge.data(), RTLD_NOW);
    }
    if (!nb) {
        LOGW("unable to get native bridge for emulation");
        return false;
    }

    auto callbacks = (NativeBridgeCallbacks *) dlsym(nb, "NativeBridgeItf");
    if (callbacks) {
        int fd = syscall(__NR_memfd_create, "anon", MFD_CLOEXEC);
        ftruncate(fd, (off_t) module.size);
        void *mem = mmap(nullptr, module.size, PROT_WRITE, MAP_SHARED, fd, 0);
        memcpy(mem, module.data, module.size);
        munmap(mem, module.size);
        munmap(module.data, module.size);
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, "/proc/self/fd/%d", fd);

        void *arm_handle;
        if (android_get_device_api_level() >= 26) {
            arm_handle = callbacks->loadLibraryExt(path, RTLD_NOW, (void *) 3);
        } else {
            arm_handle = callbacks->loadLibrary(path, RTLD_NOW);
        }
        if (arm_handle) {
            auto init = (void (*)(JavaVM *, void *)) callbacks->getTrampoline(arm_handle,
                                                                              "JNI_OnLoad",
                                                                              nullptr, 0);

            init(vms, (void *) app_name.c_str());
            return true;
        }
        close(fd);
    }

    return false;
}

std::optional<emulation_module> prepare_emulation() {
    std::string path = "/data/adb/modules/zygiskunitycriwarekeylogger/emulated/armeabi-v7a.so";

#if defined(__x86_64__)
    path = "/data/adb/modules/zygiskunitycriwarekeylogger/emulated/arm64-v8a.so";
#endif

    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        LOGW("Unable open module lib file for possible emulation");
        return std::nullopt;
    }

    emulation_module module = {};

    struct stat sb{};
    fstat(fd, &sb);
    module.size = sb.st_size;
    module.data = mmap(nullptr, module.size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    return module;
}

#if defined(__arm__) || defined(__aarch64__)
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    auto app_name = (const char *) reserved;
    run_hook_thread(app_name, std::nullopt);
    return JNI_VERSION_1_6;
}
#endif
