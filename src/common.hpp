#ifndef SP_2_COMMON_HPP
#define SP_2_COMMON_HPP

#include "check.hpp"
#include <iostream>
#include <wait.h>
#include <climits>
#include <chrono>
#include <vector>
#include <iomanip>
#include <random>
#include <algorithm>

typedef std::chrono::duration<double, std::micro> Micro;
typedef std::chrono::high_resolution_clock HRC;

struct OverallStat
{
    int     guessed;
    int     attempts;
    double  total_time;
};

bool comp_1(const int i) { return (i % 2 == 0); }
bool comp_2(const int i) { return (i % 2 != 0); }

bool is_exists(const pid_t p_id)
{
    int st;
    return (waitpid(p_id, &st, WNOHANG) <= 0);
}

void print_result(const std::pair<bool, int>& result, const double& runtime)
{
    std::cout << "Value was" << (result.first ? " " : "n't ") << "guessed\n";
    std::cout << "Number of attempts: " << result.second << '\n';
    std::cout << "Game time: " << (runtime / 1000.0) << " ms\n";
}

void print_stat(const OverallStat& stats, const int game_count)
{
    std::cout << "             Games: " << game_count << '\n';
    std::cout << "    Guessed values: " << stats.guessed << " (" << stats.guessed * 100 / game_count << "%)" << '\n';
    std::cout << "          Attempts: " << stats.attempts << '\n';
    std::cout << "  Average attempts: " << stats.attempts / game_count << '\n';
    std::cout << " Average game time: " << (stats.total_time / 1000.0 / game_count) << " ms\n";
}
#endif