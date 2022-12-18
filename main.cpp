#include <iostream>
#include <fstream>
#include "mpi.h"
#include <cmath>
#include <chrono>

double Ax = -0.353;
double Bx = 0.353;
double Ay = 0.3;
double By = 0.3;
double C = 3 * M_PI / 8;
double eps = 1e-3;

double f(double *x, int thr) {
    switch(thr){
        case 0:
            return x[0] + x[2] * cos(3 * M_PI / 2 - x[3]) - Ax;
        case 1:
            return x[1] + x[2] * cos(3 * M_PI / 2 + x[4]) - Bx;
        case 2:
            return x[2] + x[2] * sin(3 * M_PI / 2 - x[3]) - Ay;
        case 3:
            return (x[3] + x[4]) * x[2] + (x[1] - x[0]) - C;
        case 4:
            return x[2] + x[2] * sin(3 * M_PI / 2 + x[4]) - By;
        default:
            return 0;
    }
}

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::nanoseconds ns;

int main(int argc, char **argv) {   
    double* x0 = new double[5]{-0.1, 0.1, 0, 2, 2};
    double* F = new double[5]{0, 0, 0, 0, 0};
    int thr;
    int size;
    int i = 0;

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &thr);
    
    double delta = 0.001;
    auto start = Time::now();
    for(;; i++) {
        if (thr < 5) {
            if (i > 0) {
                MPI_Recv(x0, 5, MPI_DOUBLE, 5, 0, MPI_COMM_WORLD, &status);
            }
            F[thr] = f(x0, thr);
            x0[thr] = x0[thr] - delta * F[thr];
            MPI_Send(&x0[thr], 1, MPI_DOUBLE, 5, 0, MPI_COMM_WORLD);

        } else {
            for(int i = 0; i < 5; ++i)
                MPI_Recv(&x0[i], 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);

            for (int i = 0; i < 5; ++i) {
                std::cout << x0[i] << " ";
            }
            std::cout << "\n";

            if ((f(x0, 0) < eps) && (f(x0, 1) < eps) && (f(x0, 2) < eps) &&
                 (f(x0, 3) < eps) && (f(x0, 4) < eps)) {
                auto end = Time::now();
                std::ofstream fout;
                fout.open("results.txt", std::ios_base::app);
                fout << std::chrono::duration_cast<ns>(end - start).count() * 1e-9 << " ";
                fout << delta << " ";
                fout << x0[0] << " " << x0[1] << " "  << x0[2] << " "  << x0[3] << " "  << x0[4] << " "  << "\n";
                fout.close();
                return 0;
            }
            for(int i = 0; i < 5; ++i)
                MPI_Send(x0, 5, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);           
        }
    }
    MPI_Finalize();
    return 0;
}
