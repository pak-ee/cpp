#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <map>
#include <sstream>

// Квадратное уравнение
struct Equation {
    double a, b, c;
    Equation(double a, double b, double c) : a(a), b(b), c(c) {}
};

// Решение уравнения
struct Solution {
    std::vector<double> roots;
    int count;
    
    Solution() : count(0) {}
    Solution(double root) : count(1) { 
        roots.push_back(root); 
    }
    Solution(double x1, double x2) : count(2) { 
        roots.push_back(x1); 
        roots.push_back(x2); 
    }
};

// Функция для решения квадратного уравнения
Solution solve(const Equation& eq) {
    double eps = 1e-9;
    
    if (abs(eq.a) < eps) { // линейное уравнение
        if (abs(eq.b) < eps) {
            return Solution(); // 0 = 0
        }
        return Solution(-eq.c / eq.b);
    }
    
    // квадратное уравнение ax^2 + bx + c = 0
    double d = eq.b * eq.b - 4 * eq.a * eq.c;
    
    if (d < -eps) {
        return Solution(); // нет корней
    } else if (abs(d) < eps) {
        return Solution(-eq.b / (2 * eq.a)); // один корень
    } else {
        double x1 = (-eq.b + sqrt(d)) / (2 * eq.a);
        double x2 = (-eq.b - sqrt(d)) / (2 * eq.a);
        return Solution(x1, x2); // два корня
    }
}

class Student {
protected:
    std::string name;
    
public:
    Student(std::string n) : name(n) {}
    virtual ~Student() = default;
    
    std::string getName() { return name; }
    virtual Solution solveTask(const Equation& eq) = 0;
};

// Хороший студент
class GoodStudent : public Student {
public:
    GoodStudent(std::string name) : Student(name) {}
    
    Solution solveTask(const Equation& eq) override {
        return solve(eq);
    }
};

// Средний студент  
class AverageStudent : public Student {
public:
    AverageStudent(std::string name) : Student(name) {}
    
    Solution solveTask(const Equation& eq) override {
        if (rand() % 100 < 60) {
            return solve(eq);
        } else {
            // ошибочный ответ
            return Solution(rand() % 10 - 5);
        }
    }
};

// Плохой студент
class BadStudent : public Student {
public:
    BadStudent(std::string name) : Student(name) {}
    
    Solution solveTask(const Equation& eq) override {
        return Solution(0.0); // всегда 0
    }
};

// Письмо с решением
struct Mail {
    Equation eq;
    Solution answer;
    std::string student;
    
    Mail(Equation e, Solution s, std::string st) : eq(e), answer(s), student(st) {}
};

// Преподаватель
class Teacher {
private:
    std::queue<Mail> mails;
    std::map<std::string, int> scores;
    
    bool isCorrect(const Equation& eq, const Solution& ans) {
        Solution right = solve(eq);
        double eps = 1e-6;
        
        if (right.count != ans.count) return false;
        
        if (right.count == 0) return true;
        
        if (right.count == 1) {
            return abs(right.roots[0] - ans.roots[0]) < eps;
        }
        
        if (right.count == 2) {
            bool ok1 = (abs(right.roots[0] - ans.roots[0]) < eps && 
                       abs(right.roots[1] - ans.roots[1]) < eps);
            bool ok2 = (abs(right.roots[0] - ans.roots[1]) < eps && 
                       abs(right.roots[1] - ans.roots[0]) < eps);
            return ok1 || ok2;
        }
        
        return false;
    }
    
public:
    void getMail(Mail m) {
        mails.push(m);
    }
    
    void checkMails() {
        std::cout << "\nПреподаватель проверяет работы...\n";
        
        while (!mails.empty()) {
            Mail m = mails.front();
            mails.pop();
            
            if (scores.find(m.student) == scores.end()) {
                scores[m.student] = 0;
            }
            
            if (isCorrect(m.eq, m.answer)) {
                scores[m.student]++;
            }
        }
        
        std::cout << "Проверка завершена!\n";
    }
    
    void showResults() {
        std::cout << "\n--- РЕЗУЛЬТАТЫ ЗАЧЕТА ---\n";
        for (auto p : scores) {
            std::cout << p.first << ": " << p.second << " правильных\n";
        }
        std::cout << "\n";
    }
};

int main() {
    srand(time(0));
    
    std::cout << "Зачет по математике\n\n";
    
    // студенты
    std::vector<Student*> students = {
        new GoodStudent("Петров"),
        new GoodStudent("Иванова"),
        new AverageStudent("Сидоров"),
        new AverageStudent("Козлова"),
        new BadStudent("Неучёв"),
        new BadStudent("Лентяйкин")
    };
    
    Teacher teacher;
    
    // читаем задачи
    std::ifstream file("input.txt");
    if (!file.is_open()) {
        std::cout << "Ошибка чтения файла\n";
        return 1;
    }
    
    std::vector<Equation> tasks;
    std::string line;
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        double a, b, c;
        
        if (iss >> a >> b >> c) {
            tasks.push_back(Equation(a, b, c));
        }
    }
    file.close();
    
    std::cout << "Задач: " << tasks.size() << "\n";
    std::cout << "Студентов: " << students.size() << "\n\n";
    
    // студенты решают
    std::cout << "Студенты решают задачи:\n";
    for (auto task : tasks) {
        for (auto student : students) {
            Solution ans = student->solveTask(task);
            Mail mail(task, ans, student->getName());
            teacher.getMail(mail);
        }
    }
    
    // проверяем
    teacher.checkMails();
    teacher.showResults();
    
    // освобождаем память
    for (auto s : students) {
        delete s;
    }
    
    return 0;
} 