#pragma once

/* Function hooking for now is only supported for win32. */
#ifdef WIN32

#include <stdint.h>

namespace FunctionHook {

/* RVA is an offset of an entity from the module's load point. */
typedef uint32_t rva_t;

/* Try to hook specified function in specified module.
 * Can fail if module is not loaded, or if passed RVA doesn't correspond to hookable function. */
bool hookImpl(const wchar_t *moduleName, rva_t functionRVA, void *hookFunc, void *&original);
bool hookImpl(const wchar_t *moduleName, const char *functionName, void *hookFunc, void *&original);

/* Type-safe hook wrapper.
 * @param function - function to hook, specified by RVA or by name (for exported functions). */
template<typename FuncIdentity, typename FuncPtr>
inline bool hook(const wchar_t *moduleName, FuncIdentity function, FuncPtr hookFunc, FuncPtr &original)
{
	void *ori = nullptr;
	bool success = hookImpl(moduleName, function, hookFunc, ori);
	original = static_cast<FuncPtr>(ori);
	return success;
}

}

#endif
