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


// ProjectAgentImp.cpp : Implementation of CProjectAgentImp
#include "stdafx.h"
#include "resource.h"
#include "ProjectAgent.h"
#include "ProjectAgentImp.h"

#include <WBFLTools\StructuredStorageComAdapter.h>

#include <XBeamRateExt\StatusItem.h>

#include <EAF\EAFDisplayUnits.h>
#include <IFace\XBeamRateAgent.h>
#include <IFace\VersionInfo.h>


#include <PsgLib\GirderLabel.h>
#include <EAF/AutoProgress.h>

#include <PsgLib\BridgeDescription2.h>
#include <PsgLib\Helpers.h>

#include <..\..\PGSuper\Include\IFace\Bridge.h>
#include <..\..\PGSuper\Include\IFace\Alignment.h>
#include <..\..\PGSuper\Include\IFace\Intervals.h>
#include <..\..\PGSuper\Include\IFace\AnalysisResults.h>
#include <..\..\PGSuper\Include\IFace\PointOfInterest.h>
#include <..\..\PGSuper\Include\IFace\RatingSpecification.h>
#include <..\..\PGSuper\Include\IFace\ResistanceFactors.h>
#include <..\..\PGSuper\Include\IFace\EditByUI.h>
#include <..\..\PGSuper\Include\IFace\BeamFactory.h>
#include <Plugins\BeamFamilyCLSID.h>

#include <MFCTools\AutoRegistry.h>

#include "PierExporter.h"

#include <MFCTools\Prompts.h>
#include <EAF\EAFTransactions.h>
#include <txnEditProject.h>
#include <txnEditPier.h>
#include <txnEditOptions.h>

#include "PierDlg.h"
#include "OptionsDlg.h"
#include "ProjectPropertiesDlg.h"

#include <XBeamRateExt\XBeamRateUtilities.h>

#include <IFace\Project.h>
#include <IFace\RatingSpecification.h>



BEGIN_MESSAGE_MAP(CProjectAgentImp, CCmdTarget)
   ON_COMMAND(ID_EDIT_PIER, OnEditPier)
   ON_COMMAND(ID_EDIT_OPTIONS, OnEditOptions)
   ON_COMMAND(ID_EDIT_PROPERTIES, OnProjectProperties)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectAgentImp
CProjectAgentImp::CProjectAgentImp()
{
   m_pBroker = 0;
   m_EventHoldCount = 0;

   m_strBridgeName = _T("");
   m_strBridgeId   = _T("");
   m_strJobNumber  = _T("");
   m_strEngineer   = _T("");
   m_strCompany    = _T("");
   m_strComments   = _T("");

   m_SysFactorFlexure = 1.0;
   m_SysFactorShear   = 1.0;

   m_PhiC = 0.75;
   m_PhiT = 0.90;
   m_PhiV = 0.90;

   m_bRatingEnabled[pgsTypes::lrDesign_Inventory] = true;
   m_bRatingEnabled[pgsTypes::lrDesign_Operating] = true;
   m_bRatingEnabled[pgsTypes::lrLegal_Routine]    = true;
   m_bRatingEnabled[pgsTypes::lrLegal_Special] = true;
   m_bRatingEnabled[pgsTypes::lrLegal_Emergency] = true;
   m_bRatingEnabled[pgsTypes::lrPermit_Routine]   = true;
   m_bRatingEnabled[pgsTypes::lrPermit_Special]   = true;

   m_bRateForShear[pgsTypes::lrDesign_Inventory] = true;
   m_bRateForShear[pgsTypes::lrDesign_Operating] = true;
   m_bRateForShear[pgsTypes::lrLegal_Routine]    = true;
   m_bRateForShear[pgsTypes::lrLegal_Special] = true;
   m_bRateForShear[pgsTypes::lrLegal_Emergency] = true;
   m_bRateForShear[pgsTypes::lrPermit_Routine]   = true;
   m_bRateForShear[pgsTypes::lrPermit_Special]   = true;

   m_bCheckYieldStress = true;
   m_YieldStressCoefficient = 0.9;

   m_gDC_StrengthI = 1.25;
   m_gDW_StrengthI = 1.50;
   m_gCR_StrengthI = 1.25;
   m_gSH_StrengthI = 1.25;
   m_gRE_StrengthI = 1.25;
   m_gPS_StrengthI = 1.00;

   m_gDC_StrengthII = 1.25;
   m_gDW_StrengthII = 1.50;
   m_gCR_StrengthII = 1.25;
   m_gSH_StrengthII = 1.25;
   m_gRE_StrengthII = 1.25;
   m_gPS_StrengthII = 1.00;

   m_gDC_ServiceI = 1.00;
   m_gDW_ServiceI = 1.00;
   m_gCR_ServiceI = 1.00;
   m_gSH_ServiceI = 1.00;
   m_gRE_ServiceI = 1.00;
   m_gPS_ServiceI = 1.00;

   m_gLL[GET_INDEX(pgsTypes::StrengthI_Inventory)][INVALID_ID]       = 1.75;
   m_gLL[GET_INDEX(pgsTypes::StrengthI_Operating)][INVALID_ID]       = 1.35;
   m_gLL[GET_INDEX(pgsTypes::StrengthI_LegalRoutine)][INVALID_ID]    = 1.80;
   m_gLL[GET_INDEX(pgsTypes::StrengthI_LegalSpecial)][INVALID_ID] = 1.60;
   m_gLL[GET_INDEX(pgsTypes::StrengthI_LegalEmergency)][INVALID_ID] = 1.30;
   m_gLL[GET_INDEX(pgsTypes::StrengthII_PermitRoutine)][INVALID_ID]  = 1.15;
   m_gLL[GET_INDEX(pgsTypes::StrengthII_PermitSpecial)][INVALID_ID]  = 1.20;
   m_gLL[GET_INDEX(pgsTypes::ServiceI_PermitRoutine)][INVALID_ID]    = 1.00;
   m_gLL[GET_INDEX(pgsTypes::ServiceI_PermitSpecial)][INVALID_ID]    = 1.00;

   m_LiveLoadReactions[pgsTypes::lrDesign_Inventory][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("LRFD Legal Truck + Lane"),0,0));
   m_LiveLoadReactions[pgsTypes::lrDesign_Inventory][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("LRFD Legal Tandem + Lane"),0,0));
   m_LiveLoadReactions[pgsTypes::lrDesign_Inventory][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("LRFD Truck Train [90%(Truck + Lane)]"),0,0));
   m_LiveLoadReactions[pgsTypes::lrDesign_Inventory][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("LRFD Low Boy (Dual Tandem + Lane)"),0,0));

   m_LiveLoadReactions[pgsTypes::lrDesign_Operating][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("LRFD Legal Truck + Lane"),0,0));
   m_LiveLoadReactions[pgsTypes::lrDesign_Operating][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("LRFD Legal Tandem + Lane"),0,0));
   m_LiveLoadReactions[pgsTypes::lrDesign_Operating][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("LRFD Truck Train [90%(Truck + Lane)]"),0,0));
   m_LiveLoadReactions[pgsTypes::lrDesign_Operating][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("LRFD Low Boy (Dual Tandem + Lane)"),0,0));

   m_LiveLoadReactions[pgsTypes::lrLegal_Routine][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("Type 3"),0,WBFL::Units::ConvertToSysUnits(50.0,WBFL::Units::Measure::Kip)));
   m_LiveLoadReactions[pgsTypes::lrLegal_Routine][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("Type 3S2"),0,WBFL::Units::ConvertToSysUnits(72.0,WBFL::Units::Measure::Kip)));
   m_LiveLoadReactions[pgsTypes::lrLegal_Routine][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("Type 3-3"),0,WBFL::Units::ConvertToSysUnits(80.0,WBFL::Units::Measure::Kip)));
   m_LiveLoadReactions[pgsTypes::lrLegal_Routine][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("0.75(Two Type 3-3 separated by 30ft) + Lane Load"),0,WBFL::Units::ConvertToSysUnits(80.0,WBFL::Units::Measure::Kip)));

   m_LiveLoadReactions[pgsTypes::lrLegal_Special][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("Notional Rating Load (NRL)"),0,WBFL::Units::ConvertToSysUnits(80.0,WBFL::Units::Measure::Kip)));
   //m_LiveLoadReactions[pgsTypes::lrLegal_Special][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("SU4"),0,WBFL::Units::ConvertToSysUnits(54.0,WBFL::Units::Measure::Kip)));
   //m_LiveLoadReactions[pgsTypes::lrLegal_Special][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("SU5"),0,WBFL::Units::ConvertToSysUnits(62.0,WBFL::Units::Measure::Kip)));
   //m_LiveLoadReactions[pgsTypes::lrLegal_Special][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("SU6"),0,WBFL::Units::ConvertToSysUnits(69.5,WBFL::Units::Measure::Kip)));
   //m_LiveLoadReactions[pgsTypes::lrLegal_Special][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("SU7"),0,WBFL::Units::ConvertToSysUnits(77.5,WBFL::Units::Measure::Kip)));

   m_LiveLoadReactions[pgsTypes::lrLegal_Emergency][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("Type EV2"), 0, WBFL::Units::ConvertToSysUnits(57.5, WBFL::Units::Measure::Kip)));
   m_LiveLoadReactions[pgsTypes::lrLegal_Emergency][INVALID_ID].push_back(xbrLiveLoadReactionData(_T("Type EV3"), 0, WBFL::Units::ConvertToSysUnits(86.0, WBFL::Units::Measure::Kip)));

   xbrPierData pierData;
   IndexType nBearingLines = pierData.GetBearingLineCount();
   for ( IndexType brgLineIdx = 0; brgLineIdx < nBearingLines; brgLineIdx++ )
   {
      std::vector<BearingReactions> vBearingReactions;
      IndexType nBearings = pierData.GetBearingCount(brgLineIdx);
      for ( IndexType brgIdx = 0; brgIdx < nBearings; brgIdx++ )
      {
         BearingReactions reactions;
         vBearingReactions.push_back(reactions);
      }
      m_BearingReactions[brgLineIdx].insert(std::make_pair(INVALID_ID,vBearingReactions));
   }

   // pier data for the stand alone case
   m_PierData.insert(std::make_pair(INVALID_ID,pierData));

   m_AnalysisType = pgsTypes::Envelope;

   m_PermitRatingMethod = xbrTypes::prmWSDOT;
   m_EmergencyRatingMethod = xbrTypes::ermWSDOT;

   m_MaxLLStepSize = WBFL::Units::ConvertToSysUnits(1.0,WBFL::Units::Measure::Feet);
   m_MaxLoadedLanes = 4; // usually, anything beyond 4 lanes doesn't control

   m_bDoAnalyzeNegativeMomentBetweenFOC = false;
   m_MinColumnWidthForNegMoment = WBFL::Units::ConvertToSysUnits(4.0,WBFL::Units::Measure::Feet); // txdot's default

   m_bExportingModel = false;
}

void CProjectAgentImp::Invalidate()
{
   for ( int i = 0; i < RATING_LIMIT_STATE_COUNT; i++ )
   {
      m_gLL[i].clear();
   }

   for ( int i = 0; i < 2; i++ )
   {
      m_BearingReactions[i].clear();
      m_BearingReactionType[i].clear();
   }
   
   int n = (int)pgsTypes::lrLoadRatingTypeCount;
   for ( int i = 0; i < n; i++ )
   {
      std::map<PierIDType,std::vector<xbrLiveLoadReactionData>>& reactions = m_LiveLoadReactions[i];
      reactions.clear();
   }

   m_ReactionApplication.clear();
}

template <class T> 
class AutoRevert
{
public:
   AutoRevert(T* pParam, const T& value, const T& revert) : m_pParam(pParam), m_Revert(revert) { *m_pParam = value; };
   ~AutoRevert() { *m_pParam = m_Revert; };
private:
   T* m_pParam;
   T m_Revert;
};

HRESULT CProjectAgentImp::SavePier(PierIndexType pierIdx,LPCTSTR lpszPathName)
{
   GET_IFACE(IBridgeDescription,pIBridgeDesc);
   const CPierData2* pPier = pIBridgeDesc->GetPier(pierIdx);
   PierIDType pierID = pPier->GetID();

   GET_IFACE(IEAFProgress,pProgress);
   WBFL::EAF::AutoProgress ap(pProgress);

   CString strMsg;
   strMsg.Format(_T("Exporting Pier %s to XBRate"),LABEL_PIER(pierIdx));
   pProgress->UpdateMessage(strMsg);

   AutoRevert<PierIDType> revert_pier_id(&m_SavePierID, pierID, INVALID_ID); // this is the ID of the pier we are saving... revert to INVALID_ID on exit
   AutoRevert<bool> revert_exporting_model(&m_bExportingModel, true, false); // set this to true so Save() knows to leave m_SavePierID alone... revert to false on exit

   // We can't tap directly into the IEAFDocument::SaveAs or into IBrokerPersist, because they will
   // save the current PGS document. We want to create a XBR document. To accomplish this, we have to
   // replicate the prologue information that IEAFDocument and IBrokerPersist would write into the file.
   // This is a little brittle, but that prologue information is very stable so it isn't likely to change.

   CComPtr<IStructuredSave> pStrSave;
   HRESULT hr = ::CoCreateInstance(CLSID_StructuredSave, nullptr, CLSCTX_INPROC_SERVER, IID_IStructuredSave, (void**)&pStrSave);
   ATLASSERT(SUCCEEDED(hr));

   hr = pStrSave->Open(lpszPathName);
   ATLASSERT(SUCCEEDED(hr));

   pStrSave->BeginUnit(_T("XBeamRate"),1.0);

   GET_IFACE(IXBRVersionInfo,pVersionInfo);
   CString strVersion = pVersionInfo->GetVersion(true);
   pStrSave->put_Property(_T("Version"),CComVariant(strVersion));

   pStrSave->BeginUnit(_T("Broker"),1.0);
   pStrSave->BeginUnit(_T("Agent"),1.0);

   LPOLESTR postr;
   ::StringFromCLSID(CLSID_XBeamRateProjectAgent,&postr);
   pStrSave->put_Property(_T("CLSID"),CComVariant(postr));
   ::CoTaskMemFree(postr);

   // Save() takes the native WBFL::System::IStructuredSave interface; wrap the COM pointer this
   // hand-rolled mini-Broker-save already created, same as WBFL::EAF::Broker::SaveAgentData does.
   CStructuredSave native_save(pStrSave);
   Save(&native_save);

   pStrSave->EndUnit(); // Agent
   pStrSave->EndUnit(); // Broker
   pStrSave->EndUnit(); // XBeamRate

   return S_OK;
}

//////////////////////////////////////////////////////////////////////
// IAgent
bool CProjectAgentImp::RegisterInterfaces()
{
   EAF_AGENT_REGISTER_INTERFACES;

   REGISTER_INTERFACE(IXBRProject);
   REGISTER_INTERFACE(IXBRProjectProperties);
   REGISTER_INTERFACE(IXBRRatingSpecification);
   REGISTER_INTERFACE(IXBRProjectEdit);
   REGISTER_INTERFACE(IXBREvents);
   REGISTER_INTERFACE(IXBRExport);

   return true;
};

bool CProjectAgentImp::Init()
{
   EAF_AGENT_INIT;

   GET_IFACE(IEAFStatusCenter, pStatusCenter);
   m_XBeamRateStatusGroupID = pStatusCenter->CreateStatusGroupID();

   // Register status callbacks that we want to use
   m_scidBridgeInfo = pStatusCenter->RegisterCallback(std::make_shared<xbrBridgeStatusCallback>(WBFL::EAF::StatusSeverityType::Information));
   m_scidBridgeWarn = pStatusCenter->RegisterCallback(std::make_shared<xbrBridgeStatusCallback>(WBFL::EAF::StatusSeverityType::Warning));
   m_scidBridgeError = pStatusCenter->RegisterCallback(std::make_shared<xbrBridgeStatusCallback>(WBFL::EAF::StatusSeverityType::Error));

   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CWinApp* pApp = AfxGetApp();
   CAutoRegistry autoReg(_T("XBeamRate"));
   CString strProjectProperties = pApp->GetProfileString(_T("Settings"),_T("ShowProjectProperties"),_T("On"));
   if ( strProjectProperties.CompareNoCase(_T("Off")) == 0 )
   {
      ShowProjectPropertiesOnNewProject(false);
   }
   else
   {
      ShowProjectPropertiesOnNewProject(true);
   }

   //
   // Attach to connection points for interfaces this agent depends on
   //
   m_dwBridgeDescCookie = REGISTER_EVENT_SINK(IBridgeDescriptionEventSink);
   m_dwEventsCookie = REGISTER_EVENT_SINK(IEventsSink);

   return true;
}

bool CProjectAgentImp::Reset()
{
   EAF_AGENT_RESET;
   return true;
}

CLSID CProjectAgentImp::GetCLSID() const
{
   return CLSID_XBeamRateProjectAgent;
}

bool CProjectAgentImp::ShutDown()
{
   EAF_AGENT_SHUTDOWN;
   Invalidate();
   UNREGISTER_EVENT_SINK(IBridgeDescriptionEventSink, m_dwBridgeDescCookie);
   UNREGISTER_EVENT_SINK(IEventsSink, m_dwEventsCookie);

   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CAutoRegistry autoReg(_T("XBeamRate"));
   CWinApp* pApp = AfxGetApp();
   VERIFY(pApp->WriteProfileString( _T("Settings"),_T("ShowProjectProperties"),ShowProjectPropertiesOnNewProject() ? _T("On") : _T("Off") ));

   //EAF_AGENT_CLEAR_INTERFACE_CACHE;

   return true;
}

IndexType CProjectAgentImp::GetPriority() const
{
   return 1;
}

//////////////////////////////////////////////////////////////////////
// IAgentUIIntegration
bool CProjectAgentImp::IntegrateWithUI(bool bIntegrate)
{
   if ( IsPGSExtension())
   {
      // XBeam Rate is acting as an extension to PGSuper/PGSplice
      return S_OK;
   }

   if ( bIntegrate )
   {
      CreateMenus();
      CreateToolbars();
   }
   else
   {
      RemoveMenus();
      RemoveToolbars();
   }

   return S_OK;
}

