#pragma once
#include <cmath>
#include <cstring>

namespace MathHelpers {

    inline void identity(float* m) {
        std::memset(m, 0, 16*sizeof(float));
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    inline void perspective(float fovY, float aspect, float nearZ, float farZ, float* m) {
        float f = 1.0f / std::tan(fovY * 0.5f);
        std::memset(m, 0, 16*sizeof(float));
        m[0] = f / aspect;
        m[5] = f;
        m[10] = (farZ + nearZ) / (nearZ - farZ);
        m[11] = -1.0f;
        m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
    }
    
    inline void lookAt(float ex, float ey, float ez, float tx, float ty, float tz, float ux, float uy, float uz, float* m) {
        float f[3] = {tx - ex, ty - ey, tz - ez};
        float lenF = std::sqrt(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
        f[0]/=lenF; f[1]/=lenF; f[2]/=lenF;
        
        float u[3] = {ux, uy, uz};
        float s[3]; // f x u
        s[0] = f[1]*u[2] - f[2]*u[1];
        s[1] = f[2]*u[0] - f[0]*u[2];
        s[2] = f[0]*u[1] - f[1]*u[0];
        float lenS = std::sqrt(s[0]*s[0] + s[1]*s[1] + s[2]*s[2]);
        s[0]/=lenS; s[1]/=lenS; s[2]/=lenS;
        
        float u2[3]; // s x f
        u2[0] = s[1]*f[2] - s[2]*f[1];
        u2[1] = s[2]*f[0] - s[0]*f[2];
        u2[2] = s[0]*f[1] - s[1]*f[0];
        
        m[0] = s[0];  m[4] = s[1];  m[8] = s[2];
        m[1] = u2[0]; m[5] = u2[1]; m[9] = u2[2];
        m[2] = -f[0]; m[6] = -f[1]; m[10]= -f[2];
        m[3] = 0;     m[7] = 0;     m[11]= 0;
        
        m[12] = -(s[0]*ex + s[1]*ey + s[2]*ez);
        m[13] = -(u2[0]*ex + u2[1]*ey + u2[2]*ez);
        m[14] = f[0]*ex + f[1]*ey + f[2]*ez;
        m[15] = 1.0f;
    }
    
    inline void mult(const float* m, const float* v, float* res) {
        res[0] = m[0]*v[0] + m[4]*v[1] + m[8]*v[2] + m[12]*v[3];
        res[1] = m[1]*v[0] + m[5]*v[1] + m[9]*v[2] + m[13]*v[3];
        res[2] = m[2]*v[0] + m[6]*v[1] + m[10]*v[2] + m[14]*v[3];
        res[3] = m[3]*v[0] + m[7]*v[1] + m[11]*v[2] + m[15]*v[3];
    }
}
