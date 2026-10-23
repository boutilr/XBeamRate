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

#pragma once

#include <EAF\Agent.h>

#include <IFace\Project.h>
#include <IFace\XBeamRateAgent.h>
#include <IFace\RatingSpecification.h>
#include <IFace\VersionInfo.h>
#include <EAF\EAFDisplayUnits.h>

class CXBeamRateDoc;

// {0AA45B23-96C9-431c-BBC9-1D8CCC2BE5FC}
DEFINE_GUID(CLSID_XBeamRateDocProxyAgent, 
0xaa45b23, 0x96c9, 0x431c, 0xbb, 0xc9, 0x1d, 0x8c, 0xcc, 0x2b, 0xe5, 0xfc);

/*****************************************************************************
CLASS 
   CXBeamRateDocProxyAgent

   Proxy agent for CXBeamRate document.

   Instances of this object allow the CDocument class to be plugged into the
   Agent-Broker architecture.
*****************************************************************************/

class CXBeamRateDocProxyAgent : public WBFL::EAF::Agent,
   public WBFL::EAF::IAgentPriority,
   public WBFL::EAF::IAgentUIIntegration,
   public IXBRUIEvents,
   public IXBRProjectEventSink,
   public IEAFDisplayUnitsEventSink,
   public IXBRVersionInfo,
   public IXBRViews,
   public IXBREditByUI
{
public:
   CXBeamRateDocProxyAgent(CXBeamRateDoc* pDoc);
   ~CXBeamRateDocProxyAgent();

public:
   void OnStatusChanged();
   void OnUIHintsReset();

// Agent
public:
   std::_tstring GetName() const override { return _T("XBeamRate Doc Proxy Agent"); }
   bool RegisterInterfaces() override;
   bool Init() override;
   bool Reset() override;
   bool ShutDown() override;
   CLSID GetCLSID() const override;

   // IAgentPriority
public:
   IndexType GetPriority() const override;

// IAgentUIIntegration
public:
   bool IntegrateWithUI(bool bIntegrate) override;

// IXBRProjectEventSink
public:
   HRESULT OnProjectChanged() override;

// IEAFDisplayUnitsEventSink
public:
   HRESULT OnUnitsChanging() override;
   HRESULT OnUnitsChanged(WBFL::EAF::UnitMode newUnitsMode) override;

// IXBRUIEvents
public:
   void HoldEvents(bool bHold=true) override;
   void FirePendingEvents() override;
   void CancelPendingEvents() override;
   void FireEvent(CView* pSender = nullptr,LPARAM lHint = 0,std::shared_ptr<CObject> pHint = nullptr) override;

// IXBRVersionInfo
public:
   CString GetVersionString(bool bIncludeBuildNumber=false) override;
   CString GetVersion(bool bIncludeBuildNumber=false) override;

// IXBRViews
public:
   void CreateReportView(IndexType rptIdx,BOOL bPromptForSpec=TRUE) override;
   void CreateGraphView(IndexType graphIdx) override;
   void CreatePierView() override;

// IXBREditByUI
public:
   UINT GetStdToolBarID() override;

private:
   EAF_DECLARE_AGENT_DATA;

   void AdviseEventSinks();
   void UnadviseEventSinks();
   IDType m_dwProjectCookie;
   IDType m_dwDisplayUnitsCookie;

   void CreateToolBars();
   void RemoveToolBars();
   UINT m_StdToolBarID;

   void CreateAcceleratorKeys();
   void RemoveAcceleratorKeys();

   void CreateStatusBar();
   void ResetStatusBar();

   void RegisterViews();
   void UnregisterViews();
   long m_ReportViewKey;
   long m_GraphingViewKey;
   long m_PierViewKey;
   
   CXBeamRateDoc* m_pMyDocument;

   int m_EventHoldCount;
   bool m_bFiringEvents;
   struct UIEvent
   {
      CView* pSender;
      LPARAM lHint;
      std::shared_ptr<CObject> pHint;
   };
   std::vector<UIEvent> m_UIEvents;
};

