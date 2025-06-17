#include <iostream>
#include <vector>
#include <Windows.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_dx9.h"
#include "imgui/backends/imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>

struct Vector1 {
    double x;
    double y;
};

std::vector<Vector1> CoorAK = {
    {0.000000, -2.257792}, {0.323242, -2.300758}, {0.649593, -2.299759}, {0.848786, -2.259034},
    {1.075408, -2.323947}, {1.268491, -2.215956}, {1.330963, -2.236556}, {1.336833, -2.218203},
    {1.505516, -2.143454}, {1.504423, -2.233091}, {1.442116, -2.270194}, {1.478543, -2.204318},
    {1.392874, -2.165817}, {1.480824, -2.177887}, {1.597069, -2.270915}, {1.449996, -2.145893},
    {1.369179, -2.270450}, {1.582363, -2.298334}, {1.516872, -2.235066}, {1.498249, -2.238401},
    {1.465769, -2.331642}, {1.564812, -2.242621}, {1.517519, -2.303052}, {1.422433, -2.211946},
    {1.553195, -2.248043}, {1.510463, -2.285327}, {1.553878, -2.240047}, {1.520380, -2.221839},
    {1.553878, -2.240047}, {1.553195, -2.248043}
};

namespace WeaponBase {
    struct {
        const double WaitTime = 133.3333;
    } WTAK;
}

namespace ModData {
    struct {
        double Scope8X = 4.76;
        double ScopeHOLO = 1.2;
        double ScopeHAND = 0.8;
        double ScopeNONE = 1.0;
    } Scope;
    struct {
        double BarrelSILEN = 0.8;
        double BarrelNONE = 1.0;
    } Barrel;
}

bool WPAK = true; // Mặc định bật AKM
float Sensitivity = 1.0f; // Độ nhạy mặc định
int FOV = 90; // Góc nhìn mặc định
bool Mod8X = false, ModHOLO = false, ModHAND = false, ModSILEN = false;

void SleepTime(int wt) {
    LONGLONG atime;
    LONGLONG itime;
    LONGLONG mtime;
    QueryPerformanceFrequency((LARGE_INTEGER*)&atime);
    atime /= 1000;
    QueryPerformanceCounter((LARGE_INTEGER*)&mtime);
    itime = mtime / atime + wt;
    mtime = 0;
    while (mtime < itime) {
        QueryPerformanceCounter((LARGE_INTEGER*)&mtime);
        mtime /= atime;
    }
}

void SlowRec(double wt, double ct, int x1, int y1) {
    int x_ = 0, y_ = 0, t_ = 0;
    for (int i = 1; i <= (int)ct; ++i) {
        int xI = i * x1 / (int)ct;
        int yI = i * y1 / (int)ct;
        int tI = i * (int)ct / (int)ct;
        mouse_event(MOUSEEVENTF_MOVE, (int)xI - (int)x_, (int)yI - (int)y_, 0, 0);
        SleepTime((int)tI - (int)t_);
        x_ = xI; y_ = yI; t_ = tI;
    }
    SleepTime((int)wt - (int)ct);
}

int checkboost(float Delay) {
    return (int)Delay;
}

double BarrelData() {
    if (ModSILEN) {
        return ModData::Barrel.BarrelSILEN;
    }
    return ModData::Barrel.BarrelNONE;
}

double ScopeData() {
    if (Mod8X) return ModData::Scope.Scope8X;
    else if (ModHAND) return ModData::Scope.ScopeHAND;
    else if (ModHOLO) return ModData::Scope.ScopeHOLO;
    return ModData::Scope.ScopeNONE;
}

namespace NowWP {
    double x(int mermi) {
        if (GetAsyncKeyState(VK_CONTROL)) {
            return ((CoorAK[mermi].x * ScopeData()) * BarrelData() / (-0.03 * (Sensitivity * 2) * 3.0 * (FOV / 100.0)));
        }
        return ((CoorAK[mermi].x * ScopeData()) * BarrelData() / (-0.03 * Sensitivity * 3.0 * (FOV / 100.0)));
    }

    double y(int mermi) {
        if (GetAsyncKeyState(VK_CONTROL)) {
            return ((CoorAK[mermi].y * ScopeData()) * BarrelData() / (-0.03 * (Sensitivity * 2) * 3.0 * (FOV / 100.0)));
        }
        return ((CoorAK[mermi].y * ScopeData()) * BarrelData() / (-0.03 * Sensitivity * 3.0 * (FOV / 100.0)));
    }

    double Delaying() {
        return WeaponBase::WTAK.WaitTime;
    }

    int BulletCheck() {
        return CoorAK.size() - 1;
    }

    bool WPCheck() {
        return WPAK;
    }
};

static LPDIRECT3D9 g_pD3D = NULL;
static LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS g_d3dpp = {};

bool CreateDeviceD3D(HWND hWnd) {
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;
    return true;
}

void CleanupDeviceD3D() {
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

int main() {
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

    // Khởi tạo cửa sổ cho ImGui
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("AKM Recoil Control"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("AKM Recoil Control"), WS_OVERLAPPEDWINDOW, 100, 100, 420, 340, NULL, NULL, wc.hInstance, NULL);
    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    std::cout << "AKM Recoil Control started. Press Ctrl + Left Mouse + Right Mouse to activate. Press Esc to exit.\n";

    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Tạo giao diện ImGui
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Always);
        ImGui::Begin("AKM Recoil Settings");
        ImGui::Text("Adjust Recoil Parameters:");
        ImGui::SliderFloat("Sensitivity", &Sensitivity, 0.1f, 5.0f, "%.1f");
        ImGui::SliderInt("FOV", &FOV, 60, 120);
        ImGui::Checkbox("8X Scope", &Mod8X);
        ImGui::Checkbox("HOLO Scope", &ModHOLO);
        ImGui::Checkbox("HAND Scope", &ModHAND);
        ImGui::Checkbox("Silencer", &ModSILEN);
        ImGui::Text("Wait Time: %.2f ms", WeaponBase::WTAK.WaitTime);
        if (ImGui::Button("Close"))
            done = true;
        ImGui::End();

        // Xử lý recoil
        if (WPAK && (GetAsyncKeyState(VK_LBUTTON) & 0x8000) && (GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
            for (int i = 0; i <= NowWP::BulletCheck(); i++) {
                if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000) || !(GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
                    break;
                }
                SlowRec(checkboost((float)NowWP::Delaying()), checkboost(NowWP::Delaying()), (int)NowWP::x(i), (int)NowWP::y(i));
            }
        }

        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

        if (g_pd3dDevice->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}