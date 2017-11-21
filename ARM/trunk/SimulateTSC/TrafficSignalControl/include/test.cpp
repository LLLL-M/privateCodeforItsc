#include <iostream>
#include <string>
#include <unistd.h>
#include "thread.h"
#include "msg.h"

using namespace std;
using namespace HikThread;
using namespace HikMsg;
struct Test
{
	int a;
	string b;
};

Manage<SemHandle *> Sem::manage;
template<class T>
Manage<std::queue<T> *> Msg<T>::manage;

class A: public Thread
{
private:

public:
	void run(void *arg)
	{
		Msg<Test> msg("Test");
		Test t;
		cout << "enter A run()\n";
		if (msg.recv(t))
			cout << "a = " << t.a << ", b = " << t.b << endl;
	}
};

class B: public Thread
{
private:

public:
	void run(void *arg)
	{
		Msg<Test> msg("Test");
		Test t = {100, "Hello wrold!"};
		cout << "enter B run()\n";
		sleep(3);
		msg.send(t);
		cout << "class B msg.send\n";
	}
};

int main(void)
{
	Manage<int> fd;
	Msg<Test> msg("Test");
#if 1
	cout << "\n\n";
	sleep(2);
	A a;
	B b;
	Thread & threadA = a;
	Thread & threadB = b;
	threadA.start();
	sleep(2);
	cout << "\n\n";
	threadB.start();

	//Sem sem("sem");

	sleep(5);
#endif
	return 0;
}
