#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <DxLibWrap.hpp>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    static bool alreadyLoaded = false;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (alreadyLoaded) return TRUE;
        alreadyLoaded = true;
#ifdef _DEBUG
        while (!IsDebuggerPresent())
            Sleep(100);
#endif
        DisableThreadLibraryCalls(hModule);
        DxLibWrap::Init(reinterpret_cast<uintptr_t>(hModule));
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        if (lpReserved != nullptr)
            break;
    case DLL_PROCESS_DETACH:
    default:
        break;
    }
    return TRUE;
}