//////////////////////////////////////////////////////////////////////
// IAgentPersist
bool CProjectAgentImp::Save(WBFL::System::IStructuredSave* pNativeStrSave)
{
   // The internal save pipeline below is still COM-typed; unwrap back to the original COM
   // pointer here until that call graph is migrated too. See WBFLTools\StructuredStorageComAdapter.h.
   auto* pAdapter = dynamic_cast<CStructuredSave*>(pNativeStrSave);
   ATLASSERT(pAdapter);
   ::IStructuredSave* pStrSave = pAdapter->GetComInterface();

   HRESULT hr = S_OK;

   // Save project data, properties, reactions, settings, etc if we are in "stand alone" mode
   // or we are exporting a pier model from PGS.
   // Otherwise we are a PGS plug-in and all we want to save is the pier description
   if ( IsStandAlone() || m_bExportingModel )
   {
      // if we are exporting, m_SavePierID is set in Export(), so leave it alone,
      // otherwise, set it to INVALID_ID (the ID of the agent local pier model in stand alone mode)
      if ( !m_bExportingModel )
      {
         m_SavePierID = INVALID_ID;
      }

      // NOTE: Use the accessor methods to get data during the save instead of accessing the data members directly
      // The accessor methods know how to read agent local data and get data from PGS in a polymorphic way

      pStrSave->BeginUnit(_T("ProjectData"),1.0);

      pStrSave->BeginUnit(_T("ProjectProperties"),1.0);
         pStrSave->put_Property(_T("BridgeName"), CComVariant(GetBridgeName()));
         pStrSave->put_Property(_T("BridgeId"),   CComVariant(GetBridgeID()));
         pStrSave->put_Property(_T("JobNumber"),  CComVariant(GetJobNumber()));
         pStrSave->put_Property(_T("Engineer"),   CComVariant(GetEngineer()));
         pStrSave->put_Property(_T("Company"),    CComVariant(GetCompany()));
         pStrSave->put_Property(_T("Comments"),   CComVariant(GetComments()));
      pStrSave->EndUnit(); // ProjectProperties

      pStrSave->BeginUnit(_T("ProjectSettings"),1.0);
         GET_IFACE(IEAFDisplayUnits,pDisplayUnits);
         pStrSave->put_Property(_T("Units"),CComVariant(+pDisplayUnits->GetUnitMode()));
      pStrSave->EndUnit(); // ProjectSettings

      // Need to save stuff like units and project properties,system factors
      // Also, need to save/load this data per pier
      // when this is a PGSuper/PGSplice extension, there can be many piers

      pStrSave->BeginUnit(_T("RatingSpecification"),4.0);
         pStrSave->put_Property(_T("LRFD"),CComVariant(WBFL::LRFD::BDSManager::GetEditionAsString(true)));
         pStrSave->put_Property(_T("LRFR"),CComVariant(WBFL::LRFD::MBEManager::GetEditionAsString(true)));

         pStrSave->put_Property(_T("EmergencyRatingMethod"), CComVariant(GetEmergencyRatingMethod())); // added in version 3
         pStrSave->put_Property(_T("PermitRatingMethod"), CComVariant(GetPermitRatingMethod()));
         pStrSave->put_Property(_T("SystemFactorFlexure"),CComVariant(GetSystemFactorFlexure()));
         pStrSave->put_Property(_T("SystemFactorShear"),CComVariant(GetSystemFactorShear()));

         Float64 phiC,phiT;
         GetFlexureResistanceFactors(&phiC,&phiT);
         pStrSave->put_Property(_T("PhiC"),CComVariant(phiC));
         pStrSave->put_Property(_T("PhiT"),CComVariant(phiT));
         pStrSave->put_Property(_T("PhiV"),CComVariant(GetShearResistanceFactor()));

         pStrSave->put_Property(_T("MaxLiveLoadStepSize"),CComVariant(GetMaxLiveLoadStepSize()));
         pStrSave->put_Property(_T("MaxLoadedLanes"),CComVariant(GetMaxLoadedLanes()));

         pStrSave->put_Property(_T("RatingEnabled_Design_Inventory"),CComVariant(IsRatingEnabled(pgsTypes::lrDesign_Inventory)));
         pStrSave->put_Property(_T("RatingEnabled_Design_Operating"),CComVariant(IsRatingEnabled(pgsTypes::lrDesign_Operating)));
         pStrSave->put_Property(_T("RatingEnabled_Legal_Routine"),   CComVariant(IsRatingEnabled(pgsTypes::lrLegal_Routine)));
         pStrSave->put_Property(_T("RatingEnabled_Legal_Special"), CComVariant(IsRatingEnabled(pgsTypes::lrLegal_Special)));
         pStrSave->put_Property(_T("RatingEnabled_Legal_Emergency"), CComVariant(IsRatingEnabled(pgsTypes::lrLegal_Emergency))); // added in version 2
         pStrSave->put_Property(_T("RatingEnabled_Permit_Routine"),  CComVariant(IsRatingEnabled(pgsTypes::lrPermit_Routine)));
         pStrSave->put_Property(_T("RatingEnabled_Permit_Special"),  CComVariant(IsRatingEnabled(pgsTypes::lrPermit_Special)));

         pStrSave->put_Property(_T("RateForShear_Design_Inventory"),CComVariant(RateForShear(pgsTypes::lrDesign_Inventory)));
         pStrSave->put_Property(_T("RateForShear_Design_Operating"),CComVariant(RateForShear(pgsTypes::lrDesign_Operating)));
         pStrSave->put_Property(_T("RateForShear_Legal_Routine"),   CComVariant(RateForShear(pgsTypes::lrLegal_Routine)));
         pStrSave->put_Property(_T("RateForShear_Legal_Special"), CComVariant(RateForShear(pgsTypes::lrLegal_Special)));
         pStrSave->put_Property(_T("RateForShear_Legal_Emergency"), CComVariant(RateForShear(pgsTypes::lrLegal_Emergency))); // added in version 2
         pStrSave->put_Property(_T("RateForShear_Permit_Routine"),  CComVariant(RateForShear(pgsTypes::lrPermit_Routine)));
         pStrSave->put_Property(_T("RateForShear_Permit_Special"),  CComVariant(RateForShear(pgsTypes::lrPermit_Special)));

         pStrSave->put_Property(_T("CheckYieldStressLimit"), CComVariant(CheckYieldStressLimit()));
         pStrSave->put_Property(_T("YieldStressCoefficient"), CComVariant(GetYieldStressLimitCoefficient()));

         pStrSave->put_Property(_T("DoAnalyzeNegativeMomentBetweenFOC"), CComVariant(m_bDoAnalyzeNegativeMomentBetweenFOC)); // added in version 4
         pStrSave->put_Property(_T("MinColumnWidthForNegMoment"), CComVariant(m_MinColumnWidthForNegMoment)); // added in version 4
      pStrSave->EndUnit(); // RatingSpecification

      pStrSave->BeginUnit(_T("LoadFactors"),2.0);
         // NOTE: This note applies to exporting a pier model from PGSuper
         // PGSuper has DC, DW, etc load factors for every load rating limit state,
         // however, XBeamRate only has these load factors for StrengthI, StrengthII, and ServiceIII
         // regardless of the load rating type. It is assumed that the same load factor is used for
         // each rating type in PGSuper, however this may not be correct. Some data may be lost during
         // the export, but in most all practical cases the load factors will be the same for each
         // rating type.
         pStrSave->put_Property(_T("DC_StrengthI"),CComVariant(GetDCLoadFactor(pgsTypes::StrengthI_Inventory)));
         pStrSave->put_Property(_T("DW_StrengthI"),CComVariant(GetDWLoadFactor(pgsTypes::StrengthI_Inventory)));
         pStrSave->put_Property(_T("CR_StrengthI"),CComVariant(GetCRLoadFactor(pgsTypes::StrengthI_Inventory)));
         pStrSave->put_Property(_T("SH_StrengthI"),CComVariant(GetSHLoadFactor(pgsTypes::StrengthI_Inventory)));
         pStrSave->put_Property(_T("PS_StrengthI"),CComVariant(GetPSLoadFactor(pgsTypes::StrengthI_Inventory)));
         pStrSave->put_Property(_T("RE_StrengthI"),CComVariant(GetRELoadFactor(pgsTypes::StrengthI_Inventory)));

         pStrSave->put_Property(_T("DC_StrengthII"),CComVariant(GetDCLoadFactor(pgsTypes::StrengthII_PermitRoutine)));
         pStrSave->put_Property(_T("DW_StrengthII"),CComVariant(GetDWLoadFactor(pgsTypes::StrengthII_PermitRoutine)));
         pStrSave->put_Property(_T("CR_StrengthII"),CComVariant(GetCRLoadFactor(pgsTypes::StrengthII_PermitRoutine)));
         pStrSave->put_Property(_T("SH_StrengthII"),CComVariant(GetSHLoadFactor(pgsTypes::StrengthII_PermitRoutine)));
         pStrSave->put_Property(_T("PS_StrengthII"),CComVariant(GetPSLoadFactor(pgsTypes::StrengthII_PermitRoutine)));
         pStrSave->put_Property(_T("RE_StrengthII"),CComVariant(GetRELoadFactor(pgsTypes::StrengthII_PermitRoutine)));

         pStrSave->put_Property(_T("DC_ServiceI"),CComVariant(GetDCLoadFactor(pgsTypes::ServiceI_PermitRoutine)));
         pStrSave->put_Property(_T("DW_ServiceI"),CComVariant(GetDWLoadFactor(pgsTypes::ServiceI_PermitRoutine)));
         pStrSave->put_Property(_T("CR_ServiceI"),CComVariant(GetCRLoadFactor(pgsTypes::ServiceI_PermitRoutine)));
         pStrSave->put_Property(_T("SH_ServiceI"),CComVariant(GetSHLoadFactor(pgsTypes::ServiceI_PermitRoutine)));
         pStrSave->put_Property(_T("PS_ServiceI"),CComVariant(GetPSLoadFactor(pgsTypes::ServiceI_PermitRoutine)));
         pStrSave->put_Property(_T("RE_ServiceI"),CComVariant(GetRELoadFactor(pgsTypes::ServiceI_PermitRoutine)));

         pStrSave->put_Property(_T("LL_StrengthI_Inventory"),      CComVariant(GetLiveLoadFactor(m_SavePierID,pgsTypes::StrengthI_Inventory,      INVALID_INDEX)));
         pStrSave->put_Property(_T("LL_StrengthI_Operating"),      CComVariant(GetLiveLoadFactor(m_SavePierID,pgsTypes::StrengthI_Operating,      INVALID_INDEX)));
         pStrSave->put_Property(_T("LL_StrengthI_LegalRoutine"),   CComVariant(GetLiveLoadFactor(m_SavePierID,pgsTypes::StrengthI_LegalRoutine,   INVALID_INDEX)));
         pStrSave->put_Property(_T("LL_StrengthI_LegalSpecial"), CComVariant(GetLiveLoadFactor(m_SavePierID, pgsTypes::StrengthI_LegalSpecial, INVALID_INDEX)));
         pStrSave->put_Property(_T("LL_StrengthI_LegalEmergency"), CComVariant(GetLiveLoadFactor(m_SavePierID, pgsTypes::StrengthI_LegalEmergency, INVALID_INDEX))); // added in version 2
         pStrSave->put_Property(_T("LL_StrengthII_PermitRoutine"), CComVariant(GetLiveLoadFactor(m_SavePierID,pgsTypes::StrengthII_PermitRoutine, INVALID_INDEX)));
         pStrSave->put_Property(_T("LL_StrengthII_PermitSpecial"), CComVariant(GetLiveLoadFactor(m_SavePierID,pgsTypes::StrengthII_PermitSpecial, INVALID_INDEX)));
         pStrSave->put_Property(_T("LL_ServiceI_PermitRoutine"),   CComVariant(GetLiveLoadFactor(m_SavePierID,pgsTypes::ServiceI_PermitRoutine,   INVALID_INDEX)));
         pStrSave->put_Property(_T("LL_ServiceI_PermitSpecial"),   CComVariant(GetLiveLoadFactor(m_SavePierID,pgsTypes::ServiceI_PermitSpecial,   INVALID_INDEX)));
      pStrSave->EndUnit(); // LoadFactors

      pStrSave->BeginUnit(_T("Reactions"),1.0);
         pStrSave->BeginUnit(_T("DeadLoad"),1.0);
         IndexType nBearingLines = GetBearingLineCount(m_SavePierID);
         for ( IndexType brgLineIdx = 0; brgLineIdx < nBearingLines; brgLineIdx++ )
         {
            pStrSave->BeginUnit(_T("BearingLine"),1.0);
            pStrSave->put_Property(_T("ReactionType"),CComVariant(GetBearingReactionType(m_SavePierID,brgLineIdx)));
          
            IndexType nBearings = GetBearingCount(m_SavePierID,brgLineIdx);
            for ( IndexType brgIdx = 0; brgIdx < nBearings; brgIdx++ )
            {
               Float64 DC,DW,CR,SH,PS,RE,W;
               GetBearingReactions(m_SavePierID,brgLineIdx,brgIdx,&DC,&DW,&CR,&SH,&PS,&RE,&W);
               pStrSave->BeginUnit(_T("Reaction"),1.0);
                  pStrSave->put_Property(_T("DC"),CComVariant(DC));
                  pStrSave->put_Property(_T("DW"),CComVariant(DW));
                  pStrSave->put_Property(_T("CR"),CComVariant(CR));
                  pStrSave->put_Property(_T("SH"),CComVariant(SH));
                  pStrSave->put_Property(_T("PS"),CComVariant(PS));
                  pStrSave->put_Property(_T("RE"),CComVariant(RE));
                  pStrSave->put_Property(_T("W"), CComVariant(W));
               pStrSave->EndUnit(); // Reaction
            }
            pStrSave->EndUnit(); // BearingLine
         }
         pStrSave->EndUnit(); // DeadLoad

         pStrSave->BeginUnit(_T("LiveLoad"),2.0);
            pStrSave->put_Property(_T("ReactionLoadApplication"),CComVariant(GetReactionLoadApplicationType(m_SavePierID)));
            pStrSave->BeginUnit(_T("Design_Inventory"),1.0);

            std::vector<xbrLiveLoadReactionData> vReactions = GetLiveLoadReactions(m_SavePierID,pgsTypes::lrDesign_Inventory);
            for (const auto& llReaction : vReactions)
            {
               if ( llReaction.Name != NO_LIVE_LOAD_DEFINED )
               {
                  pStrSave->BeginUnit(_T("Reaction"),1.0);
                     pStrSave->put_Property(_T("Name"),CComVariant(llReaction.Name.c_str()));
                     pStrSave->put_Property(_T("LLIM"),CComVariant(llReaction.LLIM));
                     pStrSave->put_Property(_T("W"),CComVariant(llReaction.W));
                  pStrSave->EndUnit(); // Reaction
               }
            }
            pStrSave->EndUnit(); // Design_Inventory

            pStrSave->BeginUnit(_T("Design_Operating"),1.0);
            vReactions = GetLiveLoadReactions(m_SavePierID,pgsTypes::lrDesign_Operating);
            for (const auto& llReaction : vReactions)
            {
               if ( llReaction.Name != NO_LIVE_LOAD_DEFINED )
               {
                  pStrSave->BeginUnit(_T("Reaction"),1.0);
                     pStrSave->put_Property(_T("Name"),CComVariant(llReaction.Name.c_str()));
                     pStrSave->put_Property(_T("LLIM"),CComVariant(llReaction.LLIM));
                     pStrSave->put_Property(_T("W"),CComVariant(llReaction.W));
                  pStrSave->EndUnit(); // Reaction
               }
            }
            pStrSave->EndUnit(); // Design_Operating

            pStrSave->BeginUnit(_T("Legal_Routine"),1.0);
            vReactions = GetLiveLoadReactions(m_SavePierID,pgsTypes::lrLegal_Routine);
            for (const auto& llReaction : vReactions)
            {
               if ( llReaction.Name != NO_LIVE_LOAD_DEFINED )
               {
                  pStrSave->BeginUnit(_T("Reaction"),1.0);
                     pStrSave->put_Property(_T("Name"),CComVariant(llReaction.Name.c_str()));
                     pStrSave->put_Property(_T("LLIM"),CComVariant(llReaction.LLIM));
                     pStrSave->put_Property(_T("W"),CComVariant(llReaction.W));
                  pStrSave->EndUnit(); // Reaction
               }
            }
            pStrSave->EndUnit(); // Legal_Routine

            pStrSave->BeginUnit(_T("Legal_Special"),1.0);
            vReactions = GetLiveLoadReactions(m_SavePierID,pgsTypes::lrLegal_Special);
            for (const auto& llReaction : vReactions)
            {
               if ( llReaction.Name != NO_LIVE_LOAD_DEFINED )
               {
                  pStrSave->BeginUnit(_T("Reaction"),1.0);
                     pStrSave->put_Property(_T("Name"),CComVariant(llReaction.Name.c_str()));
                     pStrSave->put_Property(_T("LLIM"),CComVariant(llReaction.LLIM));
                     pStrSave->put_Property(_T("W"),CComVariant(llReaction.W));
                  pStrSave->EndUnit(); // Reaction
               }
            }
            pStrSave->EndUnit(); // Legal_Special

            // added in version 2
            pStrSave->BeginUnit(_T("Legal_Emergency"), 1.0);
            vReactions = GetLiveLoadReactions(m_SavePierID, pgsTypes::lrLegal_Emergency);
            for (const auto& llReaction : vReactions)
            {
               if (llReaction.Name != NO_LIVE_LOAD_DEFINED)
               {
                  pStrSave->BeginUnit(_T("Reaction"), 1.0);
                  pStrSave->put_Property(_T("Name"), CComVariant(llReaction.Name.c_str()));
                  pStrSave->put_Property(_T("LLIM"), CComVariant(llReaction.LLIM));
                  pStrSave->put_Property(_T("W"), CComVariant(llReaction.W));
                  pStrSave->EndUnit(); // Reaction
               }
            }
            pStrSave->EndUnit(); // Legal_Emergency

            pStrSave->BeginUnit(_T("Permit_Routine"),1.0);
            vReactions = GetLiveLoadReactions(m_SavePierID,pgsTypes::lrPermit_Routine);
            for (const auto& llReaction : vReactions)
            {
               if ( llReaction.Name != NO_LIVE_LOAD_DEFINED )
               {
                  pStrSave->BeginUnit(_T("Reaction"),1.0);
                     pStrSave->put_Property(_T("Name"),CComVariant(llReaction.Name.c_str()));
                     pStrSave->put_Property(_T("LLIM"),CComVariant(llReaction.LLIM));
                     pStrSave->put_Property(_T("W"),CComVariant(llReaction.W));
                  pStrSave->EndUnit(); // Reaction
               }
            }
            pStrSave->EndUnit(); // Permit_Routine

            pStrSave->BeginUnit(_T("Permit_Special"),1.0);
            vReactions = GetLiveLoadReactions(m_SavePierID,pgsTypes::lrPermit_Special);
            for (const auto& llReaction : vReactions)
            {
               if ( llReaction.Name != NO_LIVE_LOAD_DEFINED )
               {
                  pStrSave->BeginUnit(_T("Reaction"),1.0);
                     pStrSave->put_Property(_T("Name"),CComVariant(llReaction.Name.c_str()));
                     pStrSave->put_Property(_T("LLIM"),CComVariant(llReaction.LLIM));
                     pStrSave->put_Property(_T("W"),CComVariant(llReaction.W));
                  pStrSave->EndUnit(); // Reaction
               }
            }
            pStrSave->EndUnit(); // Permit_Special

         pStrSave->EndUnit(); // LiveLoad
      pStrSave->EndUnit(); // Reactions

      // Save the pier description information
      if (m_bExportingModel )
      {
         // the pier model we export can't have a profile defined deck... we have to use
         // the simplified model (XBRate stand alone UI doesn't support complex deck profiles)
         xbrPierData pierData = m_PierData[m_SavePierID]; // copy the pier data we want to save because we are going to tweak it
         pierData.SetID(INVALID_ID);
         pierData.SetDeckSurfaceType(xbrPierData::Simplified); // we must use a simplfied deck surface model

         GET_IFACE(IBridgeDescription, pIBridgeDesc);
         const auto* pPier = pIBridgeDesc->FindPier(m_SavePierID);
         PierIndexType pierIdx = pPier->GetIndex();

         // update the deck surface model
         GET_IFACE(IBridge, pBridge);
         Float64 pierStation = pBridge->GetPierStation(pierIdx);
         GET_IFACE(IRoadway, pAlignment);
         Float64 elev = pAlignment->GetElevation(pierStation,0.0);
         pierData.SetDeckElevation(elev);
         IndexType ctrnpt = pAlignment->GetAlignmentPointIndex(pierStation);
         pierData.SetCrownPointOffset(pAlignment->GetAlignmentOffset(ctrnpt,pierStation));

         // ok, save it
         pierData.Save(pStrSave, nullptr);
      }
      else
      {
         m_PierData[m_SavePierID].Save(pStrSave,nullptr);
      }

      pStrSave->EndUnit(); // ProjectData
   } // end if bIsStandAlone or bExportingModel
   else
   {
      pStrSave->BeginUnit(_T("RatingSpecification"),4.0);
         pStrSave->put_Property(_T("AnalysisType"),CComVariant(m_AnalysisType));
         pStrSave->put_Property(_T("EmergencyRatingMethod"), CComVariant(m_EmergencyRatingMethod)); // added in version 2
         pStrSave->put_Property(_T("PermitRatingMethod"), CComVariant(m_PermitRatingMethod));
         pStrSave->put_Property(_T("MaxLiveLoadStepSize"),CComVariant(m_MaxLLStepSize));
         pStrSave->put_Property(_T("MaxLoadedLanes"),CComVariant(m_MaxLoadedLanes));
         pStrSave->put_Property(_T("SystemFactorFlexure"), CComVariant(m_SysFactorFlexure)); // added in version 3
         pStrSave->put_Property(_T("SystemFactorShear"), CComVariant(m_SysFactorShear)); // added in version 3
         pStrSave->put_Property(_T("DoAnalyzeNegativeMomentBetweenFOC"), CComVariant(m_bDoAnalyzeNegativeMomentBetweenFOC)); // added in version 4
         pStrSave->put_Property(_T("MinColumnWidthForNegMoment"), CComVariant(m_MinColumnWidthForNegMoment)); // added in version 4
         pStrSave->EndUnit(); // RatingSpecification

      std::map<PierIDType,xbrPierData>::iterator iter(m_PierData.begin());
      std::map<PierIDType,xbrPierData>::iterator end(m_PierData.end());
      for ( ; iter != end; iter++ )
      {
         if (iter->first == INVALID_ID )
         {
            continue; // this is the data for the stand alone case... skip it
         }

         xbrPierData& pierData(iter->second);
         ATLASSERT(pierData.GetID() != INVALID_ID);
         pierData.Save(pStrSave,nullptr);
      }
   }

   return true;
}

