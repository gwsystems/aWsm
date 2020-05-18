/* Benchmark: Benchmark TinyEKF on Arduino/Teensy.  
 *
 * Copyright (C) 2015 Simon D. Levy
 *
 * MIT License
 */


#define Nsta 5     // states, will also be measurement values M
#define Mobs Nsta

#include "TinyEKF.h"

#define DEBUG   // comment this out to do timing

class Fuser : public TinyEKF {

    public:

        Fuser()
        {            
            // We approximate the process noise using a small constant
            for (int j=0; j<Nsta; ++j)
                this->setQ(j, j, .0001);

            // Same for measurement noise
            for (int j=0; j<Nsta; ++j)
                this->setR(j, j, .0001);
        }

    protected:

        virtual void model(double fx[Nsta], double F[Nsta][Nsta], double hx[Mobs], double H[Mobs][Nsta])
        {
            
            for (int j=0; j<Nsta; ++j) {

                // Process model is f(x) = x
                fx[j] = x[j];

                // So process model Jacobian is identity matrix
                F[j][j] = 1;

                // Mobseasurement function
                hx[j] = this->x[j]; 

                // Jacobian of measurement function
                H[j][j] = 1;  
            }
        }
};

extern "C" {
void __init_libc(int*, char*);
void double_out(double d);
}

void main_cpp() {
    Fuser ekf = Fuser();
    double z[Mobs];

    for (int j=0; j<Nsta; ++j)
        z[j] = j;

    for (int count=0; count <= 2000; count++) {
        ekf.step(z);

        #ifdef DEBUG
            for (int j=0; j<Nsta; ++j) {
//                double_out(ekf.getX(j));
                printf("%f", ekf.getX(j));
                printf(" ");
            }
            printf("\n");
        #else
            if (count % 1000 == 0) {
                printf("N = M = ");
                printf("%d", Nsta);
                printf(" : ");
                printf("%d", count);
                printf(" updates per second");
            }
        #endif
    }
}

// Elf auxilary vector values (see google for what those are)
#define AT_NULL		0
#define AT_IGNORE	1
#define AT_EXECFD	2
#define AT_PHDR		3
#define AT_PHENT	4
#define AT_PHNUM	5
#define AT_PAGESZ	6
#define AT_BASE		7
#define AT_FLAGS	8
#define AT_ENTRY	9
#define AT_NOTELF	10
#define AT_UID		11
#define AT_EUID		12
#define AT_GID		13
#define AT_EGID		14
#define AT_CLKTCK	17
#define	AT_SECURE	23
#define AT_BASE_PLATFORM 24
#define AT_RANDOM	25


int env_vec[] = {
    // Env variables would live here, but we don't supply any
    0,
    // We supply only the bare minimum AUX vectors
    AT_PAGESZ,
    128,
    AT_UID,
    123,
    AT_EUID,
    123,
    AT_GID,
    123,
    AT_EGID,
    123,
    AT_SECURE,
    0,
    AT_RANDOM,
    (int) 0xCAFEBABE, // It's pretty stupid to use rand here, but w/e
    0,
};

char* program_name = "tiny_efk";

void init_libc() {
//    __init_libc(&env_vec[0], program_name);
}


extern "C" {
    int main() {
        init_libc();
        main_cpp();
        return 0;
    }
}
