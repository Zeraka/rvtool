#ifndef EVENTLOOP_EVENTLOOP_HPP
#define EVENTLOOP_EVENTLOOP_HPP

#include <cstdint>

#include <queue>
#include <functional>
#include <utility>

#include <thread>
#include <mutex>
#include <condition_variable>

class EventLoop
{
public: 
    EventLoop(size_t numThreads = 1);
    virtual ~EventLoop();

    int64_t Start();
    void Stop(bool finishQueue = false);

    template <typename F>
    int64_t Post(F&& aFunc)
    {
        if(mThreadStopRequest || mThreadFinishQueueRequest)
        {
            return static_cast<int64_t>(-1);
        }

        // scoped lock for eventqueue
        {
            std::lock_guard<std::mutex> scopedLock(mEventQueueMutex);
            mEventQueue.emplace(new Event<F>(std::forward<F>(aFunc)));
        }
        mEventQueueCondition.notify_one();

        return static_cast<int64_t>(0);
    }

    void ClearAll();

private:
    class EventBase
    {
    public:
        EventBase() {}
        virtual ~EventBase() {}

        virtual void doEvent() {}
    };

    template <typename F>
    class Event : public EventBase 
    {
    public:
        Event(F&& aFunc) : EventBase(), 
            mEventFunc(std::forward<F>(aFunc)) {}

        virtual ~Event() {}

        virtual void doEvent()
        {
            mEventFunc();
        }

    private:
        const F mEventFunc;
    };

    virtual void MainLoop();

    std::queue<std::shared_ptr<EventBase>> mEventQueue;
    std::mutex mEventQueueMutex;
    std::condition_variable mEventQueueCondition;

    size_t numThreads;
    std::vector<std::shared_ptr<std::thread>> mThreads;
    bool mThreadStopRequest;
    bool mThreadFinishQueueRequest;
};

#endif // EVENTLOOP_EVENTLOOP_HPP
