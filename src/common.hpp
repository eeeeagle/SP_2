#ifndef SP_2_COMMON_HPP
#define SP_2_COMMON_HPP

#include "check.hpp"
#include <iostream>
#include <wait.h>
#include <climits>
#include <chrono>
#include <vector>

#define DELAY 1

bool comp_1(const int i) { return (i % 2 == 0); }
bool comp_2(const int i) { return (i % 2 != 0); }

void print_result(const std::pair<bool, int>& result, const double& runtime)
{
    std::cout << "Value was" << (result.first ? " " : "n't ") << "guessed\n";
    std::cout << "Number of attempts: " << result.second << '\n';
    std::cout << "Game time: " << (runtime / 1000.0) << " ms\n";
}

void print_stat(const std::pair<std::pair<int, int>, double>& stats, const int n)
{
    std::cout << "             Games: " << n << '\n';
    std::cout << "    Guessed values: " << stats.first.first << " (" << stats.first.first * 100 / n << "%)" << '\n';
    std::cout << "          Attempts: " << stats.first.second << '\n';
    std::cout << "  Average attempts: " << (int)(stats.first.second / n) << '\n';
    std::cout << " Average game time: " << (stats.second / 1000.0 / n) << " ms\n";
}

#endif
