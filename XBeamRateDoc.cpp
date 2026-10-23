///////////////////////////////////////////////////////////////////////
// XBeamRate - Cross Beam Load Rating
// Copyright © 1999-2026  Washington State Department of Transportation
//                        Bridge and Structures Office
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the Alternate Route Open Source License as 
// published by the Washington State Department of Transportation, 
// Bridge and Structures Office.
//
// This program is distributed in the hope that it will be useful, but 
// distribution is AS IS, WITHOUT ANY WARRANTY; without even the implied 
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See 
// the Alternate Route Open Source License for more details.
//
// You should have received a copy of the Alternate Route Open Source 
// License along with this program; if not, write to the Washington 
// State Department of Transportation, Bridge and Structures Office, 
// P.O. Box  47340, Olympia, WA 98503, USA or e-mail 
// Bridge_Support@wsdot.wa.gov
///////////////////////////////////////////////////////////////////////

// XBeamRateDoc.cpp : implementation of the CXBeamRateDoc class
//

#include "stdafx.h"
#include <AgentTools.h>
#include "resource.h"
#include "XBeamRateApp.h"
#include "XBeamRatePluginApp.h"
#include "XBeamRateDoc.h"
#include "XBeamRateDocProxyAgent.h"

#include "XBeamRateStatusBar.h"

#include "AboutDlg.h"

#include <BridgeLink.h>

#include <XBeamRateCatCom.h>

#include <EAF\EAFMainFrame.h>
#include <EAF\EAFUnits.h>

#include <PgsExt\StatusItem.h>

#include "XBeamRateHints.h"

#include "XBRate.hh"


/////////////////////////////////////////////////////////////////////////////
// CXBeamRateDoc

IMPLEMENT_DYNCREATE(CXBeamRateDoc, CEAFBrokerDocument)

BEGIN_MESSAGE_MAP(CXBeamRateDoc, CEAFBrokerDocument)
	//{{AFX_MSG_MAP(CXBeamRateDoc)
   ON_COMMAND(ID_HELP_ABOUT, OnAbout)
   ON_UPDATE_COMMAND_UI(ID_VIEW_GRAPHS,OnUpdateViewGraphs)
   ON_UPDATE_COMMAND_UI(ID_VIEW_REPORTS,OnUpdateViewReports)
   ON_COMMAND(ID_VIEW_PIER, OnViewPier)
   ON_COMMAND(EAFID_TOGGLE_AUTOCALC,OnAutoCalc)
   ON_UPDATE_COMMAND_UI(EAFID_TOGGLE_AUTOCALC,OnUpdateAutoCalc)
   ON_COMMAND(EAFID_AUTOCALC_UPDATENOW, OnUpdateNow)
	ON_UPDATE_COMMAND_UI(EAFID_AUTOCALC_UPDATENOW, OnUpdateUpdateNow)
   ON_COMMAND(ID_HELP_FINDER,OnHelpFinder)
	//}}AFX_MSG_MAP
   ON_UPDATE_COMMAND_UI_RANGE(EAFID_VIEW_STATUSCENTER, EAFID_VIEW_STATUSCENTER3, CEAFBrokerDocument::OnUpdateViewStatusCenter)
   ON_COMMAND_RANGE(EAFID_VIEW_STATUSCENTER, EAFID_VIEW_STATUSCENTER3, OnViewStatusCenter)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateDoc construction/destruction

#define XBR_PLUGIN_COMMAND_COUNT 256

CXBeamRateDoc::CXBeamRateDoc() :
   CEAFBrokerDocument(true,true)
{
	// TODO: add one-time construction code here
   UINT nCommands = GetPluginCommandManager()->ReserveCommandIDRange(XBR_PLUGIN_COMMAND_COUNT);
   ATLASSERT(nCommands == XBR_PLUGIN_COMMAND_COUNT);

   m_pMyDocProxyAgent = nullptr;
   m_bAutoCalcEnabled = true;

   SetCustomReportHelpID(WBFL::EAF::CustomReportHelp::CustomReport,IDH_CUSTOM_REPORT);
   SetCustomReportHelpID(WBFL::EAF::CustomReportHelp::FavoriteReport,IDH_FAVORITE_REPORT);
   SetCustomReportDefinitionHelpID(IDH_CUSTOM_REPORT_DEFINITION);

   CEAFAutoCalcDocMixin::SetDocument(this);
}

