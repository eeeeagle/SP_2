#ifndef UNNAMED_PIPE
#define UNNAMED_PIPE

#include "common.hpp"

namespace UP
{
    void riddler(const int pipe_fd[], const int n)
    {
        struct timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        srandom((time_t)ts.tv_nsec);

        int value = 1 + (int) random() % n;
        std::cout << "Guessed value: " << value << '\n';
        check(write(pipe_fd[1], &n, sizeof(n)));

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
    }

    std::pair<bool, int> guesser(const int pipe_fd[])
    {
        struct timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        srandom((time_t)ts.tv_nsec);

        int n;
        if (check(read(pipe_fd[0], &n, sizeof(int))))
        {
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
                }
                while(was_guessed[value - 1]);
                was_guessed[value - 1] = true;

                check(write(pipe_fd[1], &value, sizeof(value)));
                usleep(DELAY);

                if (check(read(pipe_fd[0], &flag, sizeof(bool))))
                    std::cout << std::setw(7) << i++ + 1 << std::setw(10) << value << std::setw(12) << (flag ? "true\n" : "false\n");
                else
                    _exit(EXIT_FAILURE);
            }
            std::cout << '\n';
            return std::make_pair(flag, i);
        }
        _exit(EXIT_FAILURE);
    }

    void player(const int i, const int fd[], const int n,
                std::pair<std::pair<int, int>, double>& stats, bool (*cmp)(const int))
    {
        if (cmp(i))
        {
            sleep(DELAY);
            riddler(fd, n);
        }
        else
        {
            auto start_time = HRC::now();
            const std::pair<bool, int> result = guesser(fd);

            auto end_time = HRC::now();
            print_result(result, Micro(end_time - start_time).count());

            if(result.first)
                stats.first.first++;
            stats.first.second += result.second;
            stats.second += Micro(end_time - start_time).count();

            sleep(DELAY);
        }
    }

    void start(const int n, const int count)
    {
        std::pair<std::pair<int, int>, double> stats {{0, 0}, 0.0};

        int fd[2];
        check(pipe(fd));
        pid_t p_id = check(fork());

        bool (*cmp)(const int);
        if (p_id)
            cmp = comp_1;
        else
            cmp = comp_2;

        for(int i = 0; i < count; i++)
            player(i, fd, n, stats, cmp);


        if(p_id)
        {
            std::pair<std::pair<int, int>, double> buffer;
            if(check(read(fd[0], &buffer, sizeof(std::pair<std::pair<int, int>, double>))))
            {
                stats.first.first += buffer.first.first;
                stats.first.second += buffer.first.second;
                stats.second += buffer.second;

                print_stat(stats, count);
            }
            close(fd[0]);
            close(fd[1]);

            waitpid(p_id, nullptr, 0);
            exit(EXIT_SUCCESS);
        }
        else
        {
            sleep(DELAY);
            check(write(fd[1], &stats, sizeof(std::pair<std::pair<int, int>, double>)));

            exit(EXIT_SUCCESS);
        }
    }
}
#endif