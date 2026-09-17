
#include "../Common/TaskFunction.h"
#include "worker.h"
#include <iostream>

void HeavyTask() {
  Matrix a(100, 100);
  Matrix b(100, 100);

  // init
  for (int i = 0; i < 100; ++i) {
    for (int j = 0; j < 100; ++j) {
      a(i, j) = static_cast<float>(i + j + 1);
      b(i, j) = static_cast<float>(i * j + 1);
    }
  }

  multiply(a, b);
}
int For = 0;
void LightTask() {
  int sink = 110000 * 22;
  For += sink;
}

int main(int argc, char *argv[]) {

  int job_count = argc > 1 ? std::stoi(argv[1]) : 1000;
  int worker_thread_count = argc > 2 ? std::stoi(argv[2]) : 4;
  int job_type = argc > 3 ? std::stoi(argv[3]) : 1; // 1: simple, 2: complex

  std::cout << "Max job count: " << job_count << std::endl;
  std::cout << "Worker thread counts: " << worker_thread_count << std::endl;
  std::cout << "Job type: " << job_type << std::endl;

  // 0. parsing arguments
  // [1] : max job count (default : 10000)
  // [2] : worker thread counts <= 32 (testing purpose) (default : 1)
  // [3] : job type (1: simple, 2: complex) (default : 1)

  Worker worker(worker_thread_count, Worker::StartPolicy::StartOnDemand);

  //   // 1. do something.
  std::cout << "Doing something before worker boot..." << std::endl;
  //   // 2. worker boot
  worker.start();

  //   // 3. add task
  for (int i = 0; i < job_count; ++i) {
    worker.addTask({job_type == 1 ? LightTask : HeavyTask});
  }

  std::cout << "end task" << std::endl;

}