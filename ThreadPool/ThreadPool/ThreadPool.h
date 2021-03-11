#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <iostream>
#include <queue>
#include <memory>

#include <thread>
#include <mutex>
#include <condition_variable>

#define MAX_THREAD_NUM 100
#define DEFAULT_TIME 10                 /*10s检测一次*/
#define MIN_WAIT_TASK_NUM 10            /*如果queue_size > MIN_WAIT_TASK_NUM 添加新的线程到线程池*/ 
#define DEFAULT_THREAD_VARY 10          /*每次创建和销毁线程的个数*/

/*task struct
*/
struct MyTask {
	void (*fun)(int, int);
	int argv1;
	int argv2;
	MyTask() {
		fun = nullptr;
		argv1 = -1;
		argv2 = -1;
	}
	void Run() {
		fun(argv1, argv2);
	}
};

class ThreadPool {
private:
	int min_num;
	int max_num;
	int live_num;
	int busy_num;
	int wait_exit_num;

	std::queue<MyTask> task;
	int curr_que_size;
	int max_que_size;

	bool shutdown;

	std::mutex g_lock;
	std::mutex busy_num_lock;

	std::condition_variable que_no_full;
	std::condition_variable que_no_empty;
	void ThreadFun(int i);
public:
	ThreadPool();
	~ThreadPool();
	ThreadPool(int min, int max, int que_size);
	void AddTask(MyTask new_task);
	void AdjustTread();
};
#endif
