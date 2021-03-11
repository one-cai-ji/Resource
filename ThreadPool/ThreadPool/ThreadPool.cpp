#include "ThreadPool.h"

ThreadPool::ThreadPool() :ThreadPool(3,100,100){
}

ThreadPool::ThreadPool(int min, int max, int que_size) :
	min_num(min),
	max_num(max),
	live_num(0),
	busy_num(0),
	wait_exit_num(0),
	shutdown(false),
	curr_que_size(0),
	max_que_size(que_size) {

	std::thread adjust_thread(&ThreadPool::AdjustTread, this);
	adjust_thread.detach();

	for (int i = 0; i < min_num; i++) {
		std::thread work_thread(&ThreadPool::ThreadFun, this, i);
		work_thread.detach();
		g_lock.lock();
		++live_num;
		g_lock.unlock();
	}
}

ThreadPool::~ThreadPool() {
	shutdown = true;
	for (int i = 0; i < max_num; i++) {
		//if live
		//if (WorkThread[i]!=nullptr) {
		//	WorkThread[i].get()->join();
		//}
	}
}

void ThreadPool::AdjustTread() {

	while (!shutdown) {
		std::this_thread::sleep_for(std::chrono::duration<int>(2));
		int m_live_num, m_busy_num, m_curr_que_size;
		{
			std::unique_lock<std::mutex> que_lock(g_lock);
			std::cout << "it is adjust thread" << std::endl;
			m_live_num = live_num;
			m_busy_num = busy_num;
			m_curr_que_size = curr_que_size;
		}


		if (m_curr_que_size >= MIN_WAIT_TASK_NUM && m_live_num < max_num) {
			std::unique_lock<std::mutex> que_lock(g_lock);
			int i = 0;
			for (int j = 0; j < max_num && i < DEFAULT_THREAD_VARY && live_num < max_num; j++) {
				std::thread work_thread(&ThreadPool::ThreadFun, this, i);
	
				work_thread.detach();
				++i;
				++live_num;

			}
		}

		{
			std::unique_lock<std::mutex> que_lock(g_lock);
			m_live_num = live_num;
			m_busy_num = busy_num;
		}

		if (m_busy_num * 2 < m_live_num && m_live_num > min_num) {
			std::unique_lock<std::mutex> que_lock(g_lock);
			wait_exit_num = DEFAULT_THREAD_VARY;
			que_lock.unlock();
			for (size_t i = 0; i < DEFAULT_THREAD_VARY; i++) {
				que_no_empty.notify_one();
			}
		}
		std::cout << "live num:" << live_num << std::endl;
		std::cout << "it is adjust end" << std::endl;
	}
	return;
}

void ThreadPool::ThreadFun(int i) {
	while (true) {
		std::unique_lock<std::mutex> que_lock(g_lock);
		while (curr_que_size == 0 && !shutdown) {
			que_no_empty.wait(que_lock);
			if (wait_exit_num > 0) {
				--wait_exit_num;
				if (live_num > min_num) {
					--live_num;
				}
				//there may be error
				//WorkThread[i].get()->detach();// = nullptr;
				if (que_lock.owns_lock()) {
					que_lock.unlock();
				}


				return;
			}
		}

		if (shutdown) {
			if (que_lock.owns_lock()) {
				que_lock.unlock();
			} /*else {
				que_lock.lock();
				WorkThread[i] = nullptr;
				que_lock.unlock();
			}*/
			return;
		}

		MyTask get_task = task.front();
		task.pop();
		--curr_que_size;
		std::cout << "thread id:" << std::this_thread::get_id() << "		val:" << std::endl;
		std::cout << "live num:" << live_num << std::endl;
		if (que_lock.owns_lock()) {
			que_lock.unlock();
		}
		que_no_full.notify_one();

		busy_num_lock.lock();
		++busy_num;
		busy_num_lock.unlock();
		std::this_thread::sleep_for(std::chrono::duration<float>(0.5));
		//do task;
		get_task.Run();
		busy_num_lock.lock();
		--busy_num;
		busy_num_lock.unlock();

	}
}

void ThreadPool::AddTask(MyTask new_task) {
	std::unique_lock<std::mutex> que_lock(g_lock);
	while (curr_que_size == max_que_size && !shutdown) {
		que_no_full.wait(que_lock);
	}
	if (shutdown) {
		que_lock.unlock();
		que_no_empty.notify_all();
		return;
	}
	task.push(new_task);
	++curr_que_size;
	que_lock.unlock();
	que_no_empty.notify_one();
	return;
}
