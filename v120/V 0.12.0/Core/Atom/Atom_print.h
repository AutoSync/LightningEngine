#ifndef PRINT_H
#define PRINT_H

#include <iostream>
#include <chrono>
#include <string>

using namespace std::chrono;
class Print
{
private: 
    steady_clock::time_point Start;
    steady_clock::time_point End;
public:
    // Write a line and jump when done
    void init()
    {
        Start = steady_clock::now();
    }
    void Log(std::string text)
    {
        End = steady_clock::now();
        std::cout << "[" << durationProcess(Start, End) << "] " << text << std::endl;
    }
    // Type text without skipping a line
    void msg(std::string text)
    {
        std::cout << text;
    }
    void now(std::string text)
    {
        End = steady_clock::now();
        std::cout << "[" << durationProcess(Start, End) << "] " << text << std::endl;
    }
private:
    std::string durationProcess(steady_clock::time_point Start, steady_clock::time_point End)
    {    
        duration<double> time = duration_cast<duration<double>>(End - Start);
        return std::to_string(time.count()) + " seconds";
    }
};



#endif

 