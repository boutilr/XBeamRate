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
#include "MomentCapacityDetailsChapterBuilder.h"
#include <EAF\EAFDisplayUnits.h>

#include <IFace\LoadRating.h>
#include <IFace\PointOfInterest.h>

#include "XBeamRateReportSpecification.h"

CMomentCapacityDetailsChapterBuilder::CMomentCapacityDetailsChapterBuilder()
{
}

LPCTSTR CMomentCapacityDetailsChapterBuilder::GetName() const
{
   return TEXT("Moment Capacity Details");
}

rptChapter* CMomentCapacityDetailsChapterBuilder::Build(const std::shared_ptr<const WBFL::ReportMgr::ReportSpecification>& pRptSpec,Uint16 level) const
{
   auto pXBRRptSpec = std::dynamic_pointer_cast<const CXBeamRateReportSpecification>(pRptSpec);

   // This report does not use the passd span and girder parameters
   rptChapter* pChapter = CXBeamRateChapterBuilder::Build(pRptSpec,level);

   auto pBroker = pXBRRptSpec->GetBroker();


   PierIDType pierID = pXBRRptSpec->GetPierID();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);
   INIT_UV_PROTOTYPE( rptXBRPointOfInterest, location, pDisplayUnits->GetSpanLengthUnit(), false );
   INIT_UV_PROTOTYPE( rptLengthUnitValue, dim, pDisplayUnits->GetComponentDimUnit(), false );
   INIT_UV_PROTOTYPE( rptMomentUnitValue, moment, pDisplayUnits->GetMomentUnit(), false );
   INIT_UV_PROTOTYPE( rptAreaUnitValue, area, pDisplayUnits->GetAreaUnit(), false);
   INIT_UV_PROTOTYPE( rptStressUnitValue, stress, pDisplayUnits->GetStressUnit(), false);

   rptParagraph* pPara = new rptParagraph;
   *pChapter << pPara;

   ColumnIndexType nColumns = 10;
   if ( WBFL::LRFD::MBEManager::Edition::SecondEditionWith2015Interims <= WBFL::LRFD::MBEManager::GetEdition() )
   {
      *pPara << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("XBeamMomentCapacity2015.png")) << rptNewLine;
   }
   else
   {
      *pPara << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("XBeamMomentCapacity.png")) << rptNewLine;
      nColumns--; // no alpha column
   }

   for ( int i = 0; i < 2; i++ )
   {
      bool bPositiveMoment = (i == 0 ? true : false);

      CString strTitle;
      strTitle.Format(_T("%s Moment"),(bPositiveMoment ? _T("Positive") : _T("Negative")));

      rptRcTable* pTable = rptStyleManager::CreateDefaultTable(nColumns,strTitle);
      *pPara << pTable << rptNewLine;


      ColumnIndexType col = 0;
      (*pTable)(0,col++) << COLHDR(_T("Location"), rptLengthUnitTag, pDisplayUnits->GetSpanLengthUnit());
      if ( WBFL::LRFD::MBEManager::Edition::SecondEditionWith2015Interims <= WBFL::LRFD::MBEManager::GetEdition() )
      {
         (*pTable)(0,col++) << Sub2(symbol(alpha),_T("1"));
      }
      (*pTable)(0,col++) << Sub2(symbol(beta),_T("1"));
      (*pTable)(0,col++) << COLHDR(_T("c"), rptLengthUnitTag, pDisplayUnits->GetComponentDimUnit());
      (*pTable)(0,col++) << COLHDR(_T("a"), rptLengthUnitTag, pDisplayUnits->GetComponentDimUnit());
      (*pTable)(0,col++) << COLHDR(_T("b"), rptLengthUnitTag, pDisplayUnits->GetComponentDimUnit());
      (*pTable)(0,col++) << _T("Reinforcement");
      (*pTable)(0,col++) << symbol(phi);
      (*pTable)(0,col++) << COLHDR(Sub2(_T("M"),_T("n")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit());
      (*pTable)(0,col++) << COLHDR(Sub2(_T("M"),_T("r")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit());

      GET_IFACE2(pBroker,IXBRPointOfInterest,pPoi);
      std::vector<xbrPointOfInterest> vPoi = pPoi->GetMomentRatingPointsOfInterest(pierID, !bPositiveMoment);

      GET_IFACE2(pBroker,IXBRMomentCapacity,pMomentCapacity);
      RowIndexType row = pTable->GetNumberOfHeaderRows();
      for (const auto& poi : vPoi)
      {
         col = 0;

         const MomentCapacityDetails& mcd = pMomentCapacity->GetMomentCapacityDetails(pierID,xbrTypes::Stage2,poi,bPositiveMoment);

         Float64 alpha1,beta1,c;
         mcd.solution->get_Alpha1(&alpha1);
         mcd.solution->get_Beta1(&beta1);
         mcd.solution->get_NeutralAxisDepth(&c);

         Float64 bw;
         mcd.rcBeam->get_bw(&bw);

#if defined _DEBUG
         Float64 b;
         mcd.rcBeam->get_b(&b);
         ATLASSERT(IsEqual(b,bw));
         Float64 hf;
         mcd.rcBeam->get_hf(&hf);
         ATLASSERT(IsZero(hf));
#endif

         (*pTable)(row,col++) << location.SetValue(poi);
         if ( WBFL::LRFD::MBEManager::Edition::SecondEditionWith2015Interims <= WBFL::LRFD::MBEManager::GetEdition() )
         {
            (*pTable)(row,col++) << alpha1;
         }
         (*pTable)(row,col++) << beta1;
         (*pTable)(row,col++) << dim.SetValue(c);
         (*pTable)(row,col++) << dim.SetValue(beta1*c);
         (*pTable)(row,col++) << dim.SetValue(bw);

         rptRcTable* pReinfTable = rptStyleManager::CreateDefaultTable(4);
         (*pTable)(row,col++) << pReinfTable;

         (*pTable)(row,col++) << mcd.phi;
         (*pTable)(row,col++) << moment.SetValue(mcd.Mn);
         (*pTable)(row,col++) << moment.SetValue(mcd.Mr);


         (*pReinfTable)(0,0) << _T("Layer");
         (*pReinfTable)(0,1) << COLHDR(Sub2(_T("d"),_T("s")),rptLengthUnitTag,pDisplayUnits->GetComponentDimUnit());
         (*pReinfTable)(0,2) << COLHDR(Sub2(_T("A"),_T("s")),rptAreaUnitTag,pDisplayUnits->GetAreaUnit());
         (*pReinfTable)(0,3) << COLHDR(Sub2(_T("f"),_T("s")),rptStressUnitTag,pDisplayUnits->GetStressUnit());

         IndexType nRebarLayers;
         mcd.rcBeam->get_RebarLayerCount(&nRebarLayers);

         CComPtr<IDblArray> vfs;
         mcd.solution->get_fs(&vfs);
//#if defined _DEBUG
//         IndexType vfs_size;
//         vfs->get_Count(&vfs_size);
//         ATLASSERT(vfs_size == nRebarLayers);
//#endif
         RowIndexType reinfTableRow = pReinfTable->GetNumberOfHeaderRows();

         for ( IndexType rebarLayerIdx = 0; rebarLayerIdx < nRebarLayers; rebarLayerIdx++, reinfTableRow++ )
         {
            Float64 As, ds, devFactor;
            mcd.rcBeam->GetRebarLayer(rebarLayerIdx,&ds,&As,&devFactor);

            Float64 fs;
            vfs->get_Item(rebarLayerIdx,&fs);

            (*pReinfTable)(reinfTableRow,0) << LABEL_INDEX(rebarLayerIdx);
            (*pReinfTable)(reinfTableRow,1) << dim.SetValue(ds);
            (*pReinfTable)(reinfTableRow,2) << area.SetValue(devFactor*As);
            (*pReinfTable)(reinfTableRow,3) << stress.SetValue(fs);
         }


         row++;
      }
   }

   return pChapter;
}