WBFL::EAF::Broker::LoadResult CProjectAgentImp::Load(WBFL::System::IStructuredLoad* pNativeStrLoad)
{
   // The internal load pipeline below is still COM-typed; unwrap back to the original COM
   // pointer here until that call graph is migrated too. See WBFLTools\StructuredStorageComAdapter.h.
   auto* pAdapter = dynamic_cast<CStructuredLoad*>(pNativeStrLoad);
   ATLASSERT(pAdapter);
   ::IStructuredLoad* pStrLoad = pAdapter->GetComInterface();

   USES_CONVERSION;
   CHRException hr;
   CComVariant var;

   bool bIsStandAlone = IsStandAlone();

   try
   {
      if ( bIsStandAlone )
      {
         hr = pStrLoad->BeginUnit(_T("ProjectData"));

         {
            hr = pStrLoad->BeginUnit(_T("ProjectProperties"));
            var.vt = VT_BSTR;
            hr = pStrLoad->get_Property(_T("BridgeName"), &var);
            m_strBridgeName = OLE2T(var.bstrVal);

            hr = pStrLoad->get_Property(_T("BridgeId"),&var);
            m_strBridgeId = OLE2T(var.bstrVal);

            var.vt = VT_BSTR;
            hr = pStrLoad->get_Property(_T("JobNumber"),&var);
            m_strJobNumber = OLE2T(var.bstrVal);

            hr = pStrLoad->get_Property(_T("Engineer"),&var);
            m_strEngineer = OLE2T(var.bstrVal);

            hr = pStrLoad->get_Property(_T("Company"),&var);
            m_strCompany = OLE2T(var.bstrVal);

            hr = pStrLoad->get_Property(_T("Comments"),&var);
            m_strComments = OLE2T(var.bstrVal);

            hr = pStrLoad->EndUnit(); // ProjectProperties
         }

         {
            hr = pStrLoad->BeginUnit(_T("ProjectSettings"));

            var.vt = VT_I4;
            hr = pStrLoad->get_Property(_T("Units"),&var);

            GET_IFACE(IEAFDisplayUnits,pDisplayUnits);
            WBFL::EAF::UnitMode unitMode = (WBFL::EAF::UnitMode)(var.iVal);
            pDisplayUnits->SetUnitMode(unitMode);

            hr = pStrLoad->EndUnit(); // ProjectSettings
         }



         {
            hr = pStrLoad->BeginUnit(_T("RatingSpecification"));
            Float64 version;
            pStrLoad->get_Version(&version);

            var.vt = VT_BSTR;
            pStrLoad->get_Property(_T("LRFD"),&var);
            WBFL::LRFD::BDSManager::Edition lrfdEdition = WBFL::LRFD::BDSManager::GetEdition(OLE2T(var.bstrVal));
            WBFL::LRFD::BDSManager::SetEdition(lrfdEdition);

            var.vt = VT_BSTR;
            pStrLoad->get_Property(_T("LRFR"),&var);
            WBFL::LRFD::MBEManager::Edition lrfrEdition = WBFL::LRFD::MBEManager::GetEdition(OLE2T(var.bstrVal));
            WBFL::LRFD::MBEManager::SetEdition(lrfrEdition);

            if (2 < version)
            {
               // added in version 3
               var.vt = VT_I4;
               hr = pStrLoad->get_Property(_T("EmergencyRatingMethod"), &var);
               m_EmergencyRatingMethod = (xbrTypes::EmergencyRatingMethod)var.lVal;
            }

            var.vt = VT_I4;
            hr = pStrLoad->get_Property(_T("PermitRatingMethod"),&var);
            m_PermitRatingMethod = (xbrTypes::PermitRatingMethod)var.lVal;

            var.vt = VT_R8;
            hr = pStrLoad->get_Property(_T("SystemFactorFlexure"),&var);
            m_SysFactorFlexure = var.dblVal;

            hr = pStrLoad->get_Property(_T("SystemFactorShear"),&var);
            m_SysFactorShear = var.dblVal;

            hr = pStrLoad->get_Property(_T("PhiC"),&var);
            m_PhiC = var.dblVal;

            hr = pStrLoad->get_Property(_T("PhiT"),&var);
            m_PhiT = var.dblVal;

            hr = pStrLoad->get_Property(_T("PhiV"),&var);
            m_PhiV = var.dblVal;

            var.vt = VT_R8;
            hr = pStrLoad->get_Property(_T("MaxLiveLoadStepSize"),&var);
            m_MaxLLStepSize = var.dblVal;

            var.vt = VT_INDEX;
            hr = pStrLoad->get_Property(_T("MaxLoadedLanes"),&var);
            m_MaxLoadedLanes = VARIANT2INDEX(var);

            var.vt = VT_BOOL;
            hr = pStrLoad->get_Property(_T("RatingEnabled_Design_Inventory"),&var);
            m_bRatingEnabled[pgsTypes::lrDesign_Inventory] = (var.boolVal == VARIANT_TRUE ? true : false);

            hr = pStrLoad->get_Property(_T("RatingEnabled_Design_Operating"),&var);
            m_bRatingEnabled[pgsTypes::lrDesign_Operating] = (var.boolVal == VARIANT_TRUE ? true : false);

            hr = pStrLoad->get_Property(_T("RatingEnabled_Legal_Routine"),&var);
            m_bRatingEnabled[pgsTypes::lrLegal_Routine] = (var.boolVal == VARIANT_TRUE ? true : false);

            hr = pStrLoad->get_Property(_T("RatingEnabled_Legal_Special"),&var);
            m_bRatingEnabled[pgsTypes::lrLegal_Special] = (var.boolVal == VARIANT_TRUE ? true : false);

            if (1 < version)
            {
               // added in version 2
               hr = pStrLoad->get_Property(_T("RatingEnabled_Legal_Emergency"), &var);
               m_bRatingEnabled[pgsTypes::lrLegal_Emergency] = (var.boolVal == VARIANT_TRUE ? true : false);
            }

            hr = pStrLoad->get_Property(_T("RatingEnabled_Permit_Routine"),&var);
            m_bRatingEnabled[pgsTypes::lrPermit_Routine] = (var.boolVal == VARIANT_TRUE ? true : false);

            hr = pStrLoad->get_Property(_T("RatingEnabled_Permit_Special"),&var);
            m_bRatingEnabled[pgsTypes::lrPermit_Special] = (var.boolVal == VARIANT_TRUE ? true : false);

            // XBRate doesn't enable/disable the same as PGS, however we've match the input parameters
            // make sure the pairing of design, legal, and permit ratings match. If the values aren't
            // equal, one is enabled, so enable both
            if ( m_bRatingEnabled[pgsTypes::lrDesign_Inventory] != m_bRatingEnabled[pgsTypes::lrDesign_Operating] )
            {
               m_bRatingEnabled[pgsTypes::lrDesign_Inventory] = true;
               m_bRatingEnabled[pgsTypes::lrDesign_Operating] = true;
            }

            if ( m_bRatingEnabled[pgsTypes::lrLegal_Routine] != m_bRatingEnabled[pgsTypes::lrLegal_Special] )
            {
               m_bRatingEnabled[pgsTypes::lrLegal_Routine] = true;
               m_bRatingEnabled[pgsTypes::lrLegal_Special] = true;
            }

            if ( m_bRatingEnabled[pgsTypes::lrPermit_Routine] != m_bRatingEnabled[pgsTypes::lrPermit_Special] )
            {
               m_bRatingEnabled[pgsTypes::lrPermit_Routine] = true;
               m_bRatingEnabled[pgsTypes::lrPermit_Special] = true;
            }

            var.vt = VT_BOOL;
            hr = pStrLoad->get_Property(_T("RateForShear_Design_Inventory"),&var);
            m_bRateForShear[pgsTypes::lrDesign_Inventory] = (var.boolVal == VARIANT_TRUE ? true : false);

            hr = pStrLoad->get_Property(_T("RateForShear_Design_Operating"),&var);
            m_bRateForShear[pgsTypes::lrDesign_Operating] = (var.boolVal == VARIANT_TRUE ? true : false);

            hr = pStrLoad->get_Property(_T("RateForShear_Legal_Routine"),&var);
            m_bRateForShear[pgsTypes::lrLegal_Routine] = (var.boolVal == VARIANT_TRUE ? true : false);

            hr = pStrLoad->get_Property(_T("RateForShear_Legal_Special"),&var);
            m_bRateForShear[pgsTypes::lrLegal_Special] = (var.boolVal == VARIANT_TRUE ? true : false);

            if (1 < version)
            {
               // added in version 2
               hr = pStrLoad->get_Property(_T("RateForShear_Legal_Emergency"), &var);
               m_bRateForShear[pgsTypes::lrLegal_Emergency] = (var.boolVal == VARIANT_TRUE ? true : false);
            }

            hr = pStrLoad->get_Property(_T("RateForShear_Permit_Routine"),&var);
            m_bRateForShear[pgsTypes::lrPermit_Routine] = (var.boolVal == VARIANT_TRUE ? true : false);

            hr = pStrLoad->get_Property(_T("RateForShear_Permit_Special"),&var);
            m_bRateForShear[pgsTypes::lrPermit_Special] = (var.boolVal == VARIANT_TRUE ? true : false);

            var.vt = VT_BOOL;
            hr = pStrLoad->get_Property(_T("CheckYieldStressLimit"), &var);
            m_bCheckYieldStress = (var.boolVal == VARIANT_TRUE ? true : false);

            var.vt = VT_R8;
            hr = pStrLoad->get_Property(_T("YieldStressCoefficient"), &var);
            m_YieldStressCoefficient = var.dblVal;

            if (3 < version)
            {
               var.vt = VT_BOOL;
               hr = pStrLoad->get_Property(_T("DoAnalyzeNegativeMomentBetweenFOC"),&var);
               m_bDoAnalyzeNegativeMomentBetweenFOC = (var.boolVal == VARIANT_TRUE ? true : false);

               var.vt = VT_R8;
               hr = pStrLoad->get_Property(_T("MinColumnWidthForNegMoment"),&var);
               m_MinColumnWidthForNegMoment = var.dblVal;
            }

            hr = pStrLoad->EndUnit(); // RatingSpecification
         }

         {
            hr = pStrLoad->BeginUnit(_T("LoadFactors"));

            Float64 version;
            pStrLoad->get_Version(&version);
            
            var.vt = VT_R8;
            hr = pStrLoad->get_Property(_T("DC_StrengthI"),&var);
            m_gDC_StrengthI = var.dblVal;

            hr = pStrLoad->get_Property(_T("DW_StrengthI"),&var);
            m_gDW_StrengthI = var.dblVal;

            hr = pStrLoad->get_Property(_T("CR_StrengthI"),&var);
            m_gCR_StrengthI = var.dblVal;

            hr = pStrLoad->get_Property(_T("SH_StrengthI"),&var);
            m_gSH_StrengthI = var.dblVal;

            hr = pStrLoad->get_Property(_T("PS_StrengthI"),&var);
            m_gPS_StrengthI = var.dblVal;

            hr = pStrLoad->get_Property(_T("RE_StrengthI"),&var);
            m_gRE_StrengthI = var.dblVal;

            
            var.vt = VT_R8;
            hr = pStrLoad->get_Property(_T("DC_StrengthII"),&var);
            m_gDC_StrengthII = var.dblVal;

            hr = pStrLoad->get_Property(_T("DW_StrengthII"),&var);
            m_gDW_StrengthII = var.dblVal;

            hr = pStrLoad->get_Property(_T("CR_StrengthII"),&var);
            m_gCR_StrengthII = var.dblVal;

            hr = pStrLoad->get_Property(_T("SH_StrengthII"),&var);
            m_gSH_StrengthII = var.dblVal;

            hr = pStrLoad->get_Property(_T("PS_StrengthII"),&var);
            m_gPS_StrengthII = var.dblVal;

            hr = pStrLoad->get_Property(_T("RE_StrengthII"),&var);
            m_gRE_StrengthII = var.dblVal;

             
            var.vt = VT_R8;
            hr = pStrLoad->get_Property(_T("DC_ServiceI"),&var);
            m_gDC_ServiceI = var.dblVal;

            hr = pStrLoad->get_Property(_T("DW_ServiceI"),&var);
            m_gDW_ServiceI = var.dblVal;

            hr = pStrLoad->get_Property(_T("CR_ServiceI"),&var);
            m_gCR_ServiceI = var.dblVal;

            hr = pStrLoad->get_Property(_T("SH_ServiceI"),&var);
            m_gSH_ServiceI = var.dblVal;

            hr = pStrLoad->get_Property(_T("PS_ServiceI"),&var);
            m_gPS_ServiceI = var.dblVal;

            hr = pStrLoad->get_Property(_T("RE_ServiceI"),&var);
            m_gRE_ServiceI = var.dblVal;

            hr = pStrLoad->get_Property(_T("LL_StrengthI_Inventory"),&var);
            m_gLL[GET_INDEX(pgsTypes::StrengthI_Inventory)][INVALID_ID] = var.dblVal;

            hr = pStrLoad->get_Property(_T("LL_StrengthI_Operating"),&var);
            m_gLL[GET_INDEX(pgsTypes::StrengthI_Operating)][INVALID_ID] = var.dblVal;

            hr = pStrLoad->get_Property(_T("LL_StrengthI_LegalRoutine"),&var);
            m_gLL[GET_INDEX(pgsTypes::StrengthI_LegalRoutine)][INVALID_ID] = var.dblVal;

            hr = pStrLoad->get_Property(_T("LL_StrengthI_LegalSpecial"),&var);
            m_gLL[GET_INDEX(pgsTypes::StrengthI_LegalSpecial)][INVALID_ID] = var.dblVal;

            if (1 < version)
            {
               hr = pStrLoad->get_Property(_T("LL_StrengthI_LegalEmergency"), &var);
               m_gLL[GET_INDEX(pgsTypes::StrengthI_LegalEmergency)][INVALID_ID] = var.dblVal;
            }

            hr = pStrLoad->get_Property(_T("LL_StrengthII_PermitRoutine"),&var);
            m_gLL[GET_INDEX(pgsTypes::StrengthII_PermitRoutine)][INVALID_ID] = var.dblVal;

            hr = pStrLoad->get_Property(_T("LL_StrengthII_PermitSpecial"),&var);
            m_gLL[GET_INDEX(pgsTypes::StrengthII_PermitSpecial)][INVALID_ID] = var.dblVal;

            hr = pStrLoad->get_Property(_T("LL_ServiceI_PermitRoutine"),&var);
            m_gLL[GET_INDEX(pgsTypes::ServiceI_PermitRoutine)][INVALID_ID] = var.dblVal;

            hr = pStrLoad->get_Property(_T("LL_ServiceI_PermitSpecial"),&var);
            m_gLL[GET_INDEX(pgsTypes::ServiceI_PermitSpecial)][INVALID_ID] = var.dblVal;

            hr = pStrLoad->EndUnit(); // LoadFactors
         }

         {
            hr = pStrLoad->BeginUnit(_T("Reactions"));
            {
               {
                  hr = pStrLoad->BeginUnit(_T("DeadLoad"));
                  m_BearingReactions[0].clear();
                  m_BearingReactions[1].clear();
                  IndexType brgLineIdx = 0;
                  while ( SUCCEEDED(pStrLoad->BeginUnit(_T("BearingLine"))) )
                  {
                     var.vt = VT_I4;
                     hr = pStrLoad->get_Property(_T("ReactionType"),&var);
                     m_BearingReactionType[brgLineIdx][INVALID_ID] = (xbrTypes::ReactionLoadType)var.lVal;
                     std::vector<BearingReactions> vBearingReactions;
                     while( SUCCEEDED(pStrLoad->BeginUnit(_T("Reaction"))) )
                     {
                        BearingReactions reaction;
                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("DC"),&var);
                        reaction.DC = var.dblVal;

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("DW"),&var);
                        reaction.DW = var.dblVal;

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("CR"),&var);
                        reaction.CR = var.dblVal;

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("SH"),&var);
                        reaction.SH = var.dblVal;

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("PS"),&var);
                        reaction.PS = var.dblVal;

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("RE"),&var);
                        reaction.RE = var.dblVal;

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("W"),&var);
                        reaction.W = var.dblVal;

                        vBearingReactions.push_back(reaction);
                        hr = pStrLoad->EndUnit(); // Reaction
                     }

                     m_BearingReactions[brgLineIdx].insert(std::make_pair(INVALID_ID,vBearingReactions));
                     brgLineIdx++;
                     hr = pStrLoad->EndUnit(); // BearingLine
                  }
                  hr = pStrLoad->EndUnit(); // DeadLoad
               }

               {
                  hr = pStrLoad->BeginUnit(_T("LiveLoad"));
                  {
                     Float64 version;
                     pStrLoad->get_Version(&version);

                     var.vt = VT_I4;
                     hr = pStrLoad->get_Property(_T("ReactionLoadApplication"),&var);
                     GetPrivateReactionLoadApplication(INVALID_ID) = (xbrTypes::ReactionLoadApplicationType)var.iVal;

                     hr = pStrLoad->BeginUnit(_T("Design_Inventory"));
                     m_LiveLoadReactions[pgsTypes::lrDesign_Inventory][INVALID_ID].clear();
                     while ( SUCCEEDED(pStrLoad->BeginUnit(_T("Reaction"))) )
                     {
                        xbrLiveLoadReactionData llReaction;
                        var.vt = VT_BSTR;
                        hr = pStrLoad->get_Property(_T("Name"),&var);
                        llReaction.Name = OLE2T(var.bstrVal);

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("LLIM"),&var);
                        llReaction.LLIM = var.dblVal;

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("W"),&var);
                        llReaction.W = var.dblVal;

                        m_LiveLoadReactions[pgsTypes::lrDesign_Inventory][INVALID_ID].push_back(llReaction);
                        hr = pStrLoad->EndUnit(); // Reaction
                     }
                     hr = pStrLoad->EndUnit(); // Design_Inventory

                     hr = pStrLoad->BeginUnit(_T("Design_Operating"));
                     m_LiveLoadReactions[pgsTypes::lrDesign_Operating][INVALID_ID].clear();
                     while ( SUCCEEDED(pStrLoad->BeginUnit(_T("Reaction"))) )
                     {
                        xbrLiveLoadReactionData llReaction;
                        var.vt = VT_BSTR;
                        hr = pStrLoad->get_Property(_T("Name"),&var);
                        llReaction.Name = OLE2T(var.bstrVal);

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("LLIM"),&var);
                        llReaction.LLIM = var.dblVal;

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("W"),&var);
                        llReaction.W = var.dblVal;

                        m_LiveLoadReactions[pgsTypes::lrDesign_Operating][INVALID_ID].push_back(llReaction);
                        hr = pStrLoad->EndUnit(); // Reaction
                     }
                     hr = pStrLoad->EndUnit(); // Design_Operating

                     hr = pStrLoad->BeginUnit(_T("Legal_Routine"));
                     m_LiveLoadReactions[pgsTypes::lrLegal_Routine][INVALID_ID].clear();
                     while ( SUCCEEDED(pStrLoad->BeginUnit(_T("Reaction"))) )
                     {
                        xbrLiveLoadReactionData llReaction;
                        var.vt = VT_BSTR;
                        hr = pStrLoad->get_Property(_T("Name"),&var);
                        llReaction.Name = OLE2T(var.bstrVal);

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("LLIM"),&var);
                        llReaction.LLIM = var.dblVal;

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("W"),&var);
                        llReaction.W = var.dblVal;

                        m_LiveLoadReactions[pgsTypes::lrLegal_Routine][INVALID_ID].push_back(llReaction);
                        hr = pStrLoad->EndUnit(); // Reaction
                     }
                     hr = pStrLoad->EndUnit(); // Legal_Routine

                     hr = pStrLoad->BeginUnit(_T("Legal_Special"));
                     m_LiveLoadReactions[pgsTypes::lrLegal_Special][INVALID_ID].clear();
                     while ( SUCCEEDED(pStrLoad->BeginUnit(_T("Reaction"))) )
                     {
                        xbrLiveLoadReactionData llReaction;
                        var.vt = VT_BSTR;
                        hr = pStrLoad->get_Property(_T("Name"),&var);
                        llReaction.Name = OLE2T(var.bstrVal);

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("LLIM"),&var);
                        llReaction.LLIM = var.dblVal;

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("W"),&var);
                        llReaction.W = var.dblVal;

                        m_LiveLoadReactions[pgsTypes::lrLegal_Special][INVALID_ID].push_back(llReaction);
                        hr = pStrLoad->EndUnit(); // Reaction
                     }
                     hr = pStrLoad->EndUnit(); // Legal_Special

                     if (1 < version)
                     {
                        // added in version 2
                        hr = pStrLoad->BeginUnit(_T("Legal_Emergency"));
                        m_LiveLoadReactions[pgsTypes::lrLegal_Emergency][INVALID_ID].clear();
                        while (SUCCEEDED(pStrLoad->BeginUnit(_T("Reaction"))))
                        {
                           xbrLiveLoadReactionData llReaction;
                           var.vt = VT_BSTR;
                           hr = pStrLoad->get_Property(_T("Name"), &var);
                           llReaction.Name = OLE2T(var.bstrVal);

                           var.vt = VT_R8;
                           hr = pStrLoad->get_Property(_T("LLIM"), &var);
                           llReaction.LLIM = var.dblVal;

                           var.vt = VT_R8;
                           hr = pStrLoad->get_Property(_T("W"), &var);
                           llReaction.W = var.dblVal;

                           m_LiveLoadReactions[pgsTypes::lrLegal_Emergency][INVALID_ID].push_back(llReaction);
                           hr = pStrLoad->EndUnit(); // Reaction
                        }
                        hr = pStrLoad->EndUnit(); // Legal_Emergency
                     }

                     hr = pStrLoad->BeginUnit(_T("Permit_Routine"));
                     m_LiveLoadReactions[pgsTypes::lrPermit_Routine][INVALID_ID].clear();
                     while ( SUCCEEDED(pStrLoad->BeginUnit(_T("Reaction"))) )
                     {
                        xbrLiveLoadReactionData llReaction;
                        var.vt = VT_BSTR;
                        hr = pStrLoad->get_Property(_T("Name"),&var);
                        llReaction.Name = OLE2T(var.bstrVal);

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("LLIM"),&var);
                        llReaction.LLIM = var.dblVal;

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("W"),&var);
                        llReaction.W = var.dblVal;

                        m_LiveLoadReactions[pgsTypes::lrPermit_Routine][INVALID_ID].push_back(llReaction);
                        hr = pStrLoad->EndUnit(); // Reaction
                     }
                     hr = pStrLoad->EndUnit(); // Permit_Routine

                     hr = pStrLoad->BeginUnit(_T("Permit_Special"));
                     m_LiveLoadReactions[pgsTypes::lrPermit_Special][INVALID_ID].clear();
                     while ( SUCCEEDED(pStrLoad->BeginUnit(_T("Reaction"))) )
                     {
                        xbrLiveLoadReactionData llReaction;
                        var.vt = VT_BSTR;
                        hr = pStrLoad->get_Property(_T("Name"),&var);
                        llReaction.Name = OLE2T(var.bstrVal);

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("LLIM"),&var);
                        llReaction.LLIM = var.dblVal;

                        var.vt = VT_R8;
                        hr = pStrLoad->get_Property(_T("W"),&var);
                        llReaction.W = var.dblVal;

                        m_LiveLoadReactions[pgsTypes::lrPermit_Special][INVALID_ID].push_back(llReaction);
                        hr = pStrLoad->EndUnit(); // Reaction
                     }
                     hr = pStrLoad->EndUnit(); // Permit_Special
                  }
                  hr = pStrLoad->EndUnit(); // LiveLoad
               }
            }
            hr = pStrLoad->EndUnit(); // Reactions
         }

         // Load the basic pier information
         hr = m_PierData[INVALID_ID].Load(pStrLoad,nullptr);
         ATLASSERT(m_PierData[INVALID_ID].GetID() == INVALID_ID);

         hr = pStrLoad->EndUnit(); // ProjectData
      } // end if bIsStandAlone
      else
      {
         hr = pStrLoad->BeginUnit(_T("RatingSpecification"));

         Float64 version;
         pStrLoad->get_Version(&version);
         
         var.vt = VT_I4;
         hr = pStrLoad->get_Property(_T("AnalysisType"),&var);
         m_AnalysisType = (pgsTypes::AnalysisType)var.lVal;

         if (1 < version)
         {
            // added in version 2
            var.vt = VT_I4;
            hr = pStrLoad->get_Property(_T("EmergencyRatingMethod"), &var);
            m_EmergencyRatingMethod = (xbrTypes::EmergencyRatingMethod)var.lVal;
         }

         var.vt = VT_I4;
         hr = pStrLoad->get_Property(_T("PermitRatingMethod"),&var);
         m_PermitRatingMethod = (xbrTypes::PermitRatingMethod)var.lVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("MaxLiveLoadStepSize"),&var);
         m_MaxLLStepSize = var.dblVal;

         var.vt = VT_INDEX;
         hr = pStrLoad->get_Property(_T("MaxLoadedLanes"),&var);
         m_MaxLoadedLanes = VARIANT2INDEX(var);

         if (2 < version)
         {
            // added in version 3
            var.vt = VT_R8;
            hr = pStrLoad->get_Property(_T("SystemFactorFlexure"), &var);
            m_SysFactorFlexure = var.dblVal;

            hr = pStrLoad->get_Property(_T("SystemFactorShear"), &var);
            m_SysFactorShear = var.dblVal;
         }

         if (3 < version)
         {
            var.vt = VT_BOOL;
            hr = pStrLoad->get_Property(_T("DoAnalyzeNegativeMomentBetweenFOC"),&var);
            m_bDoAnalyzeNegativeMomentBetweenFOC = (var.boolVal == VARIANT_TRUE ? true : false);

            var.vt = VT_R8;
            hr = pStrLoad->get_Property(_T("MinColumnWidthForNegMoment"),&var);
            m_MinColumnWidthForNegMoment = var.dblVal;
         }

         hr = pStrLoad->EndUnit(); // RatingSpecification

         xbrPierData pierData;
         while ( SUCCEEDED(pierData.Load(pStrLoad,nullptr)) )
         {
            PierIDType pierID = pierData.GetID();
            ATLASSERT(pierID != INVALID_ID);
            xbrPierData& myPierData = GetPrivatePierData(pierID);
            myPierData = pierData;
         }
      }
   }
   catch (HRESULT)
   {
      THROW_LOAD(InvalidFileFormat,pStrLoad);
   }

   // The first release of the software permitted single column piers to have a pinned fixity at the base of the column.
   // This resulted in unstable models. Detected single column pier models here and ensure the fixity is a proper value
   for (auto& value : m_PierData)
   {
      auto& pierData = value.second;
      if (pierData.GetColumnCount() == 1)
      {
         auto& column = pierData.GetColumnData(0);
         pgsTypes::ColumnTransverseFixityType fixity = column.GetTransverseFixity();
         if (fixity == pgsTypes::ctftTopFixedBottomPinned)
         {
            column.SetTransverseFixity(pgsTypes::ctftTopFixedBottomFixed);

            CString strMsg(_T("Single column piers cannot have a pinned fixity. The support fixity has been changed to Fixed"));

            GET_IFACE(IEAFStatusCenter, pStatusCenter);
            pStatusCenter->Add(std::make_shared<xbrBridgeStatusItem>(m_XBeamRateStatusGroupID, m_scidBridgeInfo, strMsg));
         }
      }
   }


   if ( !bIsStandAlone )
   {
      // since reactions are stored/loaded, need to resize the bearing reaction structure
      // so that it is the correct size
      m_BearingReactions[0].clear();
      m_BearingReactions[1].clear();
      std::map<PierIDType,xbrPierData>::iterator iter(m_PierData.begin());
      std::map<PierIDType,xbrPierData>::iterator end(m_PierData.end());
      for ( ; iter != end; iter++ )
      {
         PierIDType pierID = iter->first;
         xbrPierData& pierData(iter->second);
         IndexType nBearingLines = pierData.GetBearingLineCount();
         for ( IndexType brgLineIdx = 0; brgLineIdx < nBearingLines; brgLineIdx++ )
         {
            std::vector<BearingReactions> vBearingReactions;
            xbrBearingLineData& brgLineData = pierData.GetBearingLineData(brgLineIdx);
            vBearingReactions.resize(brgLineData.GetBearingCount());
            m_BearingReactions[brgLineIdx].insert(std::make_pair(pierID,vBearingReactions));
         }
      }

      UpdatePiers();
   }

   return WBFL::EAF::Broker::LoadResult::Success;
}

