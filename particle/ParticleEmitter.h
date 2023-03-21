#pragma once

#include <vector>

struct ParticleInitialValue;

class ParticleEmitter
{
public:
	ParticleEmitter() = default;
	virtual ~ParticleEmitter() = default;

	virtual std::vector<ParticleInitialValue> Generates(float dt) = 0;
};

class SimpleEmitter : public ParticleEmitter
{
public:
	explicit SimpleEmitter(float timeSpan);
	~SimpleEmitter() override = default;

	std::vector<ParticleInitialValue> Generates(float dt) override;

private:
	float m_Counter = 0.0;
	const float m_TimeSpan = 0.0;
};

