#include <iostream>
#include <vector>
#include <array>
#include <cstdint>
#include <iomanip>
#include <random>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <string>
#include <limits>

using namespace std;

// типы и константы AES
using AESBlock = array<uint8_t, 16>;
using AESKey = AESBlock;
using AES_IV = AESBlock;
using AESState = array<array<uint8_t, 4>, 4>;
using ExpandedAESKey = array<uint8_t, 176>;

const int Nb = 4;
const int Nk = 4;
const int Nr = 10;

// S-box и Rcon
const array<uint8_t, 256> S_BOX = {
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
const array<uint8_t, 11> R_CON = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};

// Прототипы функций
void printBlockHex(const AESBlock& block);
vector<uint8_t> readFromFile(const string& filename);
void writeToFile(const string& filename, const vector<uint8_t>& data);
AESBlock loadBlockFromFile(const string& filename);
void saveBlockToFile(const AESBlock& block, const string& filename);
AESState blockToState(const AESBlock& block);
AESBlock stateToBlock(const AESState& state);
void subBytes(AESState& state);
void shiftRows(AESState& state);
uint8_t gfMulX(uint8_t val);
void mixColumns(AESState& state);
void addRoundKey(AESState& state, const ExpandedAESKey& expandedKey, int round);
void encryptBlock(AESState& state, const ExpandedAESKey& expandedKey, bool verbose);
void keyExpansion(ExpandedAESKey& expandedKey, const AESKey& key, bool verbose);
vector<uint8_t> encryptCFB(const vector<uint8_t>& plaintext, const AESKey& key, const AES_IV& iv, bool verbose);
vector<uint8_t> decryptCFB(const vector<uint8_t>& ciphertext, const AESKey& key, const AES_IV& iv, bool verbose);


int main() {
    try {
        cout << "Выберите режим работы:\n1. Шифрование\n2. Дешифрование\n";
        int mode;
        cin >> mode;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        char verboseChoice;
        cout << "Включить подробный вывод промежуточных состояний и расширенного ключа? (y/n): ";
        cin >> verboseChoice;
        bool verbose = (verboseChoice == 'y' || verboseChoice == 'Y');
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (mode == 1) {
            vector<uint8_t> plaintext;
            cout << "Источник данных:\n1. Ввод с клавиатуры\n2. Чтение из файла\n";
            int sourceChoice;
            cin >> sourceChoice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (sourceChoice == 1) {
                string inputText;
                cout << "Введите текст для шифрования: ";
                getline(cin, inputText);
                plaintext.assign(inputText.begin(), inputText.end());
            } else if (sourceChoice == 2) {
                string inputFilename;
                cout << "Введите имя входного файла: ";
                getline(cin, inputFilename);
                plaintext = readFromFile(inputFilename);
            } else {
                cout << "Неверный выбор источника данных." << endl;
                return 1;
            }

            // Генерация случайных ключа и вектора инициализации (IV)
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<uint8_t> dis(0, 255);
            AESKey key;
            generate(key.begin(), key.end(), [&]() { return dis(gen); });
            AES_IV iv;
            generate(iv.begin(), iv.end(), [&]() { return dis(gen); });

            // Выводим сгенерированные ключ и IV.
            cout << "\nСгенерированный ключ (Hex): ";
            printBlockHex(key);
            cout << "Сгенерированный IV (Hex):   ";
            printBlockHex(iv);

            vector<uint8_t> ciphertext = encryptCFB(plaintext, key, iv, verbose);

            saveBlockToFile(key, "key.bin");
            saveBlockToFile(iv, "iv.bin");
            writeToFile("ciphertext.bin", ciphertext);
            cout << "Ключ, IV и шифротекст сохранены в файлы key.bin, iv.bin, ciphertext.bin\n";

        } else if (mode == 2) {
            AESKey key = loadBlockFromFile("key.bin");
            AES_IV iv = loadBlockFromFile("iv.bin");
            vector<uint8_t> ciphertext = readFromFile("ciphertext.bin");

            vector<uint8_t> decryptedtext = decryptCFB(ciphertext, key, iv, verbose);
            
            writeToFile("decrypted_output.txt", decryptedtext);
            cout << "Расшифрованный текст сохранен в файл decrypted_output.txt\n";
            
        } else {
            cout << "Неверный выбор режима. Пожалуйста, выберите 1 или 2." << endl;
        }

    } catch (const exception& e) {
        cerr << "Произошла ошибка: " << e.what() << endl;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        return 1;
    }
    return 0;
}

