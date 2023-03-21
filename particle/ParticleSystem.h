#pragma once

class ParticleSystem
{
public:
	virtual ~ParticleSystem() = default;

	virtual void TickLogic(float dt) = 0;
	virtual void TickRender(float dt) = 0; 
};
