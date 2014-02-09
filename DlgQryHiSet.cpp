// DlgQryHiSet.cpp : 实现文件
//

#include "stdafx.h"
#include "xTrader.h"
#include "DlgQryHiSet.h"

//extern vector<CThostFtdcSettlementInfoField*> StmiVec;
extern HANDLE g_hEvent;

IMPLEMENT_DYNAMIC(DlgQryHiSet, CDialog)

DlgQryHiSet::DlgQryHiSet(CWnd* pParent /*=NULL*/)
	: CDialog(DlgQryHiSet::IDD, pParent)
{
	//m_szHiSet=NULL;
	m_szHiSet =_T("");
	m_pQryDay = NULL;
	m_pQryMonth = NULL;

}

DlgQryHiSet::~DlgQryHiSet()
{
}

void DlgQryHiSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DTPICK, m_Date);
	DDX_Text(pDX, IDC_EDIT_HISOD, m_szHiSet);
}

BEGIN_MESSAGE_MAP(DlgQryHiSet, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BT_QRYDAY, OnClkQryDay)
	ON_BN_CLICKED(IDC_BT_QRYMONTH, OnClkQryMonth)
	ON_BN_CLICKED(IDC_BT_SAVEFILE, OnClkSave2file)
	ON_BN_CLICKED(IDC_BTCLOSE, OnClkClose)
	ON_REGISTERED_MESSAGE(WM_QRYSMI_MSG,QrySmiMsg)
END_MESSAGE_MAP()

// DlgQryHiSet 消息处理程序
void DlgQryHiSet::OnDestroy()
{
	CDialog::OnDestroy();

	delete this;
}


BOOL DlgQryHiSet::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  
}

LRESULT DlgQryHiSet::QrySmiMsg(WPARAM wParam,LPARAM lParam)
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	
	UINT uBufSize = pApp->m_cT->m_StmiVec.size()*sizeof(TThostFtdcContentType);
	char* szMsg = new char[uBufSize];
	ZeroMemory(szMsg,sizeof(szMsg));
	
	//pDlg->m_szHiSet=_T("");
	for (UINT i=0;i<pApp->m_cT->m_StmiVec.size();i++)
	{
		strcat(szMsg,(const char*)pApp->m_cT->m_StmiVec[i]->Content);
	}
	
	ansi2uni(CP_ACP,szMsg,m_szHiSet.GetBuffer(4*uBufSize));
	m_szHiSet.ReleaseBuffer();
	
	UpdateData(FALSE);
	
	delete []szMsg;
	return 0;
}

UINT DlgQryHiSet::QryDaySmi(LPVOID pParam)
{
	DlgQryHiSet* pDlg = (DlgQryHiSet*)pParam;
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();

	pApp->m_cT->m_StmiVec.clear();
	pDlg->m_szHiSet.Empty();
	
	CTime tm;
	DWORD dwResult = pDlg->m_Date.GetTime(tm);
	CString szDate;
	char szDay[9];
	
	if (dwResult == GDT_VALID)
	{
		szDate = tm.Format(_T("%Y%m%d"));
	}
	
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)szDate,szDay);
	
	pApp->m_cT->ReqQrySettlementInfo(szDay);

	DWORD dwRet = WaitForSingleObject(g_hEvent,INFINITE);
	if (dwRet==WAIT_OBJECT_0)
	{ 
		ResetEvent(g_hEvent); 
	}
	else
	{
		pDlg->m_pQryDay = NULL;
		return 0;
	}

	pDlg->m_pQryDay = NULL;
	return 0;
}

void DlgQryHiSet::OnClkQryDay()
{

	if (m_pQryDay==NULL)
	{
		m_pQryDay = AfxBeginThread((AFX_THREADPROC)DlgQryHiSet::QryDaySmi,this);
	}
	
}

UINT DlgQryHiSet::QryMonthSmi(LPVOID pParam)
{
	DlgQryHiSet* pDlg = (DlgQryHiSet*)pParam;
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();

	pApp->m_cT->m_StmiVec.clear();
	pDlg->m_szHiSet.Empty();
	
	CTime tm;
	DWORD dwResult = pDlg->m_Date.GetTime(tm);
	CString szDate;
	char szDay[7];
	
	if (dwResult == GDT_VALID)
	{
		szDate = tm.Format(_T("%Y%m"));
	}

	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)szDate,szDay);
	
	pApp->m_cT->ReqQrySettlementInfo(szDay);
	
	DWORD dwRet = WaitForSingleObject(g_hEvent,INFINITE);
	if (dwRet==WAIT_OBJECT_0)
	{ 
		ResetEvent(g_hEvent); 
	}
	else
	{
		pDlg->m_pQryMonth = NULL;
		return 0;
	}

	pDlg->m_pQryMonth = NULL;
	return 0;
}

void DlgQryHiSet::OnClkQryMonth()
{
	if (m_pQryMonth==NULL)
	{
		m_pQryMonth = AfxBeginThread((AFX_THREADPROC)DlgQryHiSet::QryMonthSmi,this);
	}
		
}


void DlgQryHiSet::OnClkSave2file()
{
	CString  strFilter = _T("文本文件(*.log;*.txt)|(*.*;*.log;*.txt;)|所有文件 |*.*||");
	
	CFileDialog* dlgSave = new CFileDialog(false, _T("*.txt"),  GenDef(_T("结算单"),_T("txt")), OFN_PATHMUSTEXIST | OFN_EXPLORER, strFilter, this);
	dlgSave->m_ofn.lStructSize=sizeof(OPENFILENAME);		//use the 2k+ open file dialog
	
	CString szFile;
	if (IDOK == dlgSave->DoModal())
	{
		szFile = dlgSave->GetPathName();
		
		CFile fLog(szFile, CFile::modeReadWrite | CFile::modeCreate | CFile::typeText);

		UpdateData(TRUE);

		int iLen = m_szHiSet.GetLength();
		char* szLog = new char[4*iLen];

		uni2ansi(CP_UTF8,(LPTSTR)(LPCTSTR)m_szHiSet,szLog);

		BYTE bBom[3]={0xEF,0xBB,0xBF};
		fLog.Write(&bBom,3);

		fLog.Write(szLog,strlen(szLog));
		fLog.Close();

		delete []szLog;
		
	}
	
	delete []dlgSave;	
}

void DlgQryHiSet::OnOK()
{
	CDialog::OnOK();
	DestroyWindow();
}

void DlgQryHiSet::OnCancel()
{
	CDialog::OnCancel();
	DestroyWindow();
}

void DlgQryHiSet::OnClkClose()
{
	OnOK();
}
