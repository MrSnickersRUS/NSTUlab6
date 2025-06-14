using System;
using System.Linq; // Используется только для простого создания вектора перестановок

public class SlaeSolver
{
    // Функция для печати матрицы с заданным именем
    public static void PrintMatrix(double[][] matrix, string name)
    {
        Console.WriteLine($"{name} =");
        foreach (var row in matrix)
        {
            foreach (var val in row)
            {
                // Форматирование: F6 - шесть знаков после запятой, 10 - общая ширина поля
                Console.Write($"{val,10:F6} ");
            }
            Console.WriteLine();
        }
        Console.WriteLine();
    }

    // Функция для печати вектора с заданным именем
    public static void PrintVector(double[] vector, string name)
    {
        Console.WriteLine($"{name} = [{string.Join(", ", vector.Select(v => v.ToString("F6")))}]");
        Console.WriteLine();
    }

    // Выполняет LU-разложение матрицы с частичным выбором главного элемента (пайвотингом)
    // Возвращает кортеж с матрицами L, U и вектором перестановок P
    public static (double[][] L, double[][] U, int[] P) LuDecomposition(double[][] aInput)
    {
        int n = aInput.Length;

        // Создаем копию матрицы A для U, чтобы не изменять исходную
        double[][] uMatrix = new double[n][];
        for (int i = 0; i < n; i++)
        {
            uMatrix[i] = (double[])aInput[i].Clone();
        }

        double[][] lMatrix = new double[n][];
        for (int i = 0; i < n; i++)
        {
            lMatrix[i] = new double[n];
        }

        int[] pVector = Enumerable.Range(0, n).ToArray();

        for (int i = 0; i < n; i++)
        {
            lMatrix[i][i] = 1.0;

            // Частичный выбор главного элемента
            int pivotRow = i;
            for (int k = i + 1; k < n; k++)
            {
                if (Math.Abs(uMatrix[k][i]) > Math.Abs(uMatrix[pivotRow][i]))
                {
                    pivotRow = k;
                }
            }

            if (pivotRow != i)
            {
                // Меняем местами строки в U
                (uMatrix[i], uMatrix[pivotRow]) = (uMatrix[pivotRow], uMatrix[i]);
                // Меняем местами элементы в векторе перестановок
                (pVector[i], pVector[pivotRow]) = (pVector[pivotRow], pVector[i]);
                // Меняем местами уже вычисленные элементы в L
                for (int k = 0; k < i; k++)
                {
                    (lMatrix[i][k], lMatrix[pivotRow][k]) = (lMatrix[pivotRow][k], lMatrix[i][k]);
                }
            }

            // Проверка на вырожденность матрицы
            if (Math.Abs(uMatrix[i][i]) < 1e-12)
                throw new InvalidOperationException("Матрица вырождена или близка к вырожденной. LU-разложение невозможно.");

            for (int j = i + 1; j < n; j++)
            {
                lMatrix[j][i] = uMatrix[j][i] / uMatrix[i][i];
                for (int k = i; k < n; k++)
                {
                    uMatrix[j][k] -= lMatrix[j][i] * uMatrix[i][k];
                }
            }
        }
        return (lMatrix, uMatrix, pVector);
    }

    // Решает систему Ly = Pb (прямой ход)
    public static double[] ForwardSubstitution(double[][] lMatrix, double[] bPermuted)
    {
        int n = lMatrix.Length;
        double[] yVector = new double[n];
        for (int i = 0; i < n; i++)
        {
            double sumLy = 0.0;
            for (int j = 0; j < i; j++)
            {
                sumLy += lMatrix[i][j] * yVector[j];
            }
            yVector[i] = (bPermuted[i] - sumLy) / lMatrix[i][i];
        }
        return yVector;
    }

    // Решает систему Ux = y (обратный ход)
    public static double[] BackwardSubstitution(double[][] uMatrix, double[] yVector)
    {
        int n = uMatrix.Length;
        double[] xVector = new double[n];
        for (int i = n - 1; i >= 0; i--)
        {
            double sumUx = 0.0;
            for (int j = i + 1; j < n; j++)
            {
                sumUx += uMatrix[i][j] * xVector[j];
            }
            if (Math.Abs(uMatrix[i][i]) < 1e-12)
            {
                throw new InvalidOperationException("Деление на ноль при обратном ходе (элемент U[i][i] равен нулю).");
            }
            xVector[i] = (yVector[i] - sumUx) / uMatrix[i][i];
        }
        return xVector;
    }

