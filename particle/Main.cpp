// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include <chrono>

#include "ParticleEmitter.h"
#include "ParticleSOA.h"
#include "ParticleSystemCPU.h"
#include "ParticleUniforms.h"
#include "Texture2D.h"

#include <d3d11.h>
#include <tchar.h>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

constexpr size_t Capacity = 8196;
using Architecture = xsimd::avx2;
constexpr float Pi = 3.141592654f;

// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

namespace
{
	std::shared_ptr<DirectX::Texture2D> g_depthStencil = nullptr;

	std::shared_ptr<ParticleSOA<Capacity, Architecture>> g_ParticleSoa = nullptr;
	std::shared_ptr<ParticleUniforms>                    g_ParticleUniforms = nullptr;
	std::shared_ptr<ParticleEmitter>                     g_ParticleEmitter = nullptr;
	std::shared_ptr<ParticleSystem>                      g_ParticleSystem = nullptr;
	std::shared_ptr<BillboardRenderer::PassConstants>    g_PassConstant = nullptr;
	std::shared_ptr<FileSelection>                       g_FilePaths = nullptr;
	std::shared_ptr<BillboardRenderer>                   g_BbRenderer = nullptr;
}

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

void InitiateParticleSystem(ImGuiIO& io);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Example", NULL };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Particle Editor", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
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

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    InitiateParticleSystem(io);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // tick particle system
        {
        	static int cnt = 0;
            static double timeSum = 0.0;
            static double timeAvg = 0.0;

        	auto start = std::chrono::steady_clock::now();
        	{
	            g_ParticleSystem->TickLogic(io.DeltaTime);
	            //if (const auto cpuParticle =
		           // std::dynamic_pointer_cast<ParticleSystemCPU<Capacity, Architecture>>(g_ParticleSystem))
		           // cpuParticle->TickLogicScalar(io.DeltaTime);
        	}
            auto end = std::chrono::steady_clock::now();
            const std::chrono::duration<double> elapsed = end - start;
            timeSum += elapsed.count();

	        if (++cnt > 100)
	        {
                timeAvg = timeSum / static_cast<double>(cnt);
                timeSum = 0.0;
                cnt = 0;
	        }

        	ImGui::Begin("Particle System CPU");
            ImGui::Text("Architecture : %s  Particle Count : %zu Time per Iteration : %3.1f us",
				Architecture::name(), g_ParticleSoa->Count(), timeAvg * 1000000.0);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, g_depthStencil->GetDsv());
    	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        g_pd3dDeviceContext->ClearDepthStencilView(g_depthStencil->GetDsv(),
                                                   D3D11_CLEAR_DEPTH, 1.0f, 0);

        g_ParticleSystem->TickRender(g_pd3dDeviceContext);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
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
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);

	D3D11_TEXTURE2D_DESC backBufferDesc;
	pBackBuffer->GetDesc(&backBufferDesc);
    CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D32_FLOAT,
        backBufferDesc.Width, backBufferDesc.Height, 1, 0, D3D11_BIND_DEPTH_STENCIL,
        D3D11_USAGE_DEFAULT);
	g_depthStencil = std::make_shared<DirectX::Texture2D>(g_pd3dDevice,
        depthStencilDesc);
	g_depthStencil->CreateViews(g_pd3dDevice);
	
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

void InitiateParticleSystem(ImGuiIO& io)
{
    g_PassConstant = std::make_shared<BillboardRenderer::PassConstants>();
    const Vector3 eyePos(0.0f, 200.0f, -200.0f);
    const Matrix view = XMMatrixLookAtLH(eyePos, Vector3(0,0,0), Vector3(0,1,0));
	const Matrix proj = DirectX::XMMatrixPerspectiveFovLH
	(Pi * 0.25f, io.DisplaySize.x / io.DisplaySize.y, 0.01f, 1000.0f);
    g_PassConstant->EyePosition = eyePos;
    g_PassConstant->ViewProj = (view * proj).Transpose();

    g_FilePaths = std::make_shared<FileSelection>();
    g_FilePaths->emplace_back(L"./texture/OIP-C.jpg");

    g_BbRenderer = std::make_shared<BillboardRenderer>(g_pd3dDevice, Capacity, g_FilePaths, g_PassConstant);
    g_BbRenderer->Initialize();

	g_ParticleSoa = std::make_shared<ParticleSOA<Capacity, Architecture>>();

	float a[] = {0.0f, -9.8f, 0.0f };
	float c[] = 
	{
		0.3f, -0.5f, 0.75f, 0.124f,
		0.3f, -0.35f, 0.25f, 2.124f,
		0.3f, -0.45f, 0.5f, -0.124f,
		0.3f, -0.65f, 1.25f, -0.424f,
	};

	g_ParticleUniforms = std::make_shared<ParticleUniforms>(a, c);
	g_ParticleEmitter = std::make_shared<SimpleEmitter>(1.0f);
	g_ParticleSystem = std::make_shared<ParticleSystemCPU<Capacity, Architecture>>
		(g_ParticleSoa, g_ParticleEmitter, g_ParticleUniforms, g_BbRenderer);
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
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
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

