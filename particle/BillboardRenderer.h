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

class BillboardRenderer
{
public:
	static constexpr int FieldBufferCount = 9;

	struct PassConstants
	{
		Matrix ViewProj;
		Vector3 EyePosition;
		float DeltaTime{};

		PassConstants() = default;
	};

	struct ParticleRenderData
	{
		float* Data[FieldBufferCount]; // posXYZ colRGBA szWH
		size_t Count;
	};

	//using ConstBuffer = StructWithFlag<PassConstants>;

	BillboardRenderer(ID3D11Device* device, UINT capacity,
		const std::shared_ptr<FileSelection>& paths,
		const std::shared_ptr<PassConstants>& constants);

	~BillboardRenderer();

	BillboardRenderer(const BillboardRenderer&) = delete;
	BillboardRenderer(BillboardRenderer&&) = delete;
	BillboardRenderer& operator=(const BillboardRenderer&) = delete;
	BillboardRenderer& operator=(BillboardRenderer&&) = delete;

	void Initialize();
	void Render(ID3D11DeviceContext* context, UINT count);
	void UpdateGpuResource(const ParticleRenderData& data, ID3D11DeviceContext* context);

private:

	//StructArray m_ConstantBuffers{};
	DirectX::ConstantBuffer<PassConstants> m_ConstantBuffer;
	std::vector<DirectX::StructuredBuffer<float>> m_FieldBuffers{};
	std::list<std::pair<std::wstring, DirectX::Texture2D>> m_TextureList{};

	std::shared_ptr<FileSelection> m_TexturePaths{};	// update outside
	std::shared_ptr<PassConstants> m_Constants{};		// update outside
	
	//ComPtr<ID3D11InputLayout> m_InputLayout{};
	ComPtr<ID3D11VertexShader> m_Vs{};
	ComPtr<ID3D11PixelShader> m_Ps{};
	ComPtr<ID3D11Buffer> m_IndexBuffer{};

	ID3D11Device* m_Device = nullptr;
	const UINT m_Capacity;
};

