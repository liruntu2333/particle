#pragma once

#include <vector>

struct ParticleInitialValue;

class ParticleGenerator
{
public:
	ParticleGenerator() = default;
	virtual ~ParticleGenerator() = default;

	virtual std::vector<ParticleInitialValue> Generates(double dt) = 0;
};

class SimpleGenerator : public ParticleGenerator
{
public:
	explicit SimpleGenerator(double timeSpan);
	~SimpleGenerator() override = default;

	std::vector<ParticleInitialValue> Generates(double dt) override;

private:
	double m_Counter = 0.0;
	const double m_TimeSpan = 0.0;
};

