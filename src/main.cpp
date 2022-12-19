#include "signal.hpp"
#include "unnamed_pipe.hpp"
#include "message_queue.hpp"
#include <getopt.h>
#include <cstring>

int get_int(const std::string& argv)
{
    try
    {
        return std::stoi(argv);
    }
    catch(std::invalid_argument const& ex)
    {
        std::cout   << "INVALID TYPE: " << std::string(argv) << '\n'
                    << "    Value must be integer\n\n";
        exit(EXIT_FAILURE);
    }
    catch(std::out_of_range const& ex)
    {
        std::cout   << "OUT OF RANGE: N = " + std::string(argv) << '\n'
                    << "    Value must be in range [INT_MIN; INT_MAX]\n\n";
        exit(EXIT_FAILURE);
    }
}

void help()
{
    std::cout   << "Use those arguments to launch program:\n"
                << "    [mode]\n"
                << "    --range (-n) [value]         (optional)\n"
                << "    --count (-c) [count]         (optional)\n\n"
                << "mode\n"
                << "    \"Guess the value\" mode:\n"
                << "    [--sig] Signal\n"
                << "    [ --up] Unnamed pipe\n"
                << "    [ --mq] Message queue\n\n"
                << "value\n"
                << "    Set maximum value to guess in the game\n"
                << "    Range for the value: [2, INT_MAX]\n\n"
                << "count\n"
                << "    Set number of games\n"
                << "    Range for the value: [10, INT_MAX]\n\n"
                << "EXAMPLE:\n"
                << "    ./SP_2 --sig --range 100 -c 50\n";
}

int main(int argc, char** argv)
{
    if(argc == 2 && (strcmp(argv[1],"--help") == 0 || strcmp(argv[1], "-h") == 0))
    {
        help();
        exit(EXIT_FAILURE);
    }

    int range = 1,
        count = 10,
         mode = -1;

    const option options[] = {{  "sig",       no_argument, nullptr, 's'},
                              {   "up",       no_argument, nullptr, 'p'},
                              {   "mq",       no_argument, nullptr, 'q'},
                              {"range", required_argument, nullptr, 'n'},
                              {"count", required_argument, nullptr, 'c'},
                              {nullptr,       no_argument, nullptr,  0 }};

    bool range_flag = false;
    int i;
    while((i = getopt_long(argc, argv, "n:c:", options, nullptr)) != -1)
    {
        switch(i)
        {
            case 's':
                mode = 's';
                break;
            case 'p':
                mode = 'p';
                break;
            case 'q':
                mode = 'q';
                break;
            case 'n':
                range = get_int(optarg);
                range_flag = true;
                break;
            case 'c':
                count = get_int(optarg);
                break;
            default:
                std::cout << "Use [--help] or [-h] argument to get launch information\n";
                std::cout << optarg;
                exit(EXIT_FAILURE);
        }
    }

    if(count < 10)
    {
        std::cout   << "count = " << count << '\n'
                    << "Value must be in range [10, INT_MAX]\n";
        exit(EXIT_FAILURE);
    }

    if(!range_flag)
    {
        std::string buf;
        std::cout << "Enter N: ";
        std::cin >> buf;
        range = get_int(buf);
    }

    if(range < 2)
    {
        std::cout   << "range = " << range << '\n'
                    << "Value must be in range [2, INT_MAX]\n";
        exit(EXIT_FAILURE);
    }

    switch(mode)
    {
        case 's':
            SIG::start(range, count);
            break;
        case 'p':
            UP::start(range, count);
            break;
        case 'q':
            MQ::start(range, count);
            break;
        default:
            std::cout   << "Use one of those arguments to launch game:\n"
                        << "    [--sig] Signal\n"
                        << "    [ --up] Unnamed pipe\n"
                        << "    [ --mq] Message queue\n\n";
            exit(EXIT_FAILURE);
    }
    return 0;
}