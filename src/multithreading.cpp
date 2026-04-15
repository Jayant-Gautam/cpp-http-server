#include "multithreading.hpp"
using namespace std;

ThreadPool::ThreadPool(int n)
{
    for (int i = 0; i < n; i++)
    {
        workers.emplace_back([this]()
                             {
                while (true)
                {
                    function<void()> task;

                    {
                        unique_lock<mutex> lock(mtx);
                        cv.wait(lock, [this]() { return !tasks.empty(); });

                        task = move(tasks.front());
                        tasks.pop();
                    }

                    task();
                } });
    }
}

void ThreadPool::enqueue(function<void()> task)
{
    {
        lock_guard<mutex> lock(mtx);
        tasks.push(task);
    }
    cv.notify_one();
}