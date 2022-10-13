#ifndef MESSAGE_QUEUE
#define MESSAGE_QUEUE

void mq_start(const int n)
{
    clock_t start_time = clock();

    clock_t end_time = clock();
    std::cout << "Runtime: " << end_time - start_time << " ms" << std::endl;
}
#endif