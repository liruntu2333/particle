#include "Renderer.h"
#include <directxtk/CommonStates.h>

using namespace DirectX;
static std::unique_ptr<CommonStates> g_States = nullptr;

constexpr int StructuredBufferCount = 7;

BillboardRenderer::BillboardRenderer(ID3D11Device* device, UINT capacity,
		const std::shared_ptr<FileSelection>& paths,
		const std::shared_ptr<StructWithFlag<PassConstants>>& constants) :
	m_TexturePaths(paths), m_Constants(constants), m_Device(device), m_Capacity(capacity)
{
	if (g_States == nullptr)
		g_States = std::make_unique<CommonStates>(device);

	paths->AddObserver();
	constants->AddObserver();
}

BillboardRenderer::~BillboardRenderer() = default;

void BillboardRenderer::Initialize()
{
	// cb0
	m_ConstantBuffer.Create(m_Device);
	m_Constants->SetDirty();

	// t0~6, space 0
	for (int i = 0; i < StructuredBufferCount; ++i)
		m_FieldBuffers.emplace_back(m_Device, m_Capacity);

	// t0 ~ , space 1
	if (m_TexturePaths == nullptr) return;
	for (const auto & path : m_TexturePaths->Object)
	{
		m_TextureArray.emplace_back(path, Texture2D{m_Device, (path.c_str())});
	}

	// inputLayout & vs
	const auto vs = LoadBinary(L"./shader/CpuParticleVS.cso");
	auto hr = m_Device->CreateVertexShader(vs->GetBufferPointer(), 
		vs->GetBufferSize(), nullptr, &m_Vs);
	ThrowIfFailed(hr);

	hr = m_Device->CreateInputLayout(nullptr, 0, 
		vs->GetBufferPointer(), vs->GetBufferSize(), m_InputLayout.GetAddressOf());
	ThrowIfFailed(hr);

	const auto ps = LoadBinary(L"./shader/CpuParticlePS.cso");
	hr = m_Device->CreatePixelShader(ps->GetBufferPointer(), 
		ps->GetBufferSize(), nullptr, &m_Ps);
	ThrowIfFailed(hr);

	// index buffer
	assert(m_Capacity * 6 < UINT16_MAX);
	std::vector<UINT16> vertices(m_Capacity * 6);
	for (int i = 0; i < m_Capacity; ++i)
	{
		vertices[i * 6 + 0] = 4 * i + 0;
		vertices[i * 6 + 1] = 4 * i + 1;
		vertices[i * 6 + 2] = 4 * i + 2;
		vertices[i * 6 + 3] = 4 * i + 2;
		vertices[i * 6 + 4] = 4 * i + 1;
		vertices[i * 6 + 5] = 4 * i + 3;
	}
	hr = CreateStaticBuffer(m_Device, vertices.data(), vertices.size(), sizeof(UINT16), 
		D3D11_BIND_INDEX_BUFFER, &m_IndexBuffer);
	ThrowIfFailed(hr);
}

void BillboardRenderer::Render(ID3D11DeviceContext* context)
{

}