// Функции преобразования 
AESState blockToState(const AESBlock& block) {
    AESState state;
    for (int i = 0; i < 16; ++i) state[i % 4][i / 4] = block[i];
    return state;
}

AESBlock stateToBlock(const AESState& state) {
    AESBlock block;
    for (int i = 0; i < 16; ++i) block[i] = state[i % 4][i / 4];
    return block;
}

// Функции ядра AES
void subBytes(AESState& state) {
    for (int r = 0; r < Nb; ++r) for (int c = 0; c < Nb; ++c) state[r][c] = S_BOX[state[r][c]];
}

void shiftRows(AESState& state) {
    rotate(state[1].begin(), state[1].begin() + 1, state[1].end());
    rotate(state[2].begin(), state[2].begin() + 2, state[2].end());
    rotate(state[3].begin(), state[3].begin() + 3, state[3].end());
}

uint8_t gfMulX(uint8_t val) {
    return (val << 1) ^ ((val >> 7) ? 0x1B : 0x00);
}

void mixColumns(AESState& state) {
    for (int c = 0; c < Nb; ++c) {
        uint8_t s0 = state[0][c], s1 = state[1][c], s2 = state[2][c], s3 = state[3][c];
        state[0][c] = gfMulX(s0) ^ s1 ^ s2 ^ gfMulX(s3) ^ s3;
        state[1][c] = gfMulX(s1) ^ s2 ^ s3 ^ gfMulX(s0) ^ s0;
        state[2][c] = gfMulX(s2) ^ s3 ^ s0 ^ gfMulX(s1) ^ s1;
        state[3][c] = gfMulX(s3) ^ s0 ^ s1 ^ gfMulX(s2) ^ s2;
    }
}

void addRoundKey(AESState& state, const ExpandedAESKey& expandedKey, int round) {
    const uint8_t* currentRoundKey = expandedKey.data() + round * Nb * 4;
    for (int r = 0; r < Nb; ++r) for (int c = 0; c < Nb; ++c) state[r][c] ^= currentRoundKey[c * Nb + r];
}

void encryptBlock(AESState& state, const ExpandedAESKey& expandedKey, bool verbose) {
    if (verbose) {
        cout << "  Начальное состояние блока (до AddRoundKey 0):\n";
        for (int r = 0; r < Nb; ++r) {
            cout << "    "; for (int c = 0; c < Nb; ++c) cout << hex << setw(2) << setfill('0') << static_cast<int>(state[r][c]) << " ";
            cout << "\n";
        }
    }

    addRoundKey(state, expandedKey, 0);
    if (verbose) {
        cout << "  После AddRoundKey (Раунд 0):\n";
        for (int r = 0; r < Nb; ++r) {
            cout << "    "; for (int c = 0; c < Nb; ++c) cout << hex << setw(2) << setfill('0') << static_cast<int>(state[r][c]) << " ";
            cout << "\n";
        }
    }
    
    for (int round = 1; round < Nr; ++round) {
        subBytes(state);
        if (verbose) {
            cout << "  После SubBytes (Раунд " << round << "):\n";
            for (int r = 0; r < Nb; ++r) { cout << "    "; for (int c = 0; c < Nb; ++c) cout << hex << setw(2) << setfill('0') << static_cast<int>(state[r][c]) << " "; cout << "\n"; }
        }
        shiftRows(state);
        if (verbose) {
            cout << "  После ShiftRows (Раунд " << round << "):\n";
            for (int r = 0; r < Nb; ++r) { cout << "    "; for (int c = 0; c < Nb; ++c) cout << hex << setw(2) << setfill('0') << static_cast<int>(state[r][c]) << " "; cout << "\n"; }
        }
        mixColumns(state);
        if (verbose) {
            cout << "  После MixColumns (Раунд " << round << "):\n";
            for (int r = 0; r < Nb; ++r) { cout << "    "; for (int c = 0; c < Nb; ++c) cout << hex << setw(2) << setfill('0') << static_cast<int>(state[r][c]) << " "; cout << "\n"; }
        }
        addRoundKey(state, expandedKey, round);
        if (verbose) {
            cout << "  После AddRoundKey (Раунд " << round << "):\n";
            for (int r = 0; r < Nb; ++r) { cout << "    "; for (int c = 0; c < Nb; ++c) cout << hex << setw(2) << setfill('0') << static_cast<int>(state[r][c]) << " "; cout << "\n"; }
        }
    }
    
    // Финальный, 10-й раунд (без MixColumns).
    subBytes(state);
    if (verbose) {
        cout << "  После финального SubBytes:\n";
        for (int r = 0; r < Nb; ++r) { cout << "    "; for (int c = 0; c < Nb; ++c) cout << hex << setw(2) << setfill('0') << static_cast<int>(state[r][c]) << " "; cout << "\n"; }
    }
    shiftRows(state);
    if (verbose) {
        cout << "  После финального ShiftRows:\n";
        for (int r = 0; r < Nb; ++r) { cout << "    "; for (int c = 0; c < Nb; ++c) cout << hex << setw(2) << setfill('0') << static_cast<int>(state[r][c]) << " "; cout << "\n"; }
    }
    addRoundKey(state, expandedKey, Nr);
    if (verbose) {
        cout << "  Финальное состояние блока (после всех раундов):\n";
        for (int r = 0; r < Nb; ++r) { cout << "    "; for (int c = 0; c < Nb; ++c) cout << hex << setw(2) << setfill('0') << static_cast<int>(state[r][c]) << " "; cout << "\n"; }
        cout << dec;
    }
}

