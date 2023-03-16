#pragma once

struct ParticleUniforms
{
	float Acceleration[3];
	float ColorParams[4][4];

	ParticleUniforms(const float* accleration, const float* colorParmas);
};

