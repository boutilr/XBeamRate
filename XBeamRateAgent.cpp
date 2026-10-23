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

#include "stdafx.h"
#include <AgentTools.h>
#include "resource.h"
#include "XBeamRateAgent.h"
#include <XBeamRateCatCom.h>

#include <IFace\Project.h>
#include <IFace\RatingSpecification.h>
#include <PsgLib\BridgeDescription2.h>

#include <IFace\DocumentType.h>
#include <IFace\Bridge.h>
#include <IFace\Selection.h>
#include <IFace\EditByUI.h>

#include <EAF\EAFBrokerDocument.h>

#include <MFCTools\VersionInfo.h>

#include <XBeamRateExt\ReinforcementPage.h>

#include "XBeamRateChildFrame.h"
#include "XBeamRateView.h"

#include "txnEditReinforcement.h"
#include "txnEditLoadRatingOptions.h"

#include "LoadRatingOptionsPage.h"

#include "XBeamRateVersionInfoImpl.h"
#include "XBeamRateCommandLineProcessor.h"


BEGIN_MESSAGE_MAP(CXBeamRateAgent, CCmdTarget)
	ON_COMMAND(ID_VIEW_PIER, &CXBeamRateAgent::OnViewPier)
   //ON_UPDATE_COMMAND_UI(ID_VIEW_PIER,OnPierCommandUpdate)
   ON_COMMAND(IDM_EXPORT_PIER, &CXBeamRateAgent::OnExportPier)
   ON_UPDATE_COMMAND_UI(IDM_EXPORT_PIER, &CXBeamRateAgent::OnPierCommandUpdate)
END_MESSAGE_MAP()

// CXBeamRateAgent


/////////////////////////////////////////////////////////////////////////
// Agent
bool CXBeamRateAgent::SetBroker(std::shared_ptr<WBFL::EAF::Broker> broker)
{
   if (!__super::SetBroker(broker))
      return false;

   // The XBeamRate agent is created when it is a plug-in to PGSuper/PGSplice.
   // In this use case, agents are being loaded based on the CATID for PGSuper/PGSplice.
   // The other XBeamRate agents don't get loaded because they are not PGSuper/PGSplice agents.
   // However, they must be loaded for XBeamRate to work properly.
   //
   // When the broker is being set, load the XBeamRateAgents manually.
   // Agents must be loaded before the Broker calls Agent::Init(), otherwise
   // the agents will not be initialized.
   if (m_pBroker)
   {
      auto errors = m_pBroker->LoadExtensionAgents(CATID_XBeamRateAgent);
      if (errors.first == false)
      {
         for (auto& error : errors.second)
         {
            ((CEAFBrokerDocument*)EAFGetDocument())->LogAgentError(error);
         }
         return false;
      }
   }

   return true;
}

bool CXBeamRateAgent::RegisterInterfaces()
{
   EAF_AGENT_REGISTER_INTERFACES;

   REGISTER_INTERFACE(IXBeamRateAgent);
   REGISTER_INTERFACE(IXBRVersionInfo);

   return true;
}

bool CXBeamRateAgent::Init()
{
   EAF_AGENT_INIT;
   //CREATE_LOGFILE(_T("XBeamRateAgent"));


   //AFX_MANAGE_STATE(AfxGetStaticModuleState());
   //VERIFY(m_bmpMenu.LoadBitmap(IDB_LOGO));

   //
   // Attach to connection points
   //
   m_dwProjectPropertiesCookie = REGISTER_EVENT_SINK(IProjectPropertiesEventSink);
   m_dwProjectCookie = REGISTER_EVENT_SINK(IXBRProjectEventSink);

   return true;
}

bool CXBeamRateAgent::Reset()
{
   EAF_AGENT_RESET;
   //m_bmpMenu.DeleteObject();
   return true;;
}

