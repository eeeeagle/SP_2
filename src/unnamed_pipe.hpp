#ifndef UNNAMED_PIPE
#define UNNAMED_PIPE

#include "check.hpp"
#include <chrono>
#include <unistd.h>
#include <wait.h>
#include <tuple>

#define DELAY 10

namespace UP
{
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

    void riddler(int pipe_fd[], const int n)
    {
        struct timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        srandom((time_t)ts.tv_nsec);

        int value = 1 + (int) random() % n;
        usleep(DELAY);
        std::cout << "Guessed value: " << value << std::endl;

        int x = check(write(pipe_fd[1], &n, sizeof(n)));
        usleep(DELAY);

        bool flag = false;
        while (!flag)
        {
            int buffer;
            x = check(read(pipe_fd[0], &buffer, sizeof(int)));
            if (x > 0)
            {
                if(buffer == value)
                    flag = true;
                x = check(write(pipe_fd[1], &flag, sizeof(flag)));
            }
        }
        usleep(13);
    }

    void guesser(int pipe_fd[], std::pair<bool, int>& result)
    {
        usleep(DELAY);
        int n;
        if (check(read(pipe_fd[0], &n, sizeof(int))))
        {
            bool flag = false;
            int i = 0;
            while (i < INT_MAX && !flag)
            {
                struct timespec ts{};
                clock_gettime(CLOCK_MONOTONIC, &ts);
                srandom((time_t)ts.tv_nsec);

                int value = 1 + (int)random() % n;
                check(write(pipe_fd[1], &value, sizeof(value)));
                usleep(DELAY);

                if (check(read(pipe_fd[0], &flag, sizeof(bool))))
                    std::cout << '[' << i++ + 1 << "]\t" << value << "\t" << (flag ? "true" : "false") << std::endl;
            }
            std::cout << std::endl;
            result = std::make_pair(flag, i);
        }
    }

    bool comp_1(const int i)
    {
        return (i % 2 == 0);
    }

    bool comp_2(const int i)
    {
        return !comp_1(i);
    }

    void shit(const int i, int fd[], const int n, std::pair<std::pair<int, int>, double>& stats, bool (*cmp)(const int))
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        if (cmp(i))
            riddler(fd, n);
        else
        {
            std::pair<bool, int> result;
            guesser(fd, result);

            auto end_time = std::chrono::high_resolution_clock::now();
            print_result(result, std::chrono::duration<double, std::micro>(end_time - start_time).count());

            if(result.first)
                stats.first.first++;
            stats.first.second += result.second;
            stats.second += std::chrono::duration<double, std::micro>(end_time - start_time).count();
        }
    }


    void start(const int n)
    {
        std::pair<std::pair<int, int>, double> stats {{0, 0}, 0.0};

        int fd[2];
        check(pipe(fd));
        pid_t p_id = check(fork());

        for(int i = 0; i < 10; i++)
        {
            if (p_id)
                shit(i, fd, n, stats, comp_1);
            else
                shit(i, fd, n, stats, comp_2);
        }


        if(p_id)
        {
            usleep(500);
            std::pair<std::pair<int, int>, double> buffer;
            if(check(read(fd[0], &buffer, sizeof(std::pair<std::pair<int, int>, double>))))
            {
                stats.first.first += buffer.first.first;
                stats.first.second += buffer.first.second;
                stats.second += buffer.second;
            }
            close(fd[0]);
            close(fd[1]);

            print_stat(stats, n);

            waitpid(p_id, nullptr, 0);
            exit(EXIT_SUCCESS);
        }
        else
        {
            check(write(fd[1], &stats, sizeof(std::pair<std::pair<int, int>, double>)));
            usleep(DELAY);
            exit(EXIT_SUCCESS);
        }
    }
}
#endif