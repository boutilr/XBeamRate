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


// ReportAgentImp.h : Declaration of the CReportAgentImp

// This agent is responsible for creating structural analysis models
// and providing analysis results

#pragma once

#include <EAF\Agent.h>
#include <ReportAgent.h>
#include "ReportAgentCLSID.h"



#include <IFace\Project.h>

/////////////////////////////////////////////////////////////////////////////
// CReportAgentImp
class CReportAgentImp : public WBFL::EAF::Agent,
   public IXBRProjectEventSink
{
public:
   CReportAgentImp() = default;

// Agent
public:
   std::_tstring GetName() const override { return _T("XBeam Rate Reporting Agent"); }
   bool RegisterInterfaces() override;
   bool Init() override;
   bool Reset() override;
   bool ShutDown() override;
   CLSID GetCLSID() const override;

// IXBRProjectEventSink
public:
   HRESULT OnProjectChanged() override;

private:
   EAF_DECLARE_AGENT_DATA;

   IDType m_dwProjectCookie;

   void InitReportBuilders();

   std::set<std::_tstring> m_ReportNames;
};

