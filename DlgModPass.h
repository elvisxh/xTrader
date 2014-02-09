#if !defined(AFX_DLGMODPASS_H__785AAAFE_6F76_40E3_B8BD_B478E1083376__INCLUDED_)
#define AFX_DLGMODPASS_H__785AAAFE_6F76_40E3_B8BD_B478E1083376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgModPass.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DlgModPass dialog

class DlgModPass : public CDialog
{
// Construction
public:
	DlgModPass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgModPass)
	enum { IDD = IDD_DLG_MODPASS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	int		m_itype;
	CString m_szNewPass,m_szOldPass,m_szNewCfm;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgModPass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgModPass)
	afx_msg void OnTradepass();
	afx_msg void OnAccpass();
	afx_msg void OnChangeOripass();
	afx_msg void OnChangeNewpass();
	afx_msg void OnChangeNewpasscfm();
	afx_msg void OnDestroy();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMODPASS_H__785AAAFE_6F76_40E3_B8BD_B478E1083376__INCLUDED_)
