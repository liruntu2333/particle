#pragma once

#include <vector>

struct ParticleInitialValue;

class ParticleEmitter
{
public:
	ParticleEmitter() = default;
	virtual ~ParticleEmitter() = default;

	virtual std::vector<ParticleInitialValue> Generates(double dt) = 0;
};

class SimpleEmitter : public ParticleEmitter
{
public:
	explicit SimpleEmitter(double timeSpan);
	~SimpleEmitter() override = default;

	std::vector<ParticleInitialValue> Generates(double dt) override;

private:
	double m_Counter = 0.0;
	const double m_TimeSpan = 0.0;
};

