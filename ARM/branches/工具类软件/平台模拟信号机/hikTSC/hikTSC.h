
// hikTSC.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// ChikTSCApp:
// �йش����ʵ�֣������ hikTSC.cpp
//

class ChikTSCApp : public CWinApp
{
public:
	ChikTSCApp();

// ��д
public:
	virtual BOOL InitInstance();
    BOOL OnlyOneInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern ChikTSCApp theApp;
