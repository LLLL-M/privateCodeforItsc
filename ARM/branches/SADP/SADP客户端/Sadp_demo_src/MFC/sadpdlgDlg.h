// sadpdlgDlg.h : header file
//

#if !defined(AFX_SADPDLGDLG_H__6BAB35E0_35E8_49BB_89BC_3C948EBCD9F6__INCLUDED_)
#define AFX_SADPDLGDLG_H__6BAB35E0_35E8_49BB_89BC_3C948EBCD9F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




/////////////////////////////////////////////////////////////////////////////
// CSadpdlgDlg dialog

class CSadpdlgDlg : public CDialog
{
// Construction
public:
	CSadpdlgDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSadpdlgDlg)
	enum { IDD = IDD_SADPDLG_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSadpdlgDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	afx_msg void OnCustomDrawList(NMHDR *pNMHDR, LRESULT *pResult);

	// Generated message map functions
	//{{AFX_MSG(CSadpdlgDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
//	afx_msg void OnButton1();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnButtonEntry();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SADPDLGDLG_H__6BAB35E0_35E8_49BB_89BC_3C948EBCD9F6__INCLUDED_)
