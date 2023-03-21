#pragma once
#include "ParticleSystem.h"
#include "ParticleSOA.h"

#include "ParticleEmitter.h"
#include "ParticleUniforms.h"

template <std::size_t Capacity, class Arch>
class ParticleSystemCPU : public ParticleSystem
{
public:

	using ParticleBatch = ParticleSOA<Capacity, Arch>;
	using FloatBatch = xsimd::batch<float, Arch>;

	ParticleSystemCPU(
		std::shared_ptr<ParticleBatch> particles,
		std::shared_ptr<ParticleEmitter> generator,
		std::shared_ptr<ParticleUniforms> uniforms);
	
	~ParticleSystemCPU() override = default;

	ParticleSystemCPU(const ParticleSystemCPU&) = delete;
	ParticleSystemCPU(ParticleSystemCPU&&) = delete;
	ParticleSystemCPU& operator=(const ParticleSystemCPU&) = delete;
	ParticleSystemCPU& operator=(ParticleSystemCPU&&) = delete;

	void TickLogic(float dt) override;
	void TickLogicScalar(float dt);

	void TickRender(float dt) override {}

	struct LifeTest
	{
		bool operator()(ParticleSOA<Capacity, Arch>* pSOA, size_t index)
		{
			return pSOA->m_Age[index] >= pSOA->m_LifeSpan[index];
		}
	};

private:

	void UpdateKinetic	(std::shared_ptr<ParticleBatch> particles, float dt, const ParticleUniforms& uniforms);
	void UpdateAge		(std::shared_ptr<ParticleBatch> particles, float dt);
	void UpdateColor	(std::shared_ptr<ParticleBatch> particles, const ParticleUniforms& uniforms);

	void UpdateKineticScalar	(std::shared_ptr<ParticleBatch> particles, float dt, const ParticleUniforms& uniforms);
	void UpdateAgeScalar		(std::shared_ptr<ParticleBatch> particles, float dt);
	void UpdateColorScalar		(std::shared_ptr<ParticleBatch> particles, const ParticleUniforms& uniforms);

	std::shared_ptr<ParticleBatch> m_Particles;
	std::shared_ptr<ParticleEmitter> m_Emitter;
	std::shared_ptr<ParticleUniforms> m_Uniforms;
};

template <std::size_t Capacity, class Arch>
ParticleSystemCPU<Capacity, Arch>::ParticleSystemCPU(
	std::shared_ptr<ParticleBatch> particles,
	std::shared_ptr<ParticleEmitter> generator,
	std::shared_ptr<ParticleUniforms> uniforms) :
	m_Particles(std::move(particles)), m_Emitter(std::move(generator)), m_Uniforms(std::move(uniforms))
{
}

template <std::size_t Capacity, class Arch>
void ParticleSystemCPU<Capacity, Arch>::TickLogic(float dt)
{
	UpdateKinetic(m_Particles, dt, *m_Uniforms);
	UpdateAge(m_Particles, dt);

	// life test
	m_Particles->EraseIf(LifeTest());

	// add new particles
	//if (particles->CAPACITY > particles->Size())
		m_Particles->Push(m_Emitter->Generates(dt));

	UpdateColor(m_Particles, *m_Uniforms);
}

template <std::size_t Capacity, class Arch>
void ParticleSystemCPU<Capacity, Arch>::TickLogicScalar(float dt)
{
	UpdateKineticScalar(m_Particles, dt, *m_Uniforms);
	UpdateAgeScalar(m_Particles, dt);

	// life test
	m_Particles->EraseIf(LifeTest());

	// add new particles
	//if (particles->CAPACITY > particles->Size())
		m_Particles->Push(m_Emitter->Generates(dt));
	UpdateColorScalar(m_Particles, *m_Uniforms);
}

template <std::size_t Capacity, class Arch>
void ParticleSystemCPU<Capacity, Arch>::UpdateKinetic(std::shared_ptr<ParticleBatch> particles, float dt, const ParticleUniforms& uniforms)
{
	const size_t arrLength = particles->Size();
	const size_t vecLength = arrLength - arrLength % xsimd::simd_type<float>::size;
	
	for (int i = 0; i < 3; ++i)
	{
		auto& pos = particles->m_Position[i];
		auto& vel = particles->m_Velocity[i];

		for (size_t j = 0; j < vecLength; j += FloatBatch::size)
		{
			// Semi-implicit Euler
			auto bVel = FloatBatch::load_aligned(vel.Get(j));

			bVel += uniforms.Acceleration[i] * dt;
			auto bPos = FloatBatch::load_aligned(pos.Get(j));
			bPos += bVel * dt;
			bPos.store_aligned(pos.Get(j));
			bVel.store_aligned(vel.Get(j));
		}

		for (size_t j = vecLength; j < arrLength; ++j)
		{
			vel[j] += uniforms.Acceleration[i] * dt;
			pos[j] += vel[j] * dt;
		}
	}
}

