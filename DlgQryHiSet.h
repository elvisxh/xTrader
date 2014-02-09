#pragma once


// DlgQryHiSet 对话框

class DlgQryHiSet : public CDialog
{
	DECLARE_DYNAMIC(DlgQryHiSet)

public:
	DlgQryHiSet(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~DlgQryHiSet();

// 对话框数据
	enum { IDD = IDD_DLG_HISETM };

public:
	CWinThread *m_pQryDay,*m_pQryMonth;
	static UINT QryDaySmi(LPVOID pParam);
	static UINT QryMonthSmi(LPVOID pParam);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CDateTimeCtrl m_Date;
	CString m_szHiSet;
	afx_msg void OnDestroy();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnClkQryDay();
	afx_msg void OnClkQryMonth();
	afx_msg void OnClkSave2file();
	afx_msg void OnClkClose();
	afx_msg LRESULT QrySmiMsg(WPARAM wParam,LPARAM lParam);
};