CXBeamRateDoc::~CXBeamRateDoc()
{
}

void CXBeamRateDoc::OnHelpFinder()
{
   EAFHelp(EAFGetDocument()->GetDocumentationSetName(),IDH_XBRATE);
}

void CXBeamRateDoc::OnViewStatusCenter(UINT nID)
{
   CEAFBrokerDocument::OnViewStatusCenter();
}

// CEAFAutoCalcDocMixin overrides
bool CXBeamRateDoc::IsAutoCalcEnabled() const
{
   return m_bAutoCalcEnabled;
}

void CXBeamRateDoc::EnableAutoCalc(bool bEnable)
{
   if ( m_bAutoCalcEnabled != bEnable )
   {
      bool bWasDisabled = !IsAutoCalcEnabled();
      m_bAutoCalcEnabled = bEnable;

      CXBeamRateStatusBar* pStatusBar = ((CXBeamRateStatusBar*)EAFGetMainFrame()->GetStatusBar());
      pStatusBar->AutoCalcEnabled( m_bAutoCalcEnabled );

      // If AutoCalc was off and now it is on,
      // Update the views.
      if ( bWasDisabled && IsAutoCalcEnabled() )
      {
        OnUpdateNow();
      }
   }
}

void CXBeamRateDoc::GetDocUnitSystem(IDocUnitSystem** ppDocUnitSystem)
{
   (*ppDocUnitSystem) = m_DocUnitSystem;
   (*ppDocUnitSystem)->AddRef();
}

BOOL CXBeamRateDoc::Init()
{
   CEAFDocTemplate* pTemplate = (CEAFDocTemplate*)GetDocTemplate();
   auto pluginApp = pTemplate->GetPluginApp();
   auto pXBeamRate = std::dynamic_pointer_cast<CXBeamRatePluginApp>(pluginApp);

   if ( !CEAFBrokerDocument::Init() )
   {
      return FALSE;
   }

   // Set up the document unit system
   CComPtr<IAppUnitSystem> appUnitSystem;
   pXBeamRate->GetAppUnitSystem(&appUnitSystem);
   EAFCreateDocUnitSystem(appUnitSystem,&m_DocUnitSystem);

   return TRUE;
}

std::pair<bool, WBFL::EAF::AgentErrors> CXBeamRateDoc::LoadSpecialAgents()
{
   auto errors = CEAFBrokerDocument::LoadSpecialAgents();

   m_pMyDocProxyAgent = std::make_shared<CXBeamRateDocProxyAgent>(this);
   auto result = m_pBroker->AddAgent(m_pMyDocProxyAgent);
   if (result.first == false)
   {
      AFX_MANAGE_STATE(AfxGetStaticModuleState());
      result.second.component.dll = AfxGetApp()->m_pszExeName;
      result.second.reason += _T(" - could not add XBeamRateDocProxyAgent to broker");
      errors.second.push_back(result.second);
   }

   errors.first = errors.second.empty();

   return errors;
}

void CXBeamRateDoc::OnChangedFavoriteReports(BOOL bIsFavorites,BOOL bFromMenu)
{
   __super::OnChangedFavoriteReports(bIsFavorites,bFromMenu);
   PopulateReportMenu();
}

void CXBeamRateDoc::ShowCustomReportHelp(WBFL::EAF::CustomReportHelp helpType)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   __super::ShowCustomReportHelp(helpType);
}

void CXBeamRateDoc::ShowCustomReportDefinitionHelp()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   __super::ShowCustomReportDefinitionHelp();
}

CATID CXBeamRateDoc::GetAgentCategoryID()
{
   return CATID_XBeamRateAgent;
}

HINSTANCE CXBeamRateDoc::GetResourceInstance()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   return AfxGetInstanceHandle();
}

