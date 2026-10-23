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

#include "StdAfx.h"
#include "ReportAgent.h"
#include "XBeamRateReportSpecificationBuilder.h"
#include "XBeamRateReportSpecification.h"
#include "PierReportDlg.h"

#include <XBeamRateExt\XBeamRateUtilities.h>
#include <..\..\PGSuper\Include\IFace\Project.h>
#include <IFace\Selection.h>



CXBeamRateReportSpecificationBuilder::CXBeamRateReportSpecificationBuilder(std::weak_ptr<WBFL::EAF::Broker> pBroker) :
WBFL::EAF::BrokerReportSpecificationBuilder(pBroker)
{
}

CXBeamRateReportSpecificationBuilder::~CXBeamRateReportSpecificationBuilder(void)
{
}

std::shared_ptr<WBFL::ReportMgr::ReportSpecification> CXBeamRateReportSpecificationBuilder::CreateReportSpec(const WBFL::ReportMgr::ReportDescription& rptDesc,std::shared_ptr<WBFL::ReportMgr::ReportSpecification> pOldRptSpec) const
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   std::shared_ptr<CXBeamRateReportSpecification> pOldGRptSpec = std::dynamic_pointer_cast<CXBeamRateReportSpecification>(pOldRptSpec);

   CPierReportDlg dlg(rptDesc,pOldRptSpec);

   if (pOldGRptSpec)
   {
      dlg.m_bReportEvenIncrements = pOldGRptSpec->GetDoReportEvenIncrements();
   }

   if ( dlg.DoModal() == IDOK )
   {
      // If possible, copy information from old spec. Otherwise header/footer and other info will be lost
      std::shared_ptr<WBFL::ReportMgr::ReportSpecification> pNewRptSpec;
      if(pOldGRptSpec)
      {
         std::shared_ptr<CXBeamRateReportSpecification> pNewGRptSpec(std::make_shared<CXBeamRateReportSpecification>(*pOldGRptSpec) );
         pNewGRptSpec->SetPierID(dlg.m_PierID);
         pNewGRptSpec->SetDoReportEvenIncrements(dlg.m_bReportEvenIncrements);

         pNewRptSpec = std::static_pointer_cast<WBFL::ReportMgr::ReportSpecification>(pNewGRptSpec);
      }
      else
      {
         pNewRptSpec = std::make_shared<CXBeamRateReportSpecification>(rptDesc.GetReportName(),m_pBroker,dlg.m_PierID, dlg.m_bReportEvenIncrements);
      }

      std::vector<std::_tstring> chList = dlg.m_ChapterList;
      rptDesc.ConfigureReportSpecification(chList,pNewRptSpec);

      return pNewRptSpec;
   }

   return nullptr;
}

std::shared_ptr<WBFL::ReportMgr::ReportSpecification> CXBeamRateReportSpecificationBuilder::CreateDefaultReportSpec(const WBFL::ReportMgr::ReportDescription& rptDesc) const
{
   PierIDType pierID = INVALID_ID;

   if ( IsPGSExtension() )
   {
      GET_IFACE2(GetBroker(),ISelection,pSelection);
      PierIndexType selPierIdx = pSelection->GetSelectedPier();

      GET_IFACE2(GetBroker(),IBridgeDescription,pIBridgeDesc);
      PierIndexType nPiers = pIBridgeDesc->GetPierCount();

      if ( selPierIdx == INVALID_INDEX )
      {
         return CreateReportSpec(rptDesc,std::shared_ptr<WBFL::ReportMgr::ReportSpecification>());
      }

      if ( selPierIdx != INVALID_INDEX )
      {
         const CPierData2* pPier = pIBridgeDesc->GetPier(selPierIdx);
         pierID = pPier->GetID();
      }
   }

   // Use all chapters at the maximum level, and report at even increments
   std::shared_ptr<WBFL::ReportMgr::ReportSpecification> pRptSpec(std::make_shared<CXBeamRateReportSpecification>(rptDesc.GetReportName(),m_pBroker,pierID,true) );
   rptDesc.ConfigureReportSpecification(pRptSpec);
   return pRptSpec;
}
