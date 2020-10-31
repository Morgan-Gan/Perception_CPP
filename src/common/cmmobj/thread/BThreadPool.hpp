#pragma once
#include <thread>
#include <memory>
#include <atomic>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stdexcept>
static const std::size_t bthreadpoll_max_threads = 30;
namespace common_cmmobj
{
	class BThreadPool
	{
	public:
		using Task = std::function<void()>;
		/** ���캯��
		* @param numThreads[in] - �̳߳����̵߳ĸ���
		*/
		explicit BThreadPool(int numThreads = 4) : mNumThreads(numThreads), mbStop(false)
		{
			if (mNumThreads > bthreadpoll_max_threads)
				mNumThreads = bthreadpoll_max_threads;
			mMaxQueueSize = mNumThreads * 10;
			StartAllThread(mNumThreads);
		}
		/**��������
		*/
		~BThreadPool()
		{
			StopAllThread();
		}
		/**��������
		*/
		template<class F, class... Args>
		auto AddTask(F&& f, Args&&... args)
			-> std::future<typename std::result_of<F(Args...)>::type>
		{
			//1. ��������
			using result_type = typename std::result_of<F(Args...)>::type;
			//2. ��װtask
			auto task = std::make_shared< std::packaged_task<result_type()> >(
				std::bind(std::forward<F>(f), std::forward<Args>(args)...));
			////3. ��ȡ����ֵ
			auto res = task->get_future();
			{
				//4. ���������������?
				//4.1 ������
				std::unique_lock<std::mutex> lck(mTaskMutex);
				//4.2 ��ѯ�Ƿ��������?
				mTaskCondFull.wait(lck, [this]()
				{
					return mbStop || mTaskQueue.size() < mMaxQueueSize;
				});
				//4.3 ������м���?
				if (mbStop)
				{
					//������ʱ���������˳��̳߳�
					throw std::runtime_error("AddTask on stopped ThreadPool");
				}
				mTaskQueue.emplace([task]() {(*task)(); });
			}
			////5. ����֪ͨ
			mTaskCondEmpty.notify_one();
			return res;
		}

		std::vector<std::thread::id> GetThreadIds()
		{
			std::vector<std::thread::id> ids;
			for (auto& i : mWorkers)
				ids.push_back(i.get_id());
			return ids;
		}
	private:
		/**���������߳�
		*/
		void StartAllThread(int numThreads)
		{
			for (int i = 0; i < numThreads; i++)
			{
				mWorkers.emplace_back(std::thread(&BThreadPool::Runing, this));
			}
			mbStop = false;
		}
		/**�߳�����
		*/
		void Runing()
		{
			while (!mbStop)
			{
				Task task;
				//1. �Ӷ�����ȡ������
				{
					//1.1 ����
					std::unique_lock<std::mutex> lck(mTaskMutex);
					//1.2 ��ѯ 
					mTaskCondEmpty.wait(lck, [this]() {
						return mbStop || (!mTaskQueue.empty());
					});
					//1.3 ȡ��
					if (mbStop)
						break;
					task = std::move(mTaskQueue.front());
					mTaskQueue.pop();
					mTaskCondFull.notify_one();
				}
				//2. �����������?
				task();
			}
		}
		/**ֹͣ�����߳�
		*/
		void StopAllThread()
		{
			{
				std::unique_lock<std::mutex> lck(mTaskMutex);
				mbStop = true;
			}
			mTaskCondFull.notify_all();
			mTaskCondEmpty.notify_all();
			for (auto& w : mWorkers)
				w.join();
		}
	private:
		int mNumThreads;                //�̳߳����̵߳ĸ���
		int mMaxQueueSize;                //�̳߳��У�������е����ֵ
		std::vector<std::thread>        mWorkers;        //�����߳�
		std::queue<Task>                mTaskQueue;        //�������?
		std::mutex                        mTaskMutex;        //ͬ����
		std::condition_variable            mTaskCondEmpty;        //
		std::condition_variable            mTaskCondFull;        //
		std::atomic<bool>                mbStop;
	};
}