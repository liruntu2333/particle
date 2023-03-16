#pragma once
#include "ParticleSOA.h"
#include "ParticleUniforms.h"

#include "ParticleGenerator.h"

template <std::size_t Capacity>
class ParticleManager
{
public:
	using ParticleBatch = ParticleSOA<Capacity>;

	ParticleManager(const std::shared_ptr<ParticleBatch>& particles, const std::shared_ptr<ParticleGenerator>& generator);
	~ParticleManager() = default;

	ParticleManager(const ParticleManager&) = delete;
	ParticleManager(ParticleManager&&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;
	ParticleManager& operator=(ParticleManager&&) = delete;

	void Tick(double dt, const ParticleUniforms& uniforms);

	struct LifeTest
	{
		bool operator()(ParticleSOA<Capacity>* pSOA, size_t index)
		{
			return pSOA->m_Life[index] >= pSOA->m_MaxLife[index];
		}
	};

private:

	void UpdateKinetic	(std::shared_ptr<ParticleBatch> particles, double dt, const ParticleUniforms& uniforms);
	void UpdateLife		(std::shared_ptr<ParticleBatch> particles, double dt);
	void UpdateColor	(std::shared_ptr<ParticleBatch> particles, const ParticleUniforms& uniforms);

	std::weak_ptr<ParticleBatch> m_Particles;
	std::weak_ptr<ParticleGenerator> m_Generator;
};

template <std::size_t Capacity>
ParticleManager<Capacity>::ParticleManager(
	const std::shared_ptr<ParticleBatch>& particles,
	const std::shared_ptr<ParticleGenerator>& generator) :
	m_Particles(particles), m_Generator(generator)
{
}

template <std::size_t Capacity>
void ParticleManager<Capacity>::Tick(double dt, const ParticleUniforms& uniforms)
{
	auto particles = m_Particles.lock();
	if (particles == nullptr) return;
	const auto generator = m_Generator.lock();
	if (generator == nullptr) return;

	// kinetic pass
	UpdateKinetic(particles, dt, uniforms);
	UpdateLife(particles, dt);

	auto newParticle = generator->Generates(dt);
	// life test
	LifeTest lifeTest;
	particles->EraseIf(lifeTest);

	// add new particles
	particles->Push(generator->Generates(dt));
	UpdateColor(particles, uniforms);
}

template <std::size_t Capacity>
void ParticleManager<Capacity>::UpdateKinetic(std::shared_ptr<ParticleBatch> particles, double dt, const ParticleUniforms& uniforms)
{
	const size_t arrLength = particles->Size();
	const size_t vecLength = arrLength - arrLength % xsimd::simd_type<float>::size;

	using floatBatch = xsimd::batch<float, xsimd::best_arch>;

	auto bDt = floatBatch(dt);

	for (int i = 0; i < 3; ++i)
	{
		auto& p = particles->m_Position[i];
		auto& v = particles->m_Velocity[i];
		
		for (size_t j = 0; j < vecLength; j += floatBatch::size)
		{
			// Semi-implicit Euler
			auto bVel = floatBatch::load_aligned(v.Get(j));

			bVel += uniforms.Acceleration[i] * dt;
			auto bPos = floatBatch::load_aligned(p.Get(j));
			bPos += bVel * bDt;
			bPos.store_aligned(p.Get(j));
			bVel.store_aligned(v.Get(j));
		}

		for (size_t j = vecLength; j < arrLength; ++j)
		{
			v[j] += uniforms.Acceleration[i] * dt;
			p[j] += v[j] * dt;
		}
	}
}

template <std::size_t Capacity>
void ParticleManager<Capacity>::UpdateLife(std::shared_ptr<ParticleBatch> particles, double dt)
{
	const size_t arrLength = particles->Size();
	const size_t vecLength = arrLength - arrLength % xsimd::simd_type<float>::size;

	using floatBatch = xsimd::batch<float, xsimd::best_arch>;

	auto& life = particles->m_Life;
	auto& maxLife = particles->m_MaxLife;

	// update life
	{
		for (size_t j = 0; j < vecLength; j += floatBatch::size)
		{
			auto bLife = floatBatch::load_aligned(life.Get(j));
			bLife += dt;
			bLife.store_aligned(life.Get(j));
		}

		for (size_t j = vecLength; j < arrLength; ++j)
		{
			life[j] += dt;
		}
	}
}

template <std::size_t Capacity>
void ParticleManager<Capacity>::UpdateColor(std::shared_ptr<ParticleBatch> particles,
                                            const ParticleUniforms& uniforms)
{
	const size_t arrLength = particles->Size();
	const size_t vecLength = arrLength - arrLength % xsimd::simd_type<float>::size;

	using floatBatch = xsimd::batch<float, xsimd::best_arch>;

	auto& life = particles->m_Life;
	auto& maxLife = particles->m_MaxLife;

	std::vector<float> normLife(arrLength);
	// compute normalized life
	{
		for (size_t j = 0; j < vecLength; j += floatBatch::size)
		{
			auto bLife = floatBatch::load_aligned(life.Get(j));
			auto bMaxLife = floatBatch::load_aligned(life.Get(j));
			(bLife / bMaxLife).store_aligned(&normLife[j]);
		}

		for (size_t j = vecLength; j < arrLength; ++j)
		{
			normLife[j] = life[j] / maxLife[j];
		}
	}

	// update color
	{
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
}

