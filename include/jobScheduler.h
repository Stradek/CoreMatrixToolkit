/*
    Copyright (c) 2024 Piotr Stradowski. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include <queue>
#include <cassert>
#include <future>

class JobScheduler
{
public:
    JobScheduler() = delete;
    JobScheduler& operator=(const JobScheduler&) = delete;

    ~JobScheduler();

    static std::shared_ptr<JobScheduler>& createInstance(size_t workerThreadsCount)
    {
        assert(!m_instance);

        m_instance = createShared(workerThreadsCount);

        return m_instance;
    }

    static std::shared_ptr<JobScheduler>& getInstance()
    {
        assert(m_instance);
        return m_instance;
    }

    template<class F, class... Args>
    auto enqueueJob(F&& function, Args... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        // don't allow enqueueing after stopping the pool
        assert(!m_stop);

        using ReturnType = typename std::invoke_result<F, Args...>::type;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(function), std::forward<Args>(args)...)
        );

        std::future<ReturnType> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_tasks.emplace([task](){ (*task)(); });
        }
        m_workerThreadWaitCondition.notify_one();
        return res;
    }

private:
    JobScheduler(size_t workerThreadsCount);

    static std::shared_ptr<JobScheduler> createShared(size_t workerThreadsCount)
    {
        return std::shared_ptr<JobScheduler>(new JobScheduler(workerThreadsCount));
    }

    static std::shared_ptr<JobScheduler> m_instance;

    std::vector<std::thread> m_workerThreadPool;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_workerThreadWaitCondition;
    bool m_stop;
};
