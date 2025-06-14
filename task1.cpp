#include <iostream>
#include <random>
#include <vector>
#include <thread>   
#include <chrono>   
#include <stdexcept>
#include <limits>   
#include <algorithm>

using namespace std;

// Глобальные генераторы случайных чисел (для эффективности)
static random_device globadRd;
static mt19937_64 globadGen(globadRd());

int randomNumGen(const int minVal, const int maxVal) {
    uniform_int_distribution<int> dist(minVal, maxVal);
    return dist(globadGen);
}

// Эта функция используется для setupRandom в Игре Жизнь
mt19937& getRandomEngine() {
    static random_device rd;
    static mt19937 genLife(rd());
    return genLife;
}

void clearScreen() {
    system("clear");
}

void sleepMilliseconds(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}

void displayGrid(const vector<vector<char>>& grid, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cout << grid[i][j];
        }
        cout << endl;
    }
}

int countLiveNeighbours(const vector<vector<char>>& grid, int r, int c, int rows, int cols) {
    int count = 0;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) { // Пропускаем саму клетку
                continue;
            }
            int nr = r + dr; // Соседняя строка
            int nc = c + dc; // Соседний столбец

            // Проверяем границы
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                if (grid[nr][nc] == '0') count++;
            }
        }
    }
    return count;
}

void calculateNextGeneration(vector<vector<char>>& grid, int rows, int cols) {
    vector<vector<char>> nextGrid = grid; // Создаем копию текущего состояния для вычислений
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int liveNeighbours = countLiveNeighbours(grid, i, j, rows, cols);
            char currentCellState = grid[i][j];

            if (currentCellState == '0') { // Если клетка жива
                // Правила для живой клетки
                if (liveNeighbours < 2 || liveNeighbours > 3) {
                    nextGrid[i][j] = ' '; // Умирает от одиночества или перенаселения
                }
                // Если соседей 2 или 3, остается живой (ничего не меняем в nextGrid)
            } else { // Если клетка мертва (' ')
                // Правила для мертвой клетки
                if (liveNeighbours == 3) nextGrid[i][j] = '0';
            }
        }
    }
    grid = nextGrid; // Обновляем основную сетку новым поколением
}

void setupRandom(vector<vector<char>>& grid, int rows, int cols) {
    uniform_int_distribution<int> dist(0, 3); // 25% клеток будут живыми
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            grid[i][j] = (dist(getRandomEngine()) == 0) ? '0' : ' ';
        }
    }
}

// Установка Switch Engine
void setupSwitchEngine(vector<vector<char>>& grid, int startRow, int startColumn, int rows, int cols) {
    for(int i=0; i<rows; ++i) fill(grid[i].begin(), grid[i].end(), ' '); // Очищаем сетку

    vector<pair<int, int>> pattern = {
        {0, 3},
        {1, 0}, {1, 3}, {1, 5},
        {2, 0}, {2, 1}, {2, 5},
        {3, 5},
        {4, 4}
    };

    // Проверяем, помещается ли паттерн
    int maxRow = 0, maxColumn = 0;
    for(const auto& p : pattern) {
        maxRow = max(maxRow, p.first);
        maxColumn = max(maxColumn, p.second);
    }

    if (startRow + maxRow < rows && startColumn + maxColumn < cols) {
        for (const auto& p : pattern) {
            grid[startRow + p.first][startColumn + p.second] = '0';
        }
    } else {
        cout << "Предупреждение: Switch Engine не помещается в указанные координаты." << endl
            << "Попробуйте увеличить размер сетки (рекомендуется хотя бы 10x15 для наблюдения)." << endl;
    }
}

