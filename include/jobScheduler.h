/*
    Copyright (c) 2024 Piotr Stradowski. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include <queue>
#include <cassert>
#include <future>

enum class JobStatus : uint8_t
{
    Pending,
    Running,
    Failed,
    Finished
};

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

    static void destroyInstance()
    {
		assert(m_instance);
		m_instance.reset();
	}

    static std::shared_ptr<JobScheduler>& getInstance()
    {
        assert(m_instance);
        return m_instance;
    }

    template<class F, class... Args>
    auto enqueueRepeatingJob(F&& function, Args... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        std::shared_ptr<std::packaged_task<JobStatus()>> task = std::make_shared<std::packaged_task<JobStatus()>>([function, args...]() -> JobStatus
        {
            JobStatus status;
            do
            {
                status = function(args...);
            } while(status == JobStatus::Running);
            
            return status;
        });
        
        return enqueueTask<F, Args...>(task);
    }

    template<class F, class... Args>
    auto enqueueOneShotJob(F&& function, Args... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        std::shared_ptr<std::packaged_task<JobStatus()>> task = std::make_shared<std::packaged_task<JobStatus()>>(std::forward<F>(function), std::forward<Args>(args)...);
        return enqueueTask<F, Args...>(task);
    }

private:
    JobScheduler(size_t workerThreadsCount);

    static std::shared_ptr<JobScheduler> createShared(size_t workerThreadsCount)
    {
        return std::shared_ptr<JobScheduler>(new JobScheduler(workerThreadsCount));
    }

    template<class F, class... Args>
    auto enqueueTask(std::shared_ptr<std::packaged_task<JobStatus()>> task) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        // don't allow enqueueing after stopping the pool
        assert(!m_stop);
        
        using ReturnType = typename std::invoke_result<F, Args...>::type;
        std::future<ReturnType> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_tasks.emplace([task](){ (*task)(); });
        }
        m_workerThreadWaitCondition.notify_one();
        return res;
    }

    static std::shared_ptr<JobScheduler> m_instance;

    std::vector<std::thread> m_workerThreadPool;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_workerThreadWaitCondition;
    bool m_stop;
};
