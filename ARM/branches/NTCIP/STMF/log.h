#ifndef _LOG_H_
#define _LOG_H_

class CLog
{
public:
	CLog();
	~CLog();
	void SaveDebugInfo(const char *pchLogInfo);
private:
	void Lock();
	void UnLock();
};

#endif
