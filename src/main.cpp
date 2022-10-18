#include "signal.hpp"
#include "unnamed_pipe.hpp"
#include "message_queue.hpp"
#include <string>

int get_int(const std::string& argv)
{
    int value;
    try
    {
        int i {std::stoi(argv)};
        value = i;
    }
    catch(std::invalid_argument const& ex)
    {
        std::cout   << "INVALID TYPE: " << std::string(argv) << '\n'
                    << "    Value must be integer\n\n";
        exit(EXIT_FAILURE);
    }
    catch(std::out_of_range const& ex)
    {
        throw std::out_of_range("OUT OF RANGE: N = " + std::string(argv));
    }
    return value;
}

void help()
{
    std::cout   << "Use those arguments to launch program:\n"
                << "    [mode]\n"
                << "    -n [value]         (optional)\n"
                << "    -c [count]         (optional)\n\n"
                << "mode\n"
                << "    \"Guess the value\" mode:\n"
                << "    [-sig] Signal\n"
                << "    [ -up] Unnamed pipe\n"
                << "    [ -mq] Message queue\n\n"
                << "value\n"
                << "    Set maximum value to guess in the game\n"
                << "    Range for the value: [2, INT_MAX]\n\n"
                << "count\n"
                << "    Set number of games\n"
                << "    Minimum: 10\n\n"
                << "EXAMPLE:\n"
                << "    ./SP_2 -sig -n 1000 -c 50\n";
}

int main(int argc, char** argv)
{
    if(argc != 2 && argc != 4 && argc != 6)
    {
        help();
        exit(EXIT_FAILURE);
    }

    int n = 1;
    int count = 10;
    try
    {
        if (std::string(argv[1]) != "-sig" &&
            std::string(argv[1]) != "-up" &&
            std::string(argv[1]) != "-mq")
            throw std::invalid_argument("INVALID MODE: " + std::string(argv[1]));

        if (argc >= 4 && (std::string(argv[2]) != "-n" && std::string(argv[2]) != "-c"))
            throw std::invalid_argument("INVALID MODE: " + std::string(argv[2]));

        if (argc == 6 && (std::string(argv[4]) != "-n" && std::string(argv[4]) != "-c"))
            throw std::invalid_argument("INVALID MODE: " + std::string(argv[4]));

        if ((argc >= 4 && std::string(argv[2]) == "-n") || (argc == 6 && std::string(argv[4]) == "-n"))
            n = get_int(std::string(argv[2]) == "-n" ? argv[3] : argv[5]);
        else
        {
            std::cout << "Enter N: ";
            std::cin >> n;
        }
        if(n < 2)
            throw std::out_of_range("OUT OF RANGE: N = " + std::to_string(n));

        if ((argc >= 4 && std::string(argv[2]) == "-c") || (argc == 6 && std::string(argv[4]) == "-c"))
        {
            count = get_int(std::string(argv[2]) == "-c" ? argv[3] : argv[5]);
            if (count < 10)
                throw std::out_of_range("OUT OF RANGE: count = " + std::to_string(count));
        }

    }
    catch(std::invalid_argument const& ex)
    {
        std::cout   << ex.what() << "\n\n"
                    << "Use those arguments to launch program:\n"
                    << "    [-sig] Signal\n"
                    << "    [ -up] Unnamed pipe\n"
                    << "    [ -mq] Message queue\n\n";
        exit(EXIT_FAILURE);
    }
    catch(std::out_of_range const& ex)
    {
        std::cout   << ex.what() << "\n\n"
                    << "Value must be in range:\n"
                    << "        N = [2, INT_MAX]\n"
                    << "    count = [10, INT_MAX]\n\n";

        exit(EXIT_FAILURE);
    }

    system("clear");
    if (std::string(argv[1]) == "-sig")
        SIG::start(n, count);
    else if (std::string(argv[1]) == "-up")
        UP::start(n, count);
    else if (std::string(argv[1]) == "-mq")
        MQ::start(n, count);
}