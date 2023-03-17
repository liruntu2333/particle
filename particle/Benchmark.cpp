#include <iostream>

#include "ParticleGenerator.h"
#include "ParticleSystem.h"
#include "ParticleSOA.h"
#include "ParticleUniforms.h"
#include "Timer.h"

constexpr size_t Capacity = 8192;
using Architecture = xsimd::default_arch;

int main(const char* arg)
{
	auto particleSoa = std::make_shared<ParticleSOA<Capacity, Architecture>>();

	float a[] = {1.0f, 0.5f, 0.25f };
	float c[] = 
	{
		0.6f, -0.5f, 0.25f, 0.124f,
		0.7f, -0.35f, 0.25f, 2.124f,
		0.8f, -0.45f, 0.25f, -0.124f,
		0.5f, -0.65f, 1.25f, 0.124f,
	};
	ParticleUniforms uniforms(a, c);
	auto generator = std::make_shared<SimpleGenerator>(0.05);
	auto timer     = std::make_shared<Timer>();
	auto manager   = std::make_shared<ParticleSystem<Capacity, Architecture>>(particleSoa, generator);

	while (true)
	{
		const double dt = timer->Tick();
		manager->Tick(dt, uniforms);
		//manager->TickScalar(dt, uniforms);
		
		static double counter = 0.0;

		static size_t iterationCount = 0;
		iterationCount++;
		if ((counter += dt) > 0.5)
		{
			system("cls");
			std::cout << 
				"Architecture : " << Architecture::name() <<
				" Particle Count : " << particleSoa->Size() <<
				" Iteration per Sec : " << iterationCount  << 
				" Sec per Iteration : " << 1000000.0 / static_cast<double>(iterationCount) << " us" << std::endl;
#ifdef _DEBUG
			particleSoa->PrintLog();
#endif

			counter = 0.0;
			iterationCount = 0;
		}
	}
}