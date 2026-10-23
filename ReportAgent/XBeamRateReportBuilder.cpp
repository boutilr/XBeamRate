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
#include "XBeamRateReportBuilder.h"
#include "XBeamRateReportSpecification.h"

#include <..\..\PGSuper\Include\IFace\Project.h>
#include <XBeamRateExt\XBeamRateUtilities.h>
#include <PsgLib\GirderLabel.h>

#include <EAF/AutoProgress.h>


CXBeamRateReportBuilder::CXBeamRateReportBuilder(LPCTSTR strName,bool bHidden,bool bIncludeTimingChapter) :
WBFL::ReportMgr::ReportBuilder(strName,bHidden,bIncludeTimingChapter)
{
}

std::shared_ptr<rptReport> CXBeamRateReportBuilder::CreateReport(const std::shared_ptr<const WBFL::ReportMgr::ReportSpecification> pRptSpec) const
{
   auto pXBRRptSpec = std::dynamic_pointer_cast<const CXBeamRateReportSpecification>(pRptSpec);

   auto pBroker = pXBRRptSpec->GetBroker();


   GET_IFACE2(pBroker,IEAFProgress, pProgress);
   WBFL::EAF::AutoProgress ap(pProgress,0);
   CString strMsg;
   strMsg.Format(_T("Building %s"),pRptSpec->GetReportName().c_str());
   pProgress->UpdateMessage(strMsg);

   PierIDType pierID = pXBRRptSpec->GetPierID();

   if ( IsPGSExtension() )
   {
      GET_IFACE2(pBroker,IBridgeDescription,pIBridgeDesc);
      const CPierData2* pPier = pIBridgeDesc->FindPier(pierID);
      if ( pPier->GetPierModelType() == pgsTypes::pmtIdealized )
      {
         std::shared_ptr<rptReport> pReport(std::make_shared<rptReport>(pRptSpec->GetReportName()) );
         rptChapter* pChapter = new rptChapter;
         (*pReport) << pChapter;

         rptParagraph* pPara = new rptParagraph;
         *pChapter << pPara;

         CString strMsg;
         strMsg.Format(_T("Pier %s is modeled as idealized. Idealized piers cannot be load rated."),LABEL_PIER(pPier->GetIndex()));
         *pPara << strMsg << rptNewLine;
         
         return pReport;
      }
   }
#if defined _DEBUG
   else
   {
      ATLASSERT(pierID == INVALID_ID);
   }
#endif

   return WBFL::ReportMgr::ReportBuilder::CreateReport(pRptSpec);
}