//////////////////////////////////////////////////////////////////////
// IXBRProjectProperties
LPCTSTR CProjectAgentImp::GetBridgeName() const
{
   if ( IsStandAlone() )
   {
      return m_strBridgeName;
   }
   else
   {
      GET_IFACE(IProjectProperties,pProjectProperties);
      return pProjectProperties->GetBridgeName();
   }
}

void CProjectAgentImp::SetBridgeName(LPCTSTR name)
{
   if ( m_strBridgeName != name )
   {
      m_strBridgeName = name;
      Fire_OnProjectPropertiesChanged();
   }
}

LPCTSTR CProjectAgentImp::GetBridgeID() const
{
   if ( IsStandAlone() )
   {
      return m_strBridgeId;
   }
   else
   {
      GET_IFACE(IProjectProperties,pProjectProperties);
      return pProjectProperties->GetBridgeID();
   }
}

void CProjectAgentImp::SetBridgeID(LPCTSTR bid)
{
   if ( m_strBridgeId != bid )
   {
      m_strBridgeId = bid;
      Fire_OnProjectPropertiesChanged();
   }
}

LPCTSTR CProjectAgentImp::GetJobNumber() const
{
   if ( IsStandAlone() )
   {
      return m_strJobNumber;
   }
   else
   {
      GET_IFACE(IProjectProperties,pProjectProperties);
      return pProjectProperties->GetJobNumber();
   }
}

void CProjectAgentImp::SetJobNumber(LPCTSTR jid)
{
   if ( m_strJobNumber != jid )
   {
      m_strJobNumber = jid;
      Fire_OnProjectPropertiesChanged();
   }
}

LPCTSTR CProjectAgentImp::GetEngineer() const
{
   if ( IsStandAlone() )
   {
      return m_strEngineer;
   }
   else
   {
      GET_IFACE(IProjectProperties,pProjectProperties);
      return pProjectProperties->GetEngineer();
   }
}

void CProjectAgentImp::SetEngineer(LPCTSTR eng)
{
   if ( m_strEngineer != eng )
   {
      m_strEngineer = eng;
      Fire_OnProjectPropertiesChanged();
   }
}

LPCTSTR CProjectAgentImp::GetCompany() const
{
   if ( IsStandAlone() )
   {
      return m_strCompany;
   }
   else
   {
      GET_IFACE(IProjectProperties,pProjectProperties);
      return pProjectProperties->GetCompany();
   }
}

void CProjectAgentImp::SetCompany(LPCTSTR company)
{
   if ( m_strCompany != company )
   {
      m_strCompany = company;
      Fire_OnProjectPropertiesChanged();
   }
}

LPCTSTR CProjectAgentImp::GetComments() const
{
   if ( IsStandAlone() )
   {
      return m_strComments;
   }
   else
   {
      GET_IFACE(IProjectProperties,pProjectProperties);
      return pProjectProperties->GetComments();
   }
}

void CProjectAgentImp::SetComments(LPCTSTR comments)
{
   if ( m_strComments != comments )
   {
      m_strComments = comments;
      Fire_OnProjectPropertiesChanged();
   }
}


//////////////////////////////////////////////////////////////////////
// IXBRProject
void CProjectAgentImp::SetPierData(const xbrPierData& pierData)
{
   std::map<PierIDType,xbrPierData>::iterator found(m_PierData.find(pierData.GetID()));
   if ( found == m_PierData.end() )
   {
      ATLASSERT(pierData.GetID() != INVALID_ID);
      m_PierData.insert(std::make_pair(pierData.GetID(),pierData));
   }
   else
   {
      m_PierData[pierData.GetID()] = pierData;
   }

   // Resize the dead load reaction containers to match the number of bearings
   // NOTE: Don't get the reactions here. There is a huge computational effort
   // required to get the reactions and we don't want to do that unless the
   // reactions are actually going to be used.
   IndexType nBearingLines = pierData.GetBearingLineCount();
   for ( IndexType brgLineIdx = 0; brgLineIdx < nBearingLines; brgLineIdx++ )
   {
      std::map<PierIDType,std::vector<BearingReactions>>::iterator foundReactions(m_BearingReactions[brgLineIdx].find(pierData.GetID()));
      if ( foundReactions == m_BearingReactions[brgLineIdx].end() )
      {
         std::vector<BearingReactions> vBearingReactions;
         m_BearingReactions[brgLineIdx].insert(std::make_pair(pierData.GetID(),vBearingReactions));
         foundReactions = m_BearingReactions[brgLineIdx].find(pierData.GetID());
      }

      std::vector<BearingReactions>& vBearingReactions = foundReactions->second;
      IndexType nBearings = pierData.GetBearingCount(brgLineIdx);
      if ( vBearingReactions.size() != nBearings )
      {
         vBearingReactions.resize(nBearings);
      }
   }

   Fire_OnProjectChanged();
}

const xbrPierData& CProjectAgentImp::GetPierData(PierIDType pierID) const
{
   return GetPrivatePierData(pierID);
}

xbrTypes::PierType CProjectAgentImp::GetPierType(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetPierType();
}

void CProjectAgentImp::SetPierType(PierIDType pierID,xbrTypes::PierType pierType)
{
   GetPrivatePierData(pierID).SetPierType(pierType);
   Fire_OnProjectChanged();
}

void CProjectAgentImp::SetDeckElevation(PierIDType pierID,Float64 deckElevation)
{
   GetPrivatePierData(pierID).SetDeckElevation(deckElevation);
   Fire_OnProjectChanged();
}

Float64 CProjectAgentImp::GetDeckElevation(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetDeckElevation();
}

void CProjectAgentImp::SetDeckThickness(PierIDType pierID,Float64 tDeck)
{
   GetPrivatePierData(pierID).SetDeckThickness(tDeck);
   Fire_OnProjectChanged();
}

Float64 CProjectAgentImp::GetDeckThickness(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetDeckThickness();
}

void CProjectAgentImp::SetCrownPointOffset(PierIDType pierID,Float64 cpo)
{
   GetPrivatePierData(pierID).SetCrownPointOffset(cpo);
   Fire_OnProjectChanged();
}

Float64 CProjectAgentImp::GetCrownPointOffset(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetCrownPointOffset();
}

void CProjectAgentImp::SetBridgeLineOffset(PierIDType pierID,Float64 blo)
{
   GetPrivatePierData(pierID).SetBridgeLineOffset(blo);
   Fire_OnProjectChanged();
}

Float64 CProjectAgentImp::GetBridgeLineOffset(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetBridgeLineOffset();
}

void CProjectAgentImp::SetOrientation(PierIDType pierID,LPCTSTR strOrientation)
{
   GetPrivatePierData(pierID).SetSkew(strOrientation);
   Fire_OnProjectChanged();
}

LPCTSTR CProjectAgentImp::GetOrientation(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetSkew();
}

pgsTypes::OffsetMeasurementType CProjectAgentImp::GetCurbLineDatum(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetCurbLineDatum();
}

void CProjectAgentImp::SetCurbLineDatum(PierIDType pierID,pgsTypes::OffsetMeasurementType datumType)
{
   GetPrivatePierData(pierID).SetCurbLineDatum(datumType);
   Fire_OnProjectChanged();
}

void CProjectAgentImp::SetCurbLineOffset(PierIDType pierID,Float64 leftCLO,Float64 rightCLO)
{
   GetPrivatePierData(pierID).SetCurbLineOffset(leftCLO,rightCLO);
   Fire_OnProjectChanged();
}

void CProjectAgentImp::GetCurbLineOffset(PierIDType pierID,Float64* pLeftCLO,Float64* pRightCLO) const
{
   GetPrivatePierData(pierID).GetCurbLineOffset(pLeftCLO,pRightCLO);
}

void CProjectAgentImp::SetCrownSlopes(PierIDType pierID,Float64 sl,Float64 sr)
{
   GetPrivatePierData(pierID).SetCrownSlope(sl,sr);
   Fire_OnProjectChanged();
}

void CProjectAgentImp::GetCrownSlopes(PierIDType pierID,Float64* psl,Float64* psr) const
{
   GetPrivatePierData(pierID).GetCrownSlope(psl,psr);
}

void CProjectAgentImp::GetDiaphragmDimensions(PierIDType pierID,Float64* pH,Float64* pW) const
{
   GetPrivatePierData(pierID).GetDiaphragmDimensions(pH,pW);
}

void CProjectAgentImp::SetDiaphragmDimensions(PierIDType pierID,Float64 H,Float64 W)
{
   GetPrivatePierData(pierID).SetDiaphragmDimensions(H,W);
   Fire_OnProjectChanged();
}

IndexType CProjectAgentImp::GetBearingLineCount(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetBearingLineCount();
}

void CProjectAgentImp::SetBearingLineCount(PierIDType pierID,IndexType nBearingLines)
{
   GetPrivatePierData(pierID).SetBearingLineCount(nBearingLines);
}

IndexType CProjectAgentImp::GetBearingCount(PierIDType pierID,IndexType brgLineIdx) const
{
   return GetPrivatePierData(pierID).GetBearingCount(brgLineIdx);
}

void CProjectAgentImp::SetBearingCount(PierIDType pierID,IndexType brgLineIdx,IndexType nBearings)
{
   GetPrivatePierData(pierID).SetBearingCount(brgLineIdx,nBearings);
   Fire_OnProjectChanged();
}

Float64 CProjectAgentImp::GetBearingSpacing(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx) const
{
   return GetPrivatePierData(pierID).GetBearingSpacing(brgLineIdx,brgIdx);
}

void CProjectAgentImp::SetBearingSpacing(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx,Float64 spacing)
{
   GetPrivatePierData(pierID).SetBearingSpacing(brgLineIdx,brgIdx,spacing);
   Fire_OnProjectChanged();
}

void CProjectAgentImp::SetBearingReactionType(PierIDType pierID,IndexType brgLineIdx,xbrTypes::ReactionLoadType brgReactionType)
{
   GetPrivateBearingReactionType(pierID,brgLineIdx) = brgReactionType;
   Fire_OnProjectChanged();
}

