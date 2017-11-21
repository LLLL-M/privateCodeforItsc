#ifndef __INDEX_H__
#define __INDEX_H__

class Index
{
private:
	int pos;
	int range;
	int add(int off)
	{
		int v = pos + off;
		if (v >= range)
			return v - range;
		else if (v < 0)
			return range + v;
		else
			return v;
	}
public:
	Index(int _range = 256, int _pos = 0)
	{
		pos = (_pos == _range) ? 0 : _pos;
		range = _range;
	}
	/*默认拷贝构造函数，无需自己实现
	Index(const Index &i)
	{
		pos = i.pos;
		range = i.range;
	}*/
	operator int() const	//读取被封装的int pos值，是个类型转换操作
	{
		return pos;
	}
	const int max()
	{
		return range;
	}
	int operator++()
	{
		pos = add(1);
		return pos;
	}
	int operator++(int)
	{
		int v = pos;
		pos = add(1);
		return v;
	}
	int operator--()
	{
		pos = add(-1);
		return pos;
	}
	int operator--(int)
	{
		int v = pos;
		pos = add(-1);
		return v;
	}
	int operator+(int v)
	{
		return add(v);
	}
	int operator-(int v)
	{
		return add(0 - v);
	}
	void operator=(int v)
	{
		pos = 0;
		pos = add(v);
	}
	bool operator==(int v)
	{
		return pos == v;
	}
	bool operator!=(int v)
	{
		return pos != v;
	}
	Index & operator+=(int v)
	{
		pos = add(v);
		return *this;
	}
	Index & operator-=(int v)
	{
		pos = add(0 - v);
		return *this;
	}
#if 0
	int operator+(Index &i)
	{
		return add(i);
	}
	int operator-(Index &i)
	{
		return add(0 - i);
	}
	void operator=(Index &i)
	{
		pos = i;
	}
	bool operator==(Index &i)
	{
		return pos == i;
	}
	bool operator!=(Index &i)
	{
		return pos != i;
	}
	Index & operator+=(Index &i)
	{
		pos = add(i);
		return *this;
	}
	Index & operator-=(Index &i)
	{
		pos = add(0 - i);
		return *this;
	}
	int operator*()
	{
		return pos;
	}
#endif
};

#endif