    // Проверяет матрицу на диагональное преобладание
    public static bool CheckDiagonalDominance(double[][] aInput)
    {
        int n = aInput.Length;
        for (int i = 0; i < n; i++)
        {
            double diagonalElement = Math.Abs(aInput[i][i]);
            double sumOffDiagonal = 0.0;
            for (int j = 0; j < n; j++)
            {
                if (i != j) sumOffDiagonal += Math.Abs(aInput[i][j]);
            }
            if (diagonalElement <= sumOffDiagonal)
            {
                return false;
            }
        }
        return true;
    }

    // Решает систему Ax = b методом простых итераций
    // Возвращает вектор решения x, или null, если не сошлось
    public static double[]? SimpleIterationMethod(double[][] aInput, double[] bInput, double tolerance, int maxIterations)
    {
        int n = aInput.Length;
        double[] xCurrent = new double[n]; // Инициализируется нулями
        double[] xNext = new double[n];

        // Преобразуем Ax = b к виду x = Cx + f
        double[][] cMatrix = new double[n][];
        for (int i = 0; i < n; i++) cMatrix[i] = new double[n];
        
        double[] fVector = new double[n];

        for (int i = 0; i < n; i++)
        {
            if (Math.Abs(aInput[i][i]) < 1e-12)
            {
                Console.Error.WriteLine($"Ошибка: Диагональный элемент A[{i}][{i}] равен нулю. Метод простых итераций неприменим без перестановок.");
                return null;
            }
            fVector[i] = bInput[i] / aInput[i][i];
            for (int j = 0; j < n; j++)
            {
                cMatrix[i][j] = (i == j) ? 0.0 : -aInput[i][j] / aInput[i][i];
            }
        }

        Console.WriteLine("Преобразованная система x = Cx + f:");
        PrintMatrix(cMatrix, "Матрица C");
        PrintVector(fVector, "Вектор f");

        for (int iter = 0; iter < maxIterations; iter++)
        {
            double maxDifference = 0.0;
            for (int i = 0; i < n; i++)
            {
                double sumCx = 0.0;
                for (int j = 0; j < n; j++)
                {
                    sumCx += cMatrix[i][j] * xCurrent[j];
                }
                xNext[i] = sumCx + fVector[i];
                maxDifference = Math.Max(maxDifference, Math.Abs(xNext[i] - xCurrent[i]));
            }
            
            xCurrent = (double[])xNext.Clone();

            if (maxDifference < tolerance)
            {
                Console.WriteLine($"Метод простых итераций сошелся за {iter + 1} итераций.");
                return xCurrent;
            }
        }

        Console.Error.WriteLine($"Метод простых итераций не сошелся за {maxIterations} итераций.");
        return null;
    }

