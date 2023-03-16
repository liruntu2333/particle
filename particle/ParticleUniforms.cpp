#include "ParticleUniforms.h"
#include <algorithm>

ParticleUniforms::ParticleUniforms(const float* accleration, const float* colorParmas)
{
	std::memcpy(Acceleration, accleration, sizeof(Acceleration));
	std::memcpy(ColorParams, colorParmas, sizeof(ColorParams));
}
