#ifndef UNNAMED_PIPE
#define UNNAMED_PIPE

#include "common.hpp"

namespace UP
{
    void player_riddler(const int pipe_fd[], const int max_range)
    {
        static std::mt19937 gen(HRC::now().time_since_epoch().count());
        std::uniform_int_distribution<> uid(1, max_range);

        const int guessed_value = uid(gen);
        std::cout << "Guessed value: " << guessed_value << "\n\n";
        check(write(pipe_fd[1], &max_range, sizeof(max_range)));

        bool is_guessed = false;
        while (!is_guessed)
        {
            int buffer;
            if (check(read(pipe_fd[0], &buffer, sizeof(int))))
            {
                if(buffer == guessed_value)
                    is_guessed = true;
                check(write(pipe_fd[1], &is_guessed, sizeof(is_guessed)));

            }
            else
                _exit(EXIT_FAILURE);
        }
    }

    std::pair<bool, int> player_guesser(const int pipe_fd[])
    {
        int max_range;
        if (check(read(pipe_fd[0], &max_range, sizeof(int))))
        {
            std::vector<int> attempt = {};
            for (int i = 1; i <= max_range; attempt.push_back(i++));
            std::random_device rd;
            std::mt19937 mt(rd());
            std::shuffle(attempt.begin(), attempt.end(), mt);

            std::cout   << std::setw(7) << "ATTEMPT"
                        << std::setw(10) << "VALUE"
                        << std::setw(12) << "FLAG\n";

            int try_count = 0;
            bool is_guessed = false;
            while (try_count < INT_MAX && !is_guessed)
            {
                int value = attempt.back();
                attempt.pop_back();

                check(write(pipe_fd[1], &value, sizeof(value)));

                if (check(read(pipe_fd[0], &is_guessed, sizeof(bool))))
                    std::cout   << std::setw(7) << try_count++ + 1
                                << std::setw(10) << value
                                << std::setw(12) << (is_guessed ? "true\n" : "false\n");
                else
                    _exit(EXIT_FAILURE);
            }
            std::cout << '\n';
            return std::make_pair(is_guessed, try_count);
        }
        _exit(EXIT_FAILURE);
    }

    void role_select(const int i, const int fd[], const int n, OverallStat& stats, bool (*cmp)(const int))
    {
        if (cmp(i))
        {
            sleep(1);
            player_riddler(fd, n);
        }
        else
        {
            auto start_time = HRC::now();
            const std::pair<bool, int> result = player_guesser(fd);
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
        OverallStat stats {0, 0, 0.0};

        int fd[2];
        check(pipe(fd));
        pid_t p_id = check(fork());

        bool (*cmp)(const int) = p_id ? comp_1 : comp_2;
        for(int game_count = 0; game_count < max_game_count; game_count++)
            role_select(game_count, fd, max_range, stats, cmp);


        if(p_id)
        {
            OverallStat buffer {0, 0, 0.0};
            if(check(read(fd[0], &buffer, sizeof(OverallStat))))
            {
                stats.guessed += buffer.guessed;
                stats.attempts += buffer.attempts;
                stats.total_time += buffer.total_time;

                print_stat(stats, max_game_count);
            }
            close(fd[0]);
            close(fd[1]);

            waitpid(p_id, nullptr, 0);
            exit(EXIT_SUCCESS);
        }
        else
        {
            sleep(1);
            check(write(fd[1], &stats, sizeof(std::pair<std::pair<int, int>, double>)));

            exit(EXIT_SUCCESS);
        }
    }
}
#endif