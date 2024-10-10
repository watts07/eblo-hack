// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include "minhook/include/MinHook.h"
HMODULE ghModule;

typedef uintptr_t(__fastcall* get_base_entity)(uintptr_t a1, int a2);
get_base_entity func;
get_base_entity ofunc;

uintptr_t entitymain;

typedef uintptr_t(__fastcall* GetPawnByController)(uintptr_t Controller);
GetPawnByController GetPawn;


uintptr_t GetController(int id) {
    return (uintptr_t)ofunc(entitymain, id);
}
uintptr_t get_base_entity_hook(uintptr_t a1, int a2)
{
    entitymain = a1;

    
    
    __int64 v2; // rcx
    signed __int64 v3; // rcx
    __int64 result; // rax

    if ((unsigned int)a2 <= 0x7FFE
        && (unsigned int)(a2 >> 9) <= 0x3F
        && (v2 = *(uintptr_t*)(a1 + 8i64 * (a2 >> 9) + 16)) != 0
        && (v3 = 120i64 * (a2 & 0x1FF) + v2) != 0
        && (*(uintptr_t*)(v3 + 16) & 0x7FFF) == a2)
    {
        result = *(uintptr_t*)v3;
    }
    else
    {
        result = 0i64;
    }
        
    

    return result;

 }


BOOL __stdcall Main() {
    AllocConsole();
    FILE* file{};
 
    freopen_s(&file, "CONOUT$", "w+", stdout);
    std::cout << "Hi!" << std::endl;

    uintptr_t client = (uintptr_t)GetModuleHandleA("client.dll");
  
    func = (get_base_entity)(client + 0x629350);
    GetPawn = (GetPawnByController)(client + 0x57E160);
   

    if (MH_Initialize() == MH_OK) {
        std::cout << "MH_OK" << std::endl;
        if (MH_CreateHook(reinterpret_cast<void**>(func), &get_base_entity_hook, reinterpret_cast<void**>(&ofunc)) == MH_OK) { std::cout << "HOOK!" << std::endl; }
    }
    MH_EnableHook(0);

    while (entitymain == 0) {
        std::cout << "Sleep!" << std::endl;
    }

    for (int i = 0; i <= 64; i++) {
        auto entcontroller = GetController(i);
        if (entcontroller) {
            std::cout << (void*)entcontroller << std::endl;
        }
    }
    while (not GetAsyncKeyState(VK_END)) {

    }
    MH_DisableHook(0);
    fclose(file);
    FreeConsole();
    FreeLibraryAndExitThread(ghModule, 0);
    return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        ghModule = hModule;
        Beep(100, 100);
        //Main();
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Main, 0, 3, 0);
    }
    return TRUE;
}