xbrTypes::ReactionLoadType CProjectAgentImp::GetBearingReactionType(PierIDType pierID,IndexType brgLineIdx) const
{
   if ( pierID == INVALID_ID )
   {
#if defined _DEBUG
      // if pierID == INVALID_ID, then we should be doing a stand-alone analysis
      // when in stand-alone mode, IXBeamRateAgent interface isn't available
      ATLASSERT(IsStandAlone());
#endif
      return GetPrivateBearingReactionType(pierID,brgLineIdx);
   }
   else
   {
      return UseUniformLoads(pierID,brgLineIdx) ? xbrTypes::rltUniform : xbrTypes::rltConcentrated;
   }
}

void CProjectAgentImp::SetBearingReactions(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx,Float64 DC,Float64 DW,Float64 CR,Float64 SH,Float64 PS,Float64 RE,Float64 W)
{
   std::vector<BearingReactions>& vReactions = GetPrivateBearingReactions(pierID,brgLineIdx);
   vReactions[brgIdx].DC = DC;
   vReactions[brgIdx].DW = DW;
   vReactions[brgIdx].CR = CR;
   vReactions[brgIdx].SH = SH;
   vReactions[brgIdx].PS = PS;
   vReactions[brgIdx].RE = RE;
   vReactions[brgIdx].W  = W;
   Fire_OnProjectChanged();
}

void CProjectAgentImp::GetBearingReactions(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx,Float64* pDC,Float64* pDW,Float64* pCR,Float64* pSH,Float64* pPS,Float64* pRE,Float64* pW) const
{
   // Detect the configurations from PGSuper/PGSplice that we can't model
   // Throws an unwind exception if we can't model this thing
   CanModelPier(pierID,m_XBeamRateStatusGroupID,m_scidBridgeError);

   // Initialize results
   *pDC = 0;
   *pDW = 0;
   *pCR = 0;
   *pSH = 0;
   *pRE = 0;
   *pPS = 0;

   if ( pierID == INVALID_ID )
   {
#if defined _DEBUG
      // if pierID == INVALID_ID, then we should be doing a stand-alone analysis
      // when in stand-alone mode, IXBeamRateAgent interface isn't available
      ATLASSERT(IsStandAlone());
#endif

      std::vector<BearingReactions>& vReactions = GetPrivateBearingReactions(pierID,brgLineIdx);
      *pDC = vReactions[brgIdx].DC;
      *pDW = vReactions[brgIdx].DW;
      *pCR = vReactions[brgIdx].CR;
      *pSH = vReactions[brgIdx].SH;
      *pPS = vReactions[brgIdx].PS;
      *pRE = vReactions[brgIdx].RE;
      *pW  = vReactions[brgIdx].W;
   }
   else
   {
      // We are in PGSuper/PGSplice Extension mode... get the reactions
      CGirderKey girderKey = GetGirderKey(pierID,brgLineIdx,brgIdx);
      PierIndexType pierIdx = GetPierIndex(pierID);

      GET_IFACE(IIntervals,pIntervals);
      IntervalIndexType loadRatingIntervalIdx = pIntervals->GetLoadRatingInterval();

      GET_IFACE(IProductForces,pProductForces);
      pgsTypes::BridgeAnalysisType bat = pProductForces->GetBridgeAnalysisType(m_AnalysisType,pgsTypes::Maximize);
      ResultsType resultsType = rtCumulative;

      xbrPierData& pierData = GetPrivatePierData(pierID);
      if ( pierData.GetPierType() == xbrTypes::pctExpansion )
      {
         GET_IFACE(IBearingDesign,pBearingDesign);

         Float64 dc[2];
         Float64 dw[2];
         Float64 cr[2];
         Float64 sh[2];
         Float64 re[2];
         Float64 ps[2];

         ReactionLocation location[2];
         location[0].PierIdx   = pierIdx;
         location[0].GirderKey = girderKey;
         location[0].Face      = rftBack;

         location[1].PierIdx   = pierIdx;
         location[1].GirderKey = girderKey;
         location[1].Face      = rftAhead;

         GET_IFACE(ILossParameters,pLossParams);
         for ( int i = 0; i < 2; i++ )
         {
            dc[i] = pBearingDesign->GetBearingCombinedReaction(loadRatingIntervalIdx,location[i],lcDC,bat,resultsType);
            dw[i] = pBearingDesign->GetBearingCombinedReaction(loadRatingIntervalIdx,location[i],lcDWRating,bat,resultsType);

            if ( pLossParams->GetLossMethod() == PrestressLossCriteria::LossMethodType::TIME_STEP )
            {
               cr[i] = pBearingDesign->GetBearingCombinedReaction(loadRatingIntervalIdx,location[i],lcCR,bat,resultsType);
               sh[i] = pBearingDesign->GetBearingCombinedReaction(loadRatingIntervalIdx,location[i],lcSH,bat,resultsType);
               re[i] = pBearingDesign->GetBearingCombinedReaction(loadRatingIntervalIdx,location[i],lcRE,bat,resultsType);
               ps[i] = pBearingDesign->GetBearingCombinedReaction(loadRatingIntervalIdx,location[i],lcPS,bat,resultsType);
            }
            else
            {
               cr[i] = 0;
               sh[i] = 0;
               re[i] = 0;
               ps[i] = 0;
            }
         }

         // superstructure pier diaphragm dead load is applied to the pier model and is included
         // in the superstructure reactions. subtract out the pier diaphragm load. don't want to count it twice
         PIER_DIAPHRAGM_LOAD_DETAILS backSide, aheadSide;
         GET_IFACE(IProductLoads,pProductLoads);
         pProductLoads->GetPierDiaphragmLoads(pierIdx, girderKey.girderIndex, &backSide, &aheadSide);
         CHECK(backSide.P <= 0);
         CHECK(aheadSide.P <= 0);
         dc[0] += backSide.P; // "add" because the load is negative
         dc[1] += aheadSide.P;

         *pDC = dc[brgLineIdx];
         *pDW = dw[brgLineIdx];
         *pCR = cr[brgLineIdx];
         *pSH = sh[brgLineIdx];
         *pRE = re[brgLineIdx];
         *pPS = ps[brgLineIdx];
      }
      else
      {
         // Integral and Continuous piers are modeled with a single bearing line
         ATLASSERT(brgLineIdx == 0);
         if ( 0 < brgLineIdx )
         {
            *pDC = 0;
            *pDW = 0;
            *pCR = 0;
            *pSH = 0;
            *pRE = 0;
            *pPS = 0;
            return;
         }

         // we want the total pier reaction
         GET_IFACE(IReactions,pReactions);

         *pDC = pReactions->GetReaction(girderKey,pierIdx,pgsTypes::stPier,loadRatingIntervalIdx,lcDC,bat,resultsType).Fy;
         *pDW = pReactions->GetReaction(girderKey,pierIdx,pgsTypes::stPier,loadRatingIntervalIdx,lcDWRating,bat,resultsType).Fy;

         GET_IFACE(ILossParameters,pLossParams);
         if ( pLossParams->GetLossMethod() == PrestressLossCriteria::LossMethodType::TIME_STEP )
         {
            *pCR = pReactions->GetReaction(girderKey,pierIdx,pgsTypes::stPier,loadRatingIntervalIdx,lcCR,bat,resultsType).Fy;
            *pSH = pReactions->GetReaction(girderKey,pierIdx,pgsTypes::stPier,loadRatingIntervalIdx,lcSH,bat,resultsType).Fy;
            *pRE = pReactions->GetReaction(girderKey,pierIdx,pgsTypes::stPier,loadRatingIntervalIdx,lcRE,bat,resultsType).Fy;
            *pPS = pReactions->GetReaction(girderKey,pierIdx,pgsTypes::stPier,loadRatingIntervalIdx,lcPS,bat,resultsType).Fy;
         }
         else
         {
            *pCR = 0;
            *pSH = 0;
            *pRE = 0;
            *pPS = 0;
         }

         // superstructure pier diaphragm dead load is applied to the pier model and is included
         // in the superstructure reactions. subtract out the pier diaphragm load. don't want to count it twice
         PIER_DIAPHRAGM_LOAD_DETAILS backSide, aheadSide;
         GET_IFACE(IProductLoads,pProductLoads);
         pProductLoads->GetPierDiaphragmLoads(pierIdx, girderKey.girderIndex, &backSide, &aheadSide);
         CHECK((backSide.P + aheadSide.P) <= 0);
         *pDC += (backSide.P + aheadSide.P); // the loads have negative values because they are downwards. Add the load to "subtract" it out of DC
      }

      if ( UseUniformLoads(pierID,brgLineIdx) )
      {
         // the beam belongs to the type of family where we want to spread the reaction out
         // as a uniform load
         GET_IFACE(IGirder,pIGirder);
         GET_IFACE(IBridge,pBridge);
         CSegmentKey segmentKey = pBridge->GetSegmentAtPier(pierIdx,girderKey);
         FlangeIndexType nBottomFlanges = pIGirder->GetBottomFlangeCount(segmentKey);
         ATLASSERT(nBottomFlanges == 0 || nBottomFlanges == 1);

         GET_IFACE(IPointOfInterest,pPoi);
         pgsPointOfInterest poi = pPoi->GetPierPointOfInterest(girderKey,pierIdx);

         Float64 W;
         if ( nBottomFlanges == 0 )
         {
            W = pIGirder->GetBottomWidth(poi);
         }
         else
         {
            W = 0;
            for ( FlangeIndexType flgIdx = 0; flgIdx < nBottomFlanges; flgIdx++ )
            {
               W += pIGirder->GetBottomFlangeWidth(poi,flgIdx);
            }
         }

         *pDC /= W;
         *pDW /= W;
         *pCR /= W;
         *pSH /= W;
         *pRE /= W;
         *pPS /= W;

         *pW = W;
      }
      else
      {
         *pW  = 0;
      }
   }
}

Float64 CProjectAgentImp::GetBearingWidth(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx) const
{
   if ( pierID == INVALID_ID )
   {
#if defined _DEBUG
      // if pierID == INVALID_ID, then we should be doing a stand-alone analysis
      // when in stand-alone mode, IXBeamRateAgent interface isn't available
      ATLASSERT(IsStandAlone());
#endif

      std::vector<BearingReactions>& vReactions = GetPrivateBearingReactions(pierID,brgLineIdx);
      return vReactions[brgIdx].W;
   }
   else
   {
      // We are in PGSuper/PGSplice Extension mode...
      CGirderKey girderKey = GetGirderKey(pierID,brgLineIdx,brgIdx);
      PierIndexType pierIdx = GetPierIndex(pierID);
      if ( UseUniformLoads(pierID,brgLineIdx) )
      {
         // the beam belongs to the type of family where we want to spread the reaction out
         // as a uniform load
         GET_IFACE(IGirder,pIGirder);
         GET_IFACE(IBridge,pBridge);
         CSegmentKey segmentKey = pBridge->GetSegmentAtPier(pierIdx,girderKey);
         FlangeIndexType nBottomFlanges = pIGirder->GetBottomFlangeCount(segmentKey);
         ATLASSERT(nBottomFlanges == 0 || nBottomFlanges == 1);

         GET_IFACE(IPointOfInterest,pPoi);
         pgsPointOfInterest poi = pPoi->GetPierPointOfInterest(girderKey,pierIdx);

         Float64 W;
         if ( nBottomFlanges == 0 )
         {
            W = pIGirder->GetBottomWidth(poi);
         }
         else
         {
            W = 0;
            for ( FlangeIndexType flgIdx = 0; flgIdx < nBottomFlanges; flgIdx++ )
            {
               W += pIGirder->GetBottomFlangeWidth(poi,flgIdx);
            }
         }

         return W;
      }
      else
      {
         return 0;
      }
   }
}

void CProjectAgentImp::GetReferenceBearing(PierIDType pierID,IndexType brgLineIdx,IndexType* pRefIdx,Float64* pRefBearingOffset,pgsTypes::OffsetMeasurementType* pRefBearingDatum) const
{
   GetPrivatePierData(pierID).GetBearingLineData(brgLineIdx).GetReferenceBearing(pRefBearingDatum,pRefIdx,pRefBearingOffset);
}

void CProjectAgentImp::SetReferenceBearing(PierIDType pierID,IndexType brgLineIdx,IndexType refIdx,Float64 refBearingOffset,pgsTypes::OffsetMeasurementType refBearingDatum)
{
   GetPrivatePierData(pierID).GetBearingLineData(brgLineIdx).SetReferenceBearing(refBearingDatum,refIdx,refBearingOffset);
   Fire_OnProjectChanged();
}

void CProjectAgentImp::SetReactionLoadApplicationType(PierIDType pierID,xbrTypes::ReactionLoadApplicationType applicationType)
{
   GetPrivateReactionLoadApplication(pierID) = applicationType;
   Fire_OnProjectChanged();
}

xbrTypes::ReactionLoadApplicationType CProjectAgentImp::GetReactionLoadApplicationType(PierIDType pierID) const
{
   if ( pierID == INVALID_ID )
   {
      return GetPrivateReactionLoadApplication(pierID);
   }
   else
   {
      if ( GetPierType(pierID) == xbrTypes::pctExpansion && !UseUniformLoads(pierID,0) )
      {
         return xbrTypes::rlaBearings;
      }
      else
      {
         return xbrTypes::rlaCrossBeam;
      }
   }
}

IndexType CProjectAgentImp::GetLiveLoadReactionCount(PierIDType pierID,pgsTypes::LoadRatingType ratingType) const
{
   std::vector<xbrLiveLoadReactionData>& vReactions = GetPrivateLiveLoadReactions(pierID,ratingType);
   return vReactions.size();
}

void CProjectAgentImp::SetLiveLoadReactions(PierIDType pierID,pgsTypes::LoadRatingType ratingType,const std::vector<xbrLiveLoadReactionData>& vLLIM)
{
   std::vector<xbrLiveLoadReactionData>& vReactions = GetPrivateLiveLoadReactions(pierID,ratingType);
   vReactions.clear();
   vReactions = vLLIM;

   Fire_OnProjectChanged();
}

std::vector<xbrLiveLoadReactionData> CProjectAgentImp::GetLiveLoadReactions(PierIDType pierID,pgsTypes::LoadRatingType ratingType) const
{
   std::vector<xbrLiveLoadReactionData>& vLLReactions = GetPrivateLiveLoadReactions(pierID,ratingType);
   return vLLReactions;
}

