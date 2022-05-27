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
   
    void Log(std::string text, int State = 0)
    {
        if (State == 0)
        {
            Start = steady_clock::now();
            std::cout << text <<"..."<< std::endl;
        }
        if (State == 1)
        {
            End = steady_clock::now();
            std::cout << text << " ";
            durationProcess(Start, End);
        }
    }
    // Type text without skipping a line
    void msg(std::string text)
    {
        std::cout << text;
    }
    void durationProcess(steady_clock::time_point Start, steady_clock::time_point End)
    {
        // std::cout << " " << duration_cast<microseconds>(End - Start).count() << " In Microseconds" << std::endl;
        duration<double> time = duration_cast<duration<double>>(End - Start);
        std::cout << " " << time.count() << " seconds" << std::endl;
    }
};



#endif

 