#include "signal.hpp"
#include "unnamed_pipe.hpp"
#include "message_queue.hpp"
#include <string>

int main(int argc, char** argv)
{
    if(argc < 2 || argc > 3)
    {
        std::cout   << "Use those arguments to launch program:"     << std::endl
                    << "    [mode]"                                 << std::endl
                    << "    [value]         (optional)"             << std::endl << std::endl
                    << "mode"                                       << std::endl
                    << "    \"Guess the value\" mode:"              << std::endl
                    << "    [-SIG] Signal"                          << std::endl
                    << "    [ -UP] Unnamed pipe"                    << std::endl
                    << "    [ -MQ] Message queue"                   << std::endl << std::endl
                    << "value"                                      << std::endl
                    << "    Set maximum value to guess in the game" << std::endl
                    << "    Range for the value: [2, INT_MAX]"      << std::endl << std::endl
                    << "EXAMPLE:"                                   << std::endl
                    << "    .../SP_2 -SIG 1000"                     << std::endl;
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
                std::cout   << "INVALID TYPE: " << std::string(argv[2]) << std::endl
                            << "    Value must be integer"              << std::endl;
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
        std::cout   << ex.what()                                    << std::endl << std::endl
                    << "Use those arguments to launch program:"     << std::endl
                    << "    [-SIG] Signal"                          << std::endl
                    << "    [ -UP] Unnamed pipe"                    << std::endl
                    << "    [ -MQ] Message queue"                   << std::endl << std::endl;
        exit(EXIT_FAILURE);
    }
    catch(std::out_of_range const& ex)
    {
        std::cout   << ex.what()                                    << std::endl << std::endl
                    << "Value must be in range:"                    << std::endl
                    << "    [2, INT_MAX]"                           << std::endl;
        exit(EXIT_FAILURE);
    }

    if (std::string(argv[1]) == "-SIG")
        SIG::start(n);
    else if (std::string(argv[1]) == "-UP")
        UP::start(n);
    else if (std::string(argv[1]) == "-MQ")
        MQ::start(n);
}