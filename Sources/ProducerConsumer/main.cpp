#include <array>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

void producer(std::queue<int> &q, std::mutex &mtx, std::condition_variable &cv,
              const int max_items = 10) {
  for (int i = 0; i < max_items; ++i) {
    {
      std::lock_guard<std::mutex> lock(mtx);
      q.push(i);
    }

    cv.notify_one();
  }
}

// void consumer(std::queue<int> &q, std::mutex &mtx, std::condition_variable
// &cv,
//               bool &done) {
//   while (true) {
//     std::unique_lock<std::mutex> lock(mtx);

//     cv.wait(lock, [&] { return !q.empty() || done; });

//     // Producer가 전부 끝났고
//     // Queue도 완전히 비었다면 종료
//     if (q.empty() && done) {
//       break;
//     }

//     int item = q.front();
//     q.pop();

//     lock.unlock();

//     // 실제 작업
//     // DoSomething(item);
//   }
// }


// For a simple but computationally intensive workload
struct Matrix {
  float m[100][100];
};

Matrix multiply(const Matrix &a, const Matrix &b) {
  Matrix result{};

  for (int i = 0; i < 100; ++i) {
    for (int j = 0; j < 100; ++j) {
      for (int k = 0; k < 100; ++k) {
        result.m[i][j] += a.m[i][k] * b.m[k][j];
      }
    }
  }

  return result;
}
void consumer(std::queue<int> &q, std::mutex &mtx, std::condition_variable &cv,
              bool &done) {
  Matrix a{};
  Matrix b{};

  for (int i = 0; i < 100; ++i) {
    for (int j = 0; j < 100; ++j) {
      a.m[i][j] = static_cast<float>(i + j + 1);
      b.m[i][j] = static_cast<float>(i * j + 1);
    }
  }

  volatile float sink = 0.0f;

  while (true) {
    std::unique_lock<std::mutex> lock(mtx);

    cv.wait(lock, [&] { return !q.empty() || done; });

    if (q.empty() && done)
      break;

    int item = q.front();
    q.pop();

    lock.unlock();

    Matrix result = multiply(a, b);

    // 컴파일러가 연산을 통째로 제거하지 못하게
    sink += result.m[item % 4][item % 4];
  }
}

int main(int argc, char *argv[]) {
  int max_items = 100000;
  int producer_count = 1;
  int consumer_count = 1;

  if (argc == 2) {
    max_items = std::stoi(argv[1]);
  }

  if (argc == 4) {
    max_items = std::stoi(argv[1]);
    producer_count = std::stoi(argv[2]);
    consumer_count = std::stoi(argv[3]);
  }

  const int item_per_producer = max_items / producer_count;

  const int residual_items = max_items % producer_count;

  std::queue<int> q;
  std::mutex mtx;
  std::condition_variable cv;

  bool done = false;

  std::vector<std::thread> producer_threads;
  std::vector<std::thread> consumer_threads;

  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < producer_count; ++i) {
    producer_threads.emplace_back(
        producer, std::ref(q), std::ref(mtx), std::ref(cv),
        item_per_producer + (i == producer_count - 1 ? residual_items : 0));
  }

  for (int i = 0; i < consumer_count; ++i) {
    consumer_threads.emplace_back(consumer, std::ref(q), std::ref(mtx),
                                  std::ref(cv), std::ref(done));
  }

  // Producer가 전부 끝날 때까지 기다림
  for (auto &t : producer_threads) {
    t.join();
  }

  // 이제 앞으로 새로운 item은 들어오지 않음
  {
    std::lock_guard<std::mutex> lock(mtx);
    done = true;
  }

  // wait 중인 consumer 전부 깨움
  cv.notify_all();

  // Consumer는 남은 Queue까지 전부 비우고 종료
  for (auto &t : consumer_threads) {
    t.join();
  }

  auto end = std::chrono::steady_clock::now();

  std::chrono::duration<double> elapsed_seconds = end - start;

  std::cout << "Items: " << max_items << "\nProducers: " << producer_count
            << "\nConsumers: " << consumer_count
            << "\nElapsed time: " << elapsed_seconds.count() << "s\n";
}