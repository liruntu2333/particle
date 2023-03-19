#pragma once

class ParticleSystem
{
public:
	virtual ~ParticleSystem() = default;

	virtual void TickLogic(double dt) = 0;
	virtual void TickRender(double dt) = 0; 
};
