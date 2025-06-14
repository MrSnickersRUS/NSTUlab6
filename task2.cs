using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;

class AES_CFB
{
    // Типы и константы AES
    // Убрали класс AESBlock, теперь используем byte[] для простоты и производительности.
    public class AESState
    {
        public byte[,] State { get; } = new byte[4, 4];

        public AESState(byte[] block)
        {
            for (int i = 0; i < 16; i++)
                State[i % 4, i / 4] = block[i];
        }

        public byte[] ToBlock()
        {
            byte[] block = new byte[16];
            for (int i = 0; i < 16; i++)
                block[i] = State[i % 4, i / 4];
            return block;
        }
    }

    const int Nb = 4; // Количество столбцов (слов) в состоянии
    const int Nk = 4; // Длина ключа в словах (4 слова * 4 байта/слово = 128 бит)
    const int Nr = 10; // Количество раундов для ключа 128 бит

    // S-box и Rcon
    static readonly byte[] S_BOX = {
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
        0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
        0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
        0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
        0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
        0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
        0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
        0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
        0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
        0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
        0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
        0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
        0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
        0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
        0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
        0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
    };

    static readonly byte[] R_CON = { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36 };

    // Основные функции AES
    static void SubBytes(AESState state)
    {
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                state.State[r, c] = S_BOX[state.State[r, c]];
    }
    
    static void ShiftRows(AESState state)
    {
        for (int r = 1; r < 4; r++)
        {
            byte[] row = new byte[4];
            for (int c = 0; c < 4; c++)
                row[c] = state.State[r, c];

            for (int c = 0; c < 4; c++)
                state.State[r, c] = row[(c + r) % 4];
        }
    }

    static byte GFMul(byte a, byte b)
    {
        byte p = 0;
        for (int i = 0; i < 8; i++)
        {
            if ((b & 1) != 0)
                p ^= a;

            bool hi_bit_set = (a & 0x80) != 0;
            a <<= 1;
            if (hi_bit_set)
                a ^= 0x1B;
            b >>= 1;
        }
        return p;
    }

    static void MixColumns(AESState state)
    {
        byte[,] temp = new byte[4, 4];
        for (int c = 0; c < 4; c++)
        {
            temp[0, c] = (byte)(GFMul(0x02, state.State[0, c]) ^ GFMul(0x03, state.State[1, c]) ^ state.State[2, c] ^ state.State[3, c]);
            temp[1, c] = (byte)(state.State[0, c] ^ GFMul(0x02, state.State[1, c]) ^ GFMul(0x03, state.State[2, c]) ^ state.State[3, c]);
            temp[2, c] = (byte)(state.State[0, c] ^ state.State[1, c] ^ GFMul(0x02, state.State[2, c]) ^ GFMul(0x03, state.State[3, c]));
            temp[3, c] = (byte)(GFMul(0x03, state.State[0, c]) ^ state.State[1, c] ^ state.State[2, c] ^ GFMul(0x02, state.State[3, c]));
        }

        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                state.State[r, c] = temp[r, c];
    }

    static void AddRoundKey(AESState state, byte[] expandedKey, int round)
    {
        int offset = round * Nb * 4;
        for (int c = 0; c < Nb; c++)
            for (int r = 0; r < Nb; r++)
                state.State[r, c] ^= expandedKey[offset + c * Nb + r];
    }

    static void EncryptBlock(AESState state, byte[] expandedKey, bool verbose)
    {
        if (verbose) { Console.WriteLine("  Начальное состояние блока:"); PrintState(state); }

        AddRoundKey(state, expandedKey, 0);

        for (int round = 1; round < Nr; round++)
        {
            SubBytes(state);
            if (verbose) { Console.WriteLine("  После SubBytes:"); PrintState(state); }
            ShiftRows(state);
            if (verbose) { Console.WriteLine("  После ShiftRows:"); PrintState(state); }
            MixColumns(state);
            if (verbose) { Console.WriteLine("  После MixColumns:"); PrintState(state); }
            AddRoundKey(state, expandedKey, round);
            if (verbose) { Console.WriteLine("  После AddRoundKey:"); PrintState(state); }
        }

        SubBytes(state);
        if (verbose) { Console.WriteLine("  После SubBytes:"); PrintState(state); }
        ShiftRows(state);
        if (verbose) { Console.WriteLine("  После ShiftRows:"); PrintState(state); }
        AddRoundKey(state, expandedKey, Nr);
        if (verbose) { Console.WriteLine("  Финальное состояние:"); PrintState(state); }
    }

