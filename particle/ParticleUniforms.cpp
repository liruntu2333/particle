#include "ParticleUniforms.h"
#include <algorithm>

ParticleUniforms::ParticleUniforms(const float* acceleration, const float* colorParams)
{
	std::memcpy(Acceleration, acceleration, sizeof(Acceleration));
	std::memcpy(ColorParams, colorParams, sizeof(ColorParams));
}