// Генерация раундовых ключей из исходного ключа.
void keyExpansion(ExpandedAESKey& expandedKey, const AESKey& key, bool verbose) {
    copy(key.begin(), key.end(), expandedKey.begin());
    array<uint8_t, 4> tempWord;
    int bytesGenerated = Nk * Nb;
    int rconIter = 1;
    
    while (bytesGenerated < (Nb * (Nr + 1) * 4)) {
        copy_n(expandedKey.begin() + bytesGenerated - 4, 4, tempWord.begin());

        if (bytesGenerated % (Nk * Nb) == 0) {
            rotate(tempWord.begin(), tempWord.begin() + 1, tempWord.end());
            transform(tempWord.begin(), tempWord.end(), tempWord.begin(), [](uint8_t b) { return S_BOX[b]; });
            tempWord[0] ^= R_CON[rconIter++];
        }
        for (int i = 0; i < 4; ++i) {
            expandedKey[bytesGenerated] = expandedKey[bytesGenerated - (Nk * Nb)] ^ tempWord[i];
            ++bytesGenerated;
        }
    }

    if (verbose) {
        for (int round = 0; round <= Nr; ++round) {
            cout << "Раунд " << setw(2) << round << ": ";
            for (int i = 0; i < Nb * 4; ++i) {
                cout << hex << setw(2) << setfill('0') << static_cast<int>(expandedKey[round * Nb * 4 + i]) << " ";
            }
            cout << "\n";
        }
        cout << dec << endl;
    }
}

// Функции режима CFB
vector<uint8_t> encryptCFB(const vector<uint8_t>& plaintext, const AESKey& key, const AES_IV& iv, bool verbose) {
    if (plaintext.empty()) return {};
    
    ExpandedAESKey expandedKey;
    keyExpansion(expandedKey, key, verbose);

    vector<uint8_t> ciphertext(plaintext.size());
    AESBlock currentFeedback = iv;

    if (verbose) {
        cout << "Начальный IV (Feedback): "; printBlockHex(iv);
    }

    for (size_t i = 0; i < plaintext.size(); i += 16) {
        if (verbose) {
            cout << "\nБлок " << (i / 16) + 1 << " (Шифрование CFB):\n";
            cout << "  Текущий блок обратной связи (вход для AES): "; printBlockHex(currentFeedback);
        }

        AESState state = blockToState(currentFeedback);
        encryptBlock(state, expandedKey, verbose);
        AESBlock keystreamBlock = stateToBlock(state);

        if (verbose) {
            cout << "  Сгенерированный поток ключей (выход AES): "; printBlockHex(keystreamBlock);
            cout << "  Исходный блок (XOR с потоком ключей): ";
            for (size_t j = 0; j < min<size_t>(16, plaintext.size() - i); ++j) {
                 cout << hex << setw(2) << setfill('0') << static_cast<int>(plaintext[i+j]) << " ";
            }
            cout << dec << "\n";
        }
        
        const size_t bytesToProcess = min<size_t>(16, plaintext.size() - i);
        for (size_t j = 0; j < bytesToProcess; ++j) {
            ciphertext[i + j] = plaintext[i + j] ^ keystreamBlock[j];
        }
        
        copy(ciphertext.begin() + i, ciphertext.begin() + i + bytesToProcess, currentFeedback.begin());

        if (verbose) {
            cout << "  Полученный зашифрованный блок: ";
            for (size_t j = 0; j < bytesToProcess; ++j) {
                cout << hex << setw(2) << setfill('0') << static_cast<int>(ciphertext[i+j]) << " ";
            }
            cout << dec << "\n";
        }
    }
    return ciphertext;
}

