#include <atomic>
#include <barrier>
#include <cstdint>
#include <iostream>
#include <thread>

constexpr std::uint64_t N = 10'000'000;

std::atomic<int> data{0};
std::atomic<int> flag{0};

std::barrier start{3};
std::barrier finish{3};

int observedFlag = 0;
int observedData = -1;

void producer()
{
    for (std::uint64_t i = 0; i < N; ++i)
    {
        start.arrive_and_wait();

        // 순서대로 쓴다고 작성
        data.store(1, std::memory_order_relaxed);
        flag.store(1, std::memory_order_relaxed);

        finish.arrive_and_wait();
    }
}

void consumer()
{
    for (std::uint64_t i = 0; i < N; ++i)
    {
        start.arrive_and_wait();

        observedFlag =
            flag.load(std::memory_order_relaxed);

        if (observedFlag == 1)
        {
            // flag == 1을 실제로 관찰한 경우에만
            // data를 확인
            observedData =
                data.load(std::memory_order_relaxed);
        }
        else
        {
            observedData = -1;
        }

        finish.arrive_and_wait();
    }
}

int main()
{
    std::thread t1(producer);
    std::thread t2(consumer);

    std::uint64_t flagSeen = 0;
    std::uint64_t reordered = 0;

    for (std::uint64_t i = 0; i < N; ++i)
    {
        // 이번 실험 초기화
        data.store(0, std::memory_order_relaxed);
        flag.store(0, std::memory_order_relaxed);

        // Producer / Consumer 동시에 출발
        start.arrive_and_wait();

        // 둘 다 이번 실험을 끝낼 때까지 대기
        finish.arrive_and_wait();

        if (observedFlag == 1)
        {
            ++flagSeen;

            // 우리가 찾는 결과
            if (observedData == 0)
            {
                ++reordered;
            }
        }
    }

    t1.join();
    t2.join();

    std::cout
        << "flag == 1 observed: "
        << flagSeen << " / " << N << '\n';

    std::cout
        << "flag == 1 && data == 0: "
        << reordered << " / " << flagSeen << '\n';
}