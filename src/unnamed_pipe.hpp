#ifndef UNNAMED_PIPE
#define UNNAMED_PIPE

#include "common.hpp"

#define DELAY 10

namespace UP
{
    void riddler(int pipe_fd[], const int n)
    {
        struct timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        srandom((time_t)ts.tv_nsec);

        int value = 1 + (int) random() % n;
        usleep(DELAY);
        std::cout << "Guessed value: " << value << std::endl;

        check(write(pipe_fd[1], &n, sizeof(n)));
        usleep(DELAY);

        bool flag = false;
        while (!flag)
        {
            int buffer;
            if (check(read(pipe_fd[0], &buffer, sizeof(int))))
            {
                if(buffer == value)
                    flag = true;
                check(write(pipe_fd[1], &flag, sizeof(flag)));
            }
            else
                _exit(EXIT_FAILURE);
        }
        usleep(13);
    }

    std::pair<bool, int> guesser(int pipe_fd[])
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
                else
                    _exit(EXIT_FAILURE);
            }
            std::cout << std::endl;
            return std::make_pair(flag, i);
        }
        _exit(EXIT_FAILURE);
    }

    bool comp_1(const int i) { return (i % 2 == 0); }
    bool comp_2(const int i) { return !comp_1(i); }

    void player(const int i, int fd[], const int n, std::pair<std::pair<int, int>, double>& stats, bool (*cmp)(const int))
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        if (cmp(i))
            riddler(fd, n);
        else
        {
            const std::pair<bool, int> result = guesser(fd);

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

        int fd[2];
        check(pipe(fd));
        pid_t p_id = check(fork());

        for(int i = 0; i < count; i++)
        {
            if (p_id)
                player(i, fd, n, stats, comp_1);
            else
                player(i, fd, n, stats, comp_2);
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

            print_stat(stats, count);

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