vector<uint8_t> decryptCFB(const vector<uint8_t>& ciphertext, const AESKey& key, const AES_IV& iv, bool verbose) {
    if (ciphertext.empty()) return {};

    ExpandedAESKey expandedKey;
    keyExpansion(expandedKey, key, verbose);

    vector<uint8_t> decryptedtext(ciphertext.size());
    AESBlock currentFeedback = iv;

    if (verbose) {
        cout << "Начальный IV (Feedback): "; printBlockHex(iv);
    }
    
    for (size_t i = 0; i < ciphertext.size(); i += 16) {
        if (verbose) {
            cout << "\nБлок " << (i / 16) + 1 << " (Дешифрование CFB):\n";
            cout << "  Текущий блок обратной связи (вход для AES): "; printBlockHex(currentFeedback);
        }

        AESState state = blockToState(currentFeedback);
        encryptBlock(state, expandedKey, verbose);
        AESBlock keystreamBlock = stateToBlock(state);

        if (verbose) {
            cout << "  Сгенерированный поток ключей (выход AES): "; printBlockHex(keystreamBlock);
            cout << "  Зашифрованный блок (XOR с потоком ключей): ";
            for (size_t j = 0; j < min<size_t>(16, ciphertext.size() - i); ++j) {
                cout << hex << setw(2) << setfill('0') << static_cast<int>(ciphertext[i+j]) << " ";
            }
            cout << dec << "\n";
        }

        AESBlock nextFeedbackBlock;
        const size_t bytesToProcess = min<size_t>(16, ciphertext.size() - i);
        copy(ciphertext.begin() + i, ciphertext.begin() + i + bytesToProcess, nextFeedbackBlock.begin());
        
        for (size_t j = 0; j < bytesToProcess; ++j) {
            decryptedtext[i + j] = ciphertext[i + j] ^ keystreamBlock[j];
        }
        
        currentFeedback = nextFeedbackBlock;

        if (verbose) {
            cout << "  Полученный расшифрованный блок: ";
            for (size_t j = 0; j < bytesToProcess; ++j) {
                cout << hex << setw(2) << setfill('0') << static_cast<int>(decryptedtext[i+j]) << " ";
            }
            cout << dec << "\n";
        }
    }
    return decryptedtext;
}

// Функции для работы с файлами
vector<uint8_t> readFromFile(const string& filename) {
    ifstream ifs(filename, ios::binary | ios::ate);
    if (!ifs) throw runtime_error("Не удалось открыть файл для чтения: " + filename);
    streamsize size = ifs.tellg();
    ifs.seekg(0, ios::beg);
    vector<uint8_t> buffer(size);
    if (!ifs.read(reinterpret_cast<char*>(buffer.data()), size)) throw runtime_error("Не удалось прочитать данные из файла: " + filename);
    cout << "Успешно прочитано " << size << " байт из файла: " << filename << endl;
    return buffer;
}

void writeToFile(const string& filename, const vector<uint8_t>& data) {
    ofstream ofs(filename, ios::binary);
    if (!ofs) throw runtime_error("Не удалось открыть файл для записи: " + filename);
    ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
    cout << "Успешно записано " << data.size() << " байт в файл: " << filename << endl;
}

AESBlock loadBlockFromFile(const string& filename) {
    ifstream ifs(filename, ios::binary);
    if (!ifs) throw runtime_error("Не удалось открыть файл для чтения: " + filename);
    AESBlock block;
    ifs.read(reinterpret_cast<char*>(block.data()), block.size());
    if (ifs.gcount() != 16) throw runtime_error("Файл " + filename + " должен содержать ровно 16 байт для блока.");
    return block;
}

void saveBlockToFile(const AESBlock& block, const string& filename) {
    ofstream ofs(filename, ios::binary);
    if (!ofs) throw runtime_error("Не удалось открыть файл для записи: " + filename);
    ofs.write(reinterpret_cast<const char*>(block.data()), block.size());
}

void printBlockHex(const AESBlock& block) {
    for (const auto& byte : block) cout << hex << setw(2) << setfill('0') << static_cast<int>(byte) << " ";
    cout << dec << endl;
}