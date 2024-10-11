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

float rad(float gradus) {
    return gradus * (180.f / 3.1415);
}

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

struct QAngle
{
    float x;
    float y;
    float z;
};

struct Vector3
{
    float x;
    float y;
    float z;
    constexpr Vector3& operator-()
    {
        this->x = -this->x;
        this->y = -this->y;
        this->z = -this->z;
        return *this;
    }
    constexpr Vector3 operator-() const
    {
        return { -this->x, -this->y, -this->z };
    }
    constexpr Vector3 operator-(const Vector3& angSubtract) const
    {
        return { this->x - angSubtract.x , this->y - angSubtract.y, this->z - angSubtract.z };
    }
};

BOOL __stdcall Main() {
    AllocConsole();
    FILE* file{};
 
    freopen_s(&file, "CONOUT$", "w+", stdout);
    std::cout << "Hi!" << std::endl;

    uintptr_t client = (uintptr_t)GetModuleHandleA("client.dll");
  
    func = (get_base_entity)(client + 0x629340);
    GetPawn = (GetPawnByController)(client + 0x57E150);
    uintptr_t localcontroller = *(uintptr_t*)(client + 0x1A0D8E8);
   
    QAngle* dwViewAngles = (QAngle*)(client + 0x1A29DA0);
    
    

    if (MH_Initialize() == MH_OK) {
        std::cout << "MH_OK" << std::endl;
        if (MH_CreateHook(reinterpret_cast<void**>(func), &get_base_entity_hook, reinterpret_cast<void**>(&ofunc)) == MH_OK) { std::cout << "HOOK!" << std::endl; }
    }
    MH_EnableHook(0);

    while (entitymain == 0) {
        std::cout << "Sleep!" << std::endl;
    }

  
    while (not GetAsyncKeyState(VK_END)) {
        Vector3 localpos = *(Vector3*)(GetPawn(localcontroller) + 0x137C);
        for (int i = 0; i <= 64; i++) {
            auto entcontroller = GetController(i);
            if (entcontroller and localcontroller != entcontroller) {
                if (GetPawn(entcontroller)) {
                    std::cout << GetPawn(entcontroller) << std::endl;
                    Vector3 pos = *(Vector3*)(GetPawn(entcontroller) + 0x137C);
                    Vector3 dif = pos - localpos;
                    float distanceSquared = dif.x * dif.x + dif.y * dif.y;
                    float invDistance = 1.0f / sqrt(distanceSquared);

                    //std::cout << " x = " << angl.x << " y = " << angl.y << " z = " << angl.z << std::endl;
                    dwViewAngles->x = rad(atan2(-dif.z, distanceSquared * invDistance));
                    dwViewAngles->y = rad(atan2(dif.y, dif.x));
                }

            }
        }
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

