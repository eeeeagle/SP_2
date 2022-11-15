#ifndef MESSAGE_QUEUE
#define MESSAGE_QUEUE

#include "common.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

namespace MQ
{
    void player_riddler(mqd_t mk_d, const int max_range)
    {
        static std::mt19937 gen(HRC::now().time_since_epoch().count());
        std::uniform_int_distribution<> uid(1, max_range);

        const int guessed_value = uid(gen);
        std::cout << "Guessed value: " << guessed_value << "\n\n";
        /*
         * TO DO
         *
         * SEND MAX RANGE TO GUESSER
         * GET ANSWER FROM GUESSER
         * SEND ANSWER TO GUESSER
         *
        */
    }

    std::pair<bool, int> player_guesser(mqd_t mq_d)
    {
        /*
         * TO DO
         *
         * GET MAX RANGE FROM RIDDLER
         * SEND VALUE TO RIDDLER
         * GET ANSWER FROM RIDDLER
         *
         */
        int max_range;
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

            if (value + 1 != 0)
                std::cout   << std::setw(7) << try_count++ + 1
                            << std::setw(10) << value
                            << std::setw(12) << (is_guessed ? "true\n" : "false\n");
            else
                _exit(EXIT_FAILURE);
        }
        std::cout << '\n';
        return std::make_pair(is_guessed, try_count);
    }

    void role_select(const int game_count, mqd_t mq_d , const int max_range, OverallStat& stats, bool (*cmp)(const int))
    {

        if (cmp(game_count))
            player_riddler(mq_d, max_range);
        else
        {
            auto start_time = HRC::now();
            const std::pair<bool, int> result = player_guesser(mq_d);
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

        pid_t p_id = check(fork());
        mqd_t mq_d = check(mq_open("/mq", O_WRONLY));

        bool (*cmp)(const int) = p_id ? comp_1 : comp_2;
        for(int game_count = 0; game_count < max_game_count; game_count++)
            role_select(game_count, mq_d, max_range, stats, cmp);

        if(p_id)
        {
            /*
             *  TO DO
             *
             *  GET STATS FROM CHILD
             */

            print_stat(stats, max_game_count);

            wait(nullptr);
            mq_unlink("/mq");
            exit(EXIT_SUCCESS);
        }
        else
        {
            /*
             * TO DO
             *
             * SEND STATS TO PARENT
            */
            exit(EXIT_SUCCESS);
        }
    }
}
#endif