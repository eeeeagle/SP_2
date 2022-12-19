#ifndef MESSAGE_QUEUE
#define MESSAGE_QUEUE

#include "common.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

namespace MQ
{
    void player_riddler(const mqd_t mq_a, const mqd_t mq_b, const int max_range)
    {
        static std::mt19937 gen(HRC::now().time_since_epoch().count());
        std::uniform_int_distribution<> uid(1, max_range);

        const int guessed_value = uid(gen);
        std::cout << "Guessed value: " << guessed_value << "\n\n";

        check(mq_send(mq_a, (char *) &max_range, sizeof(max_range), 0));

        int is_guessed = 0;
        while (is_guessed == 0)
        {
            int* buffer = new int[8 * 1024 / sizeof(int)];
            buffer[0] = -1;
            if (check(mq_receive(mq_b, (char *) buffer, 8 * 1024, nullptr)))
            {
                if (buffer[0] == guessed_value)
                {
                    is_guessed = 1;
                    delete[] buffer;
                }

                check(mq_send(mq_a, (char*) &is_guessed, sizeof(is_guessed), 0));
            } else
                _exit(EXIT_FAILURE);
        }
    }

    std::pair<bool, int> player_guesser(const mqd_t mq_a, const mqd_t mq_b, const pid_t p_id)
    {
        int* max_range = new int[8 * 1024 / sizeof(int)];
        if (check(mq_receive(mq_a, (char *) max_range, 8 * 1024, nullptr)))
        {
            std::vector<int> attempt = {};
            for (int i = 1; i <= max_range[0]; attempt.push_back(i++));
            std::random_device rd;
            std::mt19937 mt(rd());
            std::shuffle(attempt.begin(), attempt.end(), mt);

            std::cout << std::setw(7) << "ATTEMPT"
                      << std::setw(10) << "VALUE"
                      << std::setw(12) << "FLAG\n";

            int try_count = 0;
            int* is_guessed = new int[8 * 1024 / sizeof(int)];
            is_guessed[0] = 0;
            while (try_count < INT_MAX && is_guessed[0] == 0)
            {
                if (!is_exists(p_id))
                {
                    std::cout << "\n[!] JEBAITED [!]\n";
                    exit(EXIT_FAILURE);
                }
                usleep(100);

                int value = attempt.back();
                attempt.pop_back();

                check(mq_send(mq_b, (char *) &value, sizeof(value), 0));
                if (!check(mq_receive(mq_a, (char *) is_guessed, 8 * 1024, nullptr)))
                    _exit(EXIT_FAILURE);

                std::cout << std::setw(7) << try_count++ + 1
                          << std::setw(10) << value
                          << std::setw(12) << (is_guessed[0] ? "true\n" : "false\n");
            }
            std::cout << '\n';

            std::pair<bool, int> res = std::make_pair(is_guessed[0], try_count);
            delete[] is_guessed;
            delete[] max_range;
            return res;
        }
        _exit(EXIT_FAILURE);
    }

    void role_select(const int game_count, const char* mqa, const char* mqb, const pid_t p_id,
                     const int max_range, OverallStat& stats, bool (*cmp)(const int))
    {
        mqd_t mq_a = check(mq_open(mqa, O_RDWR, S_IRWXU, NULL));
        mqd_t mq_b = check(mq_open(mqb, O_RDWR, S_IRWXU, NULL));

        if (cmp(game_count))
        {
            //usleep(500);
            std::cout << "__________________________________\n\n";
            std::cout << "GAME [" << game_count + 1 << "]\n";
            player_riddler(mq_a, mq_b, max_range);
        }
        else
        {
            auto start_time = HRC::now();
            const std::pair<bool, int> result = player_guesser(mq_a, mq_b, p_id);
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
        const char* mqa = "/mq_a";
        const char* mqb = "/mq_b";

        auto mq_a = check(mq_open(mqa, O_CREAT, S_IRUSR | S_IWUSR, NULL));
        auto mq_b = check(mq_open(mqb, O_CREAT, S_IRUSR | S_IWUSR, NULL));
        check(mq_close(mq_a));
        check(mq_close(mq_b));

        pid_t p_id = check(fork());

        bool (*cmp)(const int) = p_id ? comp_1 : comp_2;
        for(int game_count = 0; game_count < max_game_count; game_count++)
            role_select(game_count, mqa, mqb, p_id, max_range, stats, cmp);

        if(p_id)
        {
            auto temp_stat = new OverallStat[8 * 1024 / sizeof(OverallStat)];
            if (!check(mq_receive(mq_a, (char *) temp_stat, 8 * 1024, nullptr)))
                _exit(EXIT_FAILURE);

            stats.guessed += temp_stat[0].guessed;
            stats.attempts += temp_stat[0].attempts;
            stats.total_time += temp_stat[0].total_time;
            delete[] temp_stat;

            std::cout << "__________________________________\n\n";
            print_stat(stats, max_game_count);
            std::cout << "__________________________________\n\n";

            mq_unlink(mqa);
            mq_unlink(mqb);
            exit(EXIT_SUCCESS);
        }
        else
        {
            sleep(1);
            check(mq_send(mq_a, (const char *) &stats, sizeof(stats), 0));

            exit(EXIT_SUCCESS);
        }
    }
}
#endif