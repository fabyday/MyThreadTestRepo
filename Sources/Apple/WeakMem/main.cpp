#include <atomic>
#include <thread>
#include <iostream>

std::atomic<int> x{0};
std::atomic<int> y{0};

int main()
{
    constexpr int N = 1'000'000;

    long long bothZero = 0;

    for (int i = 0; i < N; ++i)
    {
        x.store(0, std::memory_order_relaxed);
        y.store(0, std::memory_order_relaxed);

        int r1 = -1;
        int r2 = -1;

        std::thread t1([&] {
            r1 = y.load(std::memory_order_relaxed);
            x.store(1, std::memory_order_relaxed);
        });

        std::thread t2([&] {
            r2 = x.load(std::memory_order_relaxed   );
            y.store(1, std::memory_order_relaxed    );
        });

        t1.join();
        t2.join();

        if (r1 == 1 && r2 == 1)
            ++bothZero;
    }

    std::cout << "both zero: "
              << bothZero << "/" << N << '\n';
}