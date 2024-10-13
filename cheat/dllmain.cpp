// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include <Windows.h>
#include <iostream>
#include <vector>
#include "minhook/include/MinHook.h"
HMODULE ghModule;

uintptr_t FindPattern(uintptr_t pModulebaseAddress, const char* szSingnature, size_t nSelectResultIndex = NULL) {
    auto PatternToBytes = [](const char* szpattern) {
        auto m_iBytes = std::vector<int>{};
        const auto szStarAddr = const_cast<char*>(szpattern);
        const auto szEndAddr = const_cast<char*>(szpattern) + strlen(szpattern);

        for (auto szCurrentAddr = szStarAddr; szCurrentAddr < szEndAddr; ++szCurrentAddr) {
            if (*szCurrentAddr == '?') {
                ++szCurrentAddr;
                if (*szCurrentAddr == '?') ++szCurrentAddr;
                m_iBytes.push_back(-1);
            }
            else m_iBytes.push_back(strtoul(szCurrentAddr, &szCurrentAddr, 16));;
        }
        return m_iBytes;
        };

    const auto pDosHeader = (PIMAGE_DOS_HEADER)pModulebaseAddress;
    const auto pNTHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)pModulebaseAddress + pDosHeader->e_lfanew);
    const auto dwSizeOfImage = pNTHeaders->OptionalHeader.SizeOfImage;
    auto m_iPatternBytes = PatternToBytes(szSingnature);
    const auto pScanBytes = reinterpret_cast<std::uint8_t*>(pModulebaseAddress);
    const auto m_iPatternBytesSize = m_iPatternBytes.size();
    const auto m_iPatternBytesData = m_iPatternBytes.data();
    size_t nFoundResults = 0;

    for (auto i = 0ul; i < dwSizeOfImage - m_iPatternBytesSize; ++i) {
        bool bFound = true;

        for (auto j = 0ul; j < m_iPatternBytesSize; ++j) {
            if (pScanBytes[i + j] != m_iPatternBytesData[j] && m_iPatternBytesData[j] != -1) {
                bFound = false;
                break;
            }
        }
        if (bFound) {
            if (nSelectResultIndex != 0) {
                if (nFoundResults < nSelectResultIndex) {
                    nFoundResults++;
                    bFound = false;
                }
                else return reinterpret_cast<uintptr_t>(&pScanBytes[i]);
            }
            else return reinterpret_cast<uintptr_t>(&pScanBytes[i]);
        }
    }
    return NULL;
}

std::uint8_t* ResolveRalativeAddress(std::uint8_t* nAddressBytes, std::uint32_t nRVAOffset, std::uint32_t nRIPOffet)
{
    std::uint32_t nRVA = *reinterpret_cast<std::uint32_t*>(nAddressBytes + nRVAOffset);
    std::uint64_t nRIP = reinterpret_cast<std::uint64_t>(nAddressBytes) + nRIPOffet;

    return reinterpret_cast<std::uint8_t*>(nRVA + nRIP);
}

std::uint8_t* GetAdsoluteAddress(std::uint8_t* pRelativeAddress, int nPreOffset, int nPostOffset)
{
    pRelativeAddress += nPreOffset;
    pRelativeAddress += sizeof(std::int32_t) + *reinterpret_cast<std::int32_t*>(pRelativeAddress);
    pRelativeAddress += nPostOffset;
    return pRelativeAddress;
}


BOOL __stdcall Main() {
    AllocConsole();
    FILE* file{};
 
    freopen_s(&file, "CONOUT$", "w+", stdout);
    std::cout << "Hi!" << std::endl;

    uintptr_t cs2 = (uintptr_t)GetModuleHandleA("cs2.exe");
    uintptr_t client = (uintptr_t)GetModuleHandleA("clien.dll");
    
    //std::cout << (uintptr_t*)FindPattern(cs2, "48 89 6C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 41 83 E8") << std::endl;
    std::cout << (uintptr_t*)ResolveRalativeAddress((uint8_t*)FindPattern(client, "48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 83 EC ? 8B 0D"), 0x3, 0x7) << std::endl;

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

