#include <iostream>

#include "ParticleEmitter.h"
#include "ParticleSystemCPU.h"
#include "ParticleSOA.h"
#include "ParticleUniforms.h"
#include "Timer.h"

#include <imgui.h>

constexpr size_t Capacity = 8196;
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

	std::shared_ptr<ParticleUniforms> uniforms = std::make_shared<ParticleUniforms>(a, c);
	std::shared_ptr<ParticleEmitter> emitter = std::make_shared<SimpleEmitter>(0.001);
	std::shared_ptr<Timer> timer = std::make_shared<Timer>();
	std::shared_ptr<ParticleSystem> particleSystem = std::make_shared<ParticleSystemCPU<
		Capacity, Architecture>>(particleSoa, emitter, uniforms);

	while (true)
	{
		const double dt = timer->Tick();
		particleSystem->TickLogic(dt);
		//particleSystem->TickScalar(dt, uniforms);
		static size_t iterationCount = 0;
		iterationCount++;

		if (static double counter = 0.0; (counter += dt) >= 1.0)
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

			counter -= 1.0;
			iterationCount = 0;
		}
	}
}