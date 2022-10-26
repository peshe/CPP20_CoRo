#include <coroutine>
#include <iostream>

#ifdef _DEBUG
#define dump(text) {std::cout<<text<<'\n';}
#else
#define dump(text)
#endif

class [[nodiscard]] CoroSimple
{
public:
    struct promise_type;
    using CoroHandle = std::coroutine_handle<promise_type>;

public:
    CoroSimple(CoroHandle h) :
        hnd{ h }
    {
        dump("CoroSimple\tctor");
    }

    ~CoroSimple()
    {
        dump("CoroSimple\tdtor");
        if (hnd) {
            hnd.destroy();
        }
    }

    CoroSimple(const CoroSimple&) = delete;
    CoroSimple& operator=(const CoroSimple&) = delete;

public:
    bool resume() const 
    {
        dump("CoroSimple\tresume");
        if (!hnd || hnd.done()) {
            return false;       // nothing (more) to process
        }
        hnd.resume();           // RESUME (blocks until suspended again or the end)
        return !hnd.done();     // are we done?
    }

private:
    CoroHandle hnd;
};

struct CoroSimple::promise_type 
{
    CoroSimple get_return_object()
    {
        dump("promise_type\tget_return_object");
        return CoroSimple{ CoroHandle::from_promise(*this) };
    }

    std::suspend_always initial_suspend() const noexcept
    {
        dump("promise_type\tinitial_suspend");
        return {};
    }
    
    std::suspend_always final_suspend() const noexcept
    {
        dump("promise_type\tfinal_suspend");
        return {};
    }
    
    void unhandled_exception()
    {
        std::terminate();
    }
    
    void return_void() const noexcept 
    {
        dump("promise_type\treturn_void");
    }
};

CoroSimple printSomeNumbers(int num)
{
    std::cout << "Coroutine started with param: " << num << "\n\n";
    for (int val = 1; val <= num; ++val) {
        std::cout << " value: " << val << '\n';
        co_await std::suspend_always{};
    }
    std::cout << "\nCoroutine with param: " << num << " ended\n";
}

int main()
{
    {
        dump("main\t\tStarted\n");
        CoroSimple task = printSomeNumbers(3);
        dump("\nmain\t\tTask created\n");

        while (task.resume())
            dump("main\t\tin the loop\n");

        dump("\nmain\t\tLeave inner block\n");
    }
    dump("main\t\tEnded");
    return 0;
}
