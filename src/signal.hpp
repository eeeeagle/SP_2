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

    void default_sigmask()
    {
        struct sigaction    rt_action   {},
                            usr1_action {},
                            usr2_action {},
                            quit_action {};

        sigset_t set;
        sigemptyset(&set);
        sigfillset(&set);
        sigprocmask(SIG_SETMASK, &set, nullptr);

        rt_action.sa_sigaction = rt_sig_handler;
        rt_action.sa_flags = SA_SIGINFO;
        check(sigaction(SIGRTMAX, &rt_action, nullptr));

        usr1_action.sa_handler = sig_handler;
        check(sigaction(SIGUSR1, &usr1_action, nullptr));

        usr2_action.sa_handler = sig_handler;
        check(sigaction(SIGUSR2, &usr2_action, nullptr));

        quit_action.sa_handler = sig_handler;
        check(sigaction(SIGCHLD, &quit_action, nullptr));
    }

    bool is_exists(pid_t p, bool is_parent)
    {
        if (!is_parent)
            return (getpgid(p) >= 0);
        int stat;
        return waitpid(p, &stat, WNOHANG) == 0;
    }

    void player_left()
    {
        int stat;
        wait(&stat);
        if (last_signal_id == SIGCHLD)
            exit(EXIT_SUCCESS);
    }

    void player_riddler(const pid_t opponent, const int max_range, const bool is_parent)
    {
        static std::mt19937 gen(HRC::now().time_since_epoch().count());
        std::uniform_int_distribution<> uid(1, max_range);

        const int guessed_value = uid(gen);
        std::cout << "Guessed value: " << guessed_value << "\n\n";

        if(!is_exists(opponent, is_parent))
            exit(EXIT_FAILURE);

        check(sigqueue(opponent, SIGRTMAX, sigval{ max_range }));

        sigset_t set;
        sigemptyset(&set);
        sigfillset(&set);
        sigdelset(&set, SIGRTMAX);

        bool is_guessed = false;
        while (!is_guessed)
        {
            sigsuspend(&set);

            if(signal_value == guessed_value)
                is_guessed = true;

            if(!is_exists(opponent, is_parent))
                exit(EXIT_FAILURE);

            check(kill(opponent, (is_guessed ? SIGUSR1 : SIGUSR2)));
        }
    }

    std::pair<bool, int> player_guesser(const pid_t opponent, const bool is_parent)
    /* Returns as pair the number of attempts and a Boolean value indicating whether the guessed number was guessed */
    {
        {
            sigset_t set;
            sigemptyset(&set);
            sigfillset(&set);
            sigdelset(&set, SIGRTMAX);
            sigsuspend(&set);
        }

        const int max_range = signal_value;

        std::vector<int> attempt = {};
        for (int i = 1; i <= max_range; attempt.push_back(i++));
        std::random_device rd;
        std::mt19937 mt(rd());
        std::shuffle(attempt.begin(), attempt.end(), mt);

        std::cout << std::setw(7)  << "ATTEMPT"
                  << std::setw(10) << "VALUE"
                  << std::setw(12) << "FLAG" << std::endl;

        sigset_t set;
        sigemptyset(&set);
        sigfillset(&set);
        sigdelset(&set, SIGUSR1);
        sigdelset(&set, SIGUSR2);

        int try_count = 0;
        bool is_guessed = false;
        while (!attempt.empty() && try_count < INT_MAX && !is_guessed)
        {
            int value = attempt.back();
            attempt.pop_back();

            if(!is_exists(opponent, is_parent))
                exit(EXIT_FAILURE);
            check(sigqueue(opponent, SIGRTMAX, sigval{value}));

            do
            {
                sigsuspend(&set);

                if (last_signal_id == SIGUSR1)
                    is_guessed = true;
            } while (last_signal_id != SIGUSR1 && last_signal_id != SIGUSR2);
            std::cout << std::setw(7) << ++try_count
                      << std::setw(10) << value
                      << std::setw(12) << (is_guessed ? "true" : "false") << std::endl;
        }
        std::cout << std::endl;
        return std::make_pair(is_guessed, try_count);
    }

    void role_select(const int game_count, const pid_t opponent, const int max_range,
                     OverallStat& stats, bool (*const cmp)(const int), const bool is_parent)
    {
        if (cmp(game_count))
        {
            usleep(500);
            std::cout << "__________________________________\n\n";
            std::cout << "GAME [" << game_count + 1 << "]\n";
            player_riddler(opponent, max_range, is_parent);
        }
        else
        {
            auto start_time = HRC::now();
            const std::pair<bool, int> result = player_guesser(opponent, is_parent);
            auto end_time = HRC::now();

            print_result(result, Micro(end_time - start_time).count());

            if(result.first)
                stats.guessed++;
            stats.attempts += result.second;
            stats.total_time += Micro(end_time - start_time).count();
        }
    }

    void start(const int max_range, const int max_game_count)
    {
        default_sigmask();
        pid_t p_id = check(fork());

        const bool is_parent = p_id != 0;
        bool (* const cmp)(const int) = is_parent ? comp_1 : comp_2;

        if (atexit(player_left))
        {
            fprintf(stderr, "Failed to register role_select left function");
            exit(EXIT_FAILURE);
        }

        OverallStat stats {0, 0, 0.0};
        for(int game_count = 0; game_count < max_game_count; game_count++)
            role_select(game_count, p_id ? p_id : getppid(), max_range, stats, cmp, is_parent);

        if(p_id)
        {
            struct sigaction    rt_action   {},
                                quit_action {};
            sigset_t set;
            sigemptyset(&set);

            rt_action.sa_sigaction = rt_sig_handler;
            rt_action.sa_flags = SA_SIGINFO;
            check(sigaction(SIGRTMAX, &rt_action, nullptr));

            quit_action.sa_handler = sig_handler;
            check(sigaction(SIGCHLD, &quit_action, nullptr));

            sigsuspend(&set);
            stats.guessed += signal_value;

            sigsuspend(&set);
            stats.attempts += signal_value;

            sigsuspend(&set);
            stats.total_time += signal_value;

            std::cout << "__________________________________\n\n";
            print_stat(stats, max_game_count);
            std::cout << "__________________________________\n\n";

            exit(EXIT_SUCCESS);
        }
        else
        {
            check(sigqueue(getppid(), SIGRTMAX, sigval{stats.guessed}));
            check(sigqueue(getppid(), SIGRTMAX, sigval{stats.attempts}));
            check(sigqueue(getppid(), SIGRTMAX, sigval{(int)stats.total_time}));
            exit(EXIT_SUCCESS);
        }
    }
}
#endif