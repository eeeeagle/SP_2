#include "signal.hpp"
#include "unnamed_pipe.hpp"
#include "message_queue.hpp"
#include <string>

int main(int argc, char** argv)
{
    if(argc < 2 || argc > 3)
    {
        std::cout   << "Use those arguments to launch program:"     << '\n'
                    << "    [mode]"                                 << '\n'
                    << "    [value]         (optional)"             << '\n' << '\n'
                    << "mode"                                       << '\n'
                    << "    \"Guess the value\" mode:"              << '\n'
                    << "    [-SIG] Signal"                          << '\n'
                    << "    [ -UP] Unnamed pipe"                    << '\n'
                    << "    [ -MQ] Message queue"                   << '\n' << '\n'
                    << "value"                                      << '\n'
                    << "    Set maximum value to guess in the game" << '\n'
                    << "    Range for the value: [2, INT_MAX]"      << '\n' << '\n'
                    << "EXAMPLE:"                                   << '\n'
                    << "    .../SP_2 -SIG 1000"                     << '\n';
        exit(EXIT_FAILURE);
    }

    int n = 1;
    try
    {
        if (std::string(argv[1]) != "-SIG" &&
            std::string(argv[1]) != "-UP" &&
            std::string(argv[1]) != "-MQ")
            throw std::invalid_argument("INVALID MODE: " + std::string(argv[1]));

        if (argc == 3)
        {
            try
            {
                int i {std::stoi(argv[2])};
                n = i;
            }
            catch(std::invalid_argument const& ex)
            {
                std::cout   << "INVALID TYPE: " << std::string(argv[2]) << '\n'
                            << "    Value must be integer\n";
                exit(EXIT_FAILURE);
            }
            catch(std::out_of_range const& ex)
            {
                throw std::out_of_range("OUT OF RANGE: N = " + std::string(argv[2]));
            }
        }
        else
        {
            std::cout << "Enter N: ";
            std::cin >> n;
        }

        if(n < 2)
            throw std::out_of_range("OUT OF RANGE: N = " + std::to_string(n));
    }
    catch(std::invalid_argument const& ex)
    {
        std::cout   << ex.what()                                    << '\n' << '\n'
                    << "Use those arguments to launch program:"     << '\n'
                    << "    [-SIG] Signal"                          << '\n'
                    << "    [ -UP] Unnamed pipe"                    << '\n'
                    << "    [ -MQ] Message queue"                   << '\n' << '\n';
        exit(EXIT_FAILURE);
    }
    catch(std::out_of_range const& ex)
    {
        std::cout   << ex.what()                                    << '\n' << '\n'
                    << "Value must be in range:"                    << '\n'
                    << "    [2, INT_MAX]"                           << '\n';
        exit(EXIT_FAILURE);
    }

    if (std::string(argv[1]) == "-SIG")
        SIG::start(n);
    else if (std::string(argv[1]) == "-UP")
        UP::start(n);
    else if (std::string(argv[1]) == "-MQ")
        MQ::start(n);
}