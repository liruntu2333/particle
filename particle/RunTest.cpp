#include <chrono>
#include <iostream>
#include <vector>
#include <xsimd/xsimd.hpp>

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

void RunTest()
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
}

