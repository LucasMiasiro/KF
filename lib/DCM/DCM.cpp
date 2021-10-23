#include "DCM.h"
#include "config.h"

#include "vMath.h"
#include <math.h> 

void DCM::getStates(float *eulerAngles, float *eulerRates){
    eulerAngles[0] = atan2(2.0*(q[2]*q[3] - q[0]*q[1]),
                            2.0*(q[0]*q[0] + q[3]*q[3]) - 1); //Phi

    eulerAngles[1] = -asin(2*(q[1]*q[3] + q[0]*q[2])); //Theta

    eulerAngles[2] = atan2(2.0*(q[1]*q[2] - q[0]*q[3]),
                            2.0*(q[0]*q[0] + q[1]*q[1]) - 1); //Psi
}

void DCM::update(float* A, float* G, float* M){
    initializeVariables(A, G, M);

    gyroPred();
    magAccelPred();

    filterFusion();
}

void DCM::initializeFilter(){
    const float r[3] = {0, 0, 1};
    const float psi = 0;

    q[0] = cos(psi/2);
    q[1] = -r[0]*sin(psi/2);
    q[2] = -r[1]*sin(psi/2);
    q[3] = -r[2]*sin(psi/2);

    normalizeQuat(q);
}

void DCM::filterFusion(){

    quatProdConst(grad_f_normalized, -beta, buf_q);
    quatAdd(buf_q, q_dot_G, q_dot);
    quatProdConst(q_dot, SYSTEM_SAMPLE_PERIOD_MS/1000.0f, buf_q);
    quatAdd(buf_q, q, q);

    normalizeQuat(q);
}

void DCM::magAccelPred(){
    matProd_4x6x1(&JT[0][0], f, grad_f_normalized);
    normalizeQuat(grad_f_normalized);
}

void DCM::gyroPred(){
    quatProd(q, G_q, buf_q);
    quatProdConst(buf_q, 0.5, q_dot_G);
}

void DCM::initializeVariables(float* A, float *G, float *M){
    G_q[1] = G[0];
    G_q[2] = G[1];
    G_q[3] = G[2];

    M_q[1] = M[0];
    M_q[2] = M[1];
    M_q[3] = M[2];
    normalizeQuat(M_q);

    A_q[1] = A[0];
    A_q[2] = A[1];
    A_q[3] = A[2];
    normalizeQuat(A_q);

#if COMPENSATE
    softIronCompensation();
    gyroBiasDriftCompensation(G);
#endif

    initializeAccelTransposedJacobian(JT, f);
    initializeMagTransposedJacobian(JT, f);
}

void DCM::softIronCompensation(){
    quatConj(q, buf_q);
    quatProd(M_q, buf_q, buf2_q);
    quatProd(q, buf2_q, buf_q);

    earthMagField[0] = sqrt(buf_q[1]*buf_q[1] + buf_q[2]+buf_q[2]);
    earthMagField[2] = buf_q[3];
}

void DCM::gyroBiasDriftCompensation(float *G){
}

void DCM::initializeMagTransposedJacobian(float Matrix[4][6], float* f){
    Matrix[0][3] = -2.0*q[2]*earthMagField[2];
    Matrix[1][3] = 2.0*q[3]*earthMagField[2];
    Matrix[2][3] = -2.0*q[0]*earthMagField[2] - 4.0*q[2]*earthMagField[0];
    Matrix[3][3] = 2.0*q[1]*earthMagField[2] - 4.0*q[3]*earthMagField[0];

    Matrix[0][4] = 2.0*q[1]*earthMagField[2] - 2.0*q[3]*earthMagField[0];
    Matrix[1][4] = 2.0*q[0]*earthMagField[2] + 2.0*q[2]*earthMagField[0];
    Matrix[2][4] = 2.0*q[3]*earthMagField[2] + 2.0*q[1]*earthMagField[0];
    Matrix[3][4] = 2.0*q[2]*earthMagField[2] - 2.0*q[0]*earthMagField[0];

    Matrix[0][5] = 2.0*q[2]*earthMagField[0];
    Matrix[1][5] = -4.0*q[1]*earthMagField[2] + 2.0*q[3]*earthMagField[0];
    Matrix[2][5] = -4.0*q[1]*earthMagField[2] + 2.0*q[0]*earthMagField[0];
    Matrix[3][5] = 2.0*q[1]*earthMagField[0];

    f[3] =  2.0*(0.5f - q[2]*q[2] - q[3]*q[3])*earthMagField[0]
            + 2.0*(q[1]*q[3] - q[0]*q[2])*earthMagField[2] - M_q[1];

    f[4] =  2.0*(q[1]*q[2] - q[0]*q[3])*earthMagField[0]
            + 2.0*(q[0]*q[1] + q[2]*q[3])*earthMagField[2] - M_q[2];

    f[5] =  2.0*(q[0]*q[2] + q[1]*q[3])*earthMagField[0]
            + 2.0*(0.5 - q[1]*q[1] - q[2]*q[2])*earthMagField[2] - M_q[3];
}

void DCM::initializeAccelTransposedJacobian(float Matrix[4][6], float *f){
    Matrix[0][0] = -2.0*q[2];
    Matrix[1][0] = 2.0*q[3];
    Matrix[2][0] = -2.0*q[0];
    Matrix[3][0] = 2.0*q[1];

    Matrix[0][1] = 2.0*q[1];
    Matrix[1][1] = 2.0*q[0];
    Matrix[2][1] = 2.0*q[3];
    Matrix[3][1] = 2.0*q[2];

    Matrix[0][2] = 0;
    Matrix[1][2] = -4.0*q[1];
    Matrix[2][2] = -4.0*q[1];
    Matrix[3][2] = 0;

    f[0] = 2.0*(q[1]*q[3] - q[0]*q[2]) - A_q[1];
    f[1] = 2.0*(q[0]*q[1] + q[2]*q[3]) - A_q[2];
    f[2] = 2.0*(0.5 - q[1]*q[1] - q[2]*q[2]) - A_q[3];
}

DCM::DCM(){
};