template <std::size_t Capacity, class Arch>
void ParticleSystemCPU<Capacity, Arch>::UpdateAge(std::shared_ptr<ParticleBatch> particles, float dt)
{
	const size_t arrLength = particles->Size();
	const size_t vecLength = arrLength - arrLength % xsimd::simd_type<float>::size;

	auto& age = particles->m_Age;
	auto& span = particles->m_LifeSpan;

	// update life
	for (size_t j = 0; j < vecLength; j += FloatBatch::size)
	{
		auto bAge = FloatBatch::load_aligned(age.Get(j));
		bAge += dt;
		bAge.store_aligned(age.Get(j));
	}

	for (size_t j = vecLength; j < arrLength; ++j)
	{
		age[j] += dt;
	}
}

template <std::size_t Capacity, class Arch>
void ParticleSystemCPU<Capacity, Arch>::UpdateColor(std::shared_ptr<ParticleBatch> particles,
                                            const ParticleUniforms& uniforms)
{
	const size_t arrLength = particles->Size();
	const size_t vecLength = arrLength - arrLength % xsimd::simd_type<float>::size;

	auto& age = particles->m_Age;
	auto& span = particles->m_LifeSpan;

	std::vector<float, xsimd::aligned_allocator<float, Arch::alignment()>> normLife(arrLength);
	// compute normalized age
	for (size_t j = 0; j < vecLength; j += FloatBatch::size)
	{
		auto bAge = FloatBatch::load_aligned(age.Get(j));
		auto bSpan = FloatBatch::load_aligned(span.Get(j));
		(bAge / bSpan).store_aligned(&normLife[j]);
	}

	for (size_t j = vecLength; j < arrLength; ++j)
	{
		normLife[j] = age[j] / span[j];
	}

	// update color
	for (int i = 0; i < 4; ++i)
	{
		//polynomial fitting
		// Y = A0 + t * (A1 + t * (A2 + t * A3))
		auto& color = particles->m_Color[i];
		auto [A0, A1, A2, A3] = uniforms.ColorParams[i];
			
		for (size_t j = 0; j < vecLength; j += FloatBatch::size)
		{
			auto bNormLife = FloatBatch::load_aligned(&normLife[j]);
			FloatBatch bCol(A3);
			bCol = bCol * bNormLife + A2;
			bCol = bCol * bNormLife + A1;
			bCol = bCol * bNormLife + A0;
			bCol.store_aligned(color.Get(j));
		}

		for (size_t j = vecLength; j < arrLength; ++j)
		{
			float col = A3;
			col = col * normLife[j] + A2;
			col = col * normLife[j] + A1;
			col = col * normLife[j] + A0;
			color[j] = col;
		}
	}
}

template <std::size_t Capacity, class Arch>
void ParticleSystemCPU<Capacity, Arch>::UpdateKineticScalar(std::shared_ptr<ParticleBatch> particles, float dt,
                                                            const ParticleUniforms& uniforms)
{
	const auto size = particles->Size();
	for (int i = 0; i < 3; ++i)
	{
		auto& pos = particles->m_Position[i];
		auto& vel = particles->m_Velocity[i];
		
		for (size_t j = 0; j < size; j++)
		{
			vel[j] += uniforms.Acceleration[i] * dt;
			pos[j] += vel[j] * dt;
		}
	}
}

template <std::size_t Capacity, class Arch>
void ParticleSystemCPU<Capacity, Arch>::UpdateAgeScalar(std::shared_ptr<ParticleBatch> particles, float dt)
{
	auto& age = particles->m_Age;
	auto& span = particles->m_LifeSpan;
	const auto size = particles->Size();
	// update life
	{
		for (size_t j = 0; j < size; ++j)
		{
			age[j] += dt;
		}
	}
}

template <std::size_t Capacity, class Arch>
void ParticleSystemCPU<Capacity, Arch>::UpdateColorScalar(std::shared_ptr<ParticleBatch> particles,
	const ParticleUniforms& uniforms)
{
	const size_t size = particles->Size();
	auto& age = particles->m_Age;
	auto& span = particles->m_LifeSpan;

	std::vector<float> normLife(size);
	// compute normalized age
	for (size_t j = 0; j < size; ++j)
	{
		normLife[j] = age[j] / span[j];
	}

	// update color
	for (int i = 0; i < 4; ++i)
	{
		//polynomial fitting
		// Y = A0 + t * (A1 + t * (A2 + t * A3))
		auto& color = particles->m_Color[i];
		auto [A0, A1, A2, A3] = uniforms.ColorParams[i];

		for (size_t j = 0; j < size; ++j)
		{
			float col = A3;
			col = col * normLife[j] + A2;
			col = col * normLife[j] + A1;
			col = col * normLife[j] + A0;
			color[j] = col;
		}
	}
}

