#pragma once
#include <d3d11.h>

class ParticleSystem
{
public:
	virtual ~ParticleSystem() = default;

	virtual void TickLogic(float dt) = 0;
	virtual void TickRender(ID3D11DeviceContext* context) = 0; 
};
