#include "../src/d3d12/ShutdownCompletion.h"
#include "../src/common/ProcessExit.h"
#include <cassert>
#include <iostream>
#include <memory>

int main()
{
    using namespace ShutdownCompletion;
    uint64_t time = 0;
    unsigned reads = 0;
    auto elapsed = [&] { return time; };
    auto pause = [&] { ++time; };
    assert(Wait([] { return uint64_t{1}; }, elapsed, pause, 1, 2) == Result::Complete);
    assert(time == 0);
    assert(Wait([&] { ++reads; return time >= 1 ? uint64_t{1} : uint64_t{0}; }, elapsed, pause, 1, 2) == Result::Complete);
    assert(time == 1 && reads == 2);
    time = 0;
    assert(Wait([] { return uint64_t{0}; }, elapsed, pause, 1, 2) == Result::TimedOut);
    assert(time == 2);
    time = 0;
    assert(Wait([] { return std::numeric_limits<uint64_t>::max(); }, elapsed, pause, 1, 2) == Result::DeviceRemoved);
    assert(time == 0);
    assert(Wait([] { return uint64_t{2}; }, elapsed, pause, 1, 0) == Result::Complete);
    std::cout << "5 GPU-completion policy cases passed (mock fence/clock, no live GPU)\n";
    assert(ProcessExit::SelectAction(true) == ProcessExit::DetachAction::ProcessTermination);
    assert(ProcessExit::SelectAction(false) == ProcessExit::DetachAction::DynamicUnload);
    int destructions = 0;
    struct Tracked { int* count; ~Tracked() { ++*count; } };
    auto owner = std::make_unique<Tracked>(&destructions);
    auto* retained = owner.get();
    ProcessExit::LeaveForOperatingSystem(owner);
    assert(!owner && destructions == 0);
    // Unlike production process termination, this test continues: reclaim it.
    delete retained;
    assert(destructions == 1);
    std::cout << "3 process-exit policy cases passed (mock owner)\n";
}
