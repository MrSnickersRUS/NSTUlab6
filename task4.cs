using System;
using System.Collections.Generic;
using System.Linq;

class Program
{
    // Находит минимальное количество ходов
    static Dictionary<int, int> BfsBetween(int pStart, int sStart, int pEnd)
    {
        // Словарь для хранения результатов
        var results = new Dictionary<int, int>();

        // Очередь для BFS: (ходы, позиция, скорость)
        var queue = new Queue<(int moves, int pos, int speed)>();
        queue.Enqueue((0, pStart, sStart));

        // Посещенные состояния (позиция, скорость)
        var visited = new HashSet<(int pos, int speed)>();
        visited.Add((pStart, sStart));

        while (queue.Count > 0)
        {
            var (moves, pos, speed) = queue.Dequeue();

            // Если достигли целевой позиции
            if (pos == pEnd)
            {
                // Если еще не находили путь с такой скоростью или нашли более короткий
                if (!results.ContainsKey(speed) || moves < results[speed])
                {
                    results[speed] = moves;
                }
                continue;
            }

            // 1. Ускорение
            int speedAccel = speed + 1;
            int posAccel = pos + speedAccel;
            if (posAccel <= pEnd && !visited.Contains((posAccel, speedAccel)))
            {
                visited.Add((posAccel, speedAccel));
                queue.Enqueue((moves + 1, posAccel, speedAccel));
            }

            // 2. Движение с той же скоростью
            int speedSaved = speed;
            int positionSaved = pos + speedSaved;
            if (speedSaved > 0 && positionSaved <= pEnd && !visited.Contains((positionSaved, speedSaved)))
            {
                visited.Add((positionSaved, speedSaved));
                queue.Enqueue((moves + 1, positionSaved, speedSaved));
            }

            // 3. Замедление (скорость должна оставаться положительной)
            if (speed > 1)
            {
                int speedDecel = speed - 1;
                int positionDecel = pos + speedDecel;
                if (positionDecel <= pEnd && !visited.Contains((positionDecel, speedDecel)))
                {
                    visited.Add((positionDecel, speedDecel));
                    queue.Enqueue((moves + 1, positionDecel, speedDecel));
                }
            }
        }
        return results;
    }

    static void Main()
    {
        // Чтение входных данных
        int k = int.Parse(Console.ReadLine());
        if (k == 0)
        {
            Console.WriteLine(0);
            return;
        }

        int[] targets = Array.ConvertAll(Console.ReadLine().Split(), int.Parse);

        // dp[i] - словарь (скорость, ходы) для i-й цели
        var dp = new Dictionary<int, int>[k];

        // Базовый случай: путь от стартовой позиции (клетка 1, скорость 0) до первой цели
        dp[0] = BfsBetween(1, 0, targets[0]);

        // Основной цикл динамического программирования
        for (int i = 0; i < k - 1; i++)
        {
            int startPos = targets[i];
            int endPos = targets[i + 1];
            dp[i + 1] = new Dictionary<int, int>();

            // Перебираем все возможные состояния (скорость, ходы) на предыдущей цели
            foreach (var (startSpeed, totalMoves) in dp[i])
            {
                // Находим все возможные пути до следующей цели
                var pathsToNext = BfsBetween(startPos, startSpeed, endPos);

                // Обновляем dp[i+1] найденными путями
                foreach (var (endSpeed, movesBetween) in pathsToNext)
                {
                    int newTotalMoves = totalMoves + movesBetween;

                    // Если еще не достигали (i+1)-ю цель с такой скоростью или нашли более короткий путь
                    if (!dp[i + 1].ContainsKey(endSpeed) || newTotalMoves < dp[i + 1][endSpeed])
                    {
                        dp[i + 1][endSpeed] = newTotalMoves;
                    }
                }
            }
        }

        // Находим минимальное количество ходов среди всех возможных конечных состояний
        int minTotalMoves = int.MaxValue;
        if (k > 0 && dp[k - 1] != null && dp[k - 1].Count > 0)
        {
            minTotalMoves = dp[k - 1].Values.Min();
        }
        else
        {
            minTotalMoves = 0;
        }

        Console.WriteLine(minTotalMoves);
    }
}