bool CXBeamRateAgent::ShutDown()
{
   EAF_AGENT_SHUTDOWN;
   //
   // Detach to connection points
   //
   UNREGISTER_EVENT_SINK(IProjectPropertiesEventSink, m_dwProjectPropertiesCookie);
   UNREGISTER_EVENT_SINK(IXBRProjectEventSink, m_dwProjectCookie);

   return true;
}

CLSID CXBeamRateAgent::GetCLSID() const
{
   return CLSID_XBeamRateAgent;
}


void CXBeamRateAgent::RegisterViews()
{
   GET_IFACE(IEAFViewRegistrar,pViewRegistrar);
   auto callback = std::dynamic_pointer_cast<WBFL::EAF::ICommandCallback>(shared_from_this());
   m_PierViewKey = pViewRegistrar->RegisterView(IDR_XBEAMRATE, callback, RUNTIME_CLASS(CXBeamRateChildFrame),RUNTIME_CLASS(CXBeamRateView));
}

void CXBeamRateAgent::UnregisterViews()
{
   GET_IFACE(IEAFViewRegistrar,pViewRegistrar);
   pViewRegistrar->RemoveView(m_PierViewKey);
}

void CXBeamRateAgent::CreatePierView()
{
   GET_IFACE(IEAFViewRegistrar,pViewRegistrar);
   pViewRegistrar->CreateView(m_PierViewKey,nullptr);
}

void CXBeamRateAgent::CreateMenus()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   GET_IFACE(IEAFMainMenu,pMainMenu);
   auto pMenu = pMainMenu->GetMainMenu();

   UINT viewPos = pMenu->FindMenuItem(_T("View"));
   auto pViewMenu = pMenu->GetSubMenu(viewPos);
   UINT graphsPos = pViewMenu->FindMenuItem(_T("Graphs"));
   auto callback = std::dynamic_pointer_cast<WBFL::EAF::ICommandCallback>(shared_from_this());
   pViewMenu->InsertMenu(graphsPos,ID_VIEW_PIER,_T("&Pier View"), callback);
}

void CXBeamRateAgent::RemoveMenus()
{
   GET_IFACE(IEAFMainMenu,pMainMenu);
   auto pMenu = pMainMenu->GetMainMenu();
   UINT viewPos = pMenu->FindMenuItem(_T("View"));
   auto pViewMenu = pMenu->GetSubMenu(viewPos);
   auto callback = std::dynamic_pointer_cast<WBFL::EAF::ICommandCallback>(shared_from_this());
   pViewMenu->RemoveMenu(ID_VIEW_PIER,MF_BYCOMMAND, callback);
}

void CXBeamRateAgent::CreateToolbar()
{
   // add a button to the standard PGSuper/PGSplice toolbar to view a pier
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   GET_IFACE(IEAFToolbars,pToolBars);
   GET_IFACE(IEditByUI,pEditUI);
   UINT stdID = pEditUI->GetStdToolBarID();
   auto pStdToolBar = pToolBars->GetToolBar(stdID);

   // Put the button to the right of PGSuper/PGSplice view girder button
   // The IDs from PGSuper/PGSplice aren't available to plugins so we'll just
   // hard code the command ID.
   int idx = pStdToolBar->CommandToIndex(36896/*ID_VIEW_GIRDEREDITOR*/,nullptr); 
   auto callback = std::dynamic_pointer_cast<WBFL::EAF::ICommandCallback>(shared_from_this());
   pStdToolBar->InsertButton(idx+1,ID_VIEW_PIER,IDB_VIEW_PIER,nullptr, callback);
}

void CXBeamRateAgent::RemoveToolbar()
{
   // Remove all the buttons we added to the standard toolbar
   GET_IFACE(IEAFToolbars,pToolBars);
   GET_IFACE(IEditByUI,pEditUI);
   UINT stdID = pEditUI->GetStdToolBarID();
   auto pStdToolBar = pToolBars->GetToolBar(stdID);
   auto callback = std::dynamic_pointer_cast<WBFL::EAF::ICommandCallback>(shared_from_this());
   pStdToolBar->RemoveButtons(callback);
}

