#include <coroutine>
#include <iostream>


class [[nodiscard]] CoGenerator
{
public:
    struct promise_type;
    using CoroHandle = std::coroutine_handle<promise_type>;

public:
    CoGenerator(CoroHandle h) :
        hnd{ h }
    {}
    ~CoGenerator()
    {
        if (hnd) {
            hnd.destroy();
        }
    }

    CoGenerator(const CoGenerator&) = delete;
    CoGenerator& operator=(const CoGenerator&) = delete;

public:
    unsigned long long getNextPrime();

private:
    CoroHandle hnd;
};

struct CoGenerator::promise_type 
{
    CoGenerator get_return_object()
    {
        return { CoroHandle::from_promise(*this) };
    }

    std::suspend_always initial_suspend() const noexcept
    {
        return {};
    }

    std::suspend_never final_suspend() const noexcept
    {
        return {};
    }

    std::suspend_always yield_value(unsigned long long num) noexcept
    {
        last = num;
        return {}; // - suspend coroutine
    }

    void unhandled_exception()
    {
        std::terminate();
    }

    constexpr void return_void() const noexcept
    {}

    unsigned long long last = 0;
};

unsigned long long CoGenerator::getNextPrime()
{
    if (!hnd || hnd.done()) {
        std::cerr << "Invalid CoRo state!\n";
        return -1;
    }
    hnd.resume();
    return hnd.promise().last;
}

bool isPrime(unsigned long long num)
{
    if (num != 2 && (num < 1 || num % 2 == 0))
        return false;

    for (unsigned long long d = 3; d * d <= num; d += 2)
        if (num % d == 0)
            return false;
    return true;
}

CoGenerator allPrimeNumbers()
{
    co_yield 2;

    unsigned long long num = 3;
    for (;;) {
        if (isPrime(num)) {
            co_yield num;
        }
        num += 2;
    }
}

int main()
{
    int cnt = 10;
    CoGenerator gen = allPrimeNumbers();

    for (int i = 0; i < cnt; ++i) {
        std::cout << gen.getNextPrime() << ' ';
    }

    return 0;
}
