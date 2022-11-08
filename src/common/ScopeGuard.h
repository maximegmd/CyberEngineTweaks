#pragma once

// Stolen from: https://stackoverflow.com/questions/10270328/the-simplest-and-neatest-c11-scopeguard
class ScopeGuard
{
public:
    template<class Callable>
    ScopeGuard(Callable&& undo_func)
    try : f(std::forward<Callable>(undo_func))
    {
    }
    catch (...)
    {
        undo_func();
        throw;
    }

    ScopeGuard(ScopeGuard&& other) noexcept
        : f(std::move(other.f))
    {
        other.f = nullptr;
    }

    ~ScopeGuard()
    {
        if (f)
            f(); // must not throw
    }

    void dismiss() noexcept
    {
        f = nullptr;
    }

    ScopeGuard(const ScopeGuard&) = delete;
    void operator=(const ScopeGuard&) = delete;

private:
    std::function<void()> f;
};