//void CXBeamRateAgent::RegisterReports()
//{
//   // Register our reports
//   GET_IFACE(IEAFReportManager,pRptMgr);
//
//   //
//   // Create report spec builders
//   //
//
//   std::shared_ptr<WBFL::ReportMgr::ReportSpecificationBuilder> pMyRptSpecBuilder( std::make_shared<CMyReportSpecificationBuilder>(m_pBroker) );
//
//   // My report
//   std::unique_ptr<WBFL::ReportMgr::ReportBuilder> pRptBuilder(std::make_unique<WBFL::ReportMgr::ReportBuilder>(_T("Extension Agent Report"));
//   pRptBuilder->SetMenuBitmap(&m_bmpMenu);
//   pRptBuilder->SetTitlePageBuilder( std::make_shared<CPGSuperTitlePageBuilder>(m_pBroker,pRptBuilder->GetName(),false)) );
//   pRptBuilder->SetReportSpecificationBuilder( pMyRptSpecBuilder );
//   pRptBuilder->AddChapterBuilder( std::make_shared<CMyChapterBuilder>) );
//   pRptMgr->AddReportBuilder( pRptBuilder.release() );
//}

void CXBeamRateAgent::RegisterUIExtensions()
{
   // Tell PGSuper/PGSplice that we want to add stuff to its dialogs
   GET_IFACE(IExtendPGSuperUI,pExtendUI);
   m_EditPierCallbackID = pExtendUI->RegisterEditPierCallback(this,nullptr);
   m_EditLoadRatingOptionsCallbackID = pExtendUI->RegisterEditLoadRatingOptionsCallback(this);

   // Add a command callback to the bridge view
   GET_IFACE(IRegisterViewEvents,pBridgeViewEvents);
   m_BridgePlanViewCallbackID = pBridgeViewEvents->RegisterBridgePlanViewCallback(this);
}

void CXBeamRateAgent::UnregisterUIExtensions()
{
   // We are done adding stuff to PGS's dialogs
   GET_IFACE(IExtendPGSuperUI,pExtendUI);
   pExtendUI->UnregisterEditPierCallback(m_EditPierCallbackID);
   pExtendUI->UnregisterEditLoadRatingOptionsCallback(m_EditLoadRatingOptionsCallbackID);

   // Done with handling commands from the bridge view
   GET_IFACE(IRegisterViewEvents,pBridgeViewEvents);
   pBridgeViewEvents->UnregisterBridgePlanViewCallback(m_BridgePlanViewCallbackID);
}

///////////////////////////////////////////////////////////////////////////////////
// IXBRVersionInfo
CString CXBeamRateAgent::GetVersionString(bool bIncludeBuildNumber)
{
   CXBeamRateVersionInfoImpl vi;
   return vi.GetVersionString(bIncludeBuildNumber);
}

CString CXBeamRateAgent::GetVersion(bool bIncludeBuildNumber)
{
   CXBeamRateVersionInfoImpl vi;
   return vi.GetVersion(bIncludeBuildNumber);
}

////////////////////////////////////////////////////////////////////
// IAgentPersist
//STDMETHODIMP CXBeamRateAgent::Load(IStructuredLoad* pStrLoad)
//{
//   USES_CONVERSION;
//   CComVariant var;
//   //var.vt = VT_BSTR;
//   
//   HRESULT hr = pStrLoad->BeginUnit(_T("XBeamRateAgent"));
//   if ( FAILED(hr) )
//      return hr;
//
//   //var.vt = VT_BSTR;
//   //hr = pStrLoad->get_Property(_T("SampleData"),&var);
//   //if ( FAILED(hr) )
//   //   return hr;
//
//   //m_Answer = OLE2T(var.bstrVal);
//
//   hr = pStrLoad->EndUnit();
//   if ( FAILED(hr) )
//      return hr;
//
//   return S_OK;
//}
//
//STDMETHODIMP CXBeamRateAgent::Save(IStructuredSave* pStrSave)
//{
//   pStrSave->BeginUnit(_T("XBeamRateAgent"),1.0);
//   //pStrSave->put_Property(_T("SampleData"),CComVariant(m_Answer));
//   pStrSave->EndUnit();
//   return S_OK;
//}

