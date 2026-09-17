#pragma once
#include <vector>

#include <atomic>
#include <functional>
#include <queue>
#include <thread>
#include <iostream>
struct Task {
  std::function<void()> func;
  Task(std::function<void()> func) : func(func) {};
  void execute() { func(); }
};

struct ReturnValue {
  std::atomic_bool done;
  void *result;

  ReturnValue(bool done, void *result) : done(done), result(result) {}
  ReturnValue(const ReturnValue &other)
      : done(other.done.load()), result(other.result) {}
  ReturnValue &operator=(const ReturnValue &other) {
    done.store(other.done.load());
    result = other.result;
    return *this;
  }
};

class Worker {

public:
  enum class StartPolicy { StartImmediately, StartOnDemand };
  enum class State { Running, Stopped };

  std::atomic<State> state;
  std::vector<std::thread> threads;
  std::vector<ReturnValue> returnValues;
  int numThreads;

  std::queue<Task> taskQueue;
  std::mutex queueMutex;
  Worker(int numThreads, StartPolicy policy = StartPolicy::StartImmediately)
      : numThreads(numThreads) {

    if (policy == StartPolicy::StartImmediately) {
      start();
    };
  }

  void stop() {}
  Task getTask() {
    return Task([]() {});
  }

  void start() {
    for (int i = 0; i < numThreads; ++i) {
      std::thread([this]() {
        while (true) {
          Task task = getTask();
          if (task.func == nullptr) {
            break;
          }
          std::cout << "Thread" << std::this_thread::get_id() << " executing task." << std::endl;
          task.execute();

        }
      }).detach();
    }
  }

  ReturnValue addTask(Task task) {
    // 작업 큐에 추가

    returnValues.emplace_back(false, nullptr);
    {
      std::lock_guard<std::mutex> lock(queueMutex);
      taskQueue.push(task);
    }
    return returnValues.back();
  }
};