
// hikTSCDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"


// ChikTSCDlg �Ի���
class ChikTSCDlg : public CDialogEx
{
// ����
public:
	ChikTSCDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_HIKTSC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	int GetInputValue();
	void DisableInputEdit();
	int getLocalhostAddress();
    void DoService();
    //void VirtualTSC(VirtualTSCParams *pData);
    //int ServiceCountdown(VirtualTSCParams *pData);
    //int ServiceBasic(VirtualTSCParams *pData);
    void SetDefaultCountDownValue();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonStart();
	CComboBox m_IP;
};