////////////////////////////////////////////////////////////////////
// IXBeamRateAgent
bool CXBeamRateAgent::IsExtendingPGSuper()
{
   return true;
}

////////////////////////////////////////////////////////////////////
// IAgentUIIntegration
bool CXBeamRateAgent::IntegrateWithUI(bool bIntegrate)
{
   if ( bIntegrate )
   {
      CreateMenus();
      CreateToolbar();
      RegisterViews();
      RegisterUIExtensions();
   }
   else
   {
      RemoveMenus();
      RemoveToolbar();
      UnregisterViews();
      UnregisterUIExtensions();
   }

   return S_OK;
}

////////////////////////////////////////////////////////////////////
// IAgentDocumentationIntegration
CString CXBeamRateAgent::GetDocumentationSetName() const
{
   return _T("XBRate");
}

CString CXBeamRateAgent::GetDocumentationURL() const
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   auto strDocSetName = GetDocumentationSetName();

   CEAFApp* pApp = EAFGetApp();
   CString strDocumentationRootLocation = pApp->GetDocumentationRootLocation();

   CString strDocumentationURL;
   strDocumentationURL.Format(_T("%s%s/"),strDocumentationRootLocation,strDocSetName);

   CVersionInfo verInfo;
   CString strAppName = AfxGetAppName(); // needs module state
   strAppName += _T(".dll");
   verInfo.Load(strAppName);

   CString strVersion = verInfo.GetProductVersionAsString(false);
   std::_tstring v(strVersion);
   auto count = std::count(std::begin(v), std::end(v), _T('.'));

   // remove the hot fix and build/release number
   for (auto i = 0; i < count - 1; i++)
   {
      int pos = strVersion.ReverseFind(_T('.')); // find the last '.'
      strVersion = strVersion.Left(pos);
   }

   CString strURL;
   strURL.Format(_T("%s%s/"),strDocumentationURL,strVersion);
   strDocumentationURL = strURL;

   return strDocumentationURL;
}

bool CXBeamRateAgent::LoadDocumentationMap()
{
   USES_CONVERSION;

   auto strDocSetName = GetDocumentationSetName();

   CEAFApp* pApp = EAFGetApp();

   CString strDocumentationURL = GetDocumentationURL();

   CString strDocMapFile = EAFGetDocumentationMapFile(strDocSetName, strDocumentationURL);

   VERIFY(EAFLoadDocumentationMap(strDocMapFile,m_HelpTopics));
   return S_OK;
}

std::pair<WBFL::EAF::HelpResult,CString> CXBeamRateAgent::GetDocumentLocation(UINT nHID) const
{
   auto found = m_HelpTopics.find(nHID);
   if ( found == m_HelpTopics.end() )
   {
      return { WBFL::EAF::HelpResult::TopicNotFound, CString()};
   }

   CString strURL;
   strURL.Format(_T("%s%s"),GetDocumentationURL(),found->second);
   return { WBFL::EAF::HelpResult::OK, strURL };
}

/////////////////////////////////////////////////////////////////////////////////
// IEditPierCallback
CPropertyPage* CXBeamRateAgent::CreatePropertyPage(IEditPierData* pEditPierData)
{
   // The PGSuper/PGSplice Edit Pier dialog is being displayed... here is our
   // chance to add a property page to the dialog
   const CPierData2* pPier = pEditPierData->GetPierData();
   GET_IFACE(IXBRProject,pProject);
   m_ReinforcementPageParent.SetEditPierData(pEditPierData,pProject->GetPierData(pPier->GetID()));
   return new CReinforcementPage(&m_ReinforcementPageParent);
}

