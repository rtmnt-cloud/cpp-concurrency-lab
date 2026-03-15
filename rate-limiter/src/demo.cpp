#include "../include/request_tracker.hpp"
#include <thread>
#include <iostream>

int main()
{
    RequestTracker<int> tracker(10);

    auto worker = [&tracker]()
    {
        for(int i = 0; i < 100; i++)
        {
            tracker.recordRequest(42);
        }
    };

    std::thread t1(worker);
    std::thread t2(worker);

    t1.join();
    t2.join();

    std::cout << tracker.getCount(42) << std::endl;
}