#pragma once
#include <d3d11.h>
#include <string>
#include <vector>
#include <wrl/client.h>
#include <unordered_map>
#include "FileSelection.h"

using Microsoft::WRL::ComPtr;

class Renderer
{
public:
	Renderer() = default;
	virtual ~Renderer() = default;

	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) = delete;

	virtual void Initialize() = 0;
	virtual void Render(ID3D11DeviceContext* context) = 0;
};

class BillboardRenderer : public Renderer
{
public:

	BillboardRenderer(ID3D11Device* device);
	~BillboardRenderer() override;

	void Initialize() override;
	void Render(ID3D11DeviceContext* context) override;

private:

	using ResourceVec = std::vector<ComPtr<ID3D11Resource>>;
	using SrvVec = std::vector<ComPtr<ID3D11ShaderResourceView>>;
	using TextureMap = std::vector<std::tuple<
		std::wstring, ComPtr<ID3D11Resource>, ComPtr<ID3D11ShaderResourceView>>>;

	ResourceVec m_ConstantBuffers{};
	SrvVec m_ConstantViews{};
	ResourceVec m_InstanceBuffers{};
	SrvVec m_InstanceSrvs{};

	TextureMap m_Map{};
	std::weak_ptr<FileSelection> m_Textures{};

	ComPtr<ID3D11VertexShader> m_Vs{};
	ComPtr<ID3D11PixelShader> m_Ps{};
	ID3D11Device* m_Device = nullptr;
};
