// SortList.cpp : implementation file
//

#include "stdafx.h"
#include "SortList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSortList
int CALLBACK ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

CSortList::CSortList()
{
	m_fAsc=TRUE;
}

CSortList::~CSortList()
{
}


BEGIN_MESSAGE_MAP(CSortList, CListCtrl)
	//{{AFX_MSG_MAP(CSortList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSortList message handlers

