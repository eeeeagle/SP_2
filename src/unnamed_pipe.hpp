#ifndef UNNAMED_PIPE
#define UNNAMED_PIPE

#include "common.hpp"

namespace UP
{
    void player_riddler(const int fd_a[], const int fd_b[], const int max_range)
    {
        static std::mt19937 gen(HRC::now().time_since_epoch().count());
        std::uniform_int_distribution<> uid(1, max_range);

        const int guessed_value = uid(gen);
        std::cout << "Guessed value: " << guessed_value << "\n\n";
        check(write(fd_a[1], &max_range, sizeof(max_range)));

        bool is_guessed = false;
        while (!is_guessed)
        {
            int buffer;
            if (check(read(fd_b[0], &buffer, sizeof(int))))
            {
                if(buffer == guessed_value)
                    is_guessed = true;
                check(write(fd_a[1], &is_guessed, sizeof(is_guessed)));
            }
            else
                _exit(EXIT_FAILURE);
        }
    }

    std::pair<bool, int> player_guesser(const int fd_a[], const int fd_b[], const pid_t p_id)
    {
        int max_range;
        if (check(read(fd_a[0], &max_range, sizeof(int))))
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
                if (!is_exists(p_id))
                {
                    std::cout << "\n[!] JEBAITED [!]\n";
                    exit(EXIT_FAILURE);
                }
                usleep(100);

                int value = attempt.back();
                attempt.pop_back();

                check(write(fd_b[1], &value, sizeof(value)));

                if (check(read(fd_a[0], &is_guessed, sizeof(bool))))
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

    void role_select(const int game_count, const int p_id, const int fd_a[], const int fd_b[],
                     const int n, OverallStat& stats, bool (*cmp)(const int))
    {
        if (cmp(game_count))
        {
            usleep(500);
            std::cout << "__________________________________\n\n";
            std::cout << "GAME [" << game_count + 1 << "]\n";
            player_riddler(fd_a, fd_b, n);
        }
        else
        {
            auto start_time = HRC::now();
            const std::pair<bool, int> result = player_guesser(fd_a, fd_b, p_id);
            auto end_time = HRC::now();
            print_result(result, Micro(end_time - start_time).count());

            if(result.first)
                stats.guessed++;
            stats.attempts += result.second;
            stats.total_time += Micro(end_time - start_time).count();
        }
        sleep(1);
    }

    void start(const int max_range, const int max_game_count)
    {
        OverallStat stats {0, 0, 0.0};

        int fd_a[2];
        int fd_b[2];
        check(pipe(fd_a));
        check(pipe(fd_b));
        pid_t p_id = check(fork());

        bool (*cmp)(const int) = p_id ? comp_1 : comp_2;
        for(int game_count = 0; game_count < max_game_count; game_count++)
            role_select(game_count, p_id, fd_a, fd_b, max_range, stats, cmp);


        if(p_id)
        {
            OverallStat buffer {0, 0, 0.0};
            if(check(read(fd_a[0], &buffer, sizeof(OverallStat))))
            {
                stats.guessed += buffer.guessed;
                stats.attempts += buffer.attempts;
                stats.total_time += buffer.total_time;
            }

            close(fd_a[0]);
            close(fd_a[1]);
            close(fd_b[0]);
            close(fd_b[1]);

            std::cout << "__________________________________\n\n";
            print_stat(stats, max_game_count);
            std::cout << "__________________________________\n\n";

            waitpid(p_id, nullptr, 0);
            exit(EXIT_SUCCESS);
        }
        else
        {
            sleep(1);
            check(write(fd_a[1], &stats, sizeof(std::pair<std::pair<int, int>, double>)));

            exit(EXIT_SUCCESS);
        }
    }
}
#endif