int main() {
    try {
        {
            cout << "Введите количество строк и столбцов матрицы: ";
            int rows, cols;
            cin >> rows >> cols;
            if (rows <= 0 || cols <= 0) throw invalid_argument("Количество строк и столбцов должно быть больше нуля");
        
            vector<vector<int>> matrix(rows, vector<int>(cols));
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    matrix[i][j] = randomNumGen(-50, 50);
                }
            }
            
            long long sum = 0;
            vector<int> newArray;
            for (int i = 1; i < rows; i += 2) { // Нечетные строки
                for (int j = 0; j < cols; j += 2) { // Четные столбцы
                    if (matrix[i][j] >= 0) {
                        sum += matrix[i][j]; 
                        newArray.push_back(matrix[i][j]);
                    }
                }
            }

            cout << "Сумма неотрицательных элементов на пересечении нечетных строк и четных столбцов: " << sum << endl;
            cout << "Массив из этих элементов: ";
            for(int val : newArray) cout << val << " ";
            cout << endl;
        }
        {
            cout << "Введите порядок квадратной матрицы: ";
            int n;
            cin >> n;
            if (cin.fail() || n <= 0) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                throw invalid_argument("Порядок матрицы должен быть положительным целым числом.");
            }
    
            vector<vector<int>> matrix(n, vector<int>(n));
            cout << "Сгенерированная квадратная матрица (" << n << "x" << n << "):" << endl;
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    matrix[i][j] = randomNumGen(100, 200);
                }
            }
    
            vector<pair<long long, int>> indexedSums(n); 
            cout << "\nСуммы элементов по строкам:" << endl;
            for (int i = 0; i < n; ++i) {
                long long currentSum = 0; 
                currentSum = accumulate(matrix[i].begin(), matrix[i].end(), 0LL); 
                indexedSums[i] = {currentSum, i};
            }
    
            int secondMaxOriginalIndex = -1;

            if (n >= 2) {
                // Сортируем пары по убыванию сумм
                sort(indexedSums.begin(), indexedSums.end(), 
                     [](const pair<long long, int>& a, const pair<long long, int>& b) {
                    return a.first > b.first; 
                });

                long long maxSum = indexedSums[0].first;
                
                // Ищем первую сумму, которая меньше максимальной
                for (size_t k = 1; k < indexedSums.size(); ++k) {
                    if (indexedSums[k].first < maxSum) {
                        secondMaxOriginalIndex = indexedSums[k].second; 
                        break;
                    }
                }
            } else
                cout << "\nНедостаточно строк (N < 2) для определения второй максимальной суммы." << endl;
            
            if (secondMaxOriginalIndex != -1) {
                cout << "\nМассив, инициализированный строкой с второй максимальной суммой (исходный индекс " 
                     << secondMaxOriginalIndex << "):" << endl;
                vector<int> secMaxRow = matrix[secondMaxOriginalIndex]; 
                for (size_t i = 0; i < secMaxRow.size(); ++i) {
                    cout << secMaxRow[i] << (i == secMaxRow.size() - 1 ? "" : ", ");
                }
                cout << endl;
            } else if (n >= 2)
                cout << "\nНе удалось найти строку со второй по величине суммой (возможно, все суммы строк одинаковы)." << endl;
        }
        {
            int rows, cols, generations, delay, choice;
            
            cout << "Введите количество строк сетки: ";
            cin >> rows;
            if (cin.fail() || rows <= 0) { 
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                throw invalid_argument("Количество строк должно быть положительным числом.");
            }
            
            cout << "Введите количество столбцов сетки: ";
            cin >> cols;
            if (cin.fail() || cols <= 0) { 
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                throw invalid_argument("Количество столбцов должно быть положительным числом.");
            }

            cout << "Введите количество поколений для симуляции: ";
            cin >> generations;
            if (cin.fail() || generations <= 0) { 
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                throw invalid_argument("Количество поколений должно быть положительным числом.");
            }
            
            cout << "Введите задержку между поколениями (в миллисекундах, например, 100): ";
            cin >> delay;
            if (cin.fail() || delay < 0) { 
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                throw invalid_argument("Задержка не может быть отрицательной.");
            }

            cout << "\nВыберите начальную конфигурацию для Игры 'Жизнь':" << endl;
            cout << "1. Случайная конфигурация" << endl;
            cout << "2. Switch Engine" << endl;
            cin >> choice;
            if (cin.fail() || (choice < 1 || choice > 4)) { // Обновлена проверка выбора
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                throw invalid_argument("Неверный выбор конфигурации.");
            }
            
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Очистка буфера

            vector<vector<char>> grid(rows, vector<char>(cols, ' ')); 

            // Обновленная логика выбора паттерна
            switch (choice) {
                case 1:
                    setupRandom(grid, rows, cols);
                    break;
                case 2:
                    if (rows < 10 || cols < 15)
                        cout << "Для Switch Engine нужна сетка побольше" << endl;
                    setupSwitchEngine(grid, 2, 2, rows, cols);
                    break;
                }

            for (int gen = 0; gen < generations; ++gen) {
                clearScreen();
                cout << "Игра 'Жизнь' - Поколение: " << gen + 1 << "/" << generations << endl;
                displayGrid(grid, rows, cols);
                calculateNextGeneration(grid, rows, cols);
                sleepMilliseconds(delay);
            }

            clearScreen();
            cout << "Симуляция Игры 'Жизнь' завершена. Финальное состояние:" << endl;
            displayGrid(grid, rows, cols);
        }
    }
    catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl; 
        if (cin.fail()) { 
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        return 1; 
    }
    return 0; 
}