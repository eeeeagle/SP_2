#ifndef SIGNAL
#define SIGNAL

#include "common.hpp"
#include <csignal>
#include <pthread.h>

/* TO DO */

namespace SIG
{
    volatile sig_atomic_t last_signal_id;
    volatile sig_atomic_t signal_value;

    void sig_handler(const int signal_id)
    {
        last_signal_id = signal_id;
    }

    void sig_handler(const int signal_id, const int value)
    {
        last_signal_id = signal_id;
        signal_value = value;
    }

    void riddler(pid_t p_id, const int n)
    {
        struct timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        srandom((time_t)ts.tv_nsec);

        int value = 1 + (int) random() % n;
        check(sigqueue(p_id, SIGCONT, sigval{n}));
        sig_handler(SIGCONT, n);

        std::cout << "Guessed value: " << value << std::endl;

        bool flag = false;
        while (!flag)
        {
            std::cout << "waiting value from guesser " << std::endl;
            waitpid(p_id, nullptr, WCONTINUED);
            if(signal_value == value)
                flag = true;
            sig_handler(flag ? SIGUSR1 : SIGUSR2);
            std::cout << p_id << std::endl;
            check(kill(p_id, (flag ? SIGUSR1 : SIGUSR2)));
        }
        std::cout << "waiting guesser end" << std::endl;
        waitpid(p_id, nullptr, WCONTINUED);
    }

    std::pair<bool, int> guesser(pid_t p_id)
    {
        std::cout << "waiting max possible value " << std::endl;
        waitpid(p_id, nullptr, WCONTINUED);
        const int n = signal_value;
        std::cout << "Got max possible value: " << n << std::endl;
        sigset_t sig_set;
        sigfillset(&sig_set);
        sigdelset(&sig_set, SIGUSR1);
        sigdelset(&sig_set, SIGUSR2);

        bool flag = false;
        int i = 0;
        while (i < INT_MAX && !flag)
        {
            struct timespec ts{};
            clock_gettime(CLOCK_MONOTONIC, &ts);
            srandom((time_t)ts.tv_nsec);

            int value = 1 + (int) random() % n;
            check(sigqueue(p_id, SIGCONT, sigval{value}));
            sig_handler(SIGCONT, value);
            std::cout << "waiting signal from riddler " << std::endl;
            sigsuspend(&sig_set);

            if (last_signal_id == SIGUSR1)
                flag = true;

            std::cout << '[' << i++ + 1 << "]\t" << value << '\t' << (flag ? "true" : "false") << std::endl;
        }
        std::cout << std::endl;
        check(kill(p_id, SIGCONT));
        return std::make_pair(flag, i);
    }

    void player(const int i, pid_t p_id , const int n, std::pair<std::pair<int, int>, double>& stats, bool (*cmp)(const int))
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        if (cmp(i))
            riddler(p_id, n);
        else
        {
            const std::pair<bool, int> result = guesser(p_id);

            auto end_time = std::chrono::high_resolution_clock::now();
            print_result(result, std::chrono::duration<double, std::micro>(end_time - start_time).count());

            if(result.first)
                stats.first.first++;
            stats.first.second += result.second;
            stats.second += std::chrono::duration<double, std::micro>(end_time - start_time).count();
        }
    }

    void start(const int n, const int count = 10)
    {
        std::pair<std::pair<int, int>, double> stats {{0, 0}, 0.0};

        pid_t p_id = check(fork());

        for(int i = 0; i < count; i++)
        {
            if (p_id)
                player(i, p_id, n, stats, comp_1);
            else
                player(i, getpid(), n, stats, comp_2);
        }

        if(p_id)
        {
            waitpid(p_id, nullptr, WCONTINUED);
            stats.first.first += signal_value;
            kill(p_id, SIGCONT);

            waitpid(p_id, nullptr, WCONTINUED);
            stats.first.second += signal_value;
            kill(p_id, SIGCONT);


            waitpid(p_id, nullptr, WCONTINUED);
            stats.second += signal_value;
            kill(p_id, SIGCONT);

            print_stat(stats, count);

            waitpid(p_id, nullptr, 0);
            exit(EXIT_SUCCESS);
        }
        else
        {
            check(sigqueue(p_id, SIGCONT, sigval{stats.first.first}));
            sig_handler(SIGCONT, stats.first.first);
            waitpid(getpid(), nullptr, WCONTINUED);

            check(sigqueue(p_id, SIGCONT, sigval{stats.first.second}));
            sig_handler(SIGCONT, stats.first.second);
            waitpid(getpid(), nullptr, WCONTINUED);

            check(sigqueue(p_id, SIGCONT, sigval{(int)stats.second}));
            sig_handler(SIGCONT, (int)stats.second);
            waitpid(getpid(), nullptr, WCONTINUED);

            exit(EXIT_SUCCESS);
        }
    }
}
#endif