std::_tstring CProjectAgentImp::GetLiveLoadName(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const
{
   if ( vehicleIdx == INVALID_INDEX )
   {
      return std::_tstring(::GetLiveLoadTypeName(ratingType));
   }

   std::vector<xbrLiveLoadReactionData>& vLLReactions = GetPrivateLiveLoadReactions(pierID,ratingType);
   if ( vLLReactions.size() == 0 )
   {
      return NO_LIVE_LOAD_DEFINED;
   }
   return vLLReactions[vehicleIdx].Name;
}

Float64 CProjectAgentImp::GetLiveLoadReaction(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const
{
   ATLASSERT(vehicleIdx != INVALID_INDEX);

   // Detect the configurations from PGSuper/PGSplice that we can't model
   // Throws an unwind exception if we can't model this thing
   CanModelPier(pierID,m_XBeamRateStatusGroupID,m_scidBridgeError);

   std::vector<xbrLiveLoadReactionData>& vLLReactions = GetPrivateLiveLoadReactions(pierID,ratingType);
   if ( 0 < vLLReactions.size() )
   {
      return vLLReactions[vehicleIdx].LLIM;
   }
   else
   {
      return 0;
   }
}

Float64 CProjectAgentImp::GetVehicleWeight(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const
{
   ATLASSERT(vehicleIdx != INVALID_INDEX);

   // Detect the configurations from PGSuper/PGSplice that we can't model
   // Throws an unwind exception if we can't model this thing
   CanModelPier(pierID,m_XBeamRateStatusGroupID,m_scidBridgeError);

   std::vector<xbrLiveLoadReactionData>& vLLReactions = GetPrivateLiveLoadReactions(pierID,ratingType);
   if ( 0 < vLLReactions.size() )
   {
      return vLLReactions[vehicleIdx].W;
   }
   else
   {
      return 0;
   }
}

void CProjectAgentImp::SetRebarMaterial(PierIDType pierID,WBFL::Materials::Rebar::Type type,WBFL::Materials::Rebar::Grade grade)
{
   GetPrivatePierData(pierID).SetRebarMaterial(type,grade);
   Fire_OnProjectChanged();
}

void CProjectAgentImp::GetRebarMaterial(PierIDType pierID,WBFL::Materials::Rebar::Type* pType,WBFL::Materials::Rebar::Grade* pGrade) const
{
   GetPrivatePierData(pierID).GetRebarMaterial(pType,pGrade);
}

void CProjectAgentImp::SetConcrete(PierIDType pierID,const CConcreteMaterial& concrete)
{
   GetPrivatePierData(pierID).SetConcreteMaterial(concrete);
   Fire_OnProjectChanged();
}

const CConcreteMaterial& CProjectAgentImp::GetConcrete(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetConcreteMaterial();
}

void CProjectAgentImp::SetLowerXBeamDimensions(PierIDType pierID,Float64 h1,Float64 h2,Float64 h3,Float64 h4,Float64 x1,Float64 x2,Float64 x3,Float64 x4,Float64 w)
{
   GetPrivatePierData(pierID).SetLowerXBeamDimensions(h1,h2,h3,h4,x1,x2,x3,x4,w);
   Fire_OnProjectChanged();
}

void CProjectAgentImp::GetLowerXBeamDimensions(PierIDType pierID,Float64* ph1,Float64* ph2,Float64* ph3,Float64* ph4,Float64* px1,Float64* px2,Float64* px3,Float64* px4,Float64* pw) const
{
   GetPrivatePierData(pierID).GetLowerXBeamDimensions(ph1,ph2,ph3,ph4,px1,px2,px3,px4,pw);
}

Float64 CProjectAgentImp::GetXBeamLeftOverhang(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetX5();
}

Float64 CProjectAgentImp::GetXBeamRightOverhang(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetX6();
}

Float64 CProjectAgentImp::GetXBeamWidth(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetW();
}

void CProjectAgentImp::SetRefColumnLocation(PierIDType pierID,pgsTypes::OffsetMeasurementType refColumnDatum,IndexType refColumnIdx,Float64 refColumnOffset)
{
   GetPrivatePierData(pierID).SetRefColumnLocation(refColumnDatum,refColumnIdx,refColumnOffset);
   Fire_OnProjectChanged();
}

void CProjectAgentImp::GetRefColumnLocation(PierIDType pierID,pgsTypes::OffsetMeasurementType* prefColumnDatum,IndexType* prefColumnIdx,Float64* prefColumnOffset) const
{
   GetPrivatePierData(pierID).GetRefColumnLocation(prefColumnDatum,prefColumnIdx,prefColumnOffset);
}

IndexType CProjectAgentImp::GetColumnCount(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetColumnCount();
}

Float64 CProjectAgentImp::GetColumnHeight(PierIDType pierID,ColumnIndexType colIdx) const
{
   return GetPrivatePierData(pierID).GetColumnData(colIdx).GetColumnHeight();
}

CColumnData::ColumnHeightMeasurementType CProjectAgentImp::GetColumnHeightMeasurementType(PierIDType pierID,ColumnIndexType colIdx) const
{
   return GetPrivatePierData(pierID).GetColumnData(colIdx).GetColumnHeightMeasurementType();
}

Float64 CProjectAgentImp::GetColumnSpacing(PierIDType pierID,SpacingIndexType spaceIdx) const
{
   return GetPrivatePierData(pierID).GetColumnSpacing(spaceIdx);
}

void CProjectAgentImp::SetColumnProperties(PierIDType pierID,ColumnIndexType colIdx,CColumnData::ColumnShapeType shapeType,Float64 D1,Float64 D2,CColumnData::ColumnHeightMeasurementType heightType,Float64 H)
{
   CColumnData& columnData = GetPrivatePierData(pierID).GetColumnData(colIdx);
   
   columnData.SetColumnHeight(H,heightType);
   columnData.SetColumnShape(shapeType);
   columnData.SetColumnDimensions(D1,D2);

   Fire_OnProjectChanged();
}

void CProjectAgentImp::GetColumnProperties(PierIDType pierID,ColumnIndexType colIdx,CColumnData::ColumnShapeType* pshapeType,Float64* pD1,Float64* pD2,CColumnData::ColumnHeightMeasurementType* pheightType,Float64* pH) const
{
   CColumnData& columnData = GetPrivatePierData(pierID).GetColumnData(colIdx);

   *pshapeType = columnData.GetColumnShape();
   columnData.GetColumnDimensions(pD1,pD2);
   *pH = columnData.GetColumnHeight();
   *pheightType = columnData.GetColumnHeightMeasurementType();
}

pgsTypes::ColumnTransverseFixityType CProjectAgentImp::GetColumnFixity(PierIDType pierID,ColumnIndexType colIdx) const
{
   return GetPrivatePierData(pierID).GetColumnData(colIdx).GetTransverseFixity();
}

const xbrLongitudinalRebarData& CProjectAgentImp::GetLongitudinalRebar(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetLongitudinalRebar();
}

void CProjectAgentImp::SetLongitudinalRebar(PierIDType pierID,const xbrLongitudinalRebarData& rebar)
{
   GetPrivatePierData(pierID).SetLongitudinalRebar(rebar);
   Fire_OnProjectChanged();
}

void CProjectAgentImp::SetLowerXBeamStirrups(PierIDType pierID,const xbrStirrupData& stirrups)
{
   GetPrivatePierData(pierID).SetLowerXBeamStirrups(stirrups);
   Fire_OnProjectChanged();
}

const xbrStirrupData& CProjectAgentImp::GetLowerXBeamStirrups(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetLowerXBeamStirrups();
}

void CProjectAgentImp::SetFullDepthStirrups(PierIDType pierID,const xbrStirrupData& stirrups)
{
   GetPrivatePierData(pierID).SetFullDepthStirrups(stirrups);
   Fire_OnProjectChanged();
}

const xbrStirrupData& CProjectAgentImp::GetFullDepthStirrups(PierIDType pierID) const
{
   return GetPrivatePierData(pierID).GetFullDepthStirrups();
}
   
void CProjectAgentImp::SetFlexureResistanceFactors(Float64 phiC,Float64 phiT)
{
   m_PhiC = phiC;
   m_PhiT = phiT;
   Fire_OnProjectChanged();
}

void CProjectAgentImp::GetFlexureResistanceFactors(Float64* phiC,Float64* phiT) const
{
   if ( IsStandAlone() )
   {
      *phiC = m_PhiC;
      *phiT = m_PhiT;
   }
   else
   {
      GET_IFACE(IResistanceFactors,pResistanceFactors);
      Float64 PhiRC,PhiPS,PhiSP,PhiC;
      pResistanceFactors->GetFlexureResistanceFactors(pgsTypes::Normal,&PhiPS,&PhiRC,&PhiSP,&PhiC);
      *phiC = PhiC;
      *phiT = PhiRC;
   }
}
   
void CProjectAgentImp::SetShearResistanceFactor(Float64 phi)
{
   m_PhiV = phi;
   Fire_OnProjectChanged();
}

Float64 CProjectAgentImp::GetShearResistanceFactor() const
{
   if ( IsStandAlone() )
   {
      return m_PhiV;
   }
   else
   {
      GET_IFACE(IResistanceFactors,pResistanceFactors);
      return pResistanceFactors->GetShearResistanceFactor(false, pgsTypes::Normal); // assume no debonding ever in pier
   }
}

void CProjectAgentImp::SetSystemFactorFlexure(Float64 sysFactor)
{
   if (sysFactor != m_SysFactorFlexure)
   {
      m_SysFactorFlexure = sysFactor;
      Fire_OnProjectChanged();
   }
}

Float64 CProjectAgentImp::GetSystemFactorFlexure() const
{
   return m_SysFactorFlexure;
}

void CProjectAgentImp::SetSystemFactorShear(Float64 sysFactor)
{
   if (sysFactor != m_SysFactorShear)
   {
      m_SysFactorShear = sysFactor;
      Fire_OnProjectChanged();
   }
}

Float64 CProjectAgentImp::GetSystemFactorShear() const
{
   return m_SysFactorShear;
}

void CProjectAgentImp::SetConditionFactor(PierIDType pierID,pgsTypes::ConditionFactorType conditionFactorType,Float64 conditionFactor)
{
   GetPrivatePierData(pierID).SetConditionFactorType(conditionFactorType);
   GetPrivatePierData(pierID).SetConditionFactor(conditionFactor);
   Fire_OnProjectChanged();
}

void CProjectAgentImp::GetConditionFactor(PierIDType pierID,pgsTypes::ConditionFactorType* pConditionFactorType,Float64 *pConditionFactor) const
{
   *pConditionFactorType = GetPrivatePierData(pierID).GetConditionFactorType();
   *pConditionFactor = GetPrivatePierData(pierID).GetConditionFactor();
}

Float64 CProjectAgentImp::GetConditionFactor(PierIDType pierID) const
{
   pgsTypes::ConditionFactorType cfType;
   Float64 CF;
   GetConditionFactor(pierID,&cfType,&CF);

   // MBE 6A.4.2.3
   switch(cfType)
   {
   case pgsTypes::cfGood:
      return 1.0;

   case pgsTypes::cfFair:
      return 0.95;

   case pgsTypes::cfPoor:
      return 0.85;

   case pgsTypes::cfOther:
      return CF;
   }
   ATLASSERT(false);
   return 1.0;
}

void CProjectAgentImp::SetDCLoadFactor(pgsTypes::LimitState limitState,Float64 dc)
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   if ( ::IsStrengthILimitState(limitState) )
   {
      m_gDC_StrengthI = dc;
   }
   else if (::IsStrengthIILimitState(limitState) )
   {
      m_gDC_StrengthII = dc;
   }
   else if ( ::IsServiceLimitState(limitState) )
   {
      m_gDC_ServiceI = dc;
   }
   else
   {
      ATLASSERT(false);
   }

   Fire_OnProjectChanged();
}

Float64 CProjectAgentImp::GetDCLoadFactor(pgsTypes::LimitState limitState) const
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   if ( IsStandAlone() )
   {
      if ( ::IsStrengthILimitState(limitState) )
      {
         return m_gDC_StrengthI;
      }
      else if (::IsStrengthIILimitState(limitState) )
      {
         return m_gDC_StrengthII;
      }
      else if ( ::IsServiceLimitState(limitState) )
      {
         return m_gDC_ServiceI;
      }
      else
      {
         ATLASSERT(false);
         return 99999;
      }
   }
   else
   {
      GET_IFACE(IRatingSpecification,pRatingSpec);
      return pRatingSpec->GetDeadLoadFactor(limitState);
   }
}

void CProjectAgentImp::SetDWLoadFactor(pgsTypes::LimitState limitState,Float64 dw)
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   if ( ::IsStrengthILimitState(limitState) )
   {
      m_gDW_StrengthI = dw;
   }
   else if (::IsStrengthIILimitState(limitState) )
   {
      m_gDW_StrengthII = dw;
   }
   else if ( ::IsServiceLimitState(limitState) )
   {
      m_gDW_ServiceI = dw;
   }
   else
   {
      ATLASSERT(false);
   }
   Fire_OnProjectChanged();
}

Float64 CProjectAgentImp::GetDWLoadFactor(pgsTypes::LimitState limitState) const
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   if ( IsStandAlone() )
   {
      if ( ::IsStrengthILimitState(limitState) )
      {
         return m_gDW_StrengthI;
      }
      else if (::IsStrengthIILimitState(limitState) )
      {
         return m_gDW_StrengthII;
      }
      else if ( ::IsServiceLimitState(limitState) )
      {
         return m_gDW_ServiceI;
      }
      else
      {
         ATLASSERT(false);
         return 99999;
      }
   }
   else
   {
      GET_IFACE(IRatingSpecification,pRatingSpec);
      return pRatingSpec->GetWearingSurfaceFactor(limitState);
   }
}

void CProjectAgentImp::SetCRLoadFactor(pgsTypes::LimitState limitState,Float64 cr)
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   if ( ::IsStrengthILimitState(limitState) )
   {
      m_gCR_StrengthI = cr;
   }
   else if (::IsStrengthIILimitState(limitState) )
   {
      m_gCR_StrengthII = cr;
   }
   else if ( ::IsServiceLimitState(limitState) )
   {
      m_gCR_ServiceI = cr;
   }
   else
   {
      ATLASSERT(false);
   }
   Fire_OnProjectChanged();
}

Float64 CProjectAgentImp::GetCRLoadFactor(pgsTypes::LimitState limitState) const
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   if ( IsStandAlone() )
   {
      if ( ::IsStrengthILimitState(limitState) )
      {
         return m_gCR_StrengthI;
      }
      else if (::IsStrengthIILimitState(limitState) )
      {
         return m_gCR_StrengthII;
      }
      else if ( ::IsServiceLimitState(limitState) )
      {
         return m_gCR_ServiceI;
      }
      else
      {
         ATLASSERT(false);
         return 99999;
      }
   }
   else
   {
      GET_IFACE(IRatingSpecification,pRatingSpec);
      return pRatingSpec->GetCreepFactor(limitState);
   }
}

void CProjectAgentImp::SetSHLoadFactor(pgsTypes::LimitState limitState,Float64 sh)
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   if ( ::IsStrengthILimitState(limitState) )
   {
      m_gSH_StrengthI = sh;
   }
   else if (::IsStrengthIILimitState(limitState) )
   {
      m_gSH_StrengthII = sh;
   }
   else if ( ::IsServiceLimitState(limitState) )
   {
      m_gSH_ServiceI = sh;
   }
   else
   {
      ATLASSERT(false);
   }
   Fire_OnProjectChanged();
}

Float64 CProjectAgentImp::GetSHLoadFactor(pgsTypes::LimitState limitState) const
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   if ( IsStandAlone() )
   {
      if ( ::IsStrengthILimitState(limitState) )
      {
         return m_gSH_StrengthI;
      }
      else if (::IsStrengthIILimitState(limitState) )
      {
         return m_gSH_StrengthII;
      }
      else if ( ::IsServiceLimitState(limitState) )
      {
         return m_gSH_ServiceI;
      }
      else
      {
         ATLASSERT(false);
         return 99999;
      }
   }
   else
   {
      GET_IFACE(IRatingSpecification,pRatingSpec);
      return pRatingSpec->GetShrinkageFactor(limitState);
   }
}

void CProjectAgentImp::SetRELoadFactor(pgsTypes::LimitState limitState,Float64 re)
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   if ( ::IsStrengthILimitState(limitState) )
   {
      m_gRE_StrengthI = re;
   }
   else if (::IsStrengthIILimitState(limitState) )
   {
      m_gRE_StrengthII = re;
   }
   else if ( ::IsServiceLimitState(limitState) )
   {
      m_gRE_ServiceI = re;
   }
   else
   {
      ATLASSERT(false);
   }
   Fire_OnProjectChanged();
}

Float64 CProjectAgentImp::GetRELoadFactor(pgsTypes::LimitState limitState) const
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   if ( IsStandAlone() )
   {
      if ( ::IsStrengthILimitState(limitState) )
      {
         return m_gRE_StrengthI;
      }
      else if (::IsStrengthIILimitState(limitState) )
      {
         return m_gRE_StrengthII;
      }
      else if ( ::IsServiceLimitState(limitState) )
      {
         return m_gRE_ServiceI;
      }
      else
      {
         ATLASSERT(false);
         return 99999;
      }
   }
   else
   {
      GET_IFACE(IRatingSpecification,pRatingSpec);
      return pRatingSpec->GetRelaxationFactor(limitState);
   }
}

void CProjectAgentImp::SetPSLoadFactor(pgsTypes::LimitState limitState,Float64 ps)
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   if ( ::IsStrengthILimitState(limitState) )
   {
      m_gPS_StrengthI = ps;
   }
   else if (::IsStrengthIILimitState(limitState) )
   {
      m_gPS_StrengthII = ps;
   }
   else if ( ::IsServiceLimitState(limitState) )
   {
      m_gPS_ServiceI = ps;
   }
   else
   {
      ATLASSERT(false);
   }
   Fire_OnProjectChanged();
}

Float64 CProjectAgentImp::GetPSLoadFactor(pgsTypes::LimitState limitState) const
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   if ( IsStandAlone() )
   {
      if ( ::IsStrengthILimitState(limitState) )
      {
         return m_gPS_StrengthI;
      }
      else if (::IsStrengthIILimitState(limitState) )
      {
         return m_gPS_StrengthII;
      }
      else if ( ::IsServiceLimitState(limitState) )
      {
         return m_gPS_ServiceI;
      }
      else
      {
         ATLASSERT(false);
         return 99999;
      }
   }
   else
   {
      GET_IFACE(IRatingSpecification,pRatingSpec);
      return pRatingSpec->GetSecondaryEffectsFactor(limitState);
   }
}

void CProjectAgentImp::SetLiveLoadFactor(PierIDType pierID,pgsTypes::LimitState limitState,Float64 ll)
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   m_gLL[GET_INDEX(limitState)][pierID] = ll;
   Fire_OnProjectChanged();
}

Float64 CProjectAgentImp::GetLiveLoadFactor(PierIDType pierID,pgsTypes::LimitState limitState,VehicleIndexType vehicleIdx) const
{
   ATLASSERT(::IsRatingLimitState(limitState)); // must be a load rating limit state
   if ( IsStandAlone() )
   {
      return m_gLL[GET_INDEX(limitState)][pierID];
   }
   else
   {
      PierIndexType pierIdx = GetPierIndex(pierID);
      GirderIndexType gdrIdx = GetLongestGirderLine();
      pgsTypes::LoadRatingType ratingType = ::RatingTypeFromLimitState(limitState);
      GET_IFACE(IRatingSpecification,pRatingSpec);
      if ( ::IsStrengthLimitState(limitState) )
      {
         return pRatingSpec->GetReactionStrengthLiveLoadFactor(pierIdx,gdrIdx,ratingType,vehicleIdx);
      }
      else
      {
         return pRatingSpec->GetReactionServiceLiveLoadFactor(pierIdx,gdrIdx,ratingType,vehicleIdx);
      }
   }
}

void CProjectAgentImp::SetMaxLiveLoadStepSize(Float64 stepSize)
{
   if ( !IsEqual(m_MaxLLStepSize,stepSize) )
   {
      m_MaxLLStepSize = stepSize;
      Fire_OnProjectChanged();
   }
}

Float64 CProjectAgentImp::GetMaxLiveLoadStepSize() const
{
   return m_MaxLLStepSize;
}

void CProjectAgentImp::SetMaxLoadedLanes(IndexType nLanesMax)
{
   if ( m_MaxLoadedLanes != nLanesMax )
   {
      m_MaxLoadedLanes = nLanesMax;
      Fire_OnProjectChanged();
   }
}

IndexType CProjectAgentImp::GetMaxLoadedLanes() const
{
   return m_MaxLoadedLanes;
}

bool CProjectAgentImp::GetDoAnalyzeNegativeMomentBetweenFocOptions(Float64* pminColumnWidth)
{
   *pminColumnWidth = m_MinColumnWidthForNegMoment;
   return m_bDoAnalyzeNegativeMomentBetweenFOC;
}

void CProjectAgentImp::SetDoAnalyzeNegativeMomentBetweenFocOptions(bool bDoUseOption, Float64 minColumnWidth)
{
   if (bDoUseOption != m_bDoAnalyzeNegativeMomentBetweenFOC || (bDoUseOption && !IsEqual(minColumnWidth, m_MinColumnWidthForNegMoment)))
   {
      m_bDoAnalyzeNegativeMomentBetweenFOC = bDoUseOption;
      m_MinColumnWidthForNegMoment = minColumnWidth;
      Fire_OnProjectChanged();
   }
}

//////////////////////////////////////////////////////////
// IXBRRatingSpecification
bool CProjectAgentImp::IsRatingEnabled(pgsTypes::LoadRatingType ratingType) const
{
   if ( IsStandAlone() )
   {
      return m_bRatingEnabled[ratingType];
   }
   else
   {
      GET_IFACE(IRatingSpecification,pRatingSpec);
      return pRatingSpec->IsRatingEnabled(ratingType);
   }
}

void CProjectAgentImp::EnableRating(pgsTypes::LoadRatingType ratingType,bool bEnable)
{
   m_bRatingEnabled[ratingType] = bEnable;
}

void CProjectAgentImp::RateForShear(pgsTypes::LoadRatingType ratingType,bool bRateForShear)
{
   if (m_bRateForShear[ratingType] != bRateForShear)
   {
      m_bRateForShear[ratingType] = bRateForShear;
      Fire_OnProjectChanged();
   }
}

bool CProjectAgentImp::RateForShear(pgsTypes::LoadRatingType ratingType) const
{
   if ( IsStandAlone() )
   {
      return m_bRateForShear[ratingType];
   }
   else
   {
      GET_IFACE(IRatingSpecification,pRatingSpec);
      return pRatingSpec->RateForShear(ratingType);
   }
}

void CProjectAgentImp::CheckYieldStressLimit(bool bCheckYieldStress)
{
   if (m_bCheckYieldStress != bCheckYieldStress)
   {
      m_bCheckYieldStress = bCheckYieldStress;
      Fire_OnProjectChanged();
   }
}

bool CProjectAgentImp::CheckYieldStressLimit() const
{
   if ( IsStandAlone() )
   {
      return m_bCheckYieldStress;
   }
   else
   {
      GET_IFACE(IRatingSpecification,pRatingSpec);
      return pRatingSpec->RateForStress(pgsTypes::lrPermit_Routine);
   }
}

void CProjectAgentImp::SetYieldStressLimitCoefficient(Float64 x)
{
   if (m_YieldStressCoefficient != x)
   {
      m_YieldStressCoefficient = x;
      Fire_OnProjectChanged();
   }
}

Float64 CProjectAgentImp::GetYieldStressLimitCoefficient() const
{
   if ( IsStandAlone() )
   {
      return m_YieldStressCoefficient;
   }
   else
   {
      GET_IFACE(IRatingSpecification,pRatingSpec);
      return pRatingSpec->GetYieldStressLimitCoefficient();
   }
}

pgsTypes::AnalysisType CProjectAgentImp::GetAnalysisMethodForReactions() const
{
   return m_AnalysisType;
}

void CProjectAgentImp::SetAnalysisMethodForReactions(pgsTypes::AnalysisType analysisType)
{
   if ( analysisType != m_AnalysisType )
   {
      m_AnalysisType = analysisType;
      Fire_OnProjectChanged();
   }
}

xbrTypes::EmergencyRatingMethod CProjectAgentImp::GetEmergencyRatingMethod() const
{
   return m_EmergencyRatingMethod;
}

void CProjectAgentImp::SetEmergencyRatingMethod(xbrTypes::EmergencyRatingMethod emergencyRatingMethod)
{
   if (m_EmergencyRatingMethod != emergencyRatingMethod)
   {
      m_EmergencyRatingMethod = emergencyRatingMethod;
      Fire_OnProjectChanged();
   }
}

bool CProjectAgentImp::IsWSDOTEmergencyRating(pgsTypes::LoadRatingType ratingType) const
{
   return (::IsEmergencyRatingType(ratingType) && m_EmergencyRatingMethod == xbrTypes::ermWSDOT ? true : false);
}

xbrTypes::PermitRatingMethod CProjectAgentImp::GetPermitRatingMethod() const
{
   return m_PermitRatingMethod;
}

void CProjectAgentImp::SetPermitRatingMethod(xbrTypes::PermitRatingMethod permitRatingMethod)
{
   if ( m_PermitRatingMethod != permitRatingMethod )
   {
      m_PermitRatingMethod = permitRatingMethod;
      Fire_OnProjectChanged();
   }
}

bool CProjectAgentImp::IsWSDOTPermitRating(pgsTypes::LoadRatingType ratingType) const
{
   return (::IsPermitRatingType(ratingType) && m_PermitRatingMethod == xbrTypes::ermWSDOT ? true : false);
}

