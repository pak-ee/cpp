#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <limits>
#include <stdexcept>

// Структура для хранения коэффициентов квадратного уравнения
struct QuadraticCoefficients {
    double a;
    double b;
    double c;
};

// Структура для хранения корней квадратного уравнения
struct QuadraticRoots {
    int numRoots;     // Количество корней (0, 1 или 2)
    double root1;     // Первый корень
    double root2;     // Второй корень
};

// Функция для чтения коэффициентов из файла
QuadraticCoefficients readCoefficientsFromFile(const std::string& filename) {
    std::ifstream inputFile(filename);
    
    if (!inputFile.is_open()) {
        throw std::runtime_error("Ошибка: не удалось открыть файл " + filename);
    }
    
    QuadraticCoefficients coeffs;
    
    #ifdef _DEBUG
    std::cout << "DEBUG: Попытка чтения коэффициентов из файла " << filename << std::endl;
    #endif
    
    // Чтение коэффициентов
    if (!(inputFile >> coeffs.a >> coeffs.b >> coeffs.c)) {
        inputFile.close();
        throw std::runtime_error("Ошибка: некорректные данные в файле");
    }
    
    // Проверка, не осталось ли еще данных в файле
    double extraValue;
    if (inputFile >> extraValue) {
        inputFile.close();
        throw std::runtime_error("Ошибка: в файле больше данных, чем ожидалось");
    }
    
    inputFile.close();
    
    #ifdef _DEBUG
    std::cout << "DEBUG: Прочитаны коэффициенты: a=" << coeffs.a 
              << ", b=" << coeffs.b << ", c=" << coeffs.c << std::endl;
    #endif
    
    return coeffs;
}

// Функция для решения квадратного уравнения
QuadraticRoots solveQuadraticEquation(const QuadraticCoefficients& coeffs) {
    QuadraticRoots roots;
    roots.numRoots = 0;
    roots.root1 = 0.0;
    roots.root2 = 0.0;
    
    #ifdef _DEBUG
    std::cout << "DEBUG: Решение уравнения: " << coeffs.a << "x^2 + " 
              << coeffs.b << "x + " << coeffs.c << " = 0" << std::endl;
    #endif
    
    // Проверка на нулевой коэффициент a (линейное уравнение)
    if (std::abs(coeffs.a) < std::numeric_limits<double>::epsilon()) {
        // bx + c = 0
        if (std::abs(coeffs.b) < std::numeric_limits<double>::epsilon()) {
            // Уравнение c = 0
            if (std::abs(coeffs.c) < std::numeric_limits<double>::epsilon()) {
                // 0 = 0, бесконечно много решений
                #ifdef _DEBUG
                std::cout << "DEBUG: Уравнение имеет бесконечно много решений" << std::endl;
                #endif
                roots.numRoots = -1; // Код для бесконечно многих решений
            } else {
                // c = 0, нет решений
                #ifdef _DEBUG
                std::cout << "DEBUG: Уравнение не имеет решений" << std::endl;
                #endif
                roots.numRoots = 0;
            }
        } else {
            // bx + c = 0 => x = -c/b
            roots.numRoots = 1;
            roots.root1 = -coeffs.c / coeffs.b;
            #ifdef _DEBUG
            std::cout << "DEBUG: Линейное уравнение, корень: " << roots.root1 << std::endl;
            #endif
        }
    } else {
        // Квадратное уравнение: ax^2 + bx + c = 0
        double discriminant = coeffs.b * coeffs.b - 4.0 * coeffs.a * coeffs.c;
        
        #ifdef _DEBUG
        std::cout << "DEBUG: Дискриминант = " << discriminant << std::endl;
        #endif
        
        if (discriminant < 0.0) {
            // Нет действительных корней
            #ifdef _DEBUG
            std::cout << "DEBUG: Нет действительных корней" << std::endl;
            #endif
            roots.numRoots = 0;
        } else if (std::abs(discriminant) < std::numeric_limits<double>::epsilon()) {
            // Один корень (дискриминант = 0)
            roots.numRoots = 1;
            roots.root1 = -coeffs.b / (2.0 * coeffs.a);
            #ifdef _DEBUG
            std::cout << "DEBUG: Один корень: " << roots.root1 << std::endl;
            #endif
        } else {
            // Два корня
            roots.numRoots = 2;
            double sqrtDiscriminant = std::sqrt(discriminant);
            roots.root1 = (-coeffs.b + sqrtDiscriminant) / (2.0 * coeffs.a);
            roots.root2 = (-coeffs.b - sqrtDiscriminant) / (2.0 * coeffs.a);
            
            #ifdef _DEBUG
            std::cout << "DEBUG: Два корня: " << roots.root1 << " и " << roots.root2 << std::endl;
            #endif
        }
    }
    
    return roots;
}

// Функция для записи результатов в файл
void writeRootsToFile(const QuadraticRoots& roots, const std::string& filename) {
    std::ofstream outputFile(filename);
    
    if (!outputFile.is_open()) {
        throw std::runtime_error("Ошибка: не удалось открыть файл для записи " + filename);
    }
    
    #ifdef _DEBUG
    std::cout << "DEBUG: Запись результатов в файл " << filename << std::endl;
    #endif
    
    switch (roots.numRoots) {
        case -1:
            outputFile << "Уравнение имеет бесконечно много решений (все значения x)" << std::endl;
            break;
        case 0:
            outputFile << "Уравнение не имеет действительных корней" << std::endl;
            break;
        case 1:
            outputFile << "Уравнение имеет один корень: " << roots.root1 << std::endl;
            break;
        case 2:
            outputFile << "Уравнение имеет два корня: " << roots.root1 << " и " << roots.root2 << std::endl;
            break;
        default:
            outputFile << "Ошибка при решении уравнения" << std::endl;
    }
    
    outputFile.close();
    
    #ifdef _DEBUG
    std::cout << "DEBUG: Результаты успешно записаны в файл" << std::endl;
    #endif
}

int main(int argc, char* argv[]) {
    try {
        // Установка локали для поддержки русского языка
        std::locale::global(std::locale(""));
        
        // Имена файлов по умолчанию
        std::string inputFilename = "input.txt";
        std::string outputFilename = "output.txt";
        
        // Если заданы аргументы командной строки, используем их
        if (argc > 1) {
            inputFilename = argv[1];
        }
        
        if (argc > 2) {
            outputFilename = argv[2];
        }
        
        #ifdef _DEBUG
        std::cout << "DEBUG: Программа запущена в режиме отладки" << std::endl;
        std::cout << "DEBUG: Входной файл: " << inputFilename << std::endl;
        std::cout << "DEBUG: Выходной файл: " << outputFilename << std::endl;
        #else
        std::cout << "Программа запущена в режиме релиза" << std::endl;
        #endif
        
        // Основная логика программы:
        // 1. Чтение коэффициентов
        QuadraticCoefficients coeffs = readCoefficientsFromFile(inputFilename);
        
        // 2. Решение уравнения
        QuadraticRoots roots = solveQuadraticEquation(coeffs);
        
        // 3. Запись результатов
        writeRootsToFile(roots, outputFilename);
        
        #ifdef _DEBUG
        std::cout << "DEBUG: Программа успешно завершила работу" << std::endl;
        #else
        std::cout << "Программа успешно завершила работу" << std::endl;
        #endif
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Неизвестная ошибка" << std::endl;
        return 2;
    }
}