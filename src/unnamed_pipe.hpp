#ifndef UNNAMED_PIPE
#define UNNAMED_PIPE

#include "check.hpp"
#include <chrono>
#include <unistd.h>
#include <wait.h>

namespace UP
{
    void riddler(int pipe_fd[], const int n)
    {
        srandom(time(nullptr));
        int value = 1 + (int) random() % n;
        sleep(2);
        std::cout << "Guessed value: " << value << std::endl;

        int x = check(write(pipe_fd[1], &n, sizeof(n)));
        sleep(1);

        bool flag = true;
        while (flag)
        {
            int buffer;
            x = check(read(pipe_fd[0], &buffer, sizeof(int)));
            if (x > 0)
            {
                if(buffer == value)
                    flag = false;
                x = check(write(pipe_fd[1], &flag, sizeof(flag)));
                //sleep(1);
            }
        }
    }

    void guesser(int pipe_fd[])
    {
        sleep(2);
        int n;
        int r = check(read(pipe_fd[0], &n, sizeof(int)));
        if (r > 0)
        {
            bool flag = false;
            for (int count = 0; count < INT_MAX && !flag; count++)
            {
                srandom(time(nullptr));
                int value = 1 + (int)random() % n;
                r = check(write(pipe_fd[1], &value, sizeof(value)));
                sleep(1);

                r = check(read(pipe_fd[0], &flag, sizeof(bool)));
                if (r > 0)
                {
                    std::cout << '[' << count + 1 << "]\t" << value << "\t" << (flag ? "true" : "false") << std::endl;
                }
            }
        }
        exit(EXIT_SUCCESS);
    }

    void start(const int n)
    {
        auto start_time = std::chrono::high_resolution_clock::now();

        int fd[2];
        check(pipe(fd));
        int p_id = check(fork());
        if (p_id)
        {
            riddler(fd, n);
            int stat;
            wait(&stat);
        }
        else
        {
            guesser(fd);
        }
        check(close(fd[0]));
        check(close(fd[1]));

        auto end_time = std::chrono::high_resolution_clock::now();
        std::cout   << "Runtime: "
                    << std::chrono::duration<double, std::milli>(end_time - start_time).count()
                    << " ms" << std::endl;
        exit(EXIT_SUCCESS);
    }
}
#endif