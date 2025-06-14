#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <tuple>

using namespace std;

using Matrix = vector<vector<double>>;
using Vector = vector<double>;

// Функция для печати матрицы с заданным именем
void printMatrix(const Matrix& mat, const string& name) {
    cout << name << " =\n";
    for (const auto& row : mat) {
        for (double val : row) {
            cout << fixed << setprecision(6) << setw(10) << val << " ";
        }
        cout << "\n";
    }
    cout << "\n";
}

// Функция для печати вектора с заданным именем
void printVector(const Vector& vec, const string& name) {
    cout << name << " = [";
    for (size_t i = 0; i < vec.size(); ++i) {
        cout << fixed << setprecision(6) << vec[i];
        if (i < vec.size() - 1) {
            cout << ", ";
        }
    }
    cout << "]\n\n";
}

// Выполняет LU-разложение матрицы с частичным выбором главного элемента
tuple<Matrix, Matrix, Vector> luDecomposition(const Matrix& aInput) {
    int n = aInput.size();
    Matrix lMatrix(n, Vector(n, 0.0));
    Matrix uMatrix = aInput;
    Vector pVector(n);
    iota(pVector.begin(), pVector.end(), 0);

    for (int i = 0; i < n; ++i) {
        lMatrix[i][i] = 1.0;

        // Частичный выбор главного элемента
        int pivotRow = i;
        for (int k = i + 1; k < n; ++k) {
            if (fabs(uMatrix[k][i]) > fabs(uMatrix[pivotRow][i])) {
                pivotRow = k;
            }
        }
        if (pivotRow != i) {
            swap(uMatrix[i], uMatrix[pivotRow]);
            swap(pVector[i], pVector[pivotRow]);
            for (int k = 0; k < i; ++k) {
                swap(lMatrix[i][k], lMatrix[pivotRow][k]);
            }
        }

        // Проверка на вырожденность матрицы
        if (fabs(uMatrix[i][i]) < 1e-12)
            throw runtime_error("Матрица вырождена или близка к вырожденной. LU-разложение невозможно.");

        for (int j = i + 1; j < n; ++j) {
            lMatrix[j][i] = uMatrix[j][i] / uMatrix[i][i];
            for (int k = i; k < n; ++k) {
                uMatrix[j][k] -= lMatrix[j][i] * uMatrix[i][k];
            }
        }
    }
    return {lMatrix, uMatrix, pVector};
}

// Решает систему Ly = Pb
Vector forwardSubstitution(const Matrix& lMatrix, const Vector& bPermuted) {
    int n = lMatrix.size();
    Vector yVector(n);
    for (int i = 0; i < n; ++i) {
        double sumLy = 0.0;
        for (int j = 0; j < i; ++j) {
            sumLy += lMatrix[i][j] * yVector[j];
        }
        yVector[i] = (bPermuted[i] - sumLy) / lMatrix[i][i];
    }
    return yVector;
}

// Решает систему Ux = y
Vector backwardSubstitution(const Matrix& uMatrix, const Vector& yVector) {
    int n = uMatrix.size();
    Vector xVector(n);
    for (int i = n - 1; i >= 0; --i) {
        double sumUx = 0.0;
        for (int j = i + 1; j < n; ++j) {
            sumUx += uMatrix[i][j] * xVector[j];
        }
        if (fabs(uMatrix[i][i]) < 1e-12) {
             throw runtime_error("Деление на ноль при обратном ходе (элемент U[i][i] равен нулю).");
        }
        xVector[i] = (yVector[i] - sumUx) / uMatrix[i][i];
    }
    return xVector;
}

// Проверяет матрицу на диагональное преобладание
bool checkDiagonalDominance(const Matrix& aInput) {
    int n = aInput.size();
    for (int i = 0; i < n; ++i) {
        double diagonalElement = fabs(aInput[i][i]);
        double sumOffDiagonal = 0.0;
        for (int j = 0; j < n; ++j) {
            if (i != j) sumOffDiagonal += fabs(aInput[i][j]);
        }
        if (diagonalElement <= sumOffDiagonal) {
            return false;
        }
    }
    return true;
}

// Решает систему Ax = b методом простых итераций
// Возвращает вектор решения x, или пустой вектор, если не сошлось
Vector simpleIterationMethod(const Matrix& aInput, const Vector& bInput, double tolerance, int maxIterations) {
    int n = aInput.size();
    Vector xCurrent(n, 0.0);
    Vector xNext(n);

    // Преобразуем Ax = b к виду x = Cx + f
    Matrix cMatrix(n, Vector(n));
    Vector fVector(n);

    for (int i = 0; i < n; ++i) {
        if (fabs(aInput[i][i]) < 1e-12) {
            cerr << "Ошибка: Диагональный элемент A[" << i << "][" << i << "] равен нулю. Метод простых итераций неприменим без перестановок." << endl;
            return {};
        }
        fVector[i] = bInput[i] / aInput[i][i];
        for (int j = 0; j < n; ++j) {
            if (i == j) {
                cMatrix[i][j] = 0.0;
            } else {
                cMatrix[i][j] = -aInput[i][j] / aInput[i][i];
            }
        }
    }

    cout << "Преобразованная система x = Cx + f:" << endl;
    printMatrix(cMatrix, "Матрица C");
    printVector(fVector, "Вектор f");

    for (int iter = 0; iter < maxIterations; ++iter) {
        double maxDifference = 0.0;
        for (int i = 0; i < n; ++i) {
            double sumCx = 0.0;
            for (int j = 0; j < n; ++j) {
                sumCx += cMatrix[i][j] * xCurrent[j];
            }
            xNext[i] = sumCx + fVector[i];
            maxDifference = max(maxDifference, fabs(xNext[i] - xCurrent[i]));
        }
        xCurrent = xNext;
        if (maxDifference < tolerance) {
            cout << "Метод простых итераций сошелся за " << iter + 1 << " итераций." << endl;
            return xCurrent;
        }
    }
    cerr << "Метод простых итераций не сошелся за " << maxIterations << " итераций." << endl;
    return {};
}


