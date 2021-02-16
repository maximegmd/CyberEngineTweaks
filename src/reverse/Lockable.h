#pragma once

template<class T, class U>
struct Locked;

template<class T, class TLock = std::mutex>
struct Lockable
{
    Lockable(T aValue, TLock& aLock)
        : m_value(std::move(aValue))
        , m_lock(aLock)
    {
    }

    Lockable(const Lockable& acRhs)
        : m_value(acRhs.m_value)
        , m_lock(acRhs.m_lock)
    {
    }

    Locked<T, TLock> Lock() const
    {
        return Locked<T, TLock>(*this);
    }

private:

    friend struct Locked<T, TLock>;

    TLock& m_lock;
    T m_value;
};