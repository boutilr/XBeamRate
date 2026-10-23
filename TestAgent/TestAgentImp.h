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

// TestAgentImp.h : Declaration of the CTestAgentImp

// This agent is responsible for creating structural analysis models
// and providing analysis results

#pragma once

#include <EAF\Agent.h>
#include <TestAgent.h>
#include "TestAgentCLSID.h"

#include <IFace/Test.h>


class xbrPointOfInterest;

/////////////////////////////////////////////////////////////////////////////
// CTestAgentImp
class CTestAgentImp : public WBFL::EAF::Agent,
   public IXBRTest
{  
public:
   CTestAgentImp() = default;;

// Agent
public:
   std::_tstring GetName() const override { return _T("XBeam Rate Test Agent"); }
   bool RegisterInterfaces() override;
   bool Init() override;
   bool Reset() override;
   bool ShutDown() override;
   CLSID GetCLSID() const override;

// IXBRTest
public:
   HRESULT RunTest(PierIDType pierID,LPCTSTR lpszResultsFile) override;

private:
   EAF_DECLARE_AGENT_DATA;

   CString GetProcessID();

   void RunProductLoadActionsTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi);
   void RunCombinedLoadActionsTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi);
   void RunLiveLoadActionsTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi);
   void RunLimitStateLoadActionsTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi);
   void RunMomentCapacityTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi);
   void RunShearCapacityTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi);
   void RunCrackedSectionTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi);
   void RunLoadRatingTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID);
};

