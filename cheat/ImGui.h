#pragma once
#include"ImGui/imgui.h"
#include <dxgi.h>
#include "ImGui/imgui_impl_dx11.h"
#include <d3d11.h>
#include "ImGui/imgui_impl_win32.h"

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

bool ImGuiInit = false;
bool menu = false;

UINT vps = 1;
ImVec2 screen_size = { 0, 0 };
ImVec2 screen_center = { 0, 0 };
D3D11_VIEWPORT viewport;

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

    


    {
        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            menu = !menu;
        }
        if (menu) {
            ImGui::Begin("AAA", &menu, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
            if (ImGui::Button("+")) {
            
            }
        }
    }
    ImGui::Render();
    pContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return orighkPresent(pSwapchain, SyncInterval, Flags);
}