HRESULT CXBeamRateDoc::LoadTheDocument(IStructuredLoad* pStrLoad)
{
   Float64 version;
   HRESULT hr = pStrLoad->get_Version(&version);
   if ( FAILED(hr) )
   {
      return hr;
   }

   CComVariant var;
   var.vt = VT_BSTR;
   hr = pStrLoad->get_Property(_T("Version"),&var);
   if ( FAILED(hr) )
   {
      return hr;
   }

#if defined _DEBUG
   TRACE(_T("Loading data saved with XBeamRate Version %s\n"),CComBSTR(var.bstrVal));
#endif

   hr = CEAFBrokerDocument::LoadTheDocument(pStrLoad);
   if ( FAILED(hr) )
   {
      return hr;
   }

   return S_OK;
}

HRESULT CXBeamRateDoc::WriteTheDocument(IStructuredSave* pStrSave)
{
   // before the standard broker document persistence, write out the version
   // number of the application that created this document
   CXBeamRateApp* pApp = (CXBeamRateApp*)AfxGetApp();
   HRESULT hr = pStrSave->put_Property(_T("Version"),CComVariant(pApp->GetVersion(true)));
   if ( FAILED(hr) )
   {
      return hr;
   }

   hr = CEAFBrokerDocument::WriteTheDocument(pStrSave);
   if ( FAILED(hr) )
   {
      return hr;
   }


   return S_OK;
}

void CXBeamRateDoc::CreateReportView(IndexType rptIdx,BOOL bPrompt)
{
   m_pMyDocProxyAgent->CreateReportView(rptIdx,bPrompt);
}

void CXBeamRateDoc::CreateGraphView(IndexType graphIdx)
{
   m_pMyDocProxyAgent->CreateGraphView(graphIdx);
}

void CXBeamRateDoc::LoadDocumentSettings()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CEAFBrokerDocument::LoadDocumentSettings();

   CWinApp* pApp = AfxGetApp();

   CString strAutoCalc = pApp->GetProfileString(_T("Settings"),_T("AutoCalc"),_T("On"));
   if ( strAutoCalc.CompareNoCase(_T("Off")) == 0 )
   {
      m_bAutoCalcEnabled = false;
   }
   else
   {
      m_bAutoCalcEnabled = true;
   }
}

void CXBeamRateDoc::SaveDocumentSettings()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CEAFBrokerDocument::SaveDocumentSettings();

   CWinApp* pApp = AfxGetApp();

   VERIFY(pApp->WriteProfileString( _T("Settings"),_T("AutoCalc"),m_bAutoCalcEnabled ? _T("On") : _T("Off") ));
}

CString CXBeamRateDoc::GetDocumentationSetName()
{
   return CString(_T("XBRate"));
}

CString CXBeamRateDoc::GetDocumentationRootLocation()
{
   CEAFApp* pApp = EAFGetApp();
   return pApp->GetDocumentationRootLocation();
}

void CXBeamRateDoc::OnUIHintsReset()
{
   __super::OnUIHintsReset();
   m_pMyDocProxyAgent->OnUIHintsReset();
}

CString CXBeamRateDoc::GetRootNodeName()
{
   return _T("XBeamRate");
}

Float64 CXBeamRateDoc::GetRootNodeVersion()
{
   return 1.0;
}

BOOL CXBeamRateDoc::OnNewDocument()
{
	if (!CEAFBrokerDocument::OnNewDocument())
		return FALSE;

   CBridgeLinkApp* pApp = (CBridgeLinkApp*)EAFGetApp();
   IBridgeLink* pBL = (IBridgeLink*)pApp;
   CString engineer_name, company;
   pBL->GetUserInfo(&engineer_name,&company);

   GET_IFACE( IXBRProjectProperties, pProjectProperties );

   pProjectProperties->SetEngineer(engineer_name);
   pProjectProperties->SetCompany(company);

   if ( pProjectProperties->ShowProjectPropertiesOnNewProject() )
   {
      pProjectProperties->PromptForProjectProperties();
   }

	return TRUE;
}

