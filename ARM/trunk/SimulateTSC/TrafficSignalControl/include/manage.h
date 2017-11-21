#ifndef __TEST_H__
#define __TEST_H__

#include <string>

#if 1	//以下是用map实现的
#include <map>
namespace HikManage
{
	using namespace std;
	template<class T>
	class Manage
	{
	private:
		struct ManageData
		{
			unsigned int count;
			T data;
		};
		typedef typename map<string, ManageData>::iterator Iterator;
		map<string, ManageData> manager;
	public:
		Manage() {}
		~Manage() { manager.clear(); }
		bool get(const string & name, T & data)
		{
			Iterator i = manager.find(name);
			if (i != manager.end())
			{
				i->second.count++;
				data = i->second.data;
				return true;
			}
			else
				return false;
		}
		void add(const string & name, T & data)
		{
			if (!get(name, data))
			{
				manager[name] = {1, data};
			}
		}
		bool del(const string & name)
		{
			Iterator i = manager.find(name);
			if (i == manager.end())
				return false;
			if (i->second.count > 1)
			{
				i->second.count--;
				return false;
			}
			else
			{
				manager.erase(i);
				return true;
			}
		}
	};
}
#else	//以下是用list实现的，可以把list直接替换成vector也可以实现
#include <list>
namespace HikManage
{
	using namespace std;
	template<class T>
	class Manage
	{
	private:
		struct ManageData
		{
			string name;
			unsigned char count;
			T data;
		};
		typedef typename list<ManageData>::iterator Iterator;
		list<ManageData> manager;
		
		Iterator find(const string & name) 
		{
			Iterator i;
			for (i = manager.begin(); i != manager.end(); ++i)
			{
				if ((*i).name == name)
					break;
			}
			return i;
		}
	public:
		Manage() {}
		~Manage() { manager.clear(); }
		bool get(const string & name, T & data)
		{
			Iterator i = find(name);
			if (i != manager.end())
			{
				(*i).count++;
				data = (*i).data;
				return true;
			}
			else
				return false;
		}
		void add(const string & name, T & data)
		{
			if (!get(name, data))
			{
				ManageData ele = {name, 1, data};
				manager.push_back(ele);
			}
		}
		void del(const string & name)
		{
			Iterator i = find(name);
			if (i == manager.end())
				return;
			if ((*i).count > 1)
				(*i).count--;
			else
				manager.erase(i);
		}
	};
}
#endif

#endif
