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
        std::cout << "Guessed value: " << value << "\n\n";

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
    /* Returns as pair the number of attempts and a Boolean value indicating whether the guessed number was guessed */
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


        if(!exists(p_id, is_parent))
            exit(EXIT_FAILURE);

        do
        {
            sigsuspend(&set);
        } while (last_signal_id != SIGALRM);
        const int n = signal_value;

        struct timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        srandom((time_t)ts.tv_nsec);

        bool flag = false;
        int i = 0;
        std::vector<bool> was_guessed(n, false);
        std::cout << std::setw(7) << "ATTEMPT" << std::setw(10) << "VALUE" << std::setw(12) << "FLAG\n";
        while (i < INT_MAX && !flag)
        {
            int value;
            do
            {
                value = 1 + (int)random() % n;
            } while(was_guessed[value - 1]);
            was_guessed[value - 1] = true;

            if(!exists(p_id, is_parent))
                exit(EXIT_FAILURE);

            check(sigqueue(p_id, SIGALRM, sigval{ value }));
            do
            {
                sigsuspend(&set);
            } while (last_signal_id != SIGUSR1 && last_signal_id != SIGUSR2);

            if (last_signal_id == SIGUSR1)
                flag = true;

            std::cout << std::setw(7) << i++ + 1 << std::setw(10) << value << std::setw(12) << (flag ? "true\n" : "false\n");
        }
        std::cout << '\n';
        return std::make_pair(flag, i);
    }

    void player(const int i, pid_t p_id, const bool is_parent, const int n,
                std::pair<std::pair<int, int>, double>& stats, bool (*cmp)(const int))
    {
        if (cmp(i))
        {
            raise(SIGSTOP);

            std::cout << "GAME [" << i+1 << "]\n";
            riddler(p_id, n);

            raise(SIGSTOP);
        }
        else
        {
            std::cout << "__________________________________\n\n";
            kill(p_id, SIGCONT);

            auto start_time = HRC::now();
            const std::pair<bool, int> result = guesser(p_id, is_parent);
            auto end_time = HRC::now();

            print_result(result, std::chrono::duration<double, std::micro>(end_time - start_time).count());

            if(result.first)
                stats.first.first++;
            stats.first.second += result.second;
            stats.second += Micro(end_time - start_time).count();

            kill(p_id, SIGCONT);
        }
    }

    void player_left()
    {
        int stat;
        wait(&stat);
        if (last_signal_id == SIGCHLD)
            exit(EXIT_SUCCESS);
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

        if (atexit(player_left))
        {
            fprintf(stderr, "Failed to register function 1");
            _exit(EXIT_FAILURE);
        }

        std::pair<std::pair<int, int>, double> stats {{0, 0}, 0.0};
        for(int i = 0; i < count; i++)
            player(i, p_id ? p_id : getppid(), is_parent, n, stats, cmp);

        if(p_id)
        {
            struct sigaction rt_action {};
            sigset_t set;
            sigemptyset(&set);

            rt_action.sa_sigaction = rt_sig_handler;
            rt_action.sa_flags = SA_SIGINFO;
            check(sigaction(SIGALRM, &rt_action, nullptr));

            if(!exists(p_id, is_parent))
                exit(EXIT_FAILURE);

            do
            {
                sigsuspend(&set);
            } while(last_signal_id != SIGALRM);
            stats.first.first += signal_value;

            if(!exists(p_id, is_parent))
                exit(EXIT_FAILURE);

            do
            {
                sigsuspend(&set);

            } while(last_signal_id != SIGALRM);
            stats.first.second += signal_value;

            if(!exists(p_id, is_parent))
                exit(EXIT_FAILURE);

            do
            {
                sigsuspend(&set);

            } while(last_signal_id != SIGALRM);
            stats.second += signal_value;

            std::cout << "__________________________________\n\n";
            print_stat(stats, count);
            std::cout << "__________________________________\n\n";

            if (atexit(on_end))
            {
                fprintf(stderr, "Failed to register function 1");
                _exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        else
        {
            check(sigqueue(getppid(), SIGALRM, sigval{stats.first.first}));
            check(sigqueue(getppid(), SIGALRM, sigval{stats.first.second}));
            check(sigqueue(getppid(), SIGALRM, sigval{(int)(stats.second)}));

            exit(EXIT_SUCCESS);
        }
    }
}
#endif