void CXBeamRateDoc::OnCloseDocument()
{
   // Put report favorites options back into CXBeamRatePluginApp
   CEAFDocTemplate* pTemplate = (CEAFDocTemplate*)GetDocTemplate();
   auto pluginApp = pTemplate->GetPluginApp();
   auto pXBeamRate = std::dynamic_pointer_cast<CXBeamRatePluginApp>(pluginApp);

   BOOL bDisplayFavorites = DisplayFavoriteReports();
   std::vector<std::_tstring> vFavorites = GetFavoriteReports();

   pXBeamRate->DisplayFavoriteReports(bDisplayFavorites);
   pXBeamRate->SetFavoriteReports(vFavorites);

   // user-defined custom reports
   CEAFCustomReports reports = GetCustomReports();
   pXBeamRate->SetCustomReports(reports);

   CEAFBrokerDocument::OnCloseDocument();
}

BOOL CXBeamRateDoc::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
   // document classes can't process ON_NOTIFY
   // see http://www.codeproject.com/KB/docview/NotifierApp.aspx for details
   if ( HIWORD(nCode) == WM_NOTIFY )
   {
    // verify that this is a WM_NOTIFY message
     WORD wCode = LOWORD(nCode) ;

     AFX_NOTIFY * notify = reinterpret_cast<AFX_NOTIFY*>(pExtra) ;
     if ( notify->pNMHDR->code == TBN_DROPDOWN )
     {
        if ( notify->pNMHDR->idFrom == m_pMyDocProxyAgent->GetStdToolBarID() && ((NMTOOLBAR*)(notify->pNMHDR))->iItem == ID_VIEW_REPORTS )
        {
           return OnViewReports(notify->pNMHDR,notify->pResult); 
        }

        if ( notify->pNMHDR->idFrom == m_pMyDocProxyAgent->GetStdToolBarID() && ((NMTOOLBAR*)(notify->pNMHDR))->iItem == ID_VIEW_GRAPHS )
        {
           return OnViewGraphs(notify->pNMHDR,notify->pResult); 
        }
     }
   }

   return CEAFBrokerDocument::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void CXBeamRateDoc::LoadToolbarState()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CEAFBrokerDocument::LoadToolbarState();
}

void CXBeamRateDoc::SaveToolbarState()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CEAFBrokerDocument::SaveToolbarState();
}

CString CXBeamRateDoc::GetToolbarSectionName()
{
   return _T("XBeamRate");
}

BOOL CXBeamRateDoc::GetStatusBarMessageString(UINT nID,CString& rMessage) const
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   return CEAFBrokerDocument::GetStatusBarMessageString(nID,rMessage);
}

BOOL CXBeamRateDoc::GetToolTipMessageString(UINT nID, CString& rMessage) const
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   return CEAFBrokerDocument::GetToolTipMessageString(nID,rMessage);
}

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateDoc diagnostics

#ifdef _DEBUG
void CXBeamRateDoc::AssertValid() const
{
	CEAFBrokerDocument::AssertValid();
}

