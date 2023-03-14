#ifndef PTX_H
#define PTX_H

#include "xsimd/xsimd.hpp"

constexpr unsigned Alignment = 8;

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

    float* pPosX;
    float* pPosY;
    float* pPosZ;

    float* pVelX;
    float* pVelY;
    float* pVelZ;

    float* pColorR;
    float* pColorG;
    float* pColorB;
    float* pColorA;

    float* pSpin;

    void Tick(const ptx_uniform* _pUni, float _dt) {

        namespace xs = xsimd;

        for (size_t i = 0; i < nSize; ++i) {
            float vX = pVelX[i];
            vX += _dt * _pUni->g[0];
            float pX = pPosX[i];
            pX += vX * _dt;
            pPosX[i] = pX;
            pVelX[i] = vX;

            xs::batch<float, xs::avx2> a();
            xs::batch<float, xs::avx2> b = { 2.5f, 3.5f, 4.5f, 5.5f };
            auto mean = (a + b) / 2;
            Alignment 
        }
    }

    void NextGen(ptx_generator* _pGen) {
        
    }
};

#endif
