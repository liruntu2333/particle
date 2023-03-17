#pragma once
#include "ParticleSOA.h"
#include "ParticleUniforms.h"

#include "ParticleEmitter.h"

template <std::size_t Capacity, class Arch>
class ParticleSystem
{
public:

	using ParticleBatch = ParticleSOA<Capacity, Arch>;
	using floatBatch = xsimd::batch<float, Arch>;

	ParticleSystem(const std::shared_ptr<ParticleBatch>& particles, const std::shared_ptr<ParticleEmitter>& generator);
	~ParticleSystem() = default;

	ParticleSystem(const ParticleSystem&) = delete;
	ParticleSystem(ParticleSystem&&) = delete;
	ParticleSystem& operator=(const ParticleSystem&) = delete;
	ParticleSystem& operator=(ParticleSystem&&) = delete;

	void Tick(double dt, const ParticleUniforms& uniforms);
	void TickScalar(double dt, const ParticleUniforms& uniforms);

	struct LifeTest
	{
		bool operator()(ParticleSOA<Capacity, Arch>* pSOA, size_t index)
		{
			return pSOA->m_Age[index] >= pSOA->m_LifeSpan[index];
		}
	};

private:

	void UpdateKinetic	(std::shared_ptr<ParticleBatch> particles, double dt, const ParticleUniforms& uniforms);
	void UpdateAge		(std::shared_ptr<ParticleBatch> particles, double dt);
	void UpdateColor	(std::shared_ptr<ParticleBatch> particles, const ParticleUniforms& uniforms);

	void UpdateKineticScalar	(std::shared_ptr<ParticleBatch> particles, double dt, const ParticleUniforms& uniforms);
	void UpdateAgeScalar		(std::shared_ptr<ParticleBatch> particles, double dt);
	void UpdateColorScalar		(std::shared_ptr<ParticleBatch> particles, const ParticleUniforms& uniforms);

	std::weak_ptr<ParticleBatch> m_Particles;
	std::weak_ptr<ParticleEmitter> m_Emitter;
};

template <std::size_t Capacity, class Arch>
ParticleSystem<Capacity, Arch>::ParticleSystem(
	const std::shared_ptr<ParticleBatch>& particles, 
	const std::shared_ptr<ParticleEmitter>& generator) :
	m_Particles(particles), m_Emitter(generator)
{
}

template <std::size_t Capacity, class Arch>
void ParticleSystem<Capacity, Arch>::Tick(double dt, const ParticleUniforms& uniforms)
{
	auto particles = m_Particles.lock();
	if (particles == nullptr) return;
	const auto emitter = m_Emitter.lock();
	if (emitter == nullptr) return;

	UpdateKinetic(particles, dt, uniforms);
	UpdateAge(particles, dt);

	// life test
	particles->EraseIf(LifeTest());

	// add new particles
	//if (particles->CAPACITY > particles->Size())
		particles->Push(emitter->Generates(dt));

	UpdateColor(particles, uniforms);
}

template <std::size_t Capacity, class Arch>
void ParticleSystem<Capacity, Arch>::TickScalar(double dt, const ParticleUniforms& uniforms)
{
	auto particles = m_Particles.lock();
	if (particles == nullptr) return;
	const auto emitter = m_Emitter.lock();
	if (emitter == nullptr) return;

	UpdateKineticScalar(particles, dt, uniforms);
	UpdateAgeScalar(particles, dt);

	// life test
	particles->EraseIf(LifeTest());

	// add new particles
	//if (particles->CAPACITY > particles->Size())
		particles->Push(emitter->Generates(dt));
	UpdateColorScalar(particles, uniforms);
}

template <std::size_t Capacity, class Arch>
void ParticleSystem<Capacity, Arch>::UpdateKinetic(std::shared_ptr<ParticleBatch> particles, double dt, const ParticleUniforms& uniforms)
{
	const size_t arrLength = particles->Size();
	const size_t vecLength = arrLength - arrLength % xsimd::simd_type<float>::size;
	
	for (int i = 0; i < 3; ++i)
	{
		auto& pos = particles->m_Position[i];
		auto& vel = particles->m_Velocity[i];

		for (size_t j = 0; j < vecLength; j += floatBatch::size)
		{
			// Semi-implicit Euler
			auto bVel = floatBatch::load_aligned(vel.Get(j));

			bVel += uniforms.Acceleration[i] * dt;
			auto bPos = floatBatch::load_aligned(pos.Get(j));
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
void ParticleSystem<Capacity, Arch>::UpdateAge(std::shared_ptr<ParticleBatch> particles, double dt)
{
	const size_t arrLength = particles->Size();
	const size_t vecLength = arrLength - arrLength % xsimd::simd_type<float>::size;

	auto& age = particles->m_Age;
	auto& span = particles->m_LifeSpan;

	// update life
	for (size_t j = 0; j < vecLength; j += floatBatch::size)
	{
		auto bAge = floatBatch::load_aligned(age.Get(j));
		bAge += dt;
		bAge.store_aligned(age.Get(j));
	}

	for (size_t j = vecLength; j < arrLength; ++j)
	{
		age[j] += dt;
	}
}

template <std::size_t Capacity, class Arch>
void ParticleSystem<Capacity, Arch>::UpdateColor(std::shared_ptr<ParticleBatch> particles,
                                            const ParticleUniforms& uniforms)
{
	const size_t arrLength = particles->Size();
	const size_t vecLength = arrLength - arrLength % xsimd::simd_type<float>::size;

	auto& age = particles->m_Age;
	auto& span = particles->m_LifeSpan;

	std::vector<float, xsimd::aligned_allocator<float, xsimd::best_arch::alignment()>> normLife(arrLength);
	// compute normalized age
	for (size_t j = 0; j < vecLength; j += floatBatch::size)
	{
		auto bAge = floatBatch::load_aligned(age.Get(j));
		auto bSpan = floatBatch::load_aligned(span.Get(j));
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
			
		for (size_t j = 0; j < vecLength; j += floatBatch::size)
		{
			auto bNormLife = floatBatch::load_aligned(&normLife[j]);
			floatBatch bCol(A3);
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
void ParticleSystem<Capacity, Arch>::UpdateKineticScalar(std::shared_ptr<ParticleBatch> particles, double dt,
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
void ParticleSystem<Capacity, Arch>::UpdateAgeScalar(std::shared_ptr<ParticleBatch> particles, double dt)
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
void ParticleSystem<Capacity, Arch>::UpdateColorScalar(std::shared_ptr<ParticleBatch> particles,
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

