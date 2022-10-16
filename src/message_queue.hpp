#ifndef MESSAGE_QUEUE
#define MESSAGE_QUEUE

#include "common.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

/* TO DO */

namespace MQ
{
    void riddler(mqd_t mk_d, const int n)
    {
        struct timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        srandom((time_t)ts.tv_nsec);

        int value = 1 + (int) random() % n;
        /* TO DO
        check(sigqueue(p_id, SIGCONT, sigval{n}));
        sig_handler(SIGCONT, n);

        std::cout << "Guessed value: " << value << '\n';

        bool flag = false;
        while (!flag)
        {
            std::cout << "waiting value from guesser " << '\n';
            waitpid(p_id, nullptr, WCONTINUED);
            if(signal_value == value)
                flag = true;
            sig_handler(flag ? SIGUSR1 : SIGUSR2);
            std::cout << p_id << '\n';
            check(kill(p_id, (flag ? SIGUSR1 : SIGUSR2)));
        }
        std::cout << "waiting guesser end" << '\n';
        waitpid(p_id, nullptr, WCONTINUED);
        */
    }

    std::pair<bool, int> guesser(mqd_t mk_d)
    {
        /* TO DO
        std::cout << "waiting max possible value " << '\n';
        waitpid(p_id, nullptr, WCONTINUED);
        const int n = signal_value;
        std::cout << "Got max possible value: " << n << '\n';
        sigset_t sig_set;
        sigfillset(&sig_set);
        sigdelset(&sig_set, SIGUSR1);
        sigdelset(&sig_set, SIGUSR2);
        */
        bool flag = false;
        int i = 0;
        /*
        while (i < INT_MAX && !flag)
        {
            struct timespec ts{};
            clock_gettime(CLOCK_MONOTONIC, &ts);
            srandom((time_t)ts.tv_nsec);

            int value = 1 + (int) random() % n;
            check(sigqueue(p_id, SIGCONT, sigval{value}));
            sig_handler(SIGCONT, value);
            std::cout << "waiting signal from riddler " << '\n';
            sigsuspend(&sig_set);

            if (last_signal_id == SIGUSR1)
                flag = true;

            std::cout << '[' << i++ + 1 << "]\t" << value << '\t' << (flag ? "true" : "false") << '\n';
        }
        std::cout << '\n';
        check(kill(p_id, SIGCONT));
        */
        return std::make_pair(flag, i);
    }

    void player(const int i, mqd_t mq_d , const int n, std::pair<std::pair<int, int>, double>& stats, bool (*cmp)(const int))
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        if (cmp(i))
            riddler(mq_d, n);
        else
        {
            const std::pair<bool, int> result = guesser(mq_d);

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
        mqd_t mq_d = check(mq_open("/mq", O_WRONLY)); //open for write
        for(int i = 0; i < count; i++)
        {
            if (p_id)
                player(i, mq_d, n, stats, comp_1);
            else
                player(i, mq_d, n, stats, comp_2);
        }

        if(p_id)
        {
            int* buf = new int[8 * 1024 / sizeof(int)];
            int r;
            do
            {
                r = check(mq_receive(mq_d, (char*)buf, 8 * 1024, nullptr));
                //printf("Child Got %d from the queue\n", buf[0]);
            } while (r > 0 && *buf != -1);
            delete[] buf;

            print_stat(stats, count);

            wait(nullptr);
            mq_unlink("/mq"); //remove the queue from the filesystem
            exit(EXIT_SUCCESS);
        }
        else
        {
            /* TO DO
            check(sigqueue(p_id, SIGCONT, sigval{stats.first.first}));
            sig_handler(SIGCONT, stats.first.first);
            waitpid(getpid(), nullptr, WCONTINUED);

            check(sigqueue(p_id, SIGCONT, sigval{stats.first.second}));
            sig_handler(SIGCONT, stats.first.second);
            waitpid(getpid(), nullptr, WCONTINUED);

            check(sigqueue(p_id, SIGCONT, sigval{(int)stats.second}));
            sig_handler(SIGCONT, (int)stats.second);
            waitpid(getpid(), nullptr, WCONTINUED);
            */
            exit(EXIT_SUCCESS);
        }
    }
}
#endif