#ifndef DO_API_NO_RETURN
#define DO_API_NO_RETURN(r, n, p) DO_API(r,n,p)
#endif

DO_API(const Il2CppImage*, il2cpp_get_corlib, ());

// assembly
DO_API(const Il2CppImage*, il2cpp_assembly_get_image, (const Il2CppAssembly * assembly));

// class
DO_API(Il2CppClass*, il2cpp_class_from_name, (const Il2CppImage * image, const char* namespaze, const char *name));
DO_API(Il2CppClass*, il2cpp_class_from_system_type, (Il2CppReflectionType * type));
DO_API(const MethodInfo*, il2cpp_class_get_methods, (const Il2CppClass * klass, void* *iter));
DO_API(const MethodInfo*, il2cpp_class_get_method_from_name, (Il2CppClass * klass, const char* name, int argsCount));
DO_API(const char*, il2cpp_class_get_name, (const Il2CppClass * klass));
DO_API(Il2CppClass*, il2cpp_class_from_type, (const Il2CppType * type));
DO_API(const Il2CppType*, il2cpp_class_get_type, (Il2CppClass * klass));

// domain
DO_API(Il2CppDomain*, il2cpp_domain_get, ());
DO_API(const Il2CppAssembly**, il2cpp_domain_get_assemblies, (const Il2CppDomain * domain, size_t * size));

// method
DO_API(const char*, il2cpp_method_get_name, (const MethodInfo * method));
DO_API(uint32_t, il2cpp_method_get_param_count, (const MethodInfo * method));


// string
DO_API(Il2CppString*, il2cpp_string_new, (const char* str));

// thread
DO_API(Il2CppThread*, il2cpp_thread_attach, (Il2CppDomain * domain));
DO_API(bool, il2cpp_is_vm_thread, (Il2CppThread * thread));

// image
DO_API(const char*, il2cpp_image_get_name, (const Il2CppImage * image));
DO_API(size_t, il2cpp_image_get_class_count, (const Il2CppImage * image));
DO_API(const Il2CppClass*, il2cpp_image_get_class, (const Il2CppImage * image, size_t index));




