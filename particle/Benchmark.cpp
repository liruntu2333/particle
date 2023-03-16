#include <chrono>
#include <functional>
#include <iostream>
#include <stack>
#include <vector>

#include "ParticleGenerator.h"
#include "ParticleSystem.h"
#include "xsimd.hpp"
#include "ParticleSOA.h"
#include "ParticleUniforms.h"
#include "Timer.h"

using AlignedVector = std::vector<float, xsimd::aligned_allocator<float>>;
constexpr std::size_t SimdSize = xsimd::simd_type<float>::size;
//constexpr std::size_t SimdSize = xsimd::simd_type<double>::size;

AlignedVector va;
AlignedVector vb;
AlignedVector vc;

void BuildVector()
{
    va.resize(2048);
    vb.resize(2048);
    vc.resize(2048);
}

//void RunTest(const std::function<void()>& func)
//{
//	const auto start = std::chrono::high_resolution_clock::now();
//
//    func();
//
//	const auto end = std::chrono::high_resolution_clock::now();
//	const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>((end - start));
//    std::cout << "Time elapsed : " << duration.count() << " ms." << std::endl;
//}
//
//void SimdAdd()
//{
//    assert(va.size() == vb.size() && "size mismatch");
//    const std::size_t size = va.size();
//    const std::size_t vecSize = size - size % SimdSize;
//    const auto* dataA = va.data();
//    const auto* dataB = vb.data();
//    auto* dataC = vc.data();
//    for (std::size_t i = 0; i < vecSize; i += SimdSize)
//    {
//	    auto ba = xsimd::load_aligned<xsimd::best_arch>(dataA + i);
//        auto bb = xsimd::load_aligned<xsimd::best_arch>(dataB + i);
//        auto res = (ba + bb) * 0.5f;
//        res.store_aligned(dataC + i);
//    }
//
//    for (std::size_t i = vecSize; i < size; ++i)
//    {
//        dataC[i] = (dataA[i] + dataB[i]) * 0.5f;
//    }
//}
//
//void ScalarAdd()
//{
//    assert(va.size() == vb.size() && "size mismatch");
//    const std::size_t size = va.size();
//    const auto* dataA = va.data();
//    const auto* dataB = vb.data();
//    auto* dataC = vc.data();
//    for (std::size_t i = 0; i < size; ++i)
//    {
//	    dataC[i] = (dataA[i] + dataB[i]) * 0.5f;
//    }
//}

template <class arch>
void run_arch()
{
	assert(va.size() == vb.size() && "size mismatch");
	const std::size_t size = va.size();
	const std::size_t vecSize = size - size % SimdSize;
	const auto* dataA = va.data();
	const auto* dataB = vb.data();
	auto* dataC = vc.data();

	const auto start = std::chrono::high_resolution_clock::now();
	for (int j = 0; j < UINT16_MAX << 8; ++j)
	{
		for (std::size_t i = 0; i < vecSize; i += SimdSize)
		{
			auto ba = xsimd::load_aligned<arch>(dataA + i);
			auto bb = xsimd::load_aligned<arch>(dataB + i);
			auto res = ba * bb * ba * bb;
			dataC[0] = res.get(0);
			res.store_aligned(dataC + i);
		}

		for (std::size_t i = vecSize; i < size; ++i)
		{
			dataC[i] = (dataA[i] + dataB[i] + dataA[i] + dataB[i]);
		}
	}

	const auto end = std::chrono::high_resolution_clock::now();
	const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>((end - start));
	std::cout << "Run with " << arch::name() << std::endl;
	std::cout << "Time elapsed : " << duration.count() << " ms." << std::endl;
};

template <class T>
struct run_archlist;

template <class... Arch>
struct run_archlist<xsimd::arch_list<Arch...>>
{
    static void run()
    {
        using expand_type = int[];
        expand_type{ (run_arch<Arch>(), 0)... };
    }
};

int main()
{
	
	//constexpr  std::size_t SimdSize = xsimd::simd_type<float>::size;
 //   std::size_t vec_size = size - size % SimdSize;

 //   for (std::size_t i = 0; i < vec_size; i += SimdSize)
 //   {
 //       auto ba = xs::load_aligned(&a[i]);
 //       auto bb = xs::load_aligned(&b[i]);
 //       auto bres = (ba + bb) / 2.;
 //       bres.store_aligned(&res[i]);
 //   }

    //BuildVector();
    //run_archlist<xsimd::supported_architectures>::run();

	auto particleSoa = std::make_shared<ParticleSOA<1024>>();

	float a[] = {1.0f, 0.5f, 0.25f };
	float c[] = {
		0.6f, -0.5f, 0.25f, 0.124f,
		0.7f, -0.35f, 0.25f, 2.124f,
		0.8f, -0.45f, 0.25f, -0.124f,
		0.5f, -0.65f, 1.25f, 0.124f,
	};
	ParticleUniforms uniforms(a, c);
	auto generator   = std::make_shared<SimpleGenerator>(0.4f);
	auto timer       = std::make_shared<Timer>();
	auto manager     = std::make_shared<ParticleSystem<1024>>(particleSoa, generator);

	while (true)
	{
		
		double dt = timer->Tick();
		manager->Tick(dt, uniforms);
		
		static double counter = 0.0;
#ifdef _DEBUG
		if ((counter += dt) > 0.2)
		{
			system("cls");
			particleSoa->PrintLog();
			counter = 0.0;
		}

#endif
	}
}