    static byte[] KeyExpansion(byte[] key, bool verbose)
    {
        byte[] expandedKey = new byte[Nb * (Nr + 1) * 4];
        Array.Copy(key, expandedKey, key.Length);

        byte[] tempWord = new byte[4];
        int bytesGenerated = key.Length;
        int rconIter = 1;

        while (bytesGenerated < expandedKey.Length)
        {
            Array.Copy(expandedKey, bytesGenerated - 4, tempWord, 0, 4);

            if (bytesGenerated % (Nk * 4) == 0)
            {
                // RotWord
                byte temp = tempWord[0];
                tempWord[0] = tempWord[1];
                tempWord[1] = tempWord[2];
                tempWord[2] = tempWord[3];
                tempWord[3] = temp;

                // SubWord
                for (int i = 0; i < 4; i++)
                    tempWord[i] = S_BOX[tempWord[i]];

                tempWord[0] ^= R_CON[rconIter++];
            }

            for (int i = 0; i < 4; i++)
            {
                expandedKey[bytesGenerated] = (byte)(expandedKey[bytesGenerated - (Nk * 4)] ^ tempWord[i]);
                bytesGenerated++;
            }
        }
        
        if (verbose)
        {
            for (int round = 0; round <= Nr; round++)
            {
                Console.Write($"Раунд {round,2}: ");
                for (int i = 0; i < Nb * 4; i++)
                    Console.Write($"{expandedKey[round * Nb * 4 + i]:X2} ");
                Console.WriteLine();
            }
            Console.WriteLine();
        }
        
        return expandedKey;
    }

    // Режим CFB
    static List<byte> EncryptCFB(List<byte> plaintext, byte[] key, byte[] iv, bool verbose)
    {
        if (plaintext.Count == 0) return new List<byte>();

        byte[] expandedKey = KeyExpansion(key, verbose);
        List<byte> ciphertext = new List<byte>();
        byte[] currentFeedback = (byte[])iv.Clone();

        if (verbose) { Console.Write("Начальный IV (Feedback): "); PrintBlockHex(iv); }

        for (int i = 0; i < plaintext.Count; i += 16)
        {
            if (verbose)
            {
                Console.WriteLine($"\nБлок {(i / 16) + 1} (Шифрование CFB):");
                Console.Write("  Текущий блок обратной связи (вход для AES): ");
                PrintBlockHex(currentFeedback);
            }

            AESState state = new AESState(currentFeedback);
            EncryptBlock(state, expandedKey, verbose);
            byte[] keystreamBlock = state.ToBlock();
            
            if (verbose)
            {
                Console.Write("  Сгенерированный поток ключей (выход AES): ");
                PrintBlockHex(keystreamBlock);
            }
            
            int bytesToProcess = Math.Min(16, plaintext.Count - i);
            byte[] currentCiphertextBlock = new byte[bytesToProcess];

            for (int j = 0; j < bytesToProcess; j++)
            {
                byte encryptedByte = (byte)(plaintext[i + j] ^ keystreamBlock[j]);
                ciphertext.Add(encryptedByte);
                currentCiphertextBlock[j] = encryptedByte;
            }
            
            // Следующий блок для обратной связи - это только что полученный шифротекст
            currentFeedback = currentCiphertextBlock;

            if (verbose)
            {
                Console.Write("  Полученный зашифрованный блок: ");
                PrintBlockHex(currentFeedback);
            }
        }
        return ciphertext;
    }

    static List<byte> DecryptCFB(List<byte> ciphertext, byte[] key, byte[] iv, bool verbose)
    {
        if (ciphertext.Count == 0) return new List<byte>();
        
        byte[] expandedKey = KeyExpansion(key, verbose);
        List<byte> decryptedtext = new List<byte>();
        byte[] currentFeedback = (byte[])iv.Clone();

        if (verbose) { Console.Write("Начальный IV (Feedback): "); PrintBlockHex(iv); }

        for (int i = 0; i < ciphertext.Count; i += 16)
        {
            if (verbose)
            {
                Console.WriteLine($"\nБлок {(i / 16) + 1} (Дешифрование CFB):");
                Console.Write("  Текущий блок обратной связи (вход для AES): ");
                PrintBlockHex(currentFeedback);
            }

            AESState state = new AESState(currentFeedback);
            EncryptBlock(state, expandedKey, verbose);
            byte[] keystreamBlock = state.ToBlock();
            
            if (verbose)
            {
                Console.Write("  Сгенерированный поток ключей (выход AES): ");
                PrintBlockHex(keystreamBlock);
            }

            int bytesToProcess = Math.Min(16, ciphertext.Count - i);
            byte[] nextFeedbackBlock = new byte[bytesToProcess];
            Array.Copy(ciphertext.ToArray(), i, nextFeedbackBlock, 0, bytesToProcess);

            for (int j = 0; j < bytesToProcess; j++)
            {
                decryptedtext.Add((byte)(ciphertext[i + j] ^ keystreamBlock[j]));
            }

            // Следующий блок для обратной связи - это блок шифротекста из текущей итерации
            currentFeedback = nextFeedbackBlock;
            
            if (verbose)
            {
                Console.Write("  Полученный расшифрованный блок: ");
                // Выводим только что расшифрованные байты
                for (int k = i; k < i + bytesToProcess; k++)
                    Console.Write($"{decryptedtext[k]:X2} ");
                Console.WriteLine();
            }
        }
        return decryptedtext;
    }

