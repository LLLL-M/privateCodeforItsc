#if !defined(AFX_DLGHIKSADP_H__2A85A69B_6821_4174_BF8E_EA2F34ECC57E__INCLUDED_)
#define AFX_DLGHIKSADP_H__2A85A69B_6821_4174_BF8E_EA2F34ECC57E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgHikSadp.h : header file
//


#include "Include/Sadp.h"
#include "sadpdlg.h"
#include <afxmt.h>
#include <afxtempl.h>
#include "SortList.h"

#define MSG_FIND_DEVICE  9000    // �����豸��Ϣ

typedef struct tagSADPRet
{
	BOOL	bselect;
	char	sversion[64];
	char	sseries[64];
	char	szSerialNO[64];
	char	shwaddr[64];
	char	spraddr[64];
	char	ssubnetmask[64];
	int		dev_type;
	unsigned int	port;
	unsigned int	enccnt; 
}SSADPRET,*PSSADPRET;

typedef struct tagSADPDev
{
	char	cdev[20];
}SSADPDEV,*PSSADPDEV;

typedef struct tagSADPInfo
{
	char			sseries[64];
	char			szSerialNO[64];
	char			shwaddr[64];
	char			spraddr[64];
	char			ssubnetmask[64];
	char			ssfwversion[64];
	char			sdspversion[64];
	char			sstarttime[64];
	int				dev_type;
	unsigned int	port;
	unsigned int	enccnt; 
	unsigned int	hdiskcnt;
	char			szDevDesc[24];       //�豸��������
	char			szOEMinfo[24];       //OEM������Ϣ
	char			szIPv4Gateway[16];   //IPv4����
	char			szIPv6Address[46];	 //IPv6��ַ
	char			szIPv6Gateway[46];   //IPv6����
	unsigned char   byIPv6MaskLen;       //IPv6����ǰ׺����
	unsigned char   bySupport;           //��λ��ʾ,��ӦΪΪ1��ʾ֧��,0x01:�Ƿ�֧��Ipv6,0x02:�Ƿ�֧���޸�Ipv6����,0x04:�Ƿ�֧��Dhcp				 
	unsigned char   byDhcpEnabled;       //Dhcp״̬, 0 ������ 1 ����
	unsigned char	byOEMCode;			//0-�����豸 1-OEM�豸
	unsigned short	wHttpPort;			// Http �˿�
	unsigned short	wDigitalChannelNum;
	char			szCmsIPv4[16];
	unsigned short	wCmsPort;
	int				result;
}SSADPINFO,*PSSADPINFO;

/*int dev_type,unsigned int port,\
										unsigned int enccnt, unsigned int hdiskcnt*/
/////////////////////////////////////////////////////////////////////////////
// CDlgHikSadp dialog

class CDlgHikSadp : public CDialog
{
// Construction
public:
	CDlgHikSadp(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgHikSadp)
	enum { IDD = IDD_DLG_HIKSADP };
	CIPAddressCtrl	m_addrCmsIPv4;
	CIPAddressCtrl	m_sip;
	CIPAddressCtrl	m_smask;
	CIPAddressCtrl  m_addrIPv4Gateway;
	CSortList	m_slist;
	CString	m_sdevid;
	CString	m_sversion;
	CString m_strIPv6Adress;
	CString m_strIPv6Gateway;
	CString m_strIPv6MaskLen;
	CString m_strDevDescription;
	UINT	m_uport;
	CString	m_smac;
	CString	m_spsw;
	CString m_strSoftVersion;
	CString	m_strPUID;
	CString	m_strCmsPasswd;
	CString	m_strCmsIPv6;
	UINT	m_wCmsPort;
	CString m_origipasswd;
	short	m_wHttpPort;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgHikSadp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	afx_msg void OnCustomDrawList(NMHDR *pNMHDR, LRESULT *pResult);

	// Generated message map functions
	//{{AFX_MSG(CDlgHikSadp)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnButBack();
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonModify();
	afx_msg void OnButtonCancel();
	afx_msg void OnButtonSafe();
	afx_msg void OnButtonSelectDev();
	afx_msg void OnButtonCmsSet();
	afx_msg void OnBnClickedButtonDefaultPW();
	afx_msg void OnCheckDhcp();
	afx_msg void OnColumnclickListDevInfo(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int			m_ilanguage;

	char *		GetText(int id);

	int			m_icount;
	HICON		m_hIcon;
	SSADPRET *	m_retbuf;

	CButton *m_btnDhcp;


	CList<SSADPDEV,SSADPDEV&>	m_devlist;//�Ѵ����豸�б�

	BOOL		m_brun;
	BOOL		m_bedit;

	long		OnTabDraw(WPARAM wParam, LPARAM lParam);


	void	fDevOnLine(SSADPINFO *pinfo);
	void	fDevOffLine(SSADPINFO *pinfo);
	void	fDevReboot(SSADPINFO *pinfo);
	void	fDevRefresh(SSADPINFO *pinfo);

	void	fEnableButton(BOOL bedit);

	BOOL	fIfInUsed(char * sip);
public:
	DWORD m_dwsmask;
	DWORD m_dwsip;

	void	fInsDevice(char *dvrip);
	void	fSetRetBuf(SSADPRET * pbuf);
	//afx_msg void OnStnClickedStaticPswinput();
	
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGHIKSADP_H__2A85A69B_6821_4174_BF8E_EA2F34ECC57E__INCLUDED_)
