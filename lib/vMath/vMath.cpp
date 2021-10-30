#include "vMath.h"
#include "config.h"
#include <math.h> 
#include "esp_dsp.h"


void quat2Euler(float *q, float *euler_out){
    //Phi
    euler_out[0] = atan2(q[2]*q[3] + q[0]*q[1], 0.5 - (q[1]*q[1] + q[2]*q[2]));
    //Theta
    euler_out[1] = asin(2*(q[0]*q[2] - q[1]*q[3]));
    //Psi
    euler_out[2] = atan2(q[1]*q[2] + q[0]*q[3], 0.5 - (q[2]*q[2] + q[3]*q[3]));
}

void normalize(float *in, int len){
    dsps_mulc_f32_ae32(in, in, len, 1/norm2(in, len), 1, 1);
}

void normalizeQuat(float *q){
    quatProdConst(q, 1/norm2(q, 4), q);
}

float norm2(float *in, const int len){
    float norm;
    dsps_dotprod_f32_ae32(in, in, &norm, len);
    return norm = sqrt(norm);
}

void quatConj(float *in, float *out){
    out[0] = in[0];
    out[1] = -in[1];
    out[2] = -in[2];
    out[3] = -in[3];
}

void matProd_3x3x1(float *in1, float *in2, float *out){
    dspm_mult_f32_ae32(in1, in2, out, 3, 3, 1);
}

void matProd_4x6x1(float *in1, float *in2, float *out){
    dspm_mult_f32_ae32(in1, in2, out, 4, 6, 1);
}

void quatAdd(float *in1, float *in2, float *out){
    dsps_add_f32_ae32(in1, in2, out, 4, 1, 1, 1);
}

void quatProd(float *a, float *b, float *out){
    out[0] = a[0]*b[0] - a[1]*b[1] - a[2]*b[2] - a[3]*b[3];
    out[1] = a[0]*b[1] + a[1]*b[0] + a[2]*b[3] - a[3]*b[2];
    out[2] = a[0]*b[2] + a[2]*b[0] + a[3]*b[1] - a[1]*b[3];
    out[3] = a[0]*b[3] + a[3]*b[0] + a[1]*b[2] - a[2]*b[1];
}

void quatProdConst(float *in, float K, float *out){
    dsps_mulc_f32_ae32(in, out, 4, K, 1, 1);
}
