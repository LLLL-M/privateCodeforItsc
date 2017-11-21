// sadpdlg.h : main header file for the SADPDLG application
//

#if !defined(AFX_SADPDLG_H__DB0F7D05_1B6A_4BD1_8FC3_389B7EEAEA84__INCLUDED_)
#define AFX_SADPDLG_H__DB0F7D05_1B6A_4BD1_8FC3_389B7EEAEA84__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSadpdlgApp:
// See sadpdlg.cpp for the implementation of this class
//

class CSadpdlgApp : public CWinApp
{
public:
	CSadpdlgApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSadpdlgApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSadpdlgApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SADPDLG_H__DB0F7D05_1B6A_4BD1_8FC3_389B7EEAEA84__INCLUDED_)
