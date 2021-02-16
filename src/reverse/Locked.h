#pragma once

#include "Lockable.h"

template<class T, class TLock>
struct Locked
{
    Locked(const T& aValue, TLock& aLock)
        : m_handle(aLock)
        , m_value(aValue)
        , m_lock(aLock)
    {
    }

    Locked(const Lockable<T, TLock>& aLockable)
        : m_handle(aLockable.m_lock)
        , m_value(aLockable.m_value)
        , m_lock(aLockable.m_lock)
    {
    }

    operator Lockable<T, TLock>() const
    {
        return {m_value, m_lock};
    }

    const T& Get() const noexcept
    {
        return m_value;
    }

    T& Get() noexcept
    {
        return m_value;
    }

private:
    friend struct Lockable<T, TLock>;

    std::scoped_lock<TLock> m_handle;
    T m_value;
    TLock& m_lock;
};