    // Вспомогательные функции
    static void PrintState(AESState state)
    {
        for (int r = 0; r < 4; r++)
        {
            Console.Write("    ");
            for (int c = 0; c < 4; c++)
                Console.Write($"{state.State[r, c]:X2} ");
            Console.WriteLine();
        }
    }

    static void PrintBlockHex(byte[] block)
    {
        if (block == null) return;
        foreach (byte b in block)
            Console.Write($"{b:X2} ");
        Console.WriteLine();
    }
    
    // Функции для работы с файлами
    static byte[] ReadAllBytesFromFile(string filename)
    {
        if (!File.Exists(filename))
            throw new FileNotFoundException($"Файл не найден.", filename);
        return File.ReadAllBytes(filename);
    }
    
    static void WriteAllBytesToFile(string filename, List<byte> data)
    {
        File.WriteAllBytes(filename, data.ToArray());
        Console.WriteLine($"Данные ({data.Count} байт) успешно сохранены в файл: {filename}");
    }
    
    static void SaveBlockToFile(byte[] block, string filename)
    {
        File.WriteAllBytes(filename, block);
        Console.WriteLine($"Блок ({block.Length} байт) успешно сохранен в файл: {filename}");
    }

    static byte[] LoadBlockFromFile(string filename, int expectedLength)
    {
        byte[] bytes = ReadAllBytesFromFile(filename);
        if (bytes.Length != expectedLength)
            throw new ArgumentException($"Файл {filename} должен содержать ровно {expectedLength} байт.");
        return bytes;
    }

    static void Main()
    {
        try
        {
            Console.OutputEncoding = Encoding.UTF8;
            Console.WriteLine("Выберите режим работы:\n1. Шифрование\n2. Дешифрование");
            int.TryParse(Console.ReadLine(), out int mode);

            Console.Write("Включить подробный вывод? (y/n): ");
            bool verbose = Console.ReadLine()?.ToLower() == "y";

            if (mode == 1)
            {
                Console.WriteLine("Источник данных:\n1. Ввод с клавиатуры\n2. Чтение из файла");
                int.TryParse(Console.ReadLine(), out int sourceChoice);
                
                List<byte> plaintext;
                if (sourceChoice == 1)
                {
                    Console.Write("Введите текст для шифрования: ");
                    plaintext = Encoding.UTF8.GetBytes(Console.ReadLine() ?? "").ToList();
                }
                else if (sourceChoice == 2)
                {
                    Console.Write("Введите имя входного файла: ");
                    string? inputFilename = Console.ReadLine();
                    plaintext = ReadAllBytesFromFile(inputFilename).ToList();
                    Console.WriteLine($"Успешно прочитано {plaintext.Count} байт.");
                }
                else
                {
                    Console.WriteLine("Неверный выбор.");
                    return;
                }
                
                if(plaintext.Count == 0)
                {
                    Console.WriteLine("Нет данных для шифрования.");
                    return;
                }

                byte[] key = new byte[16];
                byte[] iv = new byte[16];
                RandomNumberGenerator.Fill(key);
                RandomNumberGenerator.Fill(iv);

                Console.Write("\nСгенерированный ключ (Hex): ");
                PrintBlockHex(key);
                Console.Write("Сгенерированный IV (Hex):   ");
                PrintBlockHex(iv);

                List<byte> ciphertext = EncryptCFB(plaintext, key, iv, verbose);

                SaveBlockToFile(key, "key.bin");
                SaveBlockToFile(iv, "iv.bin");
                WriteAllBytesToFile("ciphertext.bin", ciphertext);
            }
            else if (mode == 2)
            {
                byte[] key = LoadBlockFromFile("key.bin", 16);
                byte[] iv = LoadBlockFromFile("iv.bin", 16);
                List<byte> ciphertext = ReadAllBytesFromFile("ciphertext.bin").ToList();
                
                Console.Write("\nКлюч из файла (Hex): ");
                PrintBlockHex(key);
                Console.Write("IV из файла (Hex):   ");
                PrintBlockHex(iv);

                List<byte> decryptedtext = DecryptCFB(ciphertext, key, iv, verbose);
                
                WriteAllBytesToFile("decrypted_output.txt", decryptedtext);
                Console.WriteLine($"\nРасшифрованный текст:\n{Encoding.UTF8.GetString(decryptedtext.ToArray())}");
            }
            else
            {
                Console.WriteLine("Неверный выбор режима.");
            }
        }
        catch (Exception e)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"\nОШИБКА: {e.Message}");
            Console.ResetColor();
        }
        
        Console.WriteLine("\nНажмите любую клавишу для выхода...");
        Console.ReadKey();
    }
}