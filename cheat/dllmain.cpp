// dllmain.cpp : Определяет точку входа для приложения DLL.
#include <windows.h>
#include <iostream>
#include <vector>
#include "minhook/include/MinHook.h"
#include <d3d11.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
HMODULE ghModule;

uintptr_t FindPattern(uintptr_t pModuleBaseAddress, const char* szSignature, size_t nSelectResultIndex = NULL) {
    auto PatternToBytes = [](const char* szpattern) {
        auto       m_iBytes = std::vector<int>{};
        const auto szStartAddr = const_cast<char*>(szpattern);
        const auto szEndAddr = const_cast<char*>(szpattern) + strlen(szpattern);

        for (auto szCurrentAddr = szStartAddr; szCurrentAddr < szEndAddr; ++szCurrentAddr) {
            if (*szCurrentAddr == '?') {
                ++szCurrentAddr;
                if (*szCurrentAddr == '?') ++szCurrentAddr;
                m_iBytes.push_back(-1);
            }
            else m_iBytes.push_back(strtoul(szCurrentAddr, &szCurrentAddr, 16));
        }
        return m_iBytes;
        };

    const auto pDosHeader = (PIMAGE_DOS_HEADER)pModuleBaseAddress;
    const auto pNTHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)pModuleBaseAddress + pDosHeader->e_lfanew);
    const auto dwSizeOfImage = pNTHeaders->OptionalHeader.SizeOfImage;
    auto       m_iPatternBytes = PatternToBytes(szSignature);
    const auto pScanBytes = reinterpret_cast<std::uint8_t*>(pModuleBaseAddress);
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
std::uint8_t* ResolveRelativeAddress(std::uint8_t* nAddressBytes, std::uint32_t nRVAOffset, std::uint32_t nRIPOffset)
{
    std::uint32_t nRVA = *reinterpret_cast<std::uint32_t*>(nAddressBytes + nRVAOffset);
    std::uint64_t nRIP = reinterpret_cast<std::uint64_t>(nAddressBytes) + nRIPOffset;

    return reinterpret_cast<std::uint8_t*>(nRVA + nRIP);
}
std::uint8_t* GetAbsoluteAddress(std::uint8_t* pRelativeAddress, int nPreOffset, int nPostOffset)
{
    pRelativeAddress += nPreOffset;
    pRelativeAddress += sizeof(std::int32_t) + *reinterpret_cast<std::int32_t*>(pRelativeAddress);
    pRelativeAddress += nPostOffset;
    return pRelativeAddress;
}



typedef HRESULT(*Present)(IDXGISwapChain*, UINT, UINT);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

inline HWND hWnd = NULL;
inline ID3D11Device* pDevice = NULL;
inline ID3D11DeviceContext* pContext = NULL;
inline ID3D11RenderTargetView* pRenderTargetView;
inline WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
Present orighkPresent;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
    {
        return true;
    }
    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}
struct  QAngle_t
{
    float x;
    float y;
    float z;
};
struct view_matrix_t {
    float* operator[](int index) {
        return matrix[index];
    }
    float matrix[4][4];
};
view_matrix_t* VM;

UINT vps = 1;
ImVec2 screen_size = { 0, 0 };
ImVec2 screen_center = { 0, 0 };
D3D11_VIEWPORT viewport;
ImVec2 WorldToScreen2(const QAngle_t& pos) {
    view_matrix_t matrix = *VM;
    float _x = matrix[0][0] * pos.x + matrix[0][1] * pos.y + matrix[0][2] * pos.z + matrix[0][3];
    float _y = matrix[1][0] * pos.x + matrix[1][1] * pos.y + matrix[1][2] * pos.z + matrix[1][3];
    float w = matrix[3][0] * pos.x + matrix[3][1] * pos.y + matrix[3][2] * pos.z + matrix[3][3];

    if (w < 0.01) {
        return ImVec2(-1, -1);
    }

    float inv_w = 1.0f / w;
    _x *= inv_w;
    _y *= inv_w;


    float x = screen_size.x * 0.5f + 0.5f * _x * screen_size.x;
    float y = screen_size.y * 0.5f - 0.5f * _y * screen_size.y;

    return ImVec2(x, y);
}
bool ImGuiInit = false;
bool menu = false;
HRESULT hkPresent(IDXGISwapChain* pSwapchain, UINT SyncInterval, UINT Flags) {
    if (!ImGuiInit) {
        if (SUCCEEDED(pSwapchain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice))) {
            pDevice->GetImmediateContext(&pContext);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapchain->GetDesc(&sd);
            hWnd = sd.OutputWindow;
            ID3D11Texture2D* pBackBuffer;
            pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
            pBackBuffer->Release();
            oWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
            ImGui_ImplWin32_Init(hWnd);
            ImGui_ImplDX11_Init(pDevice, pContext);
            ImGuiInit = true;
        }
        else return orighkPresent(pSwapchain, SyncInterval, Flags);
    }

    pContext->RSGetViewports(&vps, &viewport);
    screen_size = { viewport.Width, viewport.Height };
    screen_center = { viewport.Width / 2.0f, viewport.Height / 2.0f };

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImDrawList* DrawList = ImGui::GetBackgroundDrawList();
    DrawList->AddCircle(ImVec2(100, 100), 20, ImColor(255, 0, 0, 255));

    if (WorldToScreen2(QAngle_t(0, 0, 0)).x != -1) {
        DrawList->AddLine(ImVec2(0, 0), WorldToScreen2(QAngle_t(0, 0, 0)), ImColor(255, 255, 255, 255), 1);
    }


    {
        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            menu = !menu;
        }
        if (menu) {
            ImGui::Begin("AAA", &menu, ImGuiWindowFlags_NoTitleBar);
            if (ImGui::Button("+")) {

            }
        }
    }
    ImGui::Render();
    pContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return orighkPresent(pSwapchain, SyncInterval, Flags);
}
BOOL __stdcall Main() {
    AllocConsole();
    FILE* file{};
    freopen_s(&file, "CONOUT$", "w+", stdout);
    std::cout << "Hi!" << std::endl;
    VM = reinterpret_cast<view_matrix_t*>(ResolveRelativeAddress((uint8_t*)FindPattern((uintptr_t)GetModuleHandleA("client.dll"), "48 8D 0D ? ? ? ? 48 C1 E0 06"), 0x3, 0x7));

    uintptr_t cs2 = (uintptr_t)GetModuleHandleA("cs2.exe");
    uintptr_t GameOverlayRenderer64 = (uintptr_t)GetModuleHandleA("GameOverlayRenderer64.dll");
    uintptr_t client = (uintptr_t)GetModuleHandleA("client.dll");


    if (MH_Initialize() == MH_OK) {
        MH_CreateHook(reinterpret_cast<void**>(GameOverlayRenderer64 + 0x8E4C0), &hkPresent, reinterpret_cast<void**>(&orighkPresent));
    }
    MH_EnableHook(0);
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