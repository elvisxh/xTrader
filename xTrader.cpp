// xTrader.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "xTrader.h"
#include "xTraderDlg.h"
#include "LoginDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//int requestId=0;  
HANDLE g_hEvent;
LARGE_INTEGER g_lFreq;
BOOL bRecconnect = FALSE;
//CRITICAL_SECTION cs;

CWnd* g_pCWnd = NULL;
/////////////////////////////////////////////////////////////////////////////
// CXTraderApp

BEGIN_MESSAGE_MAP(CXTraderApp, CWinApp)
	//{{AFX_MSG_MAP(CXTraderApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	//ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXTraderApp construction

CXTraderApp::CXTraderApp()
{
	m_MApi = NULL;
	m_TApi = NULL;
	m_cQ = NULL;
	m_cT = NULL;

	WSADATA Sock_Data;
	WSAStartup(MAKEWORD(2,2),&Sock_Data);

	::QueryPerformanceFrequency(&g_lFreq);
	g_hEvent=CreateEvent(NULL, true, false, NULL); 
	LoadConfig();
}

CXTraderApp::~CXTraderApp()
{
	WSACleanup();

}

void CXTraderApp::CreateApi()
{
	m_TApi = CThostFtdcTraderApi::CreateFtdcTraderApi("log\\");
	m_cT = new CtpTraderSpi(m_TApi);
	m_TApi->RegisterSpi((CThostFtdcTraderSpi*)m_cT);
	m_TApi->SubscribePublicTopic(THOST_TERT_RESTART);
	m_TApi->SubscribePrivateTopic(THOST_TERT_RESTART);
	
	m_MApi = CThostFtdcMdApi::CreateFtdcMdApi("log\\");
	m_cQ = new CtpMdSpi(m_MApi);
	m_MApi->RegisterSpi(m_cQ);
}

void CXTraderApp::ReleaseApi()
{
	if (m_TApi)
	{	
		m_TApi->RegisterSpi(NULL);
		m_TApi->Release();	
	}
	
	if(m_cT){ delete m_cT; m_cT = NULL; }
	
	if (m_MApi)
	{
		m_MApi->RegisterSpi(NULL);
		m_MApi->Release();
	}
	
	if(m_cQ){ delete m_cQ; m_cQ = NULL; }
}

void CXTraderApp::LoadConfig()
{
	CString szPath = GetSpecFilePath(_T("log\\"));
	
	if (!PathIsDirectory(szPath))
	{
		CreateDirectory(szPath,NULL);
	}

	m_szTitle = LoadString(IDS_TITLE);
	m_szNtpSvr = _T("cn.pool.ntp.org");
	ParseXml(GetSpecFilePath(BKRS_DIR));
}

//读出xml对应的期货公司名称
void CXTraderApp::ParseXml(CString strDir)
{
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strDir);
	
	xml_document doc;
	xml_node proot;
	xml_parse_result result;
	char szBrkNm[MAX_PATH];
	while (bWorking)	
	{	
		PBKRPARA pBkrp = new BKRPARA;
		bWorking = finder.FindNextFile();
		pBkrp->XmlPath = finder.GetFilePath();
		/////////////////////////////////////////////////////////////	
		result = doc.load_file((LPCTSTR)pBkrp->XmlPath);
		if (result.status == status_ok) 
		{
			proot = doc.child(ROOT).child("broker");
			if (!proot) return;
			
			strcpy(szBrkNm,proot.attribute("BrokerName").value());
			ansi2uni(CP_UTF8,szBrkNm,pBkrp->BkrName.GetBuffer(MAX_PATH));
			pBkrp->BkrName.ReleaseBuffer();	
			m_BkrParaVec.push_back(pBkrp);	
		}
	}
	finder.Close();
}

BOOL CXTraderApp::AddSvr2Ar(CStringArray& szArTs,CStringArray& szArMd,CString szGrpName,int iIndex)
{
	//CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	xml_document doc;
	xml_parse_result result = doc.load_file((LPCTSTR)m_BkrParaVec[iIndex]->XmlPath);
	
	if (result.status == status_ok)
	{
		///////////读出服务器//////////////////
		LPCSTR szSvrRt="//broker/Servers/Server",sNmae="Name",sTrading="Trading",sMData="MarketData";
		char strName[64];
		CString tName,tTrading,tMData;
		
		szArTs.RemoveAll();
		szArMd.RemoveAll();
		
		xpath_node_set sVrs = doc.select_nodes(szSvrRt);
		for (xpath_node_set::const_iterator it = sVrs.begin(); it !=  sVrs.end(); ++it)
		{
			xpath_node node = *it;
			strcpy(strName,node.node().child(sNmae).child_value());//
			ansi2uni(CP_UTF8,strName,tName.GetBuffer(MAX_PATH));
			
			if (tName.Compare(szGrpName)==0)
			{
				xml_node tool;
				
				for (tool = node.node().child(sTrading).first_child(); tool; tool = tool.next_sibling())
				{
					ansi2uni(CP_UTF8,(char*)tool.child_value(),tTrading.GetBuffer(MAX_PATH));
					tTrading.ReleaseBuffer();
					
					szArTs.Add(tTrading);
				}
				
				for (tool = node.node().child(sMData).first_child(); tool; tool = tool.next_sibling())
				{
					ansi2uni(CP_UTF8,(char*)tool.child_value(),tMData.GetBuffer(MAX_PATH));
					tMData.ReleaseBuffer();
					
					szArMd.Add(tMData);
				}
				
				break;
			}	
		}
		
	}
	else
		return FALSE;
	
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// The one and only CXTraderApp object

CXTraderApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CXTraderApp initialization

BOOL CXTraderApp::InitInstance()
{
	SetHighPriority();

	LoginDlg login;
	if (login.DoModal() != IDOK)
	{
		return FALSE;
	}

	CFrameWnd* pWnd = new CFrameWnd();			
	pWnd->Create(NULL, NULL);
	
	CXTraderDlg dlg(pWnd);
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	delete pWnd;

	if (nResponse == IDOK)
	{

	}
	else if (nResponse == IDCANCEL)
	{

	}

	return FALSE;
}

int CXTraderApp::ExitInstance()
{
	xml_document doc;
	xml_node proot;
	xml_parse_result result;
	
	if (m_cT && (m_cT->m_isessionId != -1))
	{
		result = doc.load_file(SIDS,parse_full);
		if (result.status==status_ok)
		{
			proot = doc.child(SESRT);
			
			xml_node nodeSids = proot.append_child(SESITEM);
			
			SYSTEMTIME curTime;
			::GetLocalTime(&curTime);
			
			char szEndt[MAX_PATH];
			sprintf(szEndt,"%02d:%02d:%02d.%03d",curTime.wHour,curTime.wMinute,curTime.wSecond,curTime.wMilliseconds);
			
			nodeSids.append_attribute("sid") = m_cT->m_isessionId;
			nodeSids.append_attribute("bkr") = m_sBROKER_ID;
			nodeSids.append_attribute("uid") = m_sINVESTOR_ID;
			nodeSids.append_attribute("begin") = m_cT->m_sTmBegin;
			nodeSids.append_attribute("end") = szEndt;
			doc.save_file(SIDS,PUGIXML_TEXT("\t"),format_default,encoding_utf8);
		}
	}
	

	ReleaseApi();

	return CWinApp::ExitInstance();
} 
