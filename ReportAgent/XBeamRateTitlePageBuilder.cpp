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
#include "ReportAgent.h"
#include <XBeamRateTitlePageBuilder.h>
#include <XBeamRateReportSpecification.h>

#include <IFace\VersionInfo.h>
#include <IFace\Project.h>
#include <EAF/EAFStatusCenter.h>

#include <..\..\PGSuper\Include\IFace\Project.h>
#include <EAF\EAFDocument.h>

#include <PsgLib\PierData2.h>
#include <PsgLib\GirderLabel.h>

#include <MFCTools\AutoRegistry.h>


std::unique_ptr<WBFL::ReportMgr::TitlePageBuilder> CXBeamRateTitlePageBuilder::Clone() const
{
   return std::make_unique<CXBeamRateTitlePageBuilder>(*this);
}

bool CXBeamRateTitlePageBuilder::NeedsUpdate(const std::shared_ptr<const WBFL::ReportMgr::ReportHint>& pHint,const std::shared_ptr<const WBFL::ReportMgr::ReportSpecification>& pRptSpec) const
{
   // don't let the title page control whether or not a report needs updating
   return false;
}

rptChapter* CXBeamRateTitlePageBuilder::Build(const std::shared_ptr<const WBFL::ReportMgr::ReportSpecification>& pRptSpec) const
{
   auto pXBRRptSpec = std::dynamic_pointer_cast<const CXBeamRateReportSpecification>(pRptSpec);
   auto pBroker = pXBRRptSpec->GetBroker();

   // Create a title page for the report
   rptChapter* pTitlePage = new rptChapter;

   rptParagraph* pPara = new rptParagraph;
   pPara->SetStyleName(rptStyleManager::GetReportTitleStyle());
   *pTitlePage << pPara;

   std::_tstring title = GetReportTitle();

   *pPara << title.c_str();

   if (pXBRRptSpec->GetPierID() != INVALID_INDEX)
   {
      GET_IFACE2(pBroker,IBridgeDescription, pIBridgeDesc);

      const CPierData2* pPier = pIBridgeDesc->FindPier(pXBRRptSpec->GetPierID());
      ATLASSERT(pPier);

      *pPara << rptNewLine;
      *pPara << _T("for Pier ") << LABEL_PIER(pPier->GetIndex()) << rptNewLine;
   }


   pPara = new rptParagraph;
   pPara->SetStyleName(rptStyleManager::GetReportTitleStyle());
   *pTitlePage << pPara;
#if defined _WIN64
   *pPara << _T("XBRate") << Super(symbol(TRADEMARK)) << _T(" (x64)") << rptNewLine;
#else
   *pPara << _T("XBRate") << Super(symbol(TRADEMARK)) << _T(" (x86)") << rptNewLine;
#endif

   pPara = new rptParagraph(rptStyleManager::GetCopyrightStyle());
   *pTitlePage << pPara;
   *pPara << _T("Copyright ") << symbol(COPYRIGHT) << _T(" ") << WBFL::System::Date().Year() << _T(", WSDOT, All Rights Reserved") << rptNewLine;

   pPara = new rptParagraph;
   pPara->SetStyleName(rptStyleManager::GetReportSubtitleStyle());
   *pTitlePage << pPara;
   GET_IFACE2(pBroker,IXBRVersionInfo, pVerInfo);
   *pPara << pVerInfo->GetVersionString() << rptNewLine;

   // Title page art image.
   // Can't use rptStyleManager method because it is for PGSuper/PGSplice.
   // We will just do it here locally.
   CAutoRegistry ar(_T("XBeamRate"));
   CWinApp* pApp = (CWinApp*)AfxGetApp();
   std::_tstring strImageName(pApp->GetProfileString(_T("Settings"), _T("ReportCoverImage"), _T("xbr_title_page_art.gif")));
   std::_tstring strImage;
   if (strImageName == _T("xbr_title_page_art.gif"))
   {
      strImage = std::_tstring(rptStyleManager::GetImagePath()) + strImageName;
   }
   else
   {
      strImage = strImageName;
   }

   WIN32_FIND_DATA file_find_data;
   HANDLE hFind;
   hFind = FindFirstFile(strImage.c_str(), &file_find_data);
   if (hFind != INVALID_HANDLE_VALUE)
   {
      *pPara << rptRcImage(strImage) << rptNewLine;
   }
   *pPara << rptNewLine << rptNewLine;

   ////////////////////////////
   // If this is stand alone, use the IXBRProjectProperties, otherwise
   // use the IProjectProperties and report in PGSuper format
   ////////////////////////////

   GET_IFACE2(pBroker,IEAFDocument, pDocument);
   GET_IFACE2(pBroker,IXBRProjectProperties, pProps);
   rptParagraph* pPara3 = new rptParagraph(rptStyleManager::GetHeadingStyle());
   *pTitlePage << pPara3;

   rptRcTable* pTbl = rptStyleManager::CreateTableNoHeading(2, _T("Project Properties"));

   pTbl->SetColumnStyle(0, rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   pTbl->SetColumnStyle(1, rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   pTbl->SetStripeRowColumnStyle(0, rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));
   pTbl->SetStripeRowColumnStyle(1, rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));

   *pPara3 << rptNewLine << rptNewLine << rptNewLine;

   *pPara3 << pTbl;
   (*pTbl)(0, 0) << _T("Bridge Name");
   (*pTbl)(0, 1) << pProps->GetBridgeName();
   (*pTbl)(1, 0) << _T("Bridge ID");
   (*pTbl)(1, 1) << pProps->GetBridgeID();
   (*pTbl)(2, 0) << _T("Company");
   (*pTbl)(2, 1) << pProps->GetCompany();
   (*pTbl)(3, 0) << _T("Engineer");
   (*pTbl)(3, 1) << pProps->GetEngineer();
   (*pTbl)(4, 0) << _T("Job Number");
   (*pTbl)(4, 1) << pProps->GetJobNumber();
   (*pTbl)(5, 0) << _T("Comments");
   (*pTbl)(5, 1) << pProps->GetComments();
   (*pTbl)(6, 0) << _T("File");
   (*pTbl)(6, 1) << pDocument->GetFilePath();

   // Notes table
   rptRcTable* pTable;
   int row = 0;

   pPara = new rptParagraph;
   pPara->SetStyleName(rptStyleManager::GetHeadingStyle());
   *pTitlePage << pPara;

   pTable = rptStyleManager::CreateDefaultTable(2, _T("Notes"));
   pTable->SetColumnStyle(0, rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   pTable->SetStripeRowColumnStyle(0, rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));
   pTable->SetColumnStyle(1, rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   pTable->SetStripeRowColumnStyle(1, rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));

   *pPara << pTable << rptNewLine;

   row = 0;
   (*pTable)(row, 0) << _T("Symbol");
   (*pTable)(row++, 1) << _T("Definition");

   (*pTable)(row, 0) << _T("Brg");
   (*pTable)(row++, 1) << _T("Bearing location");

   (*pTable)(row, 0) << Sub2(_T("Col"), _T("left")) << Sub2(_T(", Col"), _T("right"));
   (*pTable)(row++, 1) << _T("Just left or right of CL column");

   (*pTable)(row, 0) << _T("FOC");
   (*pTable)(row++, 1) << _T("Face of column");

   (*pTable)(row, 0) << Sub2(_T("FOC"), _T("dv")) << Sub2(_T(", FOC"), _T("dv2"));
   (*pTable)(row++, 1) << Sub2(_T("d"), _T("v")) << Sub2(_T(" or d"), _T("v")) << _T("/2 from face of column");

   (*pTable)(row, 0) << _T("MP");
   (*pTable)(row++, 1) << _T("Mid-point of cap between columns");

   (*pTable)(row, 0) << _T("ST");
   (*pTable)(row++, 1) << _T("Section change");

   ///////////////////////////////////
   // Status items
   GET_IFACE2(pBroker,IEAFStatusCenter, pStatusCenter);
   IndexType nItems = pStatusCenter->Count();

   if (0 < nItems)
   {
      pPara = new rptParagraph;
      pPara->SetStyleName(rptStyleManager::GetHeadingStyle());
      *pTitlePage << pPara;

      pTable = rptStyleManager::CreateDefaultTable(2, _T("Status Items"));
      pTable->SetColumnStyle(0, rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
      pTable->SetStripeRowColumnStyle(0, rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));
      pTable->SetColumnStyle(1, rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
      pTable->SetStripeRowColumnStyle(1, rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));

      *pPara << pTable << rptNewLine;

      (*pTable)(0, 0) << _T("Level");
      (*pTable)(0, 1) << _T("Description");

      row = 1;
      CString strSeverityType[] = { _T("Information"), _T("Warning"), _T("Error") };
      for (IndexType i = 0; i < nItems; i++)
      {
         auto pItem = pStatusCenter->GetByIndex(i);

         WBFL::EAF::StatusSeverityType severity = pStatusCenter->GetSeverity(pItem);

         // Set text and cell background
         rptRiStyle::FontColor colors[] = { rptRiStyle::LightGreen, rptRiStyle::Yellow, rptRiStyle::Red };
         rptRiStyle::FontColor color = colors[+severity];
         (*pTable)(row, 0) << new rptRcBgColor(color);
         (*pTable)(row, 0).SetFillBackGroundColor(color);

         (*pTable)(row, 0) << strSeverityType[+severity];
         (*pTable)(row++, 1) << pItem->GetDescription();
      }
   }


   // Throw in a page break
   pPara = new rptParagraph;
   *pTitlePage << pPara;
   *pPara << rptNewPage;

   return pTitlePage;
}
