#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

using namespace std;

class ThreadPool {
public:
    vector<thread> workers;
    queue<function<void()>> tasks;

    mutex mtx;
    condition_variable cv;

    ThreadPool(int n);
    void enqueue(function<void()> task);
};