int main() {
    // Параметры для варианта 11 (ваши параметры из файла)
    const double M_VAL = 0.89;
    const double N_VAL = 0.08;
    const double P_VAL = -1.21;

    // Матрица A и вектор b, заданные для варианта 11
    Matrix aMatrix = {
        {M_VAL,   -0.04,   0.21,    -18.0},
        {0.25,    -1.23,   N_VAL,   -0.09},
        {-0.21,   N_VAL,   0.8,     -0.13},
        {0.15,    -1.31,   0.06,    P_VAL}
    };

    Vector bVector = {-1.24, P_VAL, 2.56, M_VAL};

    cout << "Исходная система Ax = b:" << endl;
    printMatrix(aMatrix, "Матрица A");
    printVector(bVector, "Вектор b");

    try {
        // Метод LU-разложения с частичным выбором главного элемента
        Matrix lMatrix, uMatrix;
        Vector pVector;

        // Выполняем LU-разложение
        tie(lMatrix, uMatrix, pVector) = luDecomposition(aMatrix);

        cout << "LU-разложение с частичным выбором главного элемента:\n";
        printMatrix(lMatrix, "Матрица L");
        printMatrix(uMatrix, "Матрица U");

        // Применяем перестановки к вектору b
        Vector bPermuted(bVector.size());
        for(size_t i = 0; i < bVector.size(); ++i) {
            bPermuted[i] = bVector[static_cast<int>(pVector[i])];
        }
        cout << "Вектор b после перестановок (P*b):\n";
        printVector(bPermuted, "b_permuted");


        // 1. Решаем Ly = Pb
        Vector yVector = forwardSubstitution(lMatrix, bPermuted);
        cout << "Результат прямого хода (вектор y):\n";
        printVector(yVector, "y");

        // 2. Решаем Ux = y
        Vector xSolutionLU = backwardSubstitution(uMatrix, yVector);
        cout << "Решение системы методом LU-разложения (вектор x_LU):" << endl;
        printVector(xSolutionLU, "x_LU");

        // Проверка точности LU-метода
        cout << "Проверка решения LU-методом" << endl;
        Vector axCheckLU(aMatrix.size(), 0.0);
        for (size_t i = 0; i < aMatrix.size(); ++i) {
            for (size_t j = 0; j < aMatrix[i].size(); ++j) {
                axCheckLU[i] += aMatrix[i][j] * xSolutionLU[j];
            }
        }
        printVector(axCheckLU, "A*x_LU");

        double maxDiffLU = 0.0;
        for(size_t i = 0; i < bVector.size(); ++i) {
            maxDiffLU = max(maxDiffLU, fabs(axCheckLU[i] - bVector[i]));
        }
        const double EPS = 1e-3;
        cout << "Максимальная разница между A*x_LU и b: " << scientific << maxDiffLU << fixed << endl;
        if (maxDiffLU < EPS) cout << "Решение LU-методом найдено с требуемой точностью (<= " << EPS << ").\n";
        else cout << "Решение LU-методом найдено, но может не соответствовать требуемой точности (требуется <= " << EPS << ", получено " << maxDiffLU << ").\n";

        // Метод простых итераций с проверкой диагонального преобладания
        if (checkDiagonalDominance(aMatrix)) {
            cout << "Матрица A обладает диагональным преобладанием. Метод простых итераций должен сойтись." << endl;
            Vector xSolutionIterative = simpleIterationMethod(aMatrix, bVector, 1e-6, 1000);
            if (!xSolutionIterative.empty()) {
                cout << "Решение методом простых итераций (вектор x_Iterative):\n";
                printVector(xSolutionIterative, "x_Iterative");

                // Проверка точности итерационного метода
                Vector axCheckIterative(aMatrix.size(), 0.0);
                for (size_t i = 0; i < aMatrix.size(); ++i) {
                    for (size_t j = 0; j < aMatrix[i].size(); ++j) {
                        axCheckIterative[i] += aMatrix[i][j] * xSolutionIterative[j];
                    }
                }
                printVector(axCheckIterative, "A*x_Iterative");

                double maxDiffIterative = 0.0;
                for(size_t i = 0; i < bVector.size(); ++i) {
                    maxDiffIterative = max(maxDiffIterative, fabs(axCheckIterative[i] - bVector[i]));
                }
                cout << "Максимальная разница между A*x_Iterative и b: " << scientific << maxDiffIterative << fixed << endl;
                if (maxDiffIterative < EPS) {
                    cout << "Решение методом простых итераций найдено с требуемой точностью (<= " << EPS << ").\n";
                } else {
                    cout << "Решение методом простых итераций найдено, но может не соответствовать требуемой точности (требуется <= " << EPS << ", получено " << maxDiffIterative << ").\n";
                }

            }
        } else {
            cout << "Матрица A НЕ обладает диагональным преобладанием. Метод простых итераций может не сойтись или сойдется медленно." << endl;
            Vector xSolutionIterative = simpleIterationMethod(aMatrix, bVector, 1e-6, 1000);
             if (!xSolutionIterative.empty()) {
                cout << "Решение методом простых итераций (вектор x_Iterative):\n";
                printVector(xSolutionIterative, "x_Iterative");
            }
        }

    } catch (const exception& e) {
        cerr << "Ошибка выполнения: " << e.what() << endl;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        return 1;
    }

    return 0;
}