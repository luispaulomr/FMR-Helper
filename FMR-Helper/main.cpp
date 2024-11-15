#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#include "CModGame.h"

// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include "myimage.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <string>

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static bool LoadTextureFromMemory(const void* data, size_t data_size, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
    // Load from disk into a raw RGBA buffer
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
    pTexture->Release();

    *out_width = image_width;
    *out_height = image_height;
    stbi_image_free(image_data);

    return true;
}

static bool is_equal(const std::vector<Card_t>& fusions_1, const std::vector<Card_t>& fusions_2)
{
    if (fusions_1.size() != fusions_2.size()) {
        return false;
    }

    for (auto i = 0; i < fusions_1.size(); ++i) {
        if (fusions_1[i].card != fusions_2[i].card) {
            return false;
        }

        if (fusions_1[i].cards != fusions_2[i].cards) {
            return false;
        }

        if (fusions_1[i].uid_cards != fusions_2[i].uid_cards) {
            return false;
        }
    }

    return true;
}

static std::vector<Card_t> GetGameFusions(CModGame * p_ModGame)
{

    //std::vector<Card_t> prev_fusions;

    //while (true) {

    if (p_ModGame->IsAttached()) {

        //std::cout << "[MAIN] Game is running" << "\n";

        if (p_ModGame->IsDuel()) {
            //std::cout << "[MAIN] Duel has started" << "\n";

            return p_ModGame->GetMyFusions();

            //if (!is_equal(prev_fusions, curr_fusions)) {
            //	std::cout << "[MAIN] Fusions:" << "\n";
            //	p_ModGame->PrintMyFusions(curr_fusions);
            //	prev_fusions = curr_fusions;
            //}
        }
    }
    else {
        std::cout << "[MAIN] Game is not running" << "\n";
        p_ModGame->RetryAttach();
    }
    //}
}

static std::vector<Card_t> TestFusions()
{
    std::vector<Card_t> ret;

    {
        Card_t card;
        card.cards.push_back(100);
        card.cards.push_back(101);
        card.uid_cards.push_back(0);
        card.uid_cards.push_back(1);
        ret.push_back(card);
    }

    {
        Card_t card;
        card.cards.push_back(100);
        card.cards.push_back(101);
        card.cards.push_back(102);
        card.uid_cards.push_back(0);
        card.uid_cards.push_back(1);
        card.uid_cards.push_back(2);
        ret.push_back(card);
    }

    {
        Card_t card;
        card.cards.push_back(100);
        card.cards.push_back(101);
        card.cards.push_back(102);
        card.cards.push_back(103);
        card.uid_cards.push_back(0);
        card.uid_cards.push_back(1);
        card.uid_cards.push_back(2);
        card.uid_cards.push_back(3);
        ret.push_back(card);
    }

    return ret;
}

static void ShowFusion(
    size_t image_width,
    size_t image_height,
    const Card_t& fusion,
    ID3D11ShaderResourceView* texture)
{
    float x = ImGui::GetCursorPosX();
    float y = ImGui::GetCursorPosY();

    float curr_x = x;

    ImGui::Text("Result");
    ImGui::SameLine();
    curr_x += image_width + 15.0;
    ImGui::SetCursorPosX(curr_x);
    size_t i = 1;
    for (const auto& card : fusion.cards) {
        ImGui::Text(std::to_string(i).c_str());
        ImGui::SameLine();
        curr_x += image_width + 5.0;
        ImGui::SetCursorPosX(curr_x);
        ++i;
    }

    curr_x = x;
    y += 15.0;
    ImGui::SetCursorPos(ImVec2(curr_x, y));

    ImGui::Image((ImTextureID)texture, ImVec2(image_width, image_height));
    ImGui::SameLine();
    curr_x += image_width + 15.0;
    ImGui::SetCursorPos(ImVec2(curr_x, y));

    for (const auto& card : fusion.cards) {
        ImGui::Image((ImTextureID)texture, ImVec2(image_width, image_height));
        ImGui::SameLine();
        curr_x += image_width + 5.0;
        ImGui::SetCursorPos(ImVec2(curr_x, y));
    }

    curr_x = x;
    y += image_height + 5.0;
    ImGui::SetCursorPos(ImVec2(curr_x, y));

    ImGui::Text("ATK: ");
    ImGui::Text("DEF: ");

    y += 40.0;
    ImGui::SetCursorPosY(y);
    ImGui::Separator();
}

struct ImGuiWindowData {
    bool ShowFusions = true;
};

static ImGuiWindowData window_data;

