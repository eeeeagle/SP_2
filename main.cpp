#include "check.hpp"
#include <iostream>
#include <csignal>
#include <pthread.h>
#include <unistd.h>
#include <wait.h>

volatile sig_atomic_t last_signal_id;
volatile sig_atomic_t signal_value;

void sig_handler(int signal_id)
{
    last_signal_id = signal_id;
}

void riddler(pid_t p_id, const int n)
{
    int value = 1 + (int)random() % n;
    check(sigqueue(p_id, SIGINT, sigval{value}));

    sigset_t sig_set;
    sigfillset(&sig_set);

    sigdelset(&sig_set, SIGINT);
    sigdelset(&sig_set, SIGUSR1);
    sigdelset(&sig_set, SIGUSR2);
    sigprocmask(SIG_BLOCK, &sig_set, nullptr);

    while(true)
    {
        sigsuspend(&sig_set);
        std::cout << "SUSPENDING...\n";

        check(kill(p_id, signal_value == value ? SIGUSR1 : SIGUSR2));
        if(p_id == 0) return;
    }

}

void guesser(pid_t p_id)
{
    sigset_t sig_set;
    sigfillset(&sig_set);

    sigdelset(&sig_set, SIGINT);
    sigdelset(&sig_set, SIGUSR1);
    sigdelset(&sig_set, SIGUSR2);
    sigprocmask(SIG_BLOCK, &sig_set, nullptr);

    sigsuspend(&sig_set);
    const int n = signal_value;

    for(int count = 0; count < INT32_MAX; count++)
    {
        int value = 1 + (int)random() % n;
        check(sigqueue(p_id, SIGINT, sigval{value}));
        sigsuspend(&sig_set);
        std::cout << '[' << count + 1 << "] " << value;
        if(last_signal_id == SIGUSR1)
        {
            std::cout << " true";

        }
        else if(last_signal_id == SIGUSR2)
            std::cout << " false";
        check(kill(p_id, last_signal_id));
    }
}


int main(int argc, char** argv)
{
    int n;
    if(argc > 1)
    {
        n = std::stoi(argv[1]);
    }
    else
    {
        std::cout << "Enter N: ";
        std::cin >> n;
    }
    std::cout << "N = " << n << std::endl;

    clock_t start_time = clock();
    pid_t p_id = check(fork());
    std::cout << p_id << "\n";
    if (p_id)
        riddler(p_id, n);
    else
        guesser(p_id);
    clock_t end_time = clock();

    std::cout << "Runtime: " << end_time - start_time << " ms" << std::endl;

    return 0;
}


