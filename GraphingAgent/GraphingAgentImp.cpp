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

// GraphingAgentImp.cpp : Implementation of CGraphingAgentImp
#include "stdafx.h"
#include "GraphingAgent.h"
#include "GraphingAgentImp.h"
#include <EAF/EAFGraphManager.h>

#include "AnalysisResultsGraphBuilder.h"
#include "LiveLoadGraphBuilder.h"

#include <IFace\XBeamRateAgent.h>
#include <IFace\Views.h>
#include <EAF\EAFUIIntegration.h>
#include <EAF\EAFDocument.h>

#include <XBeamRateExt\XBeamRateUtilities.h>


bool CGraphingAgentImp::RegisterInterfaces()
{
   EAF_AGENT_REGISTER_INTERFACES;
   return true;
};

bool CGraphingAgentImp::Init()
{
   EAF_AGENT_INIT;

   InitGraphBuilders();
   //
   // Attach to connection points
   //
   m_dwProjectCookie = REGISTER_EVENT_SINK(IXBRProjectEventSink);

   return true;
}

bool CGraphingAgentImp::Reset()
{
   EAF_AGENT_RESET;
   return true;
}

CLSID CGraphingAgentImp::GetCLSID() const
{
   return CLSID_XBeamRateGraphingAgent;
}

bool CGraphingAgentImp::ShutDown()
{
   EAF_AGENT_SHUTDOWN;

   //
   // Detach to connection points
   //
   UNREGISTER_EVENT_SINK(IXBRProjectEventSink, m_dwProjectCookie);

   return true;
}

HRESULT CGraphingAgentImp::InitGraphBuilders()
{
   GET_IFACE(IEAFGraphManager,pGraphMgr);

   std::unique_ptr<CXBRAnalysisResultsGraphBuilder> pAnalysisResultsGraphBuilder = std::make_unique<CXBRAnalysisResultsGraphBuilder>();
   pAnalysisResultsGraphBuilder->InitDocumentation(_T("XBRate"),IDH_ANALYSIS_RESULTS);
   
   if ( IsPGSExtension() )
   {
      // XBeam Rate is acting as an extension to PGSuper/PGSplice
      // Change the default graph name so it doesn't conflict with PGSuper/PGSplice
      pAnalysisResultsGraphBuilder->SetName(_T("Cross Beam Analysis Results"));
   }

   VERIFY(pGraphMgr->AddGraphBuilder( std::move(pAnalysisResultsGraphBuilder) ));


   std::unique_ptr<CXBRLiveLoadGraphBuilder> pLiveLoadGraphBuilder = std::make_unique<CXBRLiveLoadGraphBuilder>();
   pLiveLoadGraphBuilder->InitDocumentation(_T("XBRate"),IDH_LIVE_LOAD_RESULTS);
   
   if ( IsPGSExtension() )
   {
      // XBeam Rate is acting as an extension to PGSuper/PGSplice
      // Change the default graph name so it doesn't conflict with PGSuper/PGSplice
      pLiveLoadGraphBuilder->SetName(_T("Cross Beam Live Load Results"));
   }

   VERIFY(pGraphMgr->AddGraphBuilder( std::move(pLiveLoadGraphBuilder) ));

   return S_OK;
}

//////////////////////////////////////////////////////////////////////
// IProjectEventSink
HRESULT CGraphingAgentImp::OnProjectChanged()
{
   if ( IsStandAlone() )
   {
      // not extending PGSuper/PGSplice so we don't need to update PGSuper/PGSplice's graph view
      return S_OK;
   }

   GET_IFACE(IViews,pViews);
   long graphingViewKey = pViews->GetGraphingViewKey();

   if ( graphingViewKey < 0 )
   {
      // view doesn't exist
      return S_OK;
   }

   AFX_MANAGE_STATE(AfxGetAppModuleState());
   GET_IFACE(IEAFViewRegistrar,pViewRegistrar);
   pViewRegistrar->UpdateRegisteredView(graphingViewKey,nullptr,0,nullptr);

   return S_OK;
}