bool CProjectAgentImp::DoCheckNegativeMomentBetweenFOCs(PierIDType pierID) const
{
   bool doCheck = true;

   // We only need to skip between FOC if user settings say so
   GET_IFACE(IXBRProject, pProject);
   Float64 minColWidth;
   bool bCheckColWidth = pProject->GetDoAnalyzeNegativeMomentBetweenFocOptions(&minColWidth);
   if (bCheckColWidth)
   {
      // We could create a lot of complexity here and vet pois column by column. But the normal case is that all columns are the same width.
      // Check if any columns are wider that the threshold, and use the same option for all
      Float64 maxColWid = -Float64_Max;
      IndexType nCols = GetColumnCount(pierID);
      for (IndexType icol = 0; icol < nCols; icol++)
      {
         CColumnData::ColumnShapeType shapeType;
         Float64 D1, D2;
         CColumnData::ColumnHeightMeasurementType columnHeightType;
         Float64 H;
         pProject->GetColumnProperties(pierID, icol, &shapeType, &D1, &D2, &columnHeightType, &H);

         maxColWid = max(maxColWid, D1);

         if (IsGE(minColWidth, maxColWid))
         {
            doCheck = false;
            break;
         }
      }
   }

   return doCheck;
}


//////////////////////////////////////////////////////////
// IXBRProjectEdit
void CProjectAgentImp::EditPier(int nPage)
{
   OnEditPier();
}

void CProjectAgentImp::EditOptions()
{
   OnEditOptions();
}

////////////////////////////////////////////////////////////////////////
// IXBREvents
void CProjectAgentImp::HoldEvents()
{
   ATLASSERT(0 <= m_EventHoldCount);
   m_EventHoldCount++;

   Fire_OnHoldEvents();

   if ( IsStandAlone() )
   {
      GET_IFACE(IXBRUIEvents,pUIEvents);
      pUIEvents->HoldEvents(true);
   }
}

void CProjectAgentImp::FirePendingEvents()
{
   if ( m_EventHoldCount == 0 )
   {
      return;
   }

   if ( m_EventHoldCount == 1 )
   {
      m_EventHoldCount--;

      // Fire our events
	   if ( WBFL::System::Flags<Uint32>::IsSet(m_PendingEvents,EVT_PROJECTPROPERTIES) )
      {
	      Fire_OnProjectPropertiesChanged();
      }

	   if ( WBFL::System::Flags<Uint32>::IsSet(m_PendingEvents,EVT_PROJECT) )
      {
	      Fire_OnProjectChanged();
      }

      // tell event listeners that it is time to fire their events
      Fire_OnFirePendingEvents(); 
   }
   else
   {
      m_EventHoldCount--;
   }

   if ( IsStandAlone() )
   {
      GET_IFACE(IXBRUIEvents,pUIEvents);
      pUIEvents->FirePendingEvents();
   }
}

void CProjectAgentImp::CancelPendingEvents()
{
   m_EventHoldCount--;
   if ( m_EventHoldCount <= 0 )
   {
      m_EventHoldCount = 0;
      Fire_OnCancelPendingEvents();
   }

   if ( IsStandAlone() )
   {
      GET_IFACE(IXBRUIEvents,pUIEvents);
      pUIEvents->CancelPendingEvents();
   }
}

//////////////////////////////////////////////////////////
// IBridgeDescriptionEventSink
HRESULT CProjectAgentImp::OnBridgeChanged(CBridgeChangedHint* pHint)
{
   GET_IFACE(IEAFStatusCenter,pStatusCenter);
   pStatusCenter->RemoveByStatusGroupID(m_XBeamRateStatusGroupID);

   UpdatePiers();
   return S_OK;
}

HRESULT CProjectAgentImp::OnGirderFamilyChanged()
{
   Invalidate();
   return S_OK;
}

HRESULT CProjectAgentImp::OnGirderChanged(const CGirderKey& girderKey,Uint32 lHint)
{
   Invalidate();
   return S_OK;
}

HRESULT CProjectAgentImp::OnLiveLoadChanged()
{
   Invalidate();
   return S_OK;
}

HRESULT CProjectAgentImp::OnLiveLoadNameChanged(LPCTSTR strOldName,LPCTSTR strNewName)
{
   Invalidate();
   return S_OK;
}

HRESULT CProjectAgentImp::OnConstructionLoadChanged()
{
   Invalidate();
   return S_OK;
}

//////////////////////////////////////////////////////////
// IEventsSink
HRESULT CProjectAgentImp::OnHoldEvents()
{
   HoldEvents();
   return S_OK;
}

HRESULT CProjectAgentImp::OnFirePendingEvents()
{
   FirePendingEvents();
   return S_OK;
}

HRESULT CProjectAgentImp::OnCancelPendingEvents()
{
   CancelPendingEvents();
   return S_OK;
}

//////////////////////////////////////////////////////////
// IXBRExport
HRESULT CProjectAgentImp::Export(PierIndexType pierIdx)
{
   ATLASSERT(IsPGSExtension());
   CPierExporter exporter(m_pBroker,this);
   return exporter.Export(pierIdx);
}

HRESULT CProjectAgentImp::BatchExport()
{
   ATLASSERT(IsPGSExtension());
   CPierExporter exporter(m_pBroker,this);
   return exporter.BatchExport();
}

//////////////////////////////////////////////////////////
void CProjectAgentImp::CreateMenus()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   GET_IFACE(IEAFMainMenu,pMainMenu);
   auto pMenu = pMainMenu->GetMainMenu();

   auto callback = std::dynamic_pointer_cast<WBFL::EAF::ICommandCallback>(shared_from_this());

   // Add our commands to the Edit menu
   UINT editPos = pMenu->FindMenuItem(_T("&Edit"));
   auto pEditMenu = pMenu->GetSubMenu(editPos);
   pEditMenu->AppendMenu(ID_EDIT_PIER,_T("&Pier..."),callback);

   // Add our commands to the Project menu
   UINT projPos = pMenu->FindMenuItem(_T("&Project"));
   auto pProjectMenu = pMenu->GetSubMenu(projPos);
   pProjectMenu->AppendMenu(EAFID_EDIT_UNITS,_T("&Units..."),nullptr);
   pProjectMenu->AppendMenu(ID_EDIT_OPTIONS,_T("Load Rating Options..."),callback);
   pProjectMenu->AppendSeparator();
   pProjectMenu->AppendMenu(ID_EDIT_PROPERTIES,_T("&Properties..."),callback);
}

void CProjectAgentImp::RemoveMenus()
{
   GET_IFACE(IEAFMainMenu,pMainMenu);
   auto pMenu = pMainMenu->GetMainMenu();

   auto callback = std::dynamic_pointer_cast<WBFL::EAF::ICommandCallback>(shared_from_this());

   // remove the our menus
   UINT editPos = pMenu->FindMenuItem(_T("&Edit"));
   auto pEditMenu = pMenu->GetSubMenu(editPos);
   pEditMenu->RemoveMenu(ID_EDIT_PIER,MF_BYCOMMAND,callback);
   pEditMenu->RemoveMenu(ID_EDIT_PROPERTIES,MF_BYCOMMAND,callback);

   UINT projPos = pMenu->FindMenuItem(_T("&Project"));
   auto pProjectMenu = pMenu->GetSubMenu(projPos);
   pProjectMenu->RemoveMenu(EAFID_EDIT_UNITS,MF_BYCOMMAND,nullptr);
   pProjectMenu->RemoveMenu(ID_EDIT_OPTIONS,MF_BYCOMMAND,callback);
   pProjectMenu->RemoveMenu(ID_EDIT_PROPERTIES,MF_BYCOMMAND,callback);
   pProjectMenu->RemoveMenu(pProjectMenu->GetMenuItemCount()-1,MF_BYPOSITION,nullptr); // remove the separator
}

void CProjectAgentImp::CreateToolbars()
{
   // add a button to the standard XBeamRate toolbar to view a pier
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   GET_IFACE(IEAFToolbars,pToolBars);
   GET_IFACE(IXBREditByUI,pEditUI);
   UINT stdID = pEditUI->GetStdToolBarID();
   auto pStdToolBar = pToolBars->GetToolBar(stdID);

   auto callback = std::dynamic_pointer_cast<WBFL::EAF::ICommandCallback>(shared_from_this());

   int idx = pStdToolBar->CommandToIndex(ID_VIEW_PIER,nullptr); 
   pStdToolBar->InsertButton(idx-1,ID_EDIT_OPTIONS,IDB_EDIT_OPTIONS,nullptr,callback);
   pStdToolBar->InsertButton(idx-1,ID_EDIT_PIER,IDB_EDIT_PIER,nullptr,callback);
}

void CProjectAgentImp::RemoveToolbars()
{
   // Remove all the buttons we added to the standard toolbar
   GET_IFACE(IEAFToolbars,pToolBars);
   GET_IFACE(IXBREditByUI,pEditUI);
   UINT stdID = pEditUI->GetStdToolBarID();
   auto pStdToolBar = pToolBars->GetToolBar(stdID);

   auto callback = std::dynamic_pointer_cast<WBFL::EAF::ICommandCallback>(shared_from_this());
   pStdToolBar->RemoveButtons(callback);
}

xbrPierData& CProjectAgentImp::GetPrivatePierData(PierIDType pierID) const
{
   std::map<PierIDType,xbrPierData>::const_iterator found(m_PierData.find(pierID));
   if ( found == m_PierData.end() )
   {
      xbrPierData pierData;
      pierData.SetID(pierID);
      m_PierData.insert(std::make_pair(pierID,pierData));
   }
   return m_PierData[pierID];
}

std::vector<CProjectAgentImp::BearingReactions>& CProjectAgentImp::GetPrivateBearingReactions(PierIDType pierID,IndexType brgLineIdx) const
{
   auto found(m_BearingReactions[brgLineIdx].find(pierID));
   if ( found == m_BearingReactions[brgLineIdx].end() )
   {
      xbrPierData& pierData = GetPrivatePierData(pierID);
      IndexType nBearingLines = pierData.GetBearingLineCount();
      for ( IndexType i = 0; i < nBearingLines; i++ )
      {
         std::vector<BearingReactions> vBearingReactions;
         vBearingReactions.resize(pierData.GetBearingCount(i));
         m_BearingReactions[i].insert(std::make_pair(pierID,vBearingReactions));
      }
   }

   return m_BearingReactions[brgLineIdx][pierID];
}

std::vector<xbrLiveLoadReactionData>& CProjectAgentImp::GetPrivateLiveLoadReactions(PierIDType pierID,pgsTypes::LoadRatingType ratingType) const
{
   auto found(m_LiveLoadReactions[ratingType].find(pierID));
   if ( found == m_LiveLoadReactions[ratingType].end() )
   {
      std::vector<xbrLiveLoadReactionData> vReactions;
      m_LiveLoadReactions[ratingType].insert(std::make_pair(pierID,vReactions));

      if ( pierID != INVALID_ID )
      {
         ATLASSERT(IsPGSExtension());
         // It is kind of expensive to get the reactions over and over so we'll get
         // them once and cache them.
         GET_IFACE_NOCHECK(IReactions,pReactions); // not used when nVehicles = 0
         GET_IFACE(IIntervals,pIntervals);
         IntervalIndexType loadRatingIntervalIdx = pIntervals->GetLoadRatingInterval();

         PierIndexType pierIdx = GetPierIndex(pierID);
         GirderIndexType gdrIdx = GetLongestGirderLine();

         // assume a single bearing line at the pier and one bearing per girder
         // when getting the girder key we'll use for live load reactions
         CGirderKey girderKey = GetGirderKey(pierID,0,gdrIdx);

         GET_IFACE(IProductForces,pProductForces);
         pgsTypes::BridgeAnalysisType bat = pProductForces->GetBridgeAnalysisType(m_AnalysisType,pgsTypes::Maximize);

         GET_IFACE(IProductLoads,pProductLoads);
         pgsTypes::LiveLoadType llType = ::GetLiveLoadType(ratingType);
         VehicleIndexType nVehicles = pProductLoads->GetVehicleCount(llType);
         if ( pProductLoads->GetLiveLoadName(llType,0) == NO_LIVE_LOAD_DEFINED )
         {
            nVehicles = 0;
         }

         for ( VehicleIndexType vehicleIdx = 0; vehicleIdx < nVehicles; vehicleIdx++ )
         {
            std::_tstring strName = pProductLoads->GetLiveLoadName(llType,vehicleIdx);


            REACTION Rmin,Rmax;
            pReactions->GetVehicularLiveLoadReaction(loadRatingIntervalIdx,llType,vehicleIdx,pierIdx,girderKey,bat,true,&Rmin,&Rmax,nullptr,nullptr);

            Float64 W = pProductLoads->GetVehicleWeight(llType,vehicleIdx);
            
            m_LiveLoadReactions[ratingType][pierID].push_back(xbrLiveLoadReactionData(strName,Rmax.Fy,W));
         }
      }
   }

   return m_LiveLoadReactions[ratingType][pierID];
}

xbrTypes::ReactionLoadApplicationType& CProjectAgentImp::GetPrivateReactionLoadApplication(PierIDType pierID) const
{
   auto found(m_ReactionApplication.find(pierID));
   if ( found == m_ReactionApplication.end() )
   {
      m_ReactionApplication.insert(std::make_pair(pierID,xbrTypes::rlaCrossBeam));
   }

   return m_ReactionApplication[pierID];
}

xbrTypes::ReactionLoadType& CProjectAgentImp::GetPrivateBearingReactionType(PierIDType pierID,IndexType brgLineIdx) const
{
   auto found(m_BearingReactionType[brgLineIdx].find(pierID));
   if ( found == m_BearingReactionType[brgLineIdx].end() )
   {
      xbrPierData& pierData = GetPrivatePierData(pierID);
      IndexType nBearingLines = pierData.GetBearingLineCount();
      for ( IndexType i = 0; i < nBearingLines; i++ )
      {
         m_BearingReactions[i].insert(std::make_pair(pierID,xbrTypes::rltConcentrated));
      }
   }

   return m_BearingReactionType[brgLineIdx][pierID];
}

void CProjectAgentImp::UpdatePiers()
{
   GET_IFACE(IBridgeDescription,pIBridgeDesc);
   const CBridgeDescription2* pBridgeDesc = pIBridgeDesc->GetBridgeDescription();
   PierIndexType nPiers = pBridgeDesc->GetPierCount();
   for ( PierIndexType pierIdx = 0; pierIdx < nPiers; pierIdx++ )
   {
      const CPierData2* pPier = pBridgeDesc->GetPier(pierIdx);
      if ( pPier->GetPierModelType() == pgsTypes::pmtPhysical )
      {
         PierIDType pierID = pPier->GetID();
         xbrPierData& pierData = GetPrivatePierData(pierID);
         UpdatePierData(pPier,pierData);
      }
   }
}

void CProjectAgentImp::UpdatePierData(const CPierData2* pPier,xbrPierData& pierData)
{
   // Updates our internal pier data with the pier data from the bridge model
   USES_CONVERSION;

   if ( pPier->IsBoundaryPier() )
   {
      pierData.SetPierType( ::GetPierType(pPier->GetBoundaryConditionType()) );
   }
   else
   {
      pierData.SetPierType( ::GetPierType(pPier->GetSegmentConnectionType()) );
   }

   PierIndexType pierIdx = pPier->GetIndex();
   PierIDType pierID = pPier->GetID();
   
   pierData.SetID(pierID);

   GET_IFACE(IBridgeDescription,pIBridgeDesc);
   Float64 alignment_offset = pIBridgeDesc->GetBridgeDescription()->GetAlignmentOffset();
   pierData.SetBridgeLineOffset(alignment_offset);

   GET_IFACE(IBridge,pBridge);
   Float64 pierStation = pBridge->GetPierStation(pierIdx);

   CComPtr<IAngle> skewAngle;
   pBridge->GetPierSkew(pierIdx,&skewAngle);
   Float64 skew;
   skewAngle->get_Value(&skew);

   GET_IFACE(IRoadway,pRoadway);
   CComPtr<IPoint2dCollection> deckProfile;
   pRoadway->GetRoadwaySurface(pierStation,skewAngle,&deckProfile);
   pierData.SetDeckSurfaceType(xbrPierData::Profile);
   pierData.SetDeckProfile(deckProfile);

   GroupIndexType backGrpIdx, aheadGrpIdx;
   pBridge->GetGirderGroupIndex(pierIdx,&backGrpIdx,&aheadGrpIdx);

   GET_IFACE(IPointOfInterest,pPoi);
   pgsPointOfInterest poi = pPoi->GetPierPointOfInterest(CGirderKey(backGrpIdx == INVALID_INDEX ? aheadGrpIdx : backGrpIdx,0),pierIdx);
   Float64 tSlab = pBridge->GetGrossSlabDepth(poi);
   pierData.SetDeckThickness(tSlab);

   Float64 leftCLO  = pBridge->GetLeftInteriorCurbOffset(pierIdx);
   Float64 rightCLO = pBridge->GetRightInteriorCurbOffset(pierIdx);
   pierData.SetCurbLineDatum(pgsTypes::omtAlignment);
   pierData.SetCurbLineOffset(leftCLO,rightCLO);

   CComPtr<IAngleDisplayUnitFormatter> angle_formatter;
   angle_formatter.CoCreateInstance(CLSID_AngleDisplayUnitFormatter);
   angle_formatter->put_Signed(VARIANT_TRUE);
   CComBSTR bstrSkew;
   angle_formatter->Format(skew,CComBSTR(),&bstrSkew);
   pierData.SetSkew(OLE2T(bstrSkew));

   // Lower Cross Beam
   Float64 H1, H2, H3, H4;
   Float64 X1, X2, X3, X4, W;
   pPier->GetXBeamDimensions(pgsTypes::stLeft, &H1,&H2,&X1,&X2);
   pPier->GetXBeamDimensions(pgsTypes::stRight,&H3,&H4,&X3,&X4);
   W = pPier->GetXBeamWidth();
   pierData.SetLowerXBeamDimensions(H1,H2,H3,H4,X1,X2,X3,X4,W);

   // Upper Cross Beam Diaphragm. Basically, this is vertical distance from top of lower cross beam to bottom of slab
   // Take max of diaphragm depth and max girder bearing deducts
   // (don't use the pPier object here... use the pBridge interface... it resolves
   // diaphragm dimensions that are computed based on bridge component geometry)
   Float64 Wback, Hback;
   pBridge->GetPierDiaphragmSize(pierIdx,pgsTypes::Back,&Wback,&Hback);
   Float64 Wahead, Hahead;
   pBridge->GetPierDiaphragmSize(pierIdx,pgsTypes::Ahead,&Wahead,&Hahead);
   W = Wback + Wahead;
   Float64 Hdiap = Max(Hback,Hahead);

   Float64 Hbd = 0;

   // Compute elevation ignoring effects on non-recoverable deformations. We don't need to perform a full structural analysis to get the values we want
   std::vector<BearingElevationDetails> vBackElevDetails = pBridge->GetBearingElevationDetails(pierIdx, pgsTypes::Back, ALL_GIRDERS,true);
   for (const auto& elevdet : vBackElevDetails)
   {
      Hbd = max(Hbd, elevdet.BrgHeight + elevdet.Hg + elevdet.SlabOffset - elevdet.GrossSlabDepth);
   }

   std::vector<BearingElevationDetails> vAheadElevDetails = pBridge->GetBearingElevationDetails(pierIdx, pgsTypes::Ahead,ALL_GIRDERS,true);
   for (const auto& elevdet : vAheadElevDetails)
   {
      Hbd = max(Hbd, elevdet.BrgHeight + elevdet.Hg + elevdet.SlabOffset - elevdet.GrossSlabDepth);
   }

   Float64 H = max(Hdiap, Hbd);

   pierData.SetDiaphragmDimensions(H,W);

   // Column Layout
   ColumnIndexType refColIdx;
   Float64 refColOffset;
   pgsTypes::OffsetMeasurementType refColMeasure;
   pPier->GetTransverseOffset(&refColIdx,&refColOffset,&refColMeasure);
   pierData.SetRefColumnLocation(refColMeasure,refColIdx,refColOffset);

   Float64 X5, X6;
   pPier->GetXBeamOverhangs(&X5,&X6);
   pierData.SetXBeamOverhangs(X5,X6);

   // Column Properties
   ColumnIndexType nColumns = pPier->GetColumnCount();
   pierData.SetColumnCount(nColumns);
   for ( ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++ )
   {
      if ( colIdx < nColumns-1 )
      {
         Float64 S = pPier->GetColumnSpacing(colIdx);
         pierData.GetColumnSpacing(colIdx) = S;
      }
   
      const CColumnData& columnData = pPier->GetColumnData(colIdx);
      pierData.GetColumnData(colIdx) = columnData;
   }

   // Materials
   pierData.SetConcreteMaterial(pPier->GetConcrete());

   // Bearing Lines and Bearing Locations
   if ( pierData.GetPierType() == xbrTypes::pctExpansion )
   {
      // two bearing lines
      if ( pPier->IsAbutment() )
      {
         pierData.SetBearingLineCount(1);
      }
      else
      {
         pierData.SetBearingLineCount(2);
      }

      GirderIndexType gdrIdx = 0;

      if ( backGrpIdx != INVALID_INDEX )
      {
         CSegmentKey backSegmentKey  = pBridge->GetSegmentAtPier(pierIdx,CGirderKey(backGrpIdx, gdrIdx));

         Float64 backBrgOffset  = pBridge->GetSegmentEndBearingOffset(backSegmentKey);
         backBrgOffset *= -1; // offset to back side of pier is < 0

         Float64 refBrgOffset = pBridge->GetGirderOffset(gdrIdx,pierIdx,pgsTypes::Back,pgsTypes::omtAlignment);

         std::vector<Float64> vBackSpacing = pBridge->GetGirderSpacing(pierIdx,pgsTypes::Back,pgsTypes::AtPierLine,pgsTypes::AlongItem);
         xbrBearingLineData backBrgLine;
         backBrgLine.SetBearingLineOffset(backBrgOffset);
         backBrgLine.SetReferenceBearing(pgsTypes::omtAlignment,gdrIdx,refBrgOffset);
         backBrgLine.SetBearingCount(vBackSpacing.size()+1);
         backBrgLine.SetSpacing(vBackSpacing);
         pierData.SetBearingLineData(0,backBrgLine);

         std::vector<BearingReactions>& vBackBrgReactions = GetPrivateBearingReactions(pierID,0);
         vBackBrgReactions.resize(backBrgLine.GetBearingCount());
      }

      if ( aheadGrpIdx != INVALID_INDEX )
      {
         CSegmentKey aheadSegmentKey = pBridge->GetSegmentAtPier(pierIdx,CGirderKey(aheadGrpIdx,gdrIdx));
         Float64 aheadBrgOffset = pBridge->GetSegmentStartBearingOffset(aheadSegmentKey);
         Float64 refBrgOffset = pBridge->GetGirderOffset(gdrIdx,pierIdx,pgsTypes::Ahead,pgsTypes::omtAlignment);
         std::vector<Float64> vAheadSpacing = pBridge->GetGirderSpacing(pierIdx,pgsTypes::Ahead,pgsTypes::AtPierLine,pgsTypes::AlongItem);
         xbrBearingLineData aheadBrgLine;
         aheadBrgLine.SetBearingLineOffset(aheadBrgOffset);
         aheadBrgLine.SetReferenceBearing(pgsTypes::omtAlignment,gdrIdx,refBrgOffset);
         aheadBrgLine.SetBearingCount(vAheadSpacing.size()+1);
         aheadBrgLine.SetSpacing(vAheadSpacing);
         IndexType brgLineIdx = (backGrpIdx == INVALID_INDEX ? 0 : 1);
         pierData.SetBearingLineData(brgLineIdx,aheadBrgLine);

         std::vector<BearingReactions>& vAheadBrgReactions = GetPrivateBearingReactions(pierID,brgLineIdx);
         vAheadBrgReactions.resize(aheadBrgLine.GetBearingCount());
      }
   }
   else
   {
      // one bearing line
      pierData.SetBearingLineCount(1);

      pgsTypes::PierFaceType pierFace(pierIdx == 0 ? pgsTypes::Ahead : pgsTypes::Back);

      GirderIndexType gdrIdx = 0;
      Float64 refBrgOffset = pBridge->GetGirderOffset(gdrIdx,pierIdx,pierFace,pgsTypes::omtAlignment);

      std::vector<Float64> vSpacing = pBridge->GetGirderSpacing(pierIdx,pierFace,pgsTypes::AtPierLine,pgsTypes::AlongItem);
      xbrBearingLineData brgLine;
      brgLine.SetBearingLineOffset(0);
      brgLine.SetReferenceBearing(pgsTypes::omtAlignment,gdrIdx,refBrgOffset);
      brgLine.SetBearingCount(vSpacing.size()+1);
      brgLine.SetSpacing(vSpacing);
      pierData.SetBearingLineData(0,brgLine);

      m_BearingReactions[0][pierID].resize(brgLine.GetBearingCount());
   }

   Fire_OnProjectChanged();
}

