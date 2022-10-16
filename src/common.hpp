#ifndef SP_2_COMMON_HPP
#define SP_2_COMMON_HPP

#include "check.hpp"
#include <iostream>
#include <unistd.h>
#include <wait.h>
#include <climits>
#include <chrono>

bool comp_1(const int i) { return (i % 2 == 0); }
bool comp_2(const int i) { return !comp_1(i); }

void print_result(const std::pair<bool, int>& result, const double& runtime)
{
    std::cout << "Value was" << (result.first ? " " : "n't ") << "guessed" << std::endl;
    std::cout << "Number of attempts: " << result.second << std::endl;
    std::cout << "Game time: " << (runtime / 1000.0) << " ms" << std::endl;
    std::cout << "__________________________________" << std::endl << std::endl;
}

void print_stat(const std::pair<std::pair<int, int>, double>& stats, const int n)
{
    std::cout << "             Games: " << n << std::endl;
    std::cout << "    Guessed values: " << stats.first.first << " (" << stats.first.first * 100 / n << "%)" << std::endl;
    std::cout << "          Attempts: " << stats.first.second << std::endl;
    std::cout << "  Average attempts: " << (int)(stats.first.second / n) << std::endl;
    std::cout << " Average game time: " << (stats.second / 1000.0 / n) << " ms" << std::endl;
    std::cout << "__________________________________" << std::endl;
}

#endif
