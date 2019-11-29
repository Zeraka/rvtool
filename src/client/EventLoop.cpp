#include <iostream>
#include <algorithm>
#include <cassert>

#include "EventLoop.hpp"

using namespace std;

EventLoop::EventLoop(size_t _numThreads)
    : numThreads(_numThreads), 
    mThreadStopRequest(true),
    mThreadFinishQueueRequest(true)
{
    assert(_numThreads > 0);
}

EventLoop::~EventLoop()
{
    Stop();
}

int64_t EventLoop::Start()
{
    mThreadStopRequest = false;
    mThreadFinishQueueRequest = false;

    for(size_t i = 0; i < numThreads; ++i)
    {
        mThreads.emplace_back(new thread([this]{MainLoop();}));
    }

    return static_cast<int64_t>(0);
}

void EventLoop::Stop(bool finishQueue) 
{
    if(finishQueue)
    {
        mThreadFinishQueueRequest = true;
    }
    else
    {
        mThreadStopRequest = true;
    }

    mEventQueueCondition.notify_all();

    if(!mThreads.empty()) 
    {
        for(auto mThread : mThreads)
        {
            if(mThread->joinable())
            {
                mThread->join();
            }
        }
    }

    ClearAll();

    mThreadStopRequest = true;
}

void EventLoop::ClearAll()
{
    lock_guard<mutex> scopedLock(mEventQueueMutex);

    
    while(!mEventQueue.empty())
    {
        mEventQueue.pop();
    }
}

void EventLoop::MainLoop()
{
    shared_ptr<EventBase> event = nullptr;

    while(!mThreadStopRequest)
    {
        // scopedLock for event queue
        {
            unique_lock<mutex> scopedLock(mEventQueueMutex);
            if(mEventQueue.empty())
            {
                if(mThreadFinishQueueRequest) break;
                mEventQueueCondition.wait(scopedLock,
                    [this]{return !mEventQueue.empty() ||
                        mThreadFinishQueueRequest ||
                        mThreadStopRequest;});
            }

            if(!mEventQueue.empty())
            {
                event = mEventQueue.front();
                mEventQueue.pop();
            }
        }

        if(event != nullptr)
        {
            event->doEvent();
        }
    }
}
