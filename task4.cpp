#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <tuple>
#include <set>
#include <algorithm>
#include <climits>

using namespace std;

// Находит минимальное количество ходов
map<int, int> bfsBetween(int pStart, int sStart, int pEnd) {
    // map для хранения результатов
    map<int, int> results;

    // Ходы, позиция, скорость
    queue<tuple<int, int, int>> q;
    q.push({0, pStart, sStart});

    // Посещенные состояния
    set<pair<int, int>> visited;
    visited.insert({pStart, sStart});

    while (!q.empty()) {
        auto [moves, pos, speed] = q.front();
        q.pop();

        // Если мы достигли целевой позиции
        if (pos == pEnd) {
            // Если мы еще не находили путь с такой скоростью или нашли более короткий
            if (results.find(speed) == results.end() || moves < results[speed]) {
                results[speed] = moves;
            }
            // Продолжаем поиск, т.к. могут быть другие пути с другими скоростями
            continue;
        }

        // 1. Ускорение
        int speedAccel = speed + 1;
        int posAccel = pos + speedAccel;
        if (posAccel <= pEnd && visited.find({posAccel, speedAccel}) == visited.end()) {
            visited.insert({posAccel, speedAccel});
            q.push({moves + 1, posAccel, speedAccel});
        }

        // 2. Движение с той же скоростью
        int speedSaved = speed;
        int positionSaved = pos + speedSaved;
        // Двигаться с нулевой скоростью нельзя
        if (speedSaved > 0 && positionSaved <= pEnd && visited.find({positionSaved, speedSaved}) == visited.end()) {
            visited.insert({positionSaved, speedSaved});
            q.push({moves + 1, positionSaved, speedSaved});
        }

        // 3. Замедление
        // Скорость должна оставаться положительной
        if (speed > 1) {
            int speedDecel = speed - 1;
            int positionDecel = pos + speedDecel;
            if (positionDecel <= pEnd && visited.find({positionDecel, speedDecel}) == visited.end()) {
                visited.insert({positionDecel, speedDecel});
                q.push({moves + 1, positionDecel, speedDecel});
            }
        }
    }
    return results;
}

int main() {
    // Чтение входных данных
    int k;
    cin >> k;
    if (k == 0) cout << 0 << endl; // Если нет целей

    vector<int> targets(k);
    for (int i = 0; i < k; ++i) {
        cin >> targets[i];
    }

    // скорость, ходы для достижения i-й цели
    vector<map<int, int>> dp(k);

    // Базовый случай: путь от стартовой позиции (клетка 1, скорость 0) до первой цели
    dp[0] = bfsBetween(1, 0, targets[0]);

    // Основной цикл динамического программирования
    for (int i = 0; i < k - 1; ++i) {
        int startPos = targets[i];
        int endPos = targets[i + 1];

        // Перебираем все возможные состояния (скорость, ходы) на предыдущей цели
        for (const auto& [startSpeed, totalMoves] : dp[i]) {
            // Находим все возможные пути до следующей цели
            map<int, int> pathsToNext = bfsBetween(startPos, startSpeed, endPos);

            // Обновляем dp[i+1] найденными путями
            for (const auto& [endSpeed, movesBetween] : pathsToNext) {
                int newTotalMoves = totalMoves + movesBetween;

                // Если мы еще не достигали (i+1)-ю цель с такой скоростью, или нашли более короткий путь, то обновляем значение.
                if (dp[i + 1].find(endSpeed) == dp[i + 1].end() || newTotalMoves < dp[i + 1][endSpeed])
                    dp[i + 1][endSpeed] = newTotalMoves;
            }
        }
    }

    // Находим минимальное количество ходов среди всех возможных конечных состояний
    int minTotalMoves = INT_MAX;
    if (k > 0 && !dp[k - 1].empty()) {
        for (const auto& [speed, moves] : dp[k - 1]) {
            minTotalMoves = min(minTotalMoves, moves);
        }
    } 
    else minTotalMoves = 0;
    
    cout << minTotalMoves << endl;
}