///////////////////////////////////////////////////////////////////////
// XBeamRate - Cross Beam Load Rating
// Copyright � 1999-2026  Washington State Department of Transportation
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


// ReportAgentImp.cpp : Implementation of CReportAgentImp
#include "stdafx.h"
#include "ReportAgent.h"
#include "ReportAgentImp.h"

#include <EAF/EAFReportManager.h>

#include "XBeamRateReportSpecificationBuilder.h"

#include <XBeamRateExt\XBeamRateUtilities.h>

#include "XBeamRateReportBuilder.h"
#include "XBeamRateTitlePageBuilder.h"

#include "LoadRatingChapterBuilder.h"
#include "LoadRatingDetailsChapterBuilder.h"
#include "PierDescriptionDetailsChapterBuilder.h"
#include "LoadingDetailsChapterBuilder.h"
#include "MomentCapacityDetailsChapterBuilder.h"
#include "ShearCapacityDetailsChapterBuilder.h"

#include <..\..\PGSuper\Include\IFace\Views.h>
#include <EAF\EAFUIIntegration.h>
#include <EAF\EAFReportView.h>

//////////////////////////////////////////////////////////////////////
// IAgent
bool CReportAgentImp::RegisterInterfaces()
{
   EAF_AGENT_REGISTER_INTERFACES;
   return true;
};

bool CReportAgentImp::Init()
{
   EAF_AGENT_INIT;

   InitReportBuilders();

   //
   // Attach to connection points
   //
   m_dwProjectCookie = REGISTER_EVENT_SINK(IXBRProjectEventSink);

   return true;
}

bool CReportAgentImp::Reset()
{
   EAF_AGENT_RESET;
   return true;
}

CLSID CReportAgentImp::GetCLSID() const
{
   return CLSID_XBeamRateReportAgent;
}

bool CReportAgentImp::ShutDown()
{
   EAF_AGENT_SHUTDOWN;
   //
   // Detach to connection points
   //
   UNREGISTER_EVENT_SINK(IXBRProjectEventSink, m_dwProjectCookie);

   return true;
}

/////////////////////////////////////////////////////////////
void CReportAgentImp::InitReportBuilders()
{
   GET_IFACE(IEAFReportManager,pRptMgr);

   std::shared_ptr<WBFL::ReportMgr::ReportSpecificationBuilder> pRptSpecBuilder(std::make_shared<CXBeamRateReportSpecificationBuilder>(m_pBroker) );

   std::shared_ptr<WBFL::ReportMgr::ReportBuilder> pReportBuilder(std::make_shared<CXBeamRateReportBuilder>(IsStandAlone() ? _T("Load Rating Report") : _T("Cross Beam Load Rating Report")));
   m_ReportNames.insert(pReportBuilder->GetName());
#if defined _DEBUG || defined _BETA_VERSION
   pReportBuilder->IncludeTimingChapter();
#endif
   pReportBuilder->SetReportSpecificationBuilder( pRptSpecBuilder );
   pReportBuilder->SetTitlePageBuilder(std::shared_ptr<WBFL::ReportMgr::TitlePageBuilder>(new CXBeamRateTitlePageBuilder(pReportBuilder->GetName())));
   pReportBuilder->AddChapterBuilder(std::shared_ptr<WBFL::ReportMgr::ChapterBuilder>(new CLoadRatingChapterBuilder()));
   pReportBuilder->AddChapterBuilder(std::shared_ptr<WBFL::ReportMgr::ChapterBuilder>(new CLoadRatingDetailsChapterBuilder()));
   pReportBuilder->AddChapterBuilder(std::shared_ptr<WBFL::ReportMgr::ChapterBuilder>(new CPierDescriptionDetailsChapterBuilder()));
   pReportBuilder->AddChapterBuilder(std::shared_ptr<WBFL::ReportMgr::ChapterBuilder>(new CLoadingDetailsChapterBuilder()));
   pReportBuilder->AddChapterBuilder(std::shared_ptr<WBFL::ReportMgr::ChapterBuilder>(new CMomentCapacityDetailsChapterBuilder()));
   pReportBuilder->AddChapterBuilder(std::shared_ptr<WBFL::ReportMgr::ChapterBuilder>(new CShearCapacityDetailsChapterBuilder()));
   pRptMgr->AddReportBuilder(pReportBuilder);
}

/////////////////////////////////////////////////////////////////////////////////
// IXBRProjectEventSink
HRESULT CReportAgentImp::OnProjectChanged()
{
   if ( !IsStandAlone() )
   {
      // we are plugged into PGSuper/PGSplice
      // Something in our project changed
      // Look at the open report views, if any of them are displaying a cross beam load rating report
      // tell that view to update.
      GET_IFACE(IViews,pViews);
      GET_IFACE(IEAFViewRegistrar,pViewRegistrar);

      // get all the report views
      long rptViewKey = pViews->GetReportViewKey();
      std::vector<CView*> vViews = pViewRegistrar->GetRegisteredView(rptViewKey);
      for (const auto& pView : vViews)
      {
         // make sure it is the right type so we can cast it
         if ( pView->IsKindOf(RUNTIME_CLASS(CEAFReportView)) )
         {
            // Get the report spec so we can get the report name
            CEAFReportView* pReportView = (CEAFReportView*)pView;
            auto pReportSpec = pReportView->GetReportSpecification();

            // is it one of our reports?
            std::set<std::_tstring>::iterator found = m_ReportNames.find(pReportSpec->GetReportName());
            if ( found != m_ReportNames.end() )
            {
               // yes, it is one of our reports... update it.
               pReportView->OnUpdate(nullptr, 0, nullptr);
            }
         }
         else
         {
            ATLASSERT(false); // did PGSuper/PGSplice change the base class of the report view?
         }
      }
   }

   return S_OK;
}
