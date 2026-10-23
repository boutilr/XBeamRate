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

// XBeamRateAgent.h : Declaration of the CXBeamRateAgent

// This agent is a PGSuper/PGSplice extension agent.

#pragma once
#include <EAF\Agent.h>
#include "CLSID.h"
#include "ReinforcementPageParent.h"


#include <EAF\EAFUIIntegration.h>
#include <EAF\ComponentObject.h>

#include <IFace\XBeamRateAgent.h>
#include <IFace\Project.h>

#include <IFace\ExtendUI.h>
#include <..\..\PGSuper\Include\IFace\Project.h>

#include <IFace\ViewEvents.h>
#include <IFace\VersionInfo.h>

class CXBeamRateAgent : public CCmdTarget, // it's very important CCmdTarget is the first parent for inheritance, see Warning C4407
   public WBFL::EAF::Agent,
   public IXBeamRateAgent,
   public WBFL::EAF::IAgentUIIntegration,
   public WBFL::EAF::IAgentDocumentationIntegration,
   public IEditPierCallback,
   public IEditLoadRatingOptionsCallback,
   public IProjectPropertiesEventSink,
   public IXBRProjectEventSink,
   public IXBRVersionInfo,
   public IEAFProcessCommandLine,
   public IBridgePlanViewEventCallback,
   public WBFL::EAF::ICommandCallback
{
public:
   CXBeamRateAgent() = default;

// Agent
public:
   bool SetBroker(std::shared_ptr<WBFL::EAF::Broker> broker) override;
   std::_tstring GetName() const override { return _T("XBeamRate Agent"); }
   bool RegisterInterfaces() override;
   bool Init() override;
   bool Reset() override;
   bool ShutDown() override;
   CLSID GetCLSID() const override;


// IXBeamRateAgent
public:
   bool IsExtendingPGSuper() override;

// IXBRVersionInfo
public:
   CString GetVersionString(bool bIncludeBuildNumber=false) override;
   CString GetVersion(bool bIncludeBuildNumber=false) override;

// IAgentUIIntegration
public:
   bool IntegrateWithUI(bool bIntegrate) override;

// IAgentDocumentationIntegration
public:
   CString GetDocumentationSetName() const override;
   bool LoadDocumentationMap() override;
   std::pair<WBFL::EAF::HelpResult,CString> GetDocumentLocation(UINT nHID) const override;

// IEditPierCallback
public:
   CPropertyPage* CreatePropertyPage(IEditPierData* pEditPierData) override;
   CPropertyPage* CreatePropertyPage(IEditPierData* pEditPierData,CPropertyPage* pBridgePropertyPage) override;
   std::unique_ptr<WBFL::EAF::Transaction> OnOK(CPropertyPage* pPage,IEditPierData* pEditPierData) override;
   IDType GetEditBridgeCallbackID() override;

// IEditLoadRatingOptionsCallback
public:
   CPropertyPage* CreatePropertyPage(IEditLoadRatingOptions* pLoadRatingOptions) override;
   std::unique_ptr<WBFL::EAF::Transaction> OnOK(CPropertyPage* pPage,IEditLoadRatingOptions* pLoadRatingOptions) override;

// IProjectPropertiesEventSink
public:
   HRESULT OnProjectPropertiesChanged() override;

// IXBRProjectEventSink
public:
   HRESULT OnProjectChanged() override;

// IEAFProcessCommandLine
public:
   BOOL ProcessCommandLineOptions(CEAFCommandLineInfo& cmdInfo) override;

// IBridgePlanViewEventCallback
public:
   void OnBackgroundContextMenu(std::shared_ptr<WBFL::EAF::Menu> menu) override;
   void OnPierContextMenu(PierIndexType pierIdx, std::shared_ptr<WBFL::EAF::Menu> menu) override;
   void OnSpanContextMenu(SpanIndexType spanIdx, std::shared_ptr<WBFL::EAF::Menu> menu) override;
   void OnDeckContextMenu(std::shared_ptr<WBFL::EAF::Menu> menu) override;
   void OnAlignmentContextMenu(std::shared_ptr<WBFL::EAF::Menu> menu) override;
   void OnSectionCutContextMenu(std::shared_ptr<WBFL::EAF::Menu> menu) override;
   void OnGirderContextMenu(const CGirderKey& girderKey, std::shared_ptr<WBFL::EAF::Menu> menu) override;
   void OnTemporarySupportContextMenu(SupportIDType tsID, std::shared_ptr<WBFL::EAF::Menu> menu) override;
   void OnGirderSegmentContextMenu(const CSegmentKey& segmentKey, std::shared_ptr<WBFL::EAF::Menu> menu) override;
   void OnClosureJointContextMenu(const CSegmentKey& closureKey, std::shared_ptr<WBFL::EAF::Menu> menu) override;

// ICommandCallback
public:
   BOOL OnCommandMessage(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) override;
   BOOL GetStatusBarMessageString(UINT nID, CString& rMessage) const override;
   BOOL GetToolTipMessageString(UINT nID, CString& rMessage) const override;

   afx_msg void OnViewPier();
   afx_msg void OnExportPier();
   afx_msg void OnPierCommandUpdate(CCmdUI* pCmdUI);
   DECLARE_MESSAGE_MAP()

private:
   EAF_DECLARE_AGENT_DATA;

   IDType m_dwProjectPropertiesCookie;
   IDType m_dwProjectCookie;

   CReinforcementPageParent m_ReinforcementPageParent;

   IDType m_BridgePlanViewCallbackID;

   CString GetDocumentationURL() const;
   std::map<UINT,CString> m_HelpTopics;

   void CreateMenus();
   void RemoveMenus();

   void CreateToolbar();
   void RemoveToolbar();

   void RegisterViews();
   void UnregisterViews();
   void CreatePierView();
   long m_PierViewKey = -1;

   void RegisterUIExtensions();
   void UnregisterUIExtensions();
   IDType m_EditPierCallbackID;
   IDType m_EditLoadRatingOptionsCallbackID;
};
