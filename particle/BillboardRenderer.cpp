#include "BillboardRenderer.h"
#include <directxtk/CommonStates.h>
#include <directxtk/GeometricPrimitive.h>

using namespace DirectX;
static std::unique_ptr<CommonStates> g_States = nullptr;

// debug plane
namespace 
{
	ComPtr<ID3D11VertexShader> g_DummyVS = nullptr;
	ComPtr<ID3D11PixelShader> g_DummyPS = nullptr;
}

BillboardRenderer::BillboardRenderer(ID3D11Device* device, UINT capacity,
		const std::shared_ptr<FileSelection>& paths,
		const std::shared_ptr<PassConstants>& constants) :
	m_TexturePaths(paths), m_Constants(constants), m_Device(device), m_Capacity(capacity)
{
	if (g_States == nullptr)
		g_States = std::make_unique<CommonStates>(device);

	if (g_DummyVS == nullptr)
	{
		const auto vs = LoadBinary(L"./shader/DummyVS.cso");
		auto hr = m_Device->CreateVertexShader(vs->GetBufferPointer(),
		                                       vs->GetBufferSize(), nullptr, &g_DummyVS);
		ThrowIfFailed(hr);
	}
	if (g_DummyPS == nullptr)
	{
		const auto ps = LoadBinary(L"./shader/DummyPS.cso");
		auto hr = m_Device->CreatePixelShader(ps->GetBufferPointer(),
		                                 ps->GetBufferSize(), nullptr, &g_DummyPS);
		ThrowIfFailed(hr);
	}

	//paths->AddObserver();
	//constants->AddObserver();
}

BillboardRenderer::~BillboardRenderer() = default;

void BillboardRenderer::Initialize()
{
	// cb0
	m_ConstantBuffer.Create(m_Device);
	//m_Constants->SetDirty();

	// t0~6, space 0
	for (int i = 0; i < FieldBufferCount; ++i)
		m_FieldBuffers.emplace_back(m_Device, m_Capacity);

	// t0 ~ , space 1
	//if (m_TexturePaths == nullptr) return;
	//for (const auto & path : m_TexturePaths)
	//{
	//	m_TextureList.emplace_back(path, Texture2D{m_Device, (path.c_str())});
	//}
	// create at first frame

	// inputLayout & vs
	const auto vs = LoadBinary(L"./shader/CpuParticleVS.cso");
	auto hr = m_Device->CreateVertexShader(vs->GetBufferPointer(), 
		vs->GetBufferSize(), nullptr, &m_Vs);
	ThrowIfFailed(hr);

	//hr = m_Device->CreateInputLayout(nullptr, 0, 
	//	vs->GetBufferPointer(), vs->GetBufferSize(), m_InputLayout.GetAddressOf());
	//ThrowIfFailed(hr);

	const auto ps = LoadBinary(L"./shader/CpuParticlePS.cso");
	hr = m_Device->CreatePixelShader(ps->GetBufferPointer(), 
		ps->GetBufferSize(), nullptr, &m_Ps);
	ThrowIfFailed(hr);

	// index buffer
	assert(m_Capacity * 6 < UINT16_MAX);
	std::vector<UINT16> vertices(m_Capacity * 6);
	for (UINT i = 0; i < m_Capacity; ++i)
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

void BillboardRenderer::Render(ID3D11DeviceContext* context, UINT count)
{
	context->IASetInputLayout(nullptr);
	context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);

	const auto cb0 = m_ConstantBuffer.GetBuffer();
	context->VSSetConstantBuffers(0, 1, &cb0);
	
	//{
	//	const auto opaque = g_States->Opaque();
	//	context->OMSetBlendState(opaque, nullptr, 0xffffffff);
	//	const auto depthWrite = g_States->DepthDefault();
	//	context->OMSetDepthStencilState(depthWrite, 0);
	//	context->RSSetState(g_States->CullCounterClockwise());
	//	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
	//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//	context->VSSetShader(g_DummyVS.Get(), nullptr, 0);
	//	context->PSSetShader(g_DummyPS.Get(), nullptr, 0);
	//	context->Draw(6, 0);
	//}

	const auto alphaBlend = g_States->AlphaBlend();
	context->OMSetBlendState(alphaBlend, nullptr, 0xffffffff);
	const auto depthRead = g_States->DepthRead();
	context->OMSetDepthStencilState(depthRead, 0);
	context->RSSetState(g_States->CullClockwise());
	
	context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(m_Vs.Get(), nullptr, 0);

	std::vector<ID3D11ShaderResourceView*> srvs;
	for (const auto & field : m_FieldBuffers)
		srvs.emplace_back(field.GetSrv());
	context->VSSetShaderResources(0, srvs.size(), srvs.data());

	context->PSSetShader(m_Ps.Get(), nullptr, 0);
	const auto linearWrap = g_States->LinearWrap();
	context->PSSetSamplers(0, 1, &linearWrap);
	srvs.clear();
	for (const auto & [path, resource] : m_TextureList)
		srvs.emplace_back(resource.GetSrv());
	context->PSSetShaderResources(0, srvs.size(), srvs.data());

	context->DrawIndexed(count * 6, 0, 0);
	//context->Draw(count * 6, 0);
	//context->Draw(count, 0);
}

void BillboardRenderer::UpdateGpuResource(const ParticleRenderData& data, ID3D11DeviceContext* context)
{
	// vs srv 0 ~ 8
	for (int i = 0; i < FieldBufferCount; ++i)
		m_FieldBuffers[i].SetData(context, data.Data[i], data.Count);

	// vs cb0
	m_ConstantBuffer.SetData(context, *m_Constants);

	// ps srvs 0 ~ n
	const auto& paths = *m_TexturePaths;
	auto iList = m_TextureList.begin();
	for (int i = 0; i < paths.size(); ++i)
	{
		if (iList == m_TextureList.end())
		{
			m_TextureList.emplace_back(paths[i], Texture2D(m_Device, paths[i].data()));
			iList = m_TextureList.end();
		}
		else 
		{
			if (iList->first != paths[i])
			{
				iList->first = paths[i];
				iList->second.Load(m_Device, paths[i].data());
			}
			++iList;
		}
	}
	if (iList != m_TextureList.end())
	{
		m_TextureList.erase(iList, m_TextureList.end());
	}
}
