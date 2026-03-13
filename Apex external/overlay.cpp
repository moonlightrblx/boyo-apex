#include "Overlay.hpp"

#include <dwmapi.h>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <d3d9.h>
#include <d3d11.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <io.h>
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

ID3D11Device* Overlay::device = nullptr;
ID3D11DeviceContext* Overlay::device_context = nullptr;
IDXGISwapChain* Overlay::swap_chain = nullptr;
ID3D11RenderTargetView* Overlay::render_targetview = nullptr;
HWND Overlay::overlay = nullptr;
WNDCLASSEX Overlay::wc = { };
HWND MyWnd = NULL;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK window_procedure(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;

    case WM_DESTROY:
        Overlay::DestroyDevice();
        Overlay::DestroyOverlay();
        Overlay::DestroyImGui();
        PostQuitMessage(0);
        return 0;

    case WM_CLOSE:
        Overlay::DestroyDevice();
        Overlay::DestroyOverlay();
        Overlay::DestroyImGui();
        return 0;
    }

    return DefWindowProc(window, msg, wParam, lParam);
}

bool Overlay::CreateDevice()
{

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));

    sd.BufferCount = 2;

    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;

    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    sd.BufferDesc.RefreshRate.Numerator = 134;
    sd.BufferDesc.RefreshRate.Denominator = 1;

    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    sd.OutputWindow = overlay;

    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;

    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

    HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0U,
        featureLevelArray,
        2,
        D3D11_SDK_VERSION,
        &sd,
        &swap_chain,
        &device,
        &featureLevel,
        &device_context);

    if (result == DXGI_ERROR_UNSUPPORTED) {
        result = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            0U,
            featureLevelArray,
            2, D3D11_SDK_VERSION,
            &sd,
            &swap_chain,
            &device,
            &featureLevel,
            &device_context);

        printf("[>>] DXGI_ERROR | Created with D3D_DRIVER_TYPE_WARP\n");
    }

    if (result != S_OK) {
        printf("[>>] Device Not Okay\n");
        return false;
    }

    ID3D11Texture2D* back_buffer{ nullptr };
    swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

    if (back_buffer)
    {
        device->CreateRenderTargetView(back_buffer, nullptr, &render_targetview);
        back_buffer->Release();

        return true;
    }

    printf("[>>] Failed to create Device\n");
    return false;
}

void Overlay::DestroyDevice()
{
    if (device) {
        device->Release();
        device_context->Release();
        swap_chain->Release();
        render_targetview->Release();
    }
    else
        printf("[>>] Device Not Found when Exiting.\n");
}

// Discord Chrome_WidgetWin_1



inline void SetClickThroughs(HWND hwnd, bool enable)
{
    LONG exStyle = GetWindowLongA(hwnd, GWL_EXSTYLE);

    if (enable)
        exStyle |= WS_EX_LAYERED | WS_EX_TRANSPARENT;
    else
        exStyle &= ~WS_EX_TRANSPARENT;

    SetWindowLongA(hwnd, GWL_EXSTYLE, exStyle);
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
}


// ("CEF-OSC-WIDGET", "NVIDIA GeForce Overlay");

void Overlay::CreateOverlay()
{

    overlay = FindWindowA("Chrome_WidgetWin_1", "Discord");

    if (!overlay)
        overlay = FindWindowA("Chrome_WidgetWin_1", nullptr);

    if (!overlay)
    {
        int result = MessageBoxA(nullptr,
            "Nvidia Overlay Not Found. Would you like to create a custom overlay instead?",
            "Overlay Not Found",
            MB_ICONQUESTION | MB_YESNO);

        if (result == IDNO)
            exit(1);
    }
    else
    {

        SetClickThroughs(overlay, true);

        SetWindowLongA(overlay, GWL_EXSTYLE,
            GetWindowLongA(overlay, GWL_EXSTYLE) |
            WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);

        MARGINS margins = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(overlay, &margins);

        SetWindowPos(overlay, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

        return;
    }


    wc.cbSize = sizeof(wc);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = window_procedure;
    wc.hInstance = GetModuleHandleA(0);
    wc.lpszClassName = "CustomOverlayWindow";

    RegisterClassEx(&wc);

    overlay = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        wc.lpszClassName,
        "Overlay",
        WS_POPUP,
        0,
        0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        NULL,
        NULL,
        wc.hInstance,
        NULL
    );

    if (!overlay)
    {
        printf("[>>] Failed to create fallback overlay window\n");
        return;
    }

    SetClickThroughs(overlay, true);

    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(overlay, &margins);

    ShowWindow(overlay, SW_SHOW);
    UpdateWindow(overlay);
}

void Overlay::CreateOverlaydc()
{

    overlay = FindWindowA("Chrome_WidgetWin_1", "Discord");

    if (!overlay)
        overlay = FindWindowA("Chrome_WidgetWin_1", nullptr);

    if (!overlay)
    {
        int result = MessageBoxA(nullptr,
            "Discord Overlay Not Found. Would you like to create a custom overlay instead?",
            "Overlay Not Found",
            MB_ICONQUESTION | MB_YESNO);

        if (result == IDNO)
            exit(1);
    }
    else
    {

        SetClickThroughs(overlay, true);

        SetWindowLongA(overlay, GWL_EXSTYLE,
            GetWindowLongA(overlay, GWL_EXSTYLE) |
            WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);

        MARGINS margins = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(overlay, &margins);

        SetWindowPos(overlay, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

        return;
    }


    wc.cbSize = sizeof(wc);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = window_procedure;
    wc.hInstance = GetModuleHandleA(0);
    wc.lpszClassName = "CustomOverlayWindow";

    RegisterClassEx(&wc);

    overlay = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        wc.lpszClassName,
        "Overlay",
        WS_POPUP,
        0,
        0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        NULL,
        NULL,
        wc.hInstance,
        NULL
    );

    if (!overlay)
    {
        printf("[>>] Failed to create fallback overlay window\n");
        return;
    }

    SetClickThroughs(overlay, true);

    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(overlay, &margins);

    ShowWindow(overlay, SW_SHOW);
    UpdateWindow(overlay);
}



void Overlay::DestroyOverlay()
{
    DestroyWindow(overlay);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
}

bool Overlay::CreateImGui()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(overlay)) {
        printf("Failed ImGui_ImplWin32_Init\n");
        return false;
    }

    if (!ImGui_ImplDX11_Init(device, device_context)) {
        printf("Failed ImGui_ImplDX11_Init\n");
        return false;
    }

    return true;
}

void Overlay::DestroyImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Overlay::StartRender()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();

    POINT p{};
    GetCursorPos(&p);
    io.MousePos = ImVec2((float)p.x, (float)p.y);

    io.MouseDown[0] = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
    io.MouseDown[1] = GetAsyncKeyState(VK_RBUTTON) & 0x8000;

    if (GetAsyncKeyState(VK_INSERT) & 1) {
        RenderMenu = !RenderMenu;

        if (RenderMenu) {
            SetWindowLong(overlay, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT);

        }
        else {
            SetWindowLong(overlay, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED);

        }
    }
}

void Overlay::EndRender()
{
    ImGui::Render();

    float color[4]{ 0, 0, 0, 0 };

    device_context->OMSetRenderTargets(1, &render_targetview, nullptr);
    device_context->ClearRenderTargetView(render_targetview, color);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    swap_chain->Present(0U, 0U);
}

void Overlay::SetForeground(HWND window)
{
    if (!IsWindowInForeground(window))
        BringToForeground(window);
}