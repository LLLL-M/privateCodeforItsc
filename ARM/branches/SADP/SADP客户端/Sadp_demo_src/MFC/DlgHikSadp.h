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

#define MSG_FIND_DEVICE  9000    // 发现设备消息

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
	char			szDevDesc[24];       //设备类型描述
	char			szOEMinfo[24];       //OEM产商信息
	char			szIPv4Gateway[16];   //IPv4网关
	char			szIPv6Address[46];	 //IPv6地址
	char			szIPv6Gateway[46];   //IPv6网关
	unsigned char   byIPv6MaskLen;       //IPv6子网前缀长度
	unsigned char   bySupport;           //按位表示,对应为为1表示支持,0x01:是否支持Ipv6,0x02:是否支持修改Ipv6参数,0x04:是否支持Dhcp				 
	unsigned char   byDhcpEnabled;       //Dhcp状态, 0 不启用 1 启用
	unsigned char	byOEMCode;			//0-基线设备 1-OEM设备
	unsigned short	wHttpPort;			// Http 端口
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


	CList<SSADPDEV,SSADPDEV&>	m_devlist;//已存在设备列表

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
