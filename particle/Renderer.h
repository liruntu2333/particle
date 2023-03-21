#pragma once
#include <d3d11.h>
#include <memory>
#include <vector>
#include <wrl/client.h>
#include "FileSelection.h"
#include <directxtk/SimpleMath.h>
#include <directxtk/BufferHelpers.h>
#include "StructuredBuffer.h"
#include "Texture2D.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;

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
	struct PassConstants
	{
		Matrix View;
		Matrix Proj;
		Vector3 EyePosition;
		float DeltaTime{};

		PassConstants() = default;
	};

	using ConstBuffer = StructWithFlag<PassConstants>;
	
	BillboardRenderer(ID3D11Device* device, UINT capacity,
		const std::shared_ptr<FileSelection>& paths,
		const std::shared_ptr<ConstBuffer>& constants);

	~BillboardRenderer() override;

	void Initialize() override;
	void Render(ID3D11DeviceContext* context) override;
	void UpdateGpuResource(size_t count);

private:

	//StructArray m_ConstantBuffers{};
	DirectX::ConstantBuffer<PassConstants> m_ConstantBuffer;
	std::vector<DirectX::StructuredBuffer<float>> m_FieldBuffers{};

	std::list<std::pair<std::wstring, DirectX::Texture2D>> m_TextureArray{};
	std::shared_ptr<FileSelection> m_TexturePaths{};
	std::shared_ptr<StructWithFlag<PassConstants>> m_Constants{};

	ComPtr<ID3D11InputLayout> m_InputLayout{};
	ComPtr<ID3D11VertexShader> m_Vs{};
	ComPtr<ID3D11PixelShader> m_Ps{};
	ComPtr<ID3D11Buffer> m_IndexBuffer{};

	ID3D11Device* m_Device = nullptr;
	const UINT m_Capacity;
};

