_Pragma("once")

#include "tsc.h"
#include "common.h"

template<typename T>
void GetJsonValue(T &t, const Json::Value &v)
{
}

template<>
void GetJsonValue(string &t, const Json::Value &v)
{
	t = v.asString();
}

template<>
void GetJsonValue(UInt8 &t, const Json::Value &v)
{
	t = static_cast<UInt8>(v.asInt());
}

template<>
void GetJsonValue(UInt16 &t, const Json::Value &v)
{	
	t = static_cast<UInt16>(v.asInt());
}

template<>
void GetJsonValue(UInt32 &t, const Json::Value &v)
{	
	t = static_cast<UInt32>(v.asInt());
}


template<>
void GetJsonValue(int &t, const Json::Value &v)
{
	t = v.asInt();
}

template<size_t N>
void GetJsonValue(bitset<N> &t, const Json::Value &v)
{
	t = v.asUInt64();
}

template<>
void GetJsonValue(bool &t, const Json::Value &v)
{
	t = v.asBool();
}

template<typename T, size_t N>
void GetJsonValue(array<T, N> &t, const Json::Value &v)
{
	for (unsigned int j = 0; j < v.size(); j++)
	{
		t[j]  = v[j].asInt();
	}
}

template<>
void GetJsonValue(vector<UInt8> &m, const Json::Value &v)
{
	m.resize(v.size());	
	for (unsigned int i = 0; i < v.size(); i++)
	{
		m[i] = (UInt8)v[i].asInt();	
	}
}


template<>
void GetJsonValue(vector<vector<UInt8>> &m, const Json::Value &v)
{
	m.resize(v.size());	
	for (unsigned int i = 0; i < v.size(); i++)
	{
		if(v[i].size() != 0)
		{
			m[i].resize(v[i].size());
			for(unsigned int j = 0; j < v[i].size(); j++)
			{
				m[i][j] = (UInt8)v[i][j].asInt();	
			}
		}
	}
}

void JsonParse(const Json::Value &root)
{
}

template <typename T, typename ... Args>  
void JsonParse(const Json::Value &root, const char *key, T &t, Args&&... args)
{
	//cout << "now:" << key <<endl; 
	if (root.isMember(key))
	{ 
		GetJsonValue(t, root[key]);
		//cout << "GetJsonValue:" << key <<endl; 
	}
	JsonParse(root, forward<Args>(args)...);
}

template<>
void GetJsonValue(map<int, TscScheme::PhaseInfo> &m, const Json::Value &v)
{	
	UInt8 phase = 0;
	for (unsigned int i = 0; i < v.size(); i++)
	{
		JsonParse(v[i], TSC_PHASE_ID ,phase);
		JsonParse(v[i], TSC_PHASE_ID ,m[phase].phase, TSC_PHASE_TIME, m[phase].time,  
				TSC_PHASE_STATUS, m[phase].status, TSC_BARRIER_ID, m[phase].barrier);
	}
}

template<>
void GetJsonValue(vector<TscTimeinterval::TimeSection> &m, const Json::Value &v)
{
	m.resize(v.size());
	for (unsigned int i = 0; i < v.size(); i++)
	{
		JsonParse(v[i],  TSC_HOUR, m[i].hour, TSC_MINUTE, m[i].minute,
		            TSC_CONTROL_MODE, m[i].mode, TSC_SCHEME_ID, m[i].scheme);
	}
}

template<>
void GetJsonValue(vector<TscSchedule::TscDate> &m, const Json::Value &v)
{
	m.resize(v.size());
	for (unsigned int i = 0; i < v.size(); i++)
	{
		JsonParse(v[i], TSC_YEAR, m[i].year, TSC_MONTH, m[i].month, TSC_DAYS, m[i].days);
	}
}

/*template<typename T>
TscRetval UpDownSingle(T &t, bool upload, zframe_t *frame)
{
	TscRetval ret;

	if ( !upload )
	{
		ret = t.Parse(frame);
	}
	else
	{
		ret = t.Json();
	}

	return ret;
}*/

/*template<typename T>
void UpdateUsr(T &t, const TscRetval &ret)
{
	hik::rwlock rwl;
	if (ret.succ == true)
	{	
		rwl.w_lock();
		t.use = t.bak;
		rwl.w_unlock();
	}
	else
	{
		rwl.w_lock();
		t.bak = t.use;
		rwl.w_unlock();
	}
}*/
