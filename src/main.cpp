#include <iostream>
#include <thread>
#include <vector>

void worker(int id)
{
    std::cout << "Worker: " << id
              << " Result: " << id * 10 << '\n';
}

int main()
{
    int number = 100;
    std::cout << "Number: " << number << '\n';

    std::vector<std::thread> threads;

    for (int i = 0; i < 3; ++i)
        threads.emplace_back(worker, i);

    for (auto &thread : threads)
        thread.join();

    return 0;
}