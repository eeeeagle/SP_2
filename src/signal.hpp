#ifndef SIGNAL
#define SIGNAL

#include "common.hpp"
#include <csignal>
#include <pthread.h>

namespace SIG
{
    volatile sig_atomic_t last_signal_id;
    volatile sig_atomic_t signal_value;

    void sig_handler(int signal_id)
    {
        last_signal_id = signal_id;
    }

    void rt_sig_handler(int signal_id, siginfo_t* signal_info, void* ctx)
    {
        last_signal_id  = signal_id;
        signal_value    = signal_info->si_value.sival_int;
    }

    bool exists(pid_t p, bool is_parent)
    {
        if (!is_parent)
            return (getpgid(p) >= 0);
        int stat;
        return waitpid(p, &stat, WNOHANG) == 0;
    }

    void riddler(pid_t p_id, const int n)
    {
        struct sigaction    rt_action {},
                            usr1_action{},
                            usr2_action{},
                            quit_action{};
        sigset_t set;
        sigemptyset(&set);

        rt_action.sa_sigaction = rt_sig_handler;
        rt_action.sa_flags = SA_SIGINFO;
        check(sigaction(SIGALRM, &rt_action, nullptr));

        usr1_action.sa_handler = sig_handler;
        check(sigaction(SIGUSR1, &usr1_action, nullptr));

        usr2_action.sa_handler = sig_handler;
        check(sigaction(SIGUSR2, &usr2_action, nullptr));

        quit_action.sa_handler = sig_handler;
        check(sigaction(SIGCHLD, &quit_action, nullptr));

        struct timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        srandom((time_t)ts.tv_nsec);

        int value = 1 + (int) random() % n;
        std::cout << "Guessed value: " << value << '\n';
        check(sigqueue(p_id, SIGALRM, sigval{ n }));

        bool flag = false;
        while (!flag)
        {
            do
            {
                sigsuspend(&set);
            } while(last_signal_id != SIGALRM);

            if(signal_value == value)
                flag = true;

            check(kill(p_id, (flag ? SIGUSR1 : SIGUSR2)));
        }
    }

    std::pair<bool, int> guesser(pid_t p_id, const bool is_parent)
    {
        struct sigaction    rt_action {},
                            usr1_action{},
                            usr2_action{},
                            quit_action{};
        sigset_t set;
        sigemptyset(&set);

        rt_action.sa_sigaction = rt_sig_handler;
        rt_action.sa_flags = SA_SIGINFO;
        check(sigaction(SIGALRM, &rt_action, nullptr));

        usr1_action.sa_handler = sig_handler;
        check(sigaction(SIGUSR1, &usr1_action, nullptr));

        usr2_action.sa_handler = sig_handler;
        check(sigaction(SIGUSR2, &usr2_action, nullptr));

        quit_action.sa_handler = sig_handler;
        check(sigaction(SIGCHLD, &quit_action, nullptr));

        sigsuspend(&set);
        const int n = signal_value;

        struct timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        srandom((time_t)ts.tv_nsec);

        bool flag = false;
        int i = 0;
        std::vector<bool> was_guessed(n, false);
        while (i < INT_MAX && !flag)
        {
            int value;
            do
            {
                value = 1 + (int)random() % n;
            } while(was_guessed[value - 1]);
            was_guessed[value - 1] = true;
            if(exists(p_id, is_parent))
                check(sigqueue(p_id, SIGALRM, sigval{ value }));

            do
            {
                sigsuspend(&set);
            } while (last_signal_id != SIGUSR1 && last_signal_id != SIGUSR2);

            if (last_signal_id == SIGUSR1)
                flag = true;

            std::cout << '[' << i++ + 1 << "]\t" << value << '\t' << (flag ? "true\n" : "false\n");
        }
        std::cout << '\n';
        return std::make_pair(flag, i);
    }

    void player(const int i, pid_t p_id, const bool is_parent, const int n,
                std::pair<std::pair<int, int>, double>& stats, bool (*cmp)(const int))
    {
        if (cmp(i))
        {
            std::cout << "GAME [" << i + 1 << "]\n";
            sleep(DELAY);
            riddler(p_id, n);
        }
        else
        {
            auto start_time = std::chrono::high_resolution_clock::now();
            const std::pair<bool, int> result = guesser(p_id, is_parent);

            auto end_time = std::chrono::high_resolution_clock::now();
            print_result(result, std::chrono::duration<double, std::micro>(end_time - start_time).count());

            if(result.first)
                stats.first.first++;
            stats.first.second += result.second;
            stats.second += std::chrono::duration<double, std::micro>(end_time - start_time).count();
            sleep(DELAY);
        }
    }

    void on_end()
    {
        int stat;
        wait(&stat);
        if (last_signal_id == SIGCHLD)
            exit(EXIT_SUCCESS);
    }

    void start(const int n, const int count)
    {
        pid_t p_id = check(fork());

        bool (*cmp)(const int);
        bool is_parent;

        if (p_id > 0)
        {
            is_parent = true;
            cmp = comp_1;
        }
        else
        {
            is_parent = false;
            cmp = comp_2;
        }

        std::pair<std::pair<int, int>, double> stats {{0, 0}, 0.0};
        for(int i = 0; i < count; i++)
            player(i, p_id ? p_id : getppid(), is_parent, n, stats, cmp);

        if(p_id)
        {
            struct sigaction    rt_action {},
                                quit_action{};
            sigset_t set;
            sigemptyset(&set);

            rt_action.sa_sigaction = rt_sig_handler;
            rt_action.sa_flags = SA_SIGINFO;
            check(sigaction(SIGALRM, &rt_action, nullptr));

            quit_action.sa_handler = sig_handler;
            check(sigaction(SIGCHLD, &quit_action, nullptr));


            sigsuspend(&set);
            stats.first.first += signal_value;

            sigsuspend(&set);
            stats.first.second += signal_value;

            sigsuspend(&set);
            stats.second += signal_value;

            print_stat(stats, count);

            if (atexit(on_end))
            {
                fprintf(stderr, "Failed to register function 1");
                _exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        else
        {
            sleep(DELAY);
            check(sigqueue(getppid(), SIGALRM, sigval{stats.first.first}));
            check(sigqueue(getppid(), SIGALRM, sigval{stats.first.second}));
            check(sigqueue(getppid(), SIGALRM, sigval{(int)stats.second}));

            exit(EXIT_SUCCESS);
        }
    }
}
#endif