CPropertyPage* CXBeamRateAgent::CreatePropertyPage(IEditPierData* pEditPierData,CPropertyPage* pBridgePropertyPage)
{
   ATLASSERT(false); // should never get here
   return nullptr;
}

std::unique_ptr<WBFL::EAF::Transaction> CXBeamRateAgent::OnOK(CPropertyPage* pPage,IEditPierData* pEditPierData)
{
   if ( pPage == nullptr )
   {
      // we aren't extending the dialog for this pier
      return nullptr;
   }

   xbrEditReinforcementData oldReinforcement;
   GET_IFACE(IXBRProject,pProject);
   const xbrPierData& pierData = pProject->GetPierData(pEditPierData->GetPierData()->GetID());
   pierData.GetRebarMaterial(&oldReinforcement.Type,&oldReinforcement.Grade);
   oldReinforcement.LongitudinalRebar  = pierData.GetLongitudinalRebar();
   oldReinforcement.LowerXBeamStirrups = pierData.GetLowerXBeamStirrups();
   oldReinforcement.FullDepthStirrups  = pierData.GetFullDepthStirrups();
   oldReinforcement.ConditionFactorType = pierData.GetConditionFactorType();
   oldReinforcement.ConditionFactor = pierData.GetConditionFactor();

   CReinforcementPage* pReinforcementPage = (CReinforcementPage*)pPage;
   xbrEditReinforcementData newReinforcement;

   IReinforcementPageParent* pParent  = pReinforcementPage->GetPageParent();
   newReinforcement.Type               = pParent->GetRebarType();
   newReinforcement.Grade              = pParent->GetRebarGrade();
   newReinforcement.LongitudinalRebar  = pParent->GetLongitudinalRebar();
   newReinforcement.LowerXBeamStirrups = pParent->GetLowerXBeamStirrups();
   newReinforcement.FullDepthStirrups  = pParent->GetFullDepthStirrups();
   newReinforcement.ConditionFactorType = pParent->GetConditionFactorType();
   newReinforcement.ConditionFactor = pParent->GetConditionFactor();

   return std::make_unique<txnEditReinforcement>(pEditPierData->GetPierData()->GetID(),oldReinforcement,newReinforcement);
}

IDType CXBeamRateAgent::GetEditBridgeCallbackID()
{
   return INVALID_ID;
}

/////////////////////////////////////////////////////////////////////////////////
// IEditLoadRatingOptionsCallback
CPropertyPage* CXBeamRateAgent::CreatePropertyPage(IEditLoadRatingOptions* pLoadRatingOptions)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CLoadRatingOptionsPage* pPage = new CLoadRatingOptionsPage();

   GET_IFACE(IXBRRatingSpecification,pSpec);
   pPage->m_AnalysisType = pSpec->GetAnalysisMethodForReactions();
   pPage->m_PermitRatingMethod = pSpec->GetPermitRatingMethod();
   pPage->m_EmergencyRatingMethod = pSpec->GetEmergencyRatingMethod();

   GET_IFACE(IXBRProject,pProject);
   pPage->m_MaxLLStepSize = pProject->GetMaxLiveLoadStepSize();
   pPage->m_MaxLoadedLanes = pProject->GetMaxLoadedLanes();
   pPage->m_SystemFactorFlexure = pProject->GetSystemFactorFlexure();
   pPage->m_SystemFactorShear = pProject->GetSystemFactorShear();

   pPage->m_bDoAnalyzeNegativeMomentBetweenFOC = pProject->GetDoAnalyzeNegativeMomentBetweenFocOptions(&(pPage->m_MinColumnWidthForNegMoment));

   return pPage;
}

std::unique_ptr<WBFL::EAF::Transaction> CXBeamRateAgent::OnOK(CPropertyPage* pPage,IEditLoadRatingOptions* pLoadRatingOptions)
{
   CLoadRatingOptionsPage* pLROPage = (CLoadRatingOptionsPage*)pPage;

   GET_IFACE(IXBRRatingSpecification,pSpec);

   txnLoadRatingOptions oldOptions;
   oldOptions.m_AnalysisType = pSpec->GetAnalysisMethodForReactions();
   oldOptions.m_PermitRatingMethod = pSpec->GetPermitRatingMethod();
   oldOptions.m_EmergencyRatingMethod = pSpec->GetEmergencyRatingMethod();

   GET_IFACE(IXBRProject,pProject);
   oldOptions.m_MaxLLStepSize = pProject->GetMaxLiveLoadStepSize();
   oldOptions.m_MaxLoadedLanes = pProject->GetMaxLoadedLanes();
   oldOptions.m_SystemFactorFlexure = pProject->GetSystemFactorFlexure();
   oldOptions.m_SystemFactorShear = pProject->GetSystemFactorShear();
   
   oldOptions.m_bDoAnalyzeNegativeMomentBetweenFOC = pProject->GetDoAnalyzeNegativeMomentBetweenFocOptions(&(oldOptions.m_MinColumnWidthForNegMoment));

   txnLoadRatingOptions newOptions;
   newOptions.m_AnalysisType = pLROPage->m_AnalysisType;
   newOptions.m_PermitRatingMethod = pLROPage->m_PermitRatingMethod;
   newOptions.m_EmergencyRatingMethod = pLROPage->m_EmergencyRatingMethod;
   newOptions.m_MaxLLStepSize = pLROPage->m_MaxLLStepSize;
   newOptions.m_MaxLoadedLanes = pLROPage->m_MaxLoadedLanes;
   newOptions.m_SystemFactorFlexure = pLROPage->m_SystemFactorFlexure;
   newOptions.m_SystemFactorShear = pLROPage->m_SystemFactorShear;
   newOptions.m_bDoAnalyzeNegativeMomentBetweenFOC = pLROPage->m_bDoAnalyzeNegativeMomentBetweenFOC;
   newOptions.m_MinColumnWidthForNegMoment = pLROPage->m_MinColumnWidthForNegMoment;

   return std::make_unique<txnEditLoadRatingOptions>(oldOptions,newOptions);
}