static void ShowFusionsWindow(
    bool* p_open,
    size_t image_width,
    size_t image_height,
    const std::vector<Card_t>& fusions,
    ID3D11ShaderResourceView* texture)
{
    size_t num_fusions = fusions.size();

    size_t max_cards = 0;
    for (const auto& fusion : fusions) {
        if (fusion.cards.size() > max_cards) {
            max_cards = fusion.cards.size();
        }
    }

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos;
    ImVec2 window_size;
    window_pos.x = work_pos.x;
    window_pos.y = work_pos.y;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
    //window_size = ImVec2(max_cards * (image_width + 10.0) + image_width + 15.0, num_fusions * (image_height + 75.0));
    window_size = ImVec2(500.0, 500.0);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

    if (ImGui::Begin("Fusions", p_open, window_flags))
    {
        for (const auto& fusion : fusions) {
            ShowFusion(image_width, image_height, fusion, texture);
        }
    }
    ImGui::End();
}

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"FMR-Helper", WS_OVERLAPPEDWINDOW, 0, 0, 500, 500, nullptr, nullptr, wc.hInstance, nullptr);
    //HWND hwnd = ::CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE, _T("Imgui Example"), NULL, WS_POPUP, 0, 0, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
    //HWND hwnd = ::CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE, _T("Imgui Example"), NULL, WS_POPUP, 0, 0, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
    //SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, ULW_COLORKEY);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ID3D11ShaderResourceView* myTexture = nullptr;
    LoadTextureFromMemory(image_data_4, sizeof(image_data_4), &myTexture, &image_width, &image_height);

    std::unique_ptr<CModGame> p_ModGame = std::unique_ptr<CModGame>(new CModGame(L"ePSXe - Enhanced PSX Emulator", L"ePSXe.exe"));

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        /* Our GUI */

        //ImGui::ShowDemoWindow();

        //auto fusions = TestFusions();
        static std::vector<Card_t> fusions;
        static size_t i_counter = 0;

        ++i_counter;

        if (!(i_counter % 10)) {
            fusions = GetGameFusions(p_ModGame.get());
        }
        ShowFusionsWindow(&window_data.ShowFusions, image_width, image_height, fusions, myTexture);

        //SetWindowsPos(hnwd, );

        //ImGui::SetCursorPos(ImGui::GetCursorStartPos());
        //auto x = ImGui::GetCursorPosX();
        //auto y = ImGui::GetCursorPosY();

        //auto curr_x = x;
        //auto curr_y = y;

        //ImGui::SetWindowSize(ImVec2(max_cards * (image_width + 5) + image_width + 15.0,
        //    num_fusions * (image_height + 65.0)));
        //{
        //    ImGui::Begin("Fusions", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

        //    for (const auto& fusion : fusions) {

        //        ImGui::Text("Result");
        //        curr_x += image_width + 15.0;
        //        ImGui::SetCursorPos(ImVec2(curr_x, curr_y));
        //        size_t i = 1;
        //        for (const auto& card : fusion.cards) {
        //            ImGui::Text(std::to_string(i).c_str());
        //            curr_x += image_width + 5.0;
        //            ImGui::SetCursorPos(ImVec2(curr_x, curr_y));
        //            ++i;
        //        }

        //        curr_y += 15.0;
        //        curr_x = x;

        //        ImGui::SetCursorPos(ImVec2(curr_x, curr_y));
        //        ImGui::Image((ImTextureID)myTexture, ImVec2(image_width, image_height));
        //        curr_x += image_width + 15.0;
        //        ImGui::SetCursorPos(ImVec2(curr_x, curr_y));

        //        for (const auto& card : fusion.cards) {
        //            ImGui::SetCursorPos(ImVec2(curr_x, curr_y));
        //            ImGui::Image((ImTextureID)myTexture, ImVec2(image_width, image_height));
        //            curr_x += image_width + 5.0;
        //            ImGui::SetCursorPos(ImVec2(curr_x, curr_y));
        //        }

        //        curr_y += image_height + 5.0;
        //        curr_x = x;

        //        ImGui::SetCursorPos(ImVec2(curr_x, curr_y));
        //        ImGui::Text("ATK: ");
        //        ImGui::Text("DEF: ");

        //        curr_y += 45.0;
        //        curr_x = x;

        //        ImGui::SetCursorPos(ImVec2(curr_x, curr_y));
        //    }
        //    ImGui::End();
        //}

        //auto xy = ImGui::GetCursorStartPos();
        //auto x = xy.x;
        //auto y = xy.y;

        //auto curr_x = x;
        //auto curr_y = y;

        //    for (const auto& fusion : fusions) {
        //        size_t i = 0;
        //        //ImGui::SetNextWindowSize(ImVec2(max_cards* (image_width + 5) + image_width + 15.0,
        //        //    num_fusions* (image_height + 65.0)));
        //        ImGui::SetNextWindowPos(ImVec2(curr_x, curr_y));
        //        ImGui::SetNextWindowSize(ImVec2(100.0, 100.0));
        //        {
        //            std::string str = "Fusion ";
        //            str += std::to_string(i);
        //            ++i;
        //            ImGui::Begin(str.c_str(), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

        //            //ImGui::SetCursorPos(ImGui::GetCursorStartPos());
        //            //ShowFusion(curr_x, curr_y, image_width, image_height, fusions[0], myTexture);
        //            curr_y += 50.0;
        //            curr_x = x;
        //            //ImGui::SetCursorPos(ImVec2(curr_x, curr_y));

        //            ImGui::End();
        //        }
        //    }

                //ImGui::SetNextWindowPos(ImVec2(0.0, 100.0));
                //ImGui::SetNextWindowSize(ImVec2(100.0, 100.0));
                //{
                //    ImGui::Begin("LUL", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
                //    auto curr_x = x;
                //    auto curr_y = y;
                //    //ImGui::SetCursorPos(ImGui::GetCursorStartPos());
                //    //ShowFusion(curr_x, curr_y, image_width, image_height, fusions[0], myTexture);
                //    curr_y += 50.0;
                //    curr_x = x;
                //    //ImGui::SetCursorPos(ImVec2(curr_x, curr_y));

                //    ImGui::End();
                //}

        //for (const auto& fusion : fusions) {
        //    ImGui::Begin("Fusion", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
        //    ShowFusion(curr_x, curr_y, image_width, image_height, fusions[0], myTexture);
        //    curr_y += 15.0;
        //    curr_x = x;
        //    ImGui::SetCursorPos(ImVec2(curr_x, curr_y));
        //    ImGui::End();
        //}

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        //const float clear_color_with_alpha[4] = {0.0f, 0.0f, 0.0f, 0.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
        //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