void CXBeamRateDoc::Dump(CDumpContext& dc) const
{
	CEAFBrokerDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateDoc commands
BOOL CXBeamRateDoc::OpenTheDocument(LPCTSTR lpszPathName)
{
   // don't fire UI events as the UI isn't completely built when the document is created
   // (view classes haven't been initialized)
   m_pMyDocProxyAgent->HoldEvents();
   // Events are released in OnCreateFinalize()

   if ( !CEAFBrokerDocument::OpenTheDocument(lpszPathName) )
   {
      return FALSE;
   }

   GET_IFACE(IEAFDisplayUnits,pDisplayUnits);
   m_DocUnitSystem->put_UnitMode( IS_US_UNITS(pDisplayUnits) ? umUS : umSI );
  
   return TRUE;
}

void CXBeamRateDoc::OnCreateFinalize()
{
   CEAFBrokerDocument::OnCreateFinalize();

   GET_IFACE(IEAFStatusCenter,pStatusCenter);
   m_scidInformationalError  = pStatusCenter->RegisterCallback(std::make_shared<pgsInformationalStatusCallback>(WBFL::EAF::StatusSeverityType::Warning)); 
   m_StatusGroupID = pStatusCenter->CreateStatusGroupID();

   // Transfer report favorites and custom reports data from CXBeamRatePluginApp to CEAFBrokerDocument (this)
   CEAFDocTemplate* pTemplate = (CEAFDocTemplate*)GetDocTemplate();
   auto pluginApp = pTemplate->GetPluginApp();
   auto pXBeamRate = std::dynamic_pointer_cast<CXBeamRatePluginApp>(pluginApp);

   BOOL bDisplayFavorites = pXBeamRate->DisplayFavoriteReports();
   std::vector<std::_tstring> vFavorites = pXBeamRate->GetFavoriteReports();

   DisplayFavoriteReports(bDisplayFavorites);
   SetFavoriteReports(vFavorites);

   CEAFCustomReports customs = pXBeamRate->GetCustomReports();
   SetCustomReports(customs);

   IntegrateCustomReports();

   PopulateReportMenu();
   PopulateGraphMenu();

   // Set the AutoCalc state on the status bar
   CXBeamRateStatusBar* pStatusBar = ((CXBeamRateStatusBar*)EAFGetMainFrame()->GetStatusBar());
   pStatusBar->AutoCalcEnabled( IsAutoCalcEnabled() );

   // views have been initialized so fire any pending events
   GET_IFACE(IXBREvents,pEvents);
   GET_IFACE(IXBRUIEvents,pUIEvents);
   pEvents->FirePendingEvents(); 
   pUIEvents->HoldEvents(false);
}

void CXBeamRateDoc::BrokerShutDown()
{
   m_pMyDocProxyAgent = nullptr;
   CEAFBrokerDocument::BrokerShutDown();
}

void CXBeamRateDoc::OnStatusChanged()
{
   CEAFBrokerDocument::OnStatusChanged();
   if (m_pMyDocProxyAgent)
   {
      m_pMyDocProxyAgent->OnStatusChanged();
   }
}

void CXBeamRateDoc::PopulateReportMenu()
{
   auto pMainMenu = GetMainMenu();

   UINT viewPos = pMainMenu->FindMenuItem(_T("&View"));
   ASSERT( 0 <= viewPos );

   auto pViewMenu = pMainMenu->GetSubMenu(viewPos);
   ASSERT( pViewMenu != nullptr );

   UINT reportsPos = pViewMenu->FindMenuItem(_T("&Reports"));
   ASSERT( 0 <= reportsPos );

   // Get the reports menu
   auto pReportsMenu = pViewMenu->GetSubMenu(reportsPos);
   ASSERT(pReportsMenu != nullptr);

   CEAFBrokerDocument::PopulateReportMenu(pReportsMenu);
}

void CXBeamRateDoc::PopulateGraphMenu()
{
   auto pMainMenu = GetMainMenu();

   UINT viewPos = pMainMenu->FindMenuItem(_T("&View"));
   ASSERT( 0 <= viewPos );

   auto pViewMenu = pMainMenu->GetSubMenu(viewPos);
   ASSERT( pViewMenu != nullptr );

   UINT graphsPos = pViewMenu->FindMenuItem(_T("&Graphs"));
   ASSERT( 0 <= graphsPos );

   // Get the graphs menu
   auto pGraphMenu = pViewMenu->GetSubMenu(graphsPos);
   ASSERT(pGraphMenu != nullptr);

   CEAFBrokerDocument::PopulateGraphMenu(pGraphMenu);
}

void CXBeamRateDoc::OnAbout()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CEAFDocTemplate* pTemplate = (CEAFDocTemplate*)GetDocTemplate();
   UINT resourceID = pTemplate->GetResourceID();

   CAboutDlg dlg(resourceID);
   dlg.DoModal();
}

