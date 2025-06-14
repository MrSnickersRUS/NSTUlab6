using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

class Program
{
    // Глобальный генератор случайных чисел (для эффективности)
    static Random globalRandom = new Random();

    static int RandomNumGen(int minVal, int maxVal)
    {
        return globalRandom.Next(minVal, maxVal + 1);
    }

    static void ClearScreen()
    {
        Console.Clear();
    }

    static void SleepMilliseconds(int ms)
    {
        Thread.Sleep(ms);
    }

    static void DisplayGrid(char[,] grid, int rows, int cols)
    {
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                Console.Write(grid[i, j]);
            }
            Console.WriteLine();
        }
    }

    static int CountLiveNeighbours(char[,] grid, int r, int c, int rows, int cols)
    {
        int count = 0;
        for (int dr = -1; dr <= 1; ++dr)
        {
            for (int dc = -1; dc <= 1; ++dc)
            {
                if (dr == 0 && dc == 0) // Пропускаем саму клетку
                    continue;

                int nr = r + dr; // Соседняя строка
                int nc = c + dc; // Соседний столбец

                // Проверяем границы
                if (nr >= 0 && nr < rows && nc >= 0 && nc < cols)
                {
                    if (grid[nr, nc] == '0') count++;
                }
            }
        }
        return count;
    }

    static void CalculateNextGeneration(char[,] grid, int rows, int cols)
    {
        char[,] nextGrid = (char[,])grid.Clone(); // Создаем копию текущего состояния
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                int liveNeighbours = CountLiveNeighbours(grid, i, j, rows, cols);
                char currentCellState = grid[i, j];

                if (currentCellState == '0') // Если клетка жива
                {
                    // Правила для живой клетки
                    if (liveNeighbours < 2 || liveNeighbours > 3)
                        nextGrid[i, j] = ' '; // Умирает
                }
                else // Если клетка мертва (' ')
                {
                    // Правила для мертвой клетки
                    if (liveNeighbours == 3) nextGrid[i, j] = '0';
                }
            }
        }
        Array.Copy(nextGrid, grid, nextGrid.Length); // Обновляем основную сетку
    }

    static void SetupRandom(char[,] grid, int rows, int cols)
    {
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                grid[i, j] = (globalRandom.Next(0, 4) == 0 ? '0' : ' '); // 25% клеток живые
            }
        }
    }

    // Установка Switch Engine
    static void SetupSwitchEngine(char[,] grid, int startRow, int startColumn, int rows, int cols)
    {
        // Очищаем сетку
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                grid[i, j] = ' ';

        var pattern = new List<(int, int)>
        {
            (0, 3),
            (1, 0), (1, 3), (1, 5),
            (2, 0), (2, 1), (2, 5),
            (3, 5),
            (4, 4)
        };

        // Проверяем, помещается ли паттерн
        int maxRow = pattern.Max(p => p.Item1);
        int maxColumn = pattern.Max(p => p.Item2);

        if (startRow + maxRow < rows && startColumn + maxColumn < cols)
        {
            foreach (var p in pattern)
            {
                grid[startRow + p.Item1, startColumn + p.Item2] = '0';
            }
        }
        else
        {
            Console.WriteLine("Предупреждение: Switch Engine не помещается в указанные координаты.");
            Console.WriteLine("Попробуйте увеличить размер сетки (рекомендуется хотя бы 10x15 для наблюдения).");
        }
    }

    static void Main()
    {
        try
        {
            // Первая часть: работа с матрицей
            {
                Console.Write("Введите количество строк и столбцов матрицы: ");
                var input = Console.ReadLine().Split();
                int rows = int.Parse(input[0]);
                int cols = int.Parse(input[1]);

                if (rows <= 0 || cols <= 0)
                    throw new ArgumentException("Количество строк и столбцов должно быть больше нуля");

                int[,] matrix = new int[rows, cols];
                for (int i = 0; i < rows; ++i)
                {
                    for (int j = 0; j < cols; ++j)
                    {
                        matrix[i, j] = RandomNumGen(-50, 50);
                    }
                }

                long sum = 0;
                var newArray = new List<int>();
                for (int i = 1; i < rows; i += 2) // Нечетные строки
                {
                    for (int j = 0; j < cols; j += 2) // Четные столбцы
                    {
                        if (matrix[i, j] >= 0)
                        {
                            sum += matrix[i, j];
                            newArray.Add(matrix[i, j]);
                        }
                    }
                }

                Console.WriteLine($"Сумма неотрицательных элементов на пересечении нечетных строк и четных столбцов: {sum}");
                Console.WriteLine("Массив из этих элементов: " + string.Join(" ", newArray));
            }

            // Вторая часть: квадратная матрица
            {
                Console.Write("Введите порядок квадратной матрицы: ");
                int n = int.Parse(Console.ReadLine());
                if (n <= 0)
                    throw new ArgumentException("Порядок матрицы должен быть положительным целым числом.");

                int[,] matrix = new int[n, n];
                Console.WriteLine($"Сгенерированная квадратная матрица ({n}x{n}):");
                for (int i = 0; i < n; ++i)
                {
                    for (int j = 0; j < n; ++j)
                    {
                        matrix[i, j] = RandomNumGen(100, 200);
                    }
                }

                var indexedSums = new List<(long Sum, int Index)>();
                Console.WriteLine("\nСуммы элементов по строкам:");
                for (int i = 0; i < n; ++i)
                {
                    long currentSum = 0;
                    for (int j = 0; j < n; ++j)
                        currentSum += matrix[i, j];
                    
                    indexedSums.Add((currentSum, i));
                }

                int secondMaxOriginalIndex = -1;

                if (n >= 2)
                {
                    // Сортируем по убыванию сумм
                    indexedSums = indexedSums.OrderByDescending(x => x.Sum).ToList();

                    long maxSum = indexedSums[0].Sum;

                    // Ищем первую сумму, которая меньше максимальной
                    for (int k = 1; k < indexedSums.Count; ++k)
                    {
                        if (indexedSums[k].Sum < maxSum)
                        {
                            secondMaxOriginalIndex = indexedSums[k].Index;
                            break;
                        }
                    }
                }
                else
                {
                    Console.WriteLine("\nНедостаточно строк (N < 2) для определения второй максимальной суммы.");
                }

                if (secondMaxOriginalIndex != -1)
                {
                    Console.WriteLine($"\nМассив, инициализированный строкой с второй максимальной суммой (исходный индекс {secondMaxOriginalIndex}):");
                    var secMaxRow = new int[n];
                    for (int j = 0; j < n; ++j)
                        secMaxRow[j] = matrix[secondMaxOriginalIndex, j];
                    
                    Console.WriteLine(string.Join(", ", secMaxRow));
                }
                else if (n >= 2)
                {
                    Console.WriteLine("\nНе удалось найти строку со второй по величине суммой (возможно, все суммы строк одинаковы).");
                }
            }

            // Третья часть: Игра "Жизнь"
            {
                Console.Write("Введите количество строк сетки: ");
                int rows = int.Parse(Console.ReadLine());
                if (rows <= 0)
                    throw new ArgumentException("Количество строк должно быть положительным числом.");

                Console.Write("Введите количество столбцов сетки: ");
                int cols = int.Parse(Console.ReadLine());
                if (cols <= 0)
                    throw new ArgumentException("Количество столбцов должно быть положительным числом.");

                Console.Write("Введите количество поколений для симуляции: ");
                int generations = int.Parse(Console.ReadLine());
                if (generations <= 0)
                    throw new ArgumentException("Количество поколений должно быть положительным числом.");

                Console.Write("Введите задержку между поколениями (в миллисекундах, например, 100): ");
                int delay = int.Parse(Console.ReadLine());
                if (delay < 0)
                    throw new ArgumentException("Задержка не может быть отрицательной.");

                Console.WriteLine("\nВыберите начальную конфигурацию для Игры 'Жизнь':");
                Console.WriteLine("1. Случайная конфигурация");
                Console.WriteLine("2. Switch Engine");
                int choice = int.Parse(Console.ReadLine());
                if (choice < 1 || choice > 2)
                    throw new ArgumentException("Неверный выбор конфигурации.");

                char[,] grid = new char[rows, cols];
                for (int i = 0; i < rows; ++i)
                    for (int j = 0; j < cols; ++j)
                        grid[i, j] = ' ';

                switch (choice)
                {
                    case 1:
                        SetupRandom(grid, rows, cols);
                        break;
                    case 2:
                        if (rows < 10 || cols < 15)
                            Console.WriteLine("Для Switch Engine нужна сетка побольше");
                        SetupSwitchEngine(grid, 2, 2, rows, cols);
                        break;
                }

                for (int gen = 0; gen < generations; ++gen)
                {
                    ClearScreen();
                    Console.WriteLine($"Игра 'Жизнь' - Поколение: {gen + 1}/{generations}");
                    DisplayGrid(grid, rows, cols);
                    CalculateNextGeneration(grid, rows, cols);
                    SleepMilliseconds(delay);
                }

                ClearScreen();
                Console.WriteLine("Симуляция Игры 'Жизнь' завершена. Финальное состояние:");
                DisplayGrid(grid, rows, cols);
            }
        }
        catch (Exception e)
        {
            Console.Error.WriteLine($"Ошибка: {e.Message}");
            Environment.Exit(1);
        }
    }
}