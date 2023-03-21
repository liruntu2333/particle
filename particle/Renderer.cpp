#include "Renderer.h"
#include <directxtk/CommonStates.h>

using namespace DirectX;
static std::unique_ptr<CommonStates> g_States = nullptr;

constexpr int FieldBufferCount = 7;

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
	for (int i = 0; i < FieldBufferCount; ++i)
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
	unsigned int stride = 0;
	unsigned int offset = 0;
	context->IASetInputLayout(m_InputLayout.Get());
	context->IASetVertexBuffers(0, 0, nullptr, &stride, &offset);
	context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(m_Vs.Get(), nullptr, 0);
	auto cb0 = m_ConstantBuffer.GetBuffer();
	context->VSSetConstantBuffers(0, 1, &cb0);
	std::vector<ID3D11ShaderResourceView*> srvs;
	for (const auto & field : m_FieldBuffers)
		srvs.emplace_back(field.GetSrv());
	context->VSSetShaderResources(0, srvs.size(), srvs.data());

	context->PSSetShader(m_Ps.Get(), nullptr, 0);
	auto linearWrap = g_States->LinearWrap();
	context->PSSetSamplers(0, 1, &linearWrap);
	srvs.clear();
	for (const auto & [path, resource] : m_TextureArray)
		srvs.emplace_back(resource.GetSrv());
	context->PSSetShaderResources(0, srvs.size(), srvs.data());

	auto alphaBlend = g_States->AlphaBlend();
	context->OMSetBlendState(alphaBlend, nullptr, 0xffffffff);
	auto depthTest = g_States->DepthDefault();
	context->OMSetDepthStencilState(depthTest, 0);

	// TODO
	//context->DrawIndexed();
}
