// videodetect.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CvideodetectApp:
// �йش����ʵ�֣������ videodetect.cpp
//

class CvideodetectApp : public CWinApp
{
public:
	CvideodetectApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CvideodetectApp theApp;