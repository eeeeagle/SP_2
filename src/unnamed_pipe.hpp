#ifndef UNNAMED_PIPE
#define UNNAMED_PIPE

#include "check.hpp"
#include <chrono>
#include <unistd.h>
#include <wait.h>
#define SLEEP_TIME 1

namespace UP
{
    void on_exit()
    {
        std::cout << "KASASGKSGKSAGSAGKSK SHIIIIIIIIIIT" << std::endl;
    }

    void print_stat(const std::pair<bool, int>& result, const double& runtime)
    {
        std::cout << std::endl;
        std::cout << "Value was" << (result.first ? " " : "n't ") << "guessed" << std::endl;
        std::cout << "Number of attempts: " << result.second << std::endl;
        std::cout << "Game time: " << runtime << " seconds" << std::endl;
        std::cout << "__________________________________" << std::endl << std::endl;
    }

    void riddler(int pipe_fd[], const int n)
    {
        srandom(time(nullptr));
        int value = 1 + (int) random() % n;
        sleep(SLEEP_TIME);
        std::cout << "Guessed value: " << value << std::endl;

        int x = check(write(pipe_fd[1], &n, sizeof(n)));
        sleep(SLEEP_TIME);

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
    }

    void guesser(int pipe_fd[], std::pair<bool, int>& result)
    {
        sleep(SLEEP_TIME);
        int n;
        int r = check(read(pipe_fd[0], &n, sizeof(int)));
        if (r > 0)
        {
            bool flag = false;
            int i = 0;
            while (i < INT_MAX && !flag)
            {
                srandom(time(nullptr));
                int value = 1 + (int)random() % n;
                r = check(write(pipe_fd[1], &value, sizeof(value)));
                sleep(SLEEP_TIME);

                r = check(read(pipe_fd[0], &flag, sizeof(bool)));
                if (r > 0)
                {
                    std::cout << '[' << i + 1 << "]\t" << value << "\t" << (flag ? "true" : "false") << std::endl;
                }
                i++;
            }
            result = std::make_pair(flag, i + 1);
        }
    }

    void start(const int n)
    {
        std::pair<bool, int> result;

        int fd[2];
        check(pipe(fd));
        int p_id = check(fork());

        for(int i = 0; i < 10; i++)
        {
            auto start_time = std::chrono::high_resolution_clock::now();

            if (p_id)
            {
                std::cout << "GAME [" << i + 1 << "]" << std::endl << std::endl;

                if (i % 2)
                    riddler(fd, n);
                else
                    guesser(fd, result);
                int stat;
                wait(&stat);

                auto end_time = std::chrono::high_resolution_clock::now();
                print_stat(result, std::chrono::duration<double, std::milli>(end_time - start_time).count() / 1000.0);
            }
            else
            {
                atexit(on_exit);
                if (i % 2)
                    guesser(fd, result);
                else
                    riddler(fd, n);
            }
        }
        exit(EXIT_SUCCESS);
    }
}
#endif