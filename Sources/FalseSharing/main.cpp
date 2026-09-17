#include <chrono>
#include <iostream>
#include <thread>

struct alignas(128) Value {
     long long value = 0;
};

Value num1;
Value num2;
std::atomic<long long> num3{0};


// False Sharing Test
// struct Shared {
//     std::atomic<long long> num1{0};
//     std::atomic<long long> num2{0};
// };

// THIS is M1 Apple Cache Line Size 128 Byte
struct Shared {
    alignas(128) std::atomic<long long> num1{0};
    alignas(128) std::atomic<long long> num2{0};
};
Shared shared;

void fun1()
{
    for (long long i = 0; i < 1'000'000'000; ++i)
        shared.num1.fetch_add(1, std::memory_order_relaxed);
}

void fun2()
{
    for (long long i = 0; i < 1'000'000'000; ++i)
        shared.num2.fetch_add(1, std::memory_order_relaxed);
}

void fun3() {
  for (long long i = 0; i < 2000000000; i++) {
    num3.fetch_add(1, std::memory_order_relaxed);
  }
}
int main() {
  auto beginTime = std::chrono::high_resolution_clock::now();

  std::thread t1(fun1); // Multi Thread 실행
  std::thread t2(fun2); // Multi Thread 실행

  t1.join();
  t2.join();

  auto endTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> resultTime = endTime - beginTime;

  printf("%lld\n", shared.num1.load() + shared.num2.load());
  std::cout << resultTime.count() << std::endl;
  printf("--------------------\n");
  beginTime = std::chrono::high_resolution_clock::now();

  fun3(); // Single Thread 실행

  endTime = std::chrono::high_resolution_clock::now();
  resultTime = endTime - beginTime;
  printf("%lld\n", num3.load());
  std::cout << resultTime.count() << std::endl;
}