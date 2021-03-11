#include <iostream>
#include <string>
#include "ThreadPool.h"
using namespace std;
//mutex m_mutex;

//void fun() {
//
//	unique_lock<mutex> temp(m_mutex);
//	cout << std::this_thread::get_id() << endl;
//	std::this_thread::sleep_for(std::chrono::duration<int>(1));
//
//	if (temp.owns_lock()) {
//		//这儿可能会断开所以不安全的；
//		temp.unlock();
//	}
//}

void fun(int a, int b) {
	cout << a << b << endl;
}

int main(int argc, const char* argv[]) {

	//test ThreadPool

	ThreadPool pool;
	MyTask new_task;
	new_task.fun = &fun;
	for (int i = 0; i < 2000; i++) {
		new_task.argv1 = i;
		pool.AddTask(new_task);
	}
	std::this_thread::sleep_for(std::chrono::duration<int>(20s));
	for (int i = 0; i < 2000; i++) {
		new_task.argv1 = i;
		pool.AddTask(new_task);
	}
	return 0;


	//if (argc < 3) {
	//	cout << "eg: ./a.out port path\n" << endl;
	//	exit(1);
	//}
	//
	//int port = stoi(argv[1]);
}