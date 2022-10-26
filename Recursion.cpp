#include <coroutine>
#include <iostream>


class [[nodiscard]] CoState
{
public:
    struct promise_type;
    using CoroHandle = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        CoState get_return_object()
        {
            return { CoroHandle::from_promise(*this) };
        }

        std::suspend_always initial_suspend() const noexcept
        {
            return {};
        }

        std::suspend_always final_suspend() const noexcept
        {
            return {};
        }

        void unhandled_exception()
        {
            std::terminate();
        }

        void return_void() const noexcept
        {}

        CoState::CoroHandle nextHandle = nullptr;
    };

public:
    CoState(CoroHandle h) :
        hnd {h}
    {}
    ~CoState()
    {
        if (hnd) {
            hnd.destroy();
        }
    }

    CoState(const CoState&) = delete;
    CoState& operator=(const CoState&) = delete;

    bool nextStep() const
    {
        if (!hnd || hnd.done()) {
            return false;
        }

        // find deepest sub-coroutine not done yet:
        CoroHandle innerHdl = hnd;
        while (innerHdl.promise().nextHandle && !innerHdl.promise().nextHandle.done()) {
            innerHdl = innerHdl.promise().nextHandle;
        }
        innerHdl.resume(); // RESUME it

        return !hnd.done();
    }


public:
// Awaiter part
    bool await_ready()
    {
        return false;   // DO suspend!
    }

    void await_suspend(CoroHandle caller)
    {
        caller.promise().nextHandle = hnd; // store sub-coroutine and suspend
    }

    void await_resume() { }

private:
    CoroHandle hnd;
};


CoState fun1(int num);
CoState fun2(int num);
CoState fun3(int num);

CoState fun1(int num)
{
    std::cout << " State 1 " << num << "\n";
    if (num > 3) {
        co_await fun2(num - 3);
    }
    else {
        co_await fun3(7);
    }
    std::cout << "State 1: END\n";
}

CoState fun2(int num)
{
    std::cout << "State 2 " << num << "\n";
    co_await fun1(num - 2); 
    std::cout << "State 2 again ...\n";
    co_await fun3(num);
    std::cout << "State 2: END\n";
}

CoState fun3(int num)
{
    std::cout << "State 3 " << num << "\n";
    if (num > 0) {
        co_await fun3(num - 2);
    }
    std::cout << "State 3 END\n";
}

int main()
{
    CoState machine = fun1(10);
    
    while (machine.nextStep()) {
        std::cout << "Next step in the SM\n";
    }

    return 0;
}