PierIndexType CProjectAgentImp::GetPierIndex(PierIDType pierID) const
{
   if ( pierID == INVALID_ID )
   {
      return INVALID_INDEX;
   }

   GET_IFACE(IBridgeDescription,pIBridgeDesc);
   const CPierData2* pPier = pIBridgeDesc->FindPier(pierID);
   ATLASSERT(pPier != nullptr);
   return pPier->GetIndex();
}

CGirderKey CProjectAgentImp::GetGirderKey(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx) const
{
   ATLASSERT(pierID != INVALID_ID); // this method is only for piers that are part of a bridge 
   ATLASSERT(brgLineIdx < 2); // brgLineIdx can only be 0 or 1

   pgsTypes::PierFaceType pierFace = (brgLineIdx == 0 ? pgsTypes::Back : pgsTypes::Ahead);

   GET_IFACE(IBridgeDescription,pIBridgeDesc);
   const CPierData2* pPier = pIBridgeDesc->FindPier(pierID);
   ATLASSERT(pPier != nullptr);

   if ( pPier->IsAbutment() && pPier->GetIndex() == 0 )
   {
      pierFace = pgsTypes::Ahead;
   }

   // Get the girder group
   const CGirderGroupData* pGroup = pPier->GetGirderGroup(pierFace);
   ATLASSERT(pGroup != nullptr);
   GroupIndexType grpIdx = pGroup->GetIndex();

   // Assuming there is a one-to-one relationship between bearings and girders
   // This may change if we assume 2-bearings for U-Beams
   ATLASSERT(brgIdx < pGroup->GetGirderCount());
   GirderIndexType gdrIdx = brgIdx;

   return CGirderKey(grpIdx,gdrIdx);
}

bool CProjectAgentImp::UseUniformLoads(PierIDType pierID,IndexType brgLineIdx) const
{
   if ( pierID == INVALID_ID )
   {
      ATLASSERT(IsStandAlone());
      return (GetBearingReactionType(pierID,brgLineIdx) == xbrTypes::rltUniform);
   }

   CGirderKey girderKey = GetGirderKey(pierID,brgLineIdx,0);

   GET_IFACE(IBridgeDescription,pIBridgeDesc);
   const CSplicedGirderData* pGirder = pIBridgeDesc->GetGirder(girderKey);
   const GirderLibraryEntry* pGdrLibEntry = pGirder->GetGirderLibraryEntry();

   auto beamFactory = pGdrLibEntry->GetBeamFactory();

   CLSID clsidBeamFamily = beamFactory->GetFamilyCLSID();
   if ( (clsidBeamFamily == CLSID_SlabBeamFamily) ||
        (clsidBeamFamily == CLSID_BoxBeamFamily)  ||
        (clsidBeamFamily == CLSID_DeckedSlabBeamFamily)
      )
   {
      return true;
   }
   else
   {
      return false;
   }
}

GirderIndexType CProjectAgentImp::GetLongestGirderLine() const
{
   GET_IFACE(IBridge,pBridge);

   // base live load reactions on the longest girder line
   Float64 LglMax = -DBL_MAX;
   GirderIndexType nGirderlines = pBridge->GetGirderlineCount();
   GirderIndexType gdrIdx;
   for ( GirderIndexType glIdx = 0; glIdx < nGirderlines; glIdx++ )
   {
      Float64 Lgl = pBridge->GetGirderlineLength(glIdx);
      if ( LglMax < Lgl )
      {
         LglMax = Lgl;
         gdrIdx = glIdx;
      }
   }
   return gdrIdx;
}


void CProjectAgentImp::OnEditPier()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   GET_IFACE(IXBRProject, pProject);
   CPierDlg dlg(_T("Edit Pier"));

   PierIDType pierID = INVALID_ID;

   txnEditPierData oldPierData;
   oldPierData.m_PierData = pProject->GetPierData(pierID);

   IndexType nBearingLines = oldPierData.m_PierData.GetBearingLineCount();
   for (IndexType brgLineIdx = 0; brgLineIdx < nBearingLines; brgLineIdx++)
   {
      oldPierData.m_DeadLoadReactionType[brgLineIdx] = pProject->GetBearingReactionType(pierID, brgLineIdx);
      IndexType nBearings = oldPierData.m_PierData.GetBearingCount(brgLineIdx);
      for (IndexType brgIdx = 0; brgIdx < nBearings; brgIdx++)
      {
         txnDeadLoadReaction reaction;
         pProject->GetBearingReactions(pierID, brgLineIdx, brgIdx, &reaction.m_DC, &reaction.m_DW, &reaction.m_CR, &reaction.m_SH, &reaction.m_PS, &reaction.m_RE, &reaction.m_W);
         oldPierData.m_DeadLoadReactions[brgLineIdx].push_back(reaction);
      }
   }

   ATLASSERT(IsStandAlone());
   oldPierData.m_gDC_StrengthI = pProject->GetDCLoadFactor(pgsTypes::StrengthI_Inventory);
   oldPierData.m_gDW_StrengthI = pProject->GetDWLoadFactor(pgsTypes::StrengthI_Inventory);
   oldPierData.m_gCR_StrengthI = pProject->GetCRLoadFactor(pgsTypes::StrengthI_Inventory);
   oldPierData.m_gSH_StrengthI = pProject->GetSHLoadFactor(pgsTypes::StrengthI_Inventory);
   oldPierData.m_gPS_StrengthI = pProject->GetPSLoadFactor(pgsTypes::StrengthI_Inventory);

   oldPierData.m_gDC_StrengthII = pProject->GetDCLoadFactor(pgsTypes::StrengthII_PermitRoutine);
   oldPierData.m_gDW_StrengthII = pProject->GetDWLoadFactor(pgsTypes::StrengthII_PermitRoutine);
   oldPierData.m_gCR_StrengthII = pProject->GetCRLoadFactor(pgsTypes::StrengthII_PermitRoutine);
   oldPierData.m_gSH_StrengthII = pProject->GetSHLoadFactor(pgsTypes::StrengthII_PermitRoutine);
   oldPierData.m_gPS_StrengthII = pProject->GetPSLoadFactor(pgsTypes::StrengthII_PermitRoutine);

   oldPierData.m_gDC_ServiceI = pProject->GetDCLoadFactor(pgsTypes::ServiceI_PermitRoutine);
   oldPierData.m_gDW_ServiceI = pProject->GetDWLoadFactor(pgsTypes::ServiceI_PermitRoutine);
   oldPierData.m_gCR_ServiceI = pProject->GetCRLoadFactor(pgsTypes::ServiceI_PermitRoutine);
   oldPierData.m_gSH_ServiceI = pProject->GetSHLoadFactor(pgsTypes::ServiceI_PermitRoutine);
   oldPierData.m_gPS_ServiceI = pProject->GetPSLoadFactor(pgsTypes::ServiceI_PermitRoutine);

   oldPierData.m_gLL[0] = pProject->GetLiveLoadFactor(pierID, pgsTypes::StrengthI_Inventory, INVALID_INDEX);
   oldPierData.m_gLL[1] = pProject->GetLiveLoadFactor(pierID, pgsTypes::StrengthI_Operating, INVALID_INDEX);
   oldPierData.m_gLL[2] = pProject->GetLiveLoadFactor(pierID, pgsTypes::StrengthI_LegalRoutine, INVALID_INDEX);
   oldPierData.m_gLL[3] = pProject->GetLiveLoadFactor(pierID, pgsTypes::StrengthI_LegalSpecial, INVALID_INDEX);
   oldPierData.m_gLL[4] = pProject->GetLiveLoadFactor(pierID, pgsTypes::StrengthI_LegalEmergency, INVALID_INDEX);
   oldPierData.m_gLL[5] = pProject->GetLiveLoadFactor(pierID, pgsTypes::StrengthII_PermitRoutine, INVALID_INDEX);
   oldPierData.m_gLL[6] = pProject->GetLiveLoadFactor(pierID, pgsTypes::StrengthII_PermitSpecial, INVALID_INDEX);
   oldPierData.m_gLL[7] = pProject->GetLiveLoadFactor(pierID, pgsTypes::ServiceI_PermitRoutine, INVALID_INDEX);
   oldPierData.m_gLL[8] = pProject->GetLiveLoadFactor(pierID, pgsTypes::ServiceI_PermitSpecial, INVALID_INDEX);

   oldPierData.m_DesignLiveLoad.m_LLIM = pProject->GetLiveLoadReactions(pierID, pgsTypes::lrDesign_Inventory);
   oldPierData.m_LegalRoutineLiveLoad.m_LLIM = pProject->GetLiveLoadReactions(pierID, pgsTypes::lrLegal_Routine);
   oldPierData.m_LegalSpecialLiveLoad.m_LLIM = pProject->GetLiveLoadReactions(pierID, pgsTypes::lrLegal_Special);
   oldPierData.m_LegalEmergencyLiveLoad.m_LLIM = pProject->GetLiveLoadReactions(pierID, pgsTypes::lrLegal_Emergency);
   oldPierData.m_PermitRoutineLiveLoad.m_LLIM = pProject->GetLiveLoadReactions(pierID, pgsTypes::lrPermit_Routine);
   oldPierData.m_PermitSpecialLiveLoad.m_LLIM = pProject->GetLiveLoadReactions(pierID, pgsTypes::lrPermit_Special);

   dlg.SetEditPierData(oldPierData);
   if (dlg.DoModal() == IDOK)
   {
      txnEditPierData newPierData = dlg.GetEditPierData();
      txnEditPier txn(oldPierData, newPierData);
      GET_IFACE(IEAFTransactions, pTransactions);
      pTransactions->Execute(txn);
   }
}

void CProjectAgentImp::OnEditOptions()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   GET_IFACE(IXBRRatingSpecification, pRatingSpec);

   txnEditOptionsData oldOptions;
   oldOptions.m_LRFDEdition = WBFL::LRFD::BDSManager::GetEdition();
   oldOptions.m_LRFREdition = WBFL::LRFD::MBEManager::GetEdition();

   oldOptions.m_bDesignRating = pRatingSpec->IsRatingEnabled(pgsTypes::lrDesign_Inventory);
   oldOptions.m_bDesignRateForShear = pRatingSpec->RateForShear(pgsTypes::lrDesign_Inventory);

   oldOptions.m_bLegalRating = pRatingSpec->IsRatingEnabled(pgsTypes::lrLegal_Routine);
   oldOptions.m_bLegalRateForShear = pRatingSpec->RateForShear(pgsTypes::lrLegal_Routine);
   oldOptions.m_EmergencyRatingMethod = pRatingSpec->GetEmergencyRatingMethod();

   oldOptions.m_bPermitRating = pRatingSpec->IsRatingEnabled(pgsTypes::lrPermit_Routine);
   oldOptions.m_bPermitRateForShear = pRatingSpec->RateForShear(pgsTypes::lrPermit_Routine);
   oldOptions.m_bCheckYieldStress = pRatingSpec->CheckYieldStressLimit();
   oldOptions.m_YieldStressCoefficient = pRatingSpec->GetYieldStressLimitCoefficient();
   oldOptions.m_PermitRatingMethod = pRatingSpec->GetPermitRatingMethod();

   GET_IFACE(IXBRProject, pProject);
   oldOptions.m_MaxLLStepSize = pProject->GetMaxLiveLoadStepSize();
   oldOptions.m_MaxLoadedLanes = pProject->GetMaxLoadedLanes();
   oldOptions.m_LiveLoadReactionApplication = pProject->GetReactionLoadApplicationType(INVALID_ID);
   oldOptions.m_SystemFactorFlexure = pProject->GetSystemFactorFlexure();
   oldOptions.m_SystemFactorShear = pProject->GetSystemFactorShear();

   pProject->GetFlexureResistanceFactors(&oldOptions.m_PhiC, &oldOptions.m_PhiT);
   oldOptions.m_PhiV = pProject->GetShearResistanceFactor();

   oldOptions.m_bDoAnalyzeNegativeMomentBetweenFOC = pProject->GetDoAnalyzeNegativeMomentBetweenFocOptions(&oldOptions.m_MinColumnWidthForNegMoment);

   COptionsDlg dlg;
   dlg.SetOptions(oldOptions);
   if (dlg.DoModal() == IDOK)
   {
      txnEditOptionsData newOptions = dlg.GetOptions();

      txnEditOptions txn(oldOptions, newOptions);
      GET_IFACE(IEAFTransactions, pTransactions);
      pTransactions->Execute(txn);
   }
}

void CProjectAgentImp::OnProjectProperties()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   GET_IFACE(IXBRProjectProperties, pProjProp);

   CProjectPropertiesDlg dlg;

   dlg.m_Bridge = pProjProp->GetBridgeName();
   dlg.m_BridgeID = pProjProp->GetBridgeID();
   dlg.m_JobNumber = pProjProp->GetJobNumber();
   dlg.m_Engineer = pProjProp->GetEngineer();
   dlg.m_Company = pProjProp->GetCompany();
   dlg.m_Comments = pProjProp->GetComments();
   dlg.m_bShowProjectProperties = ShowProjectPropertiesOnNewProject();


   if (dlg.DoModal() == IDOK)
   {
      std::unique_ptr<txnEditProjectProperties> pTxn(std::make_unique<txnEditProjectProperties>(pProjProp->GetBridgeName(), dlg.m_Bridge,
         pProjProp->GetBridgeID(), dlg.m_BridgeID,
         pProjProp->GetJobNumber(), dlg.m_JobNumber,
         pProjProp->GetEngineer(), dlg.m_Engineer,
         pProjProp->GetCompany(), dlg.m_Company,
         pProjProp->GetComments(), dlg.m_Comments));


      ShowProjectPropertiesOnNewProject(dlg.m_bShowProjectProperties);

      GET_IFACE(IEAFTransactions, pTransactions);
      pTransactions->Execute(std::move(pTxn));
   }

}

void CProjectAgentImp::ShowProjectPropertiesOnNewProject(bool bShow)
{
   m_bShowProjectProperties = bShow;
}

bool CProjectAgentImp::ShowProjectPropertiesOnNewProject() const
{
   return m_bShowProjectProperties;
}

void CProjectAgentImp::PromptForProjectProperties()
{
   return OnProjectProperties();
}

//////////////////////////////////////////////////////////////////////
// IEAFCommandCallback
BOOL CProjectAgentImp::OnCommandMessage(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
   return OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CProjectAgentImp::GetStatusBarMessageString(UINT nID, CString& rMessage) const
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
      TRACE1("Warning (XBeamRate::ProjectAgent): no message line prompt for ID 0x%04X.\n", nID);
   }

   return TRUE;
}

BOOL CProjectAgentImp::GetToolTipMessageString(UINT nID, CString& rMessage) const
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CString string;
   // load appropriate string
   if (string.LoadString(nID))
   {
      // tip is after first newline 
      int pos = string.Find('\n');
      if (0 < pos)
      {
         rMessage = string.Mid(pos + 1);
      }
   }
   else
   {
      // not found
      TRACE1("Warning (XBeamRate::ProjectAgent): no tool tip for ID 0x%04X.\n", nID);
   }

   return TRUE;
}
