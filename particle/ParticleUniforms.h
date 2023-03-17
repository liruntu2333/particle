#pragma once

struct ParticleUniforms
{
	float Acceleration[3]{};
	float ColorParams[4][4]{};

	ParticleUniforms(const float* acceleration, const float* colorParams);
};