/////////////////////////////////////////////////////////////////////////////////
//IProjectPropertiesEventSink
HRESULT CXBeamRateAgent::OnProjectPropertiesChanged()
{
   GET_IFACE(IProjectProperties,pProjectProps);
   GET_IFACE(IXBRProjectProperties, pXBRProjectProps);
   pXBRProjectProps->SetBridgeName(pProjectProps->GetBridgeName());
   pXBRProjectProps->SetBridgeID(pProjectProps->GetBridgeID());
   pXBRProjectProps->SetJobNumber(pProjectProps->GetJobNumber());
   pXBRProjectProps->SetEngineer(pProjectProps->GetEngineer());
   pXBRProjectProps->SetCompany(pProjectProps->GetCompany());
   pXBRProjectProps->SetComments(pProjectProps->GetComments());
   return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////
// IXBRProjectEventSink
HRESULT CXBeamRateAgent::OnProjectChanged()
{
   if ( 0 <= m_PierViewKey )
   {
      CEAFDocument* pDoc = EAFGetDocument();
      pDoc->SetModifiedFlag();
      pDoc->UpdateRegisteredView(m_PierViewKey,0,0,0);
   }

   return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// IEAFProcessCommandLine
BOOL CXBeamRateAgent::ProcessCommandLineOptions(CEAFCommandLineInfo& cmdInfo)
{
   CXBRateCommandLineProcessor processor;
   return processor.ProcessCommandLineOptions(cmdInfo);
}


void CXBeamRateAgent::OnViewPier()
{
   CreatePierView();
}

void CXBeamRateAgent::OnExportPier()
{
   GET_IFACE(ISelection, pSelection);
   PierIndexType selPierIdx = pSelection->GetSelectedPier();

   GET_IFACE(IXBRExport, pExport);
   pExport->Export(selPierIdx);
}

void CXBeamRateAgent::OnPierCommandUpdate(CCmdUI* pCmdUI)
{
   GET_IFACE(IBridgeDescription, pIBridgeDesc);
   PierIndexType nPiers = pIBridgeDesc->GetPierCount();

   GET_IFACE(ISelection, pSelection);
   PierIndexType selPierIdx = pSelection->GetSelectedPier();
   const CPierData2* pPier = pIBridgeDesc->GetPier(selPierIdx);
   if (pPier == nullptr || pPier->GetPierModelType() == pgsTypes::pmtPhysical)
   {
      pCmdUI->Enable(TRUE);
   }
   else
   {
      pCmdUI->Enable(FALSE);
   }
}


// IBridgePlanViewEventCallback
void CXBeamRateAgent::OnBackgroundContextMenu(std::shared_ptr<WBFL::EAF::Menu> menu)
{
}

void CXBeamRateAgent::OnPierContextMenu(PierIndexType pierIdx, std::shared_ptr<WBFL::EAF::Menu> menu)
{
   auto callback = std::dynamic_pointer_cast<WBFL::EAF::ICommandCallback>(shared_from_this());
   menu->AppendMenu(ID_VIEW_PIER, _T("View Pier"), callback);
   menu->AppendMenu(IDM_EXPORT_PIER, _T("Export to XBRate"), callback);
}

void CXBeamRateAgent::OnSpanContextMenu(SpanIndexType spanIdx, std::shared_ptr<WBFL::EAF::Menu> menu) {}
void CXBeamRateAgent::OnDeckContextMenu(std::shared_ptr<WBFL::EAF::Menu> menu) {}
void CXBeamRateAgent::OnAlignmentContextMenu(std::shared_ptr<WBFL::EAF::Menu> menu) {}
void CXBeamRateAgent::OnSectionCutContextMenu(std::shared_ptr<WBFL::EAF::Menu> menu) {}
void CXBeamRateAgent::OnGirderContextMenu(const CGirderKey& girderKey, std::shared_ptr<WBFL::EAF::Menu> menu) {}
void CXBeamRateAgent::OnTemporarySupportContextMenu(SupportIDType tsID, std::shared_ptr<WBFL::EAF::Menu> menu) {}
void CXBeamRateAgent::OnGirderSegmentContextMenu(const CSegmentKey& segmentKey, std::shared_ptr<WBFL::EAF::Menu> menu) {}
void CXBeamRateAgent::OnClosureJointContextMenu(const CSegmentKey& closureKey, std::shared_ptr<WBFL::EAF::Menu> menu) {}


/////////////////////////////////////////////////////////////////////////////////
// ICommandCallback
BOOL CXBeamRateAgent::OnCommandMessage(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
   return OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CXBeamRateAgent::GetStatusBarMessageString(UINT nID, CString& rMessage) const
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   // load appropriate string
   if (rMessage.LoadString(nID))
   {
      // first newline terminates actual string
      rMessage.Replace('\n', '\0');
   }
   else
   {
      // not found
      TRACE1("Warning (CXBeamRateAgent): no message line prompt for ID %d.\n", nID);
   }

   return TRUE;
}

BOOL CXBeamRateAgent::GetToolTipMessageString(UINT nID, CString& rMessage) const
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CString string;
   // load appropriate string
   if (string.LoadString(nID))
   {
      // tip is after first newline 
      int pos = string.Find('\n');
      if (0 < pos)
         rMessage = string.Mid(pos + 1);
   }
   else
   {
      // not found
      TRACE1("Warning (CXBeamRateAgent): no tool tip for ID %d.\n", nID);
   }

   return TRUE;
}