    public static void Main(string[] args)
    {
        // Параметры для варианта 11
        const double M_VAL = 0.89;
        const double N_VAL = 0.08;
        const double P_VAL = -1.21;

        // Матрица A и вектор b, заданные для варианта 11
        double[][] aMatrix = new double[][]
        {
            new double[] { M_VAL, -0.04,  0.21, -18.0 },
            new double[] { 0.25,  -1.23, N_VAL, -0.09 },
            new double[] { -0.21, N_VAL,  0.8,  -0.13 },
            new double[] { 0.15,  -1.31,  0.06, P_VAL }
        };

        double[] bVector = { -1.24, P_VAL, 2.56, M_VAL };

        Console.WriteLine("Исходная система Ax = b:");
        PrintMatrix(aMatrix, "Матрица A");
        PrintVector(bVector, "Вектор b");

        try
        {
            // Выполняем LU-разложение
            var (lMatrix, uMatrix, pVector) = LuDecomposition(aMatrix);

            Console.WriteLine("LU-разложение с частичным выбором главного элемента:");
            PrintMatrix(lMatrix, "Матрица L");
            PrintMatrix(uMatrix, "Матрица U");

            // Применяем перестановки к вектору b
            double[] bPermuted = new double[bVector.Length];
            for (int i = 0; i < bVector.Length; i++)
            {
                bPermuted[i] = bVector[pVector[i]];
            }
            Console.WriteLine("Вектор b после перестановок (P*b):");
            PrintVector(bPermuted, "b_permuted");

            // 1. Решаем Ly = Pb
            double[] yVector = ForwardSubstitution(lMatrix, bPermuted);
            Console.WriteLine("Результат прямого хода (вектор y):");
            PrintVector(yVector, "y");

            // 2. Решаем Ux = y
            double[] xSolutionLU = BackwardSubstitution(uMatrix, yVector);
            Console.WriteLine("Решение системы методом LU-разложения (вектор x_LU):");
            PrintVector(xSolutionLU, "x_LU");

            // Проверка точности LU-метода
            Console.WriteLine("Проверка решения LU-методом");
            double[] axCheckLU = new double[aMatrix.Length];
            for (int i = 0; i < aMatrix.Length; i++)
            {
                for (int j = 0; j < aMatrix[i].Length; j++)
                {
                    axCheckLU[i] += aMatrix[i][j] * xSolutionLU[j];
                }
            }
            PrintVector(axCheckLU, "A*x_LU");

            double maxDiffLU = 0.0;
            for (int i = 0; i < bVector.Length; i++)
            {
                maxDiffLU = Math.Max(maxDiffLU, Math.Abs(axCheckLU[i] - bVector[i]));
            }
            const double EPS = 1e-3;
            // Форматирование: E - научная нотация
            Console.WriteLine($"Максимальная разница между A*x_LU и b: {maxDiffLU:E}");
            if (maxDiffLU < EPS)
                Console.WriteLine($"Решение LU-методом найдено с требуемой точностью (<= {EPS}).\n");
            else
                Console.WriteLine($"Решение LU-методом найдено, но может не соответствовать требуемой точности (требуется <= {EPS}, получено {maxDiffLU}).\n");

            // Метод простых итераций с проверкой диагонального преобладания
            if (CheckDiagonalDominance(aMatrix))
            {
                Console.WriteLine("Матрица A обладает диагональным преобладанием. Метод простых итераций должен сойтись.");
                double[]? xSolutionIterative = SimpleIterationMethod(aMatrix, bVector, 1e-6, 1000);
                if (xSolutionIterative != null)
                {
                    Console.WriteLine("Решение методом простых итераций (вектор x_Iterative):");
                    PrintVector(xSolutionIterative, "x_Iterative");

                    // Проверка точности итерационного метода
                    double[] axCheckIterative = new double[aMatrix.Length];
                    for (int i = 0; i < aMatrix.Length; i++)
                    {
                        for (int j = 0; j < aMatrix[i].Length; j++)
                        {
                            axCheckIterative[i] += aMatrix[i][j] * xSolutionIterative[j];
                        }
                    }
                    PrintVector(axCheckIterative, "A*x_Iterative");

                    double maxDiffIterative = 0.0;
                    for (int i = 0; i < bVector.Length; i++)
                    {
                        maxDiffIterative = Math.Max(maxDiffIterative, Math.Abs(axCheckIterative[i] - bVector[i]));
                    }
                    Console.WriteLine($"Максимальная разница между A*x_Iterative и b: {maxDiffIterative:E}");
                    if (maxDiffIterative < EPS)
                    {
                        Console.WriteLine($"Решение методом простых итераций найдено с требуемой точностью (<= {EPS}).\n");
                    }
                    else
                    {
                        Console.WriteLine($"Решение методом простых итераций найдено, но может не соответствовать требуемой точности (требуется <= {EPS}, получено {maxDiffIterative}).\n");
                    }
                }
            }
            else
            {
                Console.WriteLine("Матрица A НЕ обладает диагональным преобладанием. Метод простых итераций может не сойтись или сойдется медленно.");
                double[]? xSolutionIterative = SimpleIterationMethod(aMatrix, bVector, 1e-6, 1000);
                if (xSolutionIterative != null)
                {
                    Console.WriteLine("Решение методом простых итераций (вектор x_Iterative):");
                    PrintVector(xSolutionIterative, "x_Iterative");
                }
            }
        }
        catch (Exception e)
        {
            Console.Error.WriteLine($"Ошибка выполнения: {e.Message}");
        }
    }
}