/*
    Copyright (c) 2024 Piotr Stradowski. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "jobScheduler.h"

std::shared_ptr<JobScheduler> JobScheduler::m_instance = nullptr;

JobScheduler::JobScheduler(size_t workerThreadsCount) : m_stop(false)
{
    for (size_t i = 0; i < workerThreadsCount; i++)
    {
        m_workerThreadPool.emplace_back([this]()
            {
                while (true)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(m_queueMutex);

                        // wait for a job or stop signal
                        m_workerThreadWaitCondition.wait(lock, [this]
                            {
                                return m_stop || !m_tasks.empty();
                            });

                        if (m_stop && m_tasks.empty()) return;

                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }
                    task();
                }
            });
    }
}

JobScheduler::~JobScheduler()
{
    {
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_stop = true;
	}
	m_workerThreadWaitCondition.notify_all();

    for (std::thread& workerThread : m_workerThreadPool)
    {
		workerThread.join();
	}
}