void CXBeamRateDoc::OnUpdateViewGraphs(CCmdUI* pCmdUI)
{
   GET_IFACE(IEAFGraphManager,pGraphMgr);
   pCmdUI->Enable( 0 < pGraphMgr->GetGraphBuilderCount() );
}

BOOL CXBeamRateDoc::OnViewGraphs(NMHDR* pnmhdr,LRESULT* plr) 
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   // This method gets called when the down arrow toolbar button is used
   // It creates the drop down menu with the report names on it
   NMTOOLBAR* pnmtb = (NMTOOLBAR*)(pnmhdr);
   if ( pnmtb->iItem != ID_VIEW_GRAPHS )
   {
      return FALSE; // not our button
   }

   CMenu menu;
   VERIFY( menu.LoadMenu(IDR_GRAPHS) );
   CMenu* pMenu = menu.GetSubMenu(0);
   pMenu->RemoveMenu(0,MF_BYPOSITION); // remove the placeholder

   auto contextMenu = WBFL::EAF::Menu::CreateMenu(pMenu->Detach(),GetPluginCommandManager());


   BuildGraphMenu(contextMenu);

   GET_IFACE(IEAFToolbars,pToolBars);
   auto pToolBar = pToolBars->GetToolBar( m_pMyDocProxyAgent->GetStdToolBarID() );
   int idx = pToolBar->CommandToIndex(ID_VIEW_GRAPHS,nullptr);
   CRect rect;
   pToolBar->GetItemRect(idx,&rect);

   CPoint point(rect.left,rect.bottom);
   pToolBar->ClientToScreen(&point);
   contextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x,point.y, EAFGetMainFrame() );

   return TRUE;
}

void CXBeamRateDoc::OnUpdateViewReports(CCmdUI* pCmdUI)
{
   GET_IFACE(IEAFReportManager,pReportMgr);
   pCmdUI->Enable( 0 < pReportMgr->GetReportBuilderCount() );
}

BOOL CXBeamRateDoc::OnViewReports(NMHDR* pnmhdr,LRESULT* plr) 
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   // This method gets called when the down arrow toolbar button is used
   // It creates the drop down menu with the report names on it
   NMTOOLBAR* pnmtb = (NMTOOLBAR*)(pnmhdr);
   if ( pnmtb->iItem != ID_VIEW_REPORTS )
   {
      return FALSE; // not our button
   }

   CMenu menu;
   VERIFY( menu.LoadMenu(IDR_REPORTS) );
   CMenu* pMenu = menu.GetSubMenu(0);
   pMenu->RemoveMenu(0,MF_BYPOSITION); // remove the placeholder

   auto contextMenu = WBFL::EAF::Menu::CreateMenu(pMenu->Detach(),GetPluginCommandManager());

   CEAFBrokerDocument::PopulateReportMenu(contextMenu);

   GET_IFACE(IEAFToolbars,pToolBars);
   auto pToolBar = pToolBars->GetToolBar( m_pMyDocProxyAgent->GetStdToolBarID() );
   int idx = pToolBar->CommandToIndex(ID_VIEW_REPORTS,nullptr);
   CRect rect;
   pToolBar->GetItemRect(idx,&rect);

   CPoint point(rect.left,rect.bottom);
   pToolBar->ClientToScreen(&point);
   contextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x,point.y, EAFGetMainFrame() );

   return TRUE;
}

void CXBeamRateDoc::OnViewPier()
{
   m_pMyDocProxyAgent->CreatePierView();
}

void CXBeamRateDoc::OnAutoCalc()
{
   CEAFAutoCalcDocMixin::OnAutoCalc();   
}

void CXBeamRateDoc::OnUpdateAutoCalc(CCmdUI* pCmdUI)
{
   CEAFAutoCalcDocMixin::OnUpdateAutoCalc(pCmdUI);   
}

void CXBeamRateDoc::OnUpdateNow()
{
   CEAFAutoCalcDocMixin::OnUpdateNow();
}

void CXBeamRateDoc::OnUpdateUpdateNow(CCmdUI* pCmdUI)
{
   CEAFAutoCalcDocMixin::OnUpdateUpdateNow(pCmdUI);
}
