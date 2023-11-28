#include <string>
#include <optional>
#include "emulated.h"
#include "hook.h"
#include "zygisk.h"


using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

class MyModule : public zygisk::ModuleBase {
 public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *raw_app_name = env->GetStringUTFChars(args->nice_name, nullptr);
        this->app_name = std::string(raw_app_name);
        this->env->ReleaseStringUTFChars(args->nice_name, raw_app_name);

#if defined(__i386__) || defined(__x86_64__)
        emulated_module = prepare_emulation();
#endif
    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        if (!should_hook(this->app_name)) {
            this->api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        run_hook_thread(app_name, emulated_module);
    }

 private:
    Api *api;
    JNIEnv *env;
    std::string app_name;
    std::optional <emulation_module> emulated_module = std::nullopt;
};

REGISTER_ZYGISK_MODULE(MyModule)
