// Copyright 2023 Novostroev Ivan
#include "task_3/novostroev_i_matrix_mult_strassen/strassen.h"

std::vector<double> genVec(int n) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 9.0);
    std::vector<double> vec(n);

    for (int i = 0; i < n; i++)
        vec[i] = dis(gen);

    return vec;
}

std::vector<double> mult(const std::vector<double>& A,
                               const std::vector<double>& B, int n) {
    std::vector<double> C(n * n, 0);

    for (std::size_t i = 0; i < n; i++) {
        for (std::size_t j = 0; j < n; j++) {
            for (std::size_t k = 0; k < n; k++) {
                C[i * n + j] += A.at(i * n + k) * B.at(k * n + j);
            }
        }
    }
    return C;
}

std::vector<double> add(std::vector<double> A, std::vector<double> B) {
    std::vector<double> C(A.size());
    for (int i = 0; i < A.size(); i++) C[i] = A[i] + B[i];
    return C;
}

std::vector<double> sub(const std::vector<double>& A,
 const std::vector<double>& B) {
    std::vector<double> C(A.size());
    for (int i = 0; i < A.size(); i++) C[i] = A[i] - B[i];
    return C;
}

std::vector<double> strassen(const std::vector<double>& A,
 const std::vector<double>& B, std::size_t n) {
    if (n == 1) {
        std::vector<double> C = {A[0] * B[0]};
        return C;
    }

    std::size_t m = n / 2;
    std::vector<double> A11(m * m), A12(m * m), A21(m * m), A22(m * m);
    std::vector<double> B11(m * m), B12(m * m), B21(m * m), B22(m * m);
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            A11[i * m + j] = A[i * n + j];
            A12[i * m + j] = A[i * n + (j + m)];
            A21[i * m + j] = A[(i + m) * n + j];
            A22[i * m + j] = A[(i + m) * n + (j + m)];
            B11[i * m + j] = B[i * n + j];
            B12[i * m + j] = B[i * n + (j + m)];
            B21[i * m + j] = B[(i + m) * n + j];
            B22[i * m + j] = B[(i + m) * n + (j + m)];
        }
    }
    std::vector<double> P1 = strassen(A11, sub(B12, B22), m);
    std::vector<double> P2 = strassen(add(A11, A12), B22, m);
    std::vector<double> P3 = strassen(add(A21, A22), B11, m);
    std::vector<double> P4 = strassen(A22, sub(B21, B11), m);
    std::vector<double> P5 = strassen(add(A11, A22), add(B11, B22), m);
    std::vector<double> P6 = strassen(sub(A12, A22), add(B21, B22), m);
    std::vector<double> P7 = strassen(sub(A11, A21), add(B11, B12), m);

    std::vector<double> C(n * n);
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            C[i * n + j] =
                P5[i * m + j] + P4[i * m + j] - P2[i * m + j] + P6[i * m + j];
            C[i * n + (j + m)] = P1[i * m + j] + P2[i * m + j];
            C[(i + m) * n + j] = P3[i * m + j] + P4[i * m + j];
            C[(i + m) * n + (j + m)] =
                P5[i * m + j] + P1[i * m + j] - P3[i * m + j] - P7[i * m + j];
        }
    }

    return C;
}

std::vector<double> strassenMPI(const std::vector<double>& A,
                                const std::vector<double>& B,
                                std::size_t n, int rank, int size) {
    std::size_t m = n / 2;
    std::vector<double> A_local(m * m), B_local(m * m), C_local(m * m, 0.0);

    MPI_Scatter(A.data(), m * m,
     MPI_DOUBLE, A_local.data(), m * m,
      MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(B.data(), m * m,
     MPI_DOUBLE, B_local.data(), m * m,
      MPI_DOUBLE, 0, MPI_COMM_WORLD);

    std::vector<double> P_local = strassen(A_local, B_local, m);

    MPI_Gather(P_local.data(), m * m,
     MPI_DOUBLE, C_local.data(), m * m,
      MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        std::vector<double> C(n * n);
        for (int i = 0; i < size; ++i) {
            for (std::size_t j = 0; j < m; ++j) {
                for (std::size_t k = 0; k < m; ++k) {
                    C[((i / (n / m)) * m + j) * n + (i % (n / m) * m + k)]
                     = C_local[i * m * m + j * m + k];
                }
            }
        }
        return C;
    }

    return std::vector<double>();
}
