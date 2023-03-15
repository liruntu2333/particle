#ifndef PTX_H
#define PTX_H

#include "AlignedStack.hpp"

constexpr int FloatAlignment = 4;

struct ptx_uniform {

    float g[3];

    float ColorRParam[4];
    float ColorGParam[4];
    float ColorBParam[4];
    float ColorAParam[4];
};

// glm 

struct ptx_generator {

    float Center[3];
    float radius;
    float nCountPreSecont;
};


struct ptx_soa {

    unsigned nCap;
    unsigned nSize;

    float* pCurLife;
    float* pMaxLife;

    //float* PositionChunk[3];
    //float* VelocityChunk[3];
    //float* ColorChunk[4];
    std::array<AlignedStack<float, FloatAlignment>, 3> Position;
    std::array<AlignedVector<float, FloatAlignment>, 3> Velocity;
    std::array<AlignedVector<float, FloatAlignment>, 3> Color;

    float* pSpin;

    void Tick(const ptx_uniform* _pUni, float _dt) {
	    for (int i = 0; i < 3; ++i)
	    {
		    for (int j = 0; j < MAX; ++j)
		    {
			    
		    }
            xsimd::batch<float, xsimd::avx2> v;
            v = xsimd::batch<float, xsimd::avx2>::load_aligned(pVelX);
            xsimd::batch<float, xsimd::avx2> p;
            p = xsimd::batch<float, xsimd::avx2>::load_aligned(pPosX);

            v += _pUni->g[0];
            p = p + _dt * v;

            p.store_aligned(pPosX);

	    }

        for (size_t i = 0; i < nSize; i += FloatAligment) {
            //float vX = pVelX[i];
            //vX += _dt * _pUni->g[0];
            //float pX = pPosX[i];
            //pX += vX * _dt;
            //pPosX[i] = pX;
            //pVelX[i] = vX;

            // implicit Euler

            xsimd::batch<float, xsimd::avx2> v;
            v = xsimd::batch<float, xsimd::avx2>::load_aligned(pVelX);
            xsimd::batch<float, xsimd::avx2> p;
            p = xsimd::batch<float, xsimd::avx2>::load_aligned(pPosX);

            v += _pUni->g[0];
            p = p + _dt * v;

            p.store_aligned(pPosX);

            
        }
    }

    void NextGen(ptx_generator* _pGen) {
        T
    }
};

#endif
