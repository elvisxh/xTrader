
#pragma once

#include "stdafx.h"
#include "mdspi.h"
#include "xTraderDlg.h"
#include "xTrader.h"

#pragma warning(disable : 4996)

//extern int requestId;  
extern HANDLE g_hEvent;
extern CWnd* g_pCWnd;
extern BOOL bRecconnect;

BOOL bMdSignal = FALSE;
void CtpMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast)
{
  IsErrorRspInfo(pRspInfo);
}

void CtpMdSpi::OnFrontDisconnected(int nReason)
{
	if (g_pCWnd)
	{
		bMdSignal = FALSE;
		CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;
		//pDlg->SetStatusTxt(_T("MD×"),0);
		pDlg->SetTipTxt( _T("行情断开"),IDS_MD_TIPS);
		pDlg->SetPaneTxtColor(0,BLUE);
	}

  //cerr<<" 响应 | 连接中断..." 
   // << " reason=" << nReason << endl;
}
		
void CtpMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
  cerr<<" 响应 | 心跳超时警告..." 
    << " TimerLapse = " << nTimeLapse << endl;
}

void CtpMdSpi::OnFrontConnected()
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	if (g_pCWnd)
	{
		bRecconnect = TRUE;
		CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;

		//pDlg->SetStatusTxt(_T("MD√"),0);


		ReqUserLogin(pApp->m_sBROKER_ID);//,pApp->m_sINVESTOR_ID,pApp->m_sPASSWORD);
		DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{
			pDlg->SetTipTxt(_T("行情在线"),IDS_MD_TIPS);
			pDlg->SetPaneTxtColor(0,RED);
			ResetEvent(g_hEvent);
		}
		else
		{
			return;
		}

		//int iLen = pDlg->m_szInst.GetLength();
		char szInst[MAX_PATH];
		uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)pDlg->m_szInst,szInst);

		SubscribeMarketData(szInst);
		dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{
			ResetEvent(g_hEvent);
		}
			//Sleep(100);
			bMdSignal = TRUE;
	}
	
	else
	{
		ReqUserLogin(pApp->m_sBROKER_ID);
	}


  //if (g_bOnce)SetEvent(g_hEvent);
}

void CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType	appId)
	     //TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(m_sBkrID,appId);
	//strcpy(req.UserID, userId);
	//strcpy(req.Password, passwd);

	pUserApi->ReqUserLogin(&req, ++m_iRequestID);
  //cerr<<" 请求 | 发送登录..."<<((ret == 0) ? "成功" :"失败") << endl;	
  //SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{
		//m_pDlg->ProgressUpdate(_T("登陆行情成功!"), 60);
    //cerr<<" 响应 | 登录成功...当前交易日:"<<pRspUserLogin->TradingDay<<endl;
	}
  if(bIsLast) SetEvent(g_hEvent);
}


void CtpMdSpi::ReqUserLogout()
{
	CThostFtdcUserLogoutField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_sBkrID);
	
	pUserApi->ReqUserLogout(&req, ++m_iRequestID);
}

///登出请求响应
void CtpMdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pUserLogout)
	{
		
	}
	if(bIsLast) SetEvent(g_hEvent);
}

//TThostFtdcInstrumentIDType instId

void CtpMdSpi::SubscribeMarketData(char* instIdList)
{
  vector<char*> list;

  char* token = strsep(&instIdList, ",");
  while( token != NULL ){
    list.push_back(token); 
    token = strsep(&instIdList, ",");
  }
  
  UINT len = list.size();
  char** pInstId = new char*[len];
  for(UINT i=0; i<len;i++)  {pInstId[i]=list[i];}
  //pInstId[len] = NULL;

  pUserApi->SubscribeMarketData(pInstId, len); 
  //SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspSubMarketData(
         CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument)
	{
		bMdSignal = TRUE;
	
	}
	else
	{
		bMdSignal = FALSE;
	}
	//cerr<<" 响应 |  行情订阅...成功"<<endl;
  if(bIsLast)  SetEvent(g_hEvent);
}
void CtpMdSpi::UnSubscribeMarketData(char* instIdList)
{
  vector<char*> list;

  char* token = strsep(&instIdList, ",");
  while( token != NULL ){
    list.push_back(token); 
    token = strsep(&instIdList, ",");
  }
	UINT len = list.size();
	char** pInstId = new char*[len];
	for(UINT i=0; i<len;i++)  {pInstId[i]=list[i]; }
	//pInstId[len] = NULL;
	pUserApi->UnSubscribeMarketData(pInstId, len);
	//SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspUnSubMarketData(
             CThostFtdcSpecificInstrumentField *pSpecificInstrument,
             CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument)
	{
		bMdSignal = FALSE;
	}
	else
	{
		bMdSignal = TRUE;
	}
  if(bIsLast)  SetEvent(g_hEvent);
}

void CtpMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	//__int64 dwMs1,dwMs2;
	//dwMs1 = GetCycleCount();
	if (g_pCWnd!=NULL)
	{
		CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;

		//CThostFtdcDepthMarketDataField *pDepthMd = new CThostFtdcDepthMarketDataField;
		memcpy(pDlg->m_pDepthMd,pDepthMarketData,sizeof(CThostFtdcDepthMarketDataField));

		pDlg->RefreshMdPane();
		//PostMessage(pDlg->m_hWnd,WM_UPDATEMD_MSG,0,(LPARAM)pDepthMd);
		//SendNotifyMessage(pDlg->m_hWnd,WM_UPDATEMD_MSG,0,(LPARAM)pDepthMarketData);
		
	}
	
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{	
  bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
  if (ret){
    cerr<<" 响应 | "<<pRspInfo->ErrorMsg<<endl;
  }
  return ret;
}