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
#include "LoadingDetailsChapterBuilder.h"
#include <EAF\EAFDisplayUnits.h>

#include <IFace\Project.h>
#include <IFace\AnalysisResults.h>

#include "XBeamRateReportSpecification.h"

#include <XBeamRateExt\LongitudinalRebarData.h>
#include <XBeamRateExt\StirrupData.h>

#include <PsgLib\GirderLabel.h>


void write_limit_state_details(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID);
void write_dead_load_details(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID);
void write_bearing_layout_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID);
void write_live_load_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID);

CLoadingDetailsChapterBuilder::CLoadingDetailsChapterBuilder()
{
}

LPCTSTR CLoadingDetailsChapterBuilder::GetName() const
{
   return TEXT("Loading Details");
}

rptChapter* CLoadingDetailsChapterBuilder::Build(const std::shared_ptr<const WBFL::ReportMgr::ReportSpecification>& pRptSpec,Uint16 level) const
{
   USES_CONVERSION;

   auto pXBRRptSpec = std::dynamic_pointer_cast<const CXBeamRateReportSpecification>(pRptSpec);

   // This report does not use the passed span and girder parameters
   rptChapter* pChapter = CXBeamRateChapterBuilder::Build(pRptSpec,level);

   auto pBroker = pXBRRptSpec->GetBroker();

   PierIDType pierID = pXBRRptSpec->GetPierID();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   write_limit_state_details(pBroker,pDisplayUnits,pChapter,pierID);
   write_dead_load_details(pBroker,pDisplayUnits,pChapter,pierID);
   write_bearing_layout_data(pBroker,pDisplayUnits,pChapter,pierID);
   write_live_load_data(pBroker,pDisplayUnits,pChapter,pierID);

   return pChapter;
}

void write_limit_state_details(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID)
{
   GET_IFACE2(pBroker,IXBRProject,pProject);
   rptParagraph* pPara = new rptParagraph(rptStyleManager::GetHeadingStyle());
   *pChapter << pPara;
   *pPara << _T("Limit State Load Factors") << rptNewLine;

   pPara = new rptParagraph;
   *pChapter << pPara;

   rptRcTable* pTable = rptStyleManager::CreateDefaultTable(8);
   pTable->SetColumnStyle(0,rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   pTable->SetStripeRowColumnStyle(0,rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));
   *pPara << pTable << rptNewLine;

   ColumnIndexType col = 0;
   (*pTable)(0,col++) << _T("Limit State");
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("DC"));
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("DW"));
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("CR"));
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("SH"));
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("RE"));
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("PS"));
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("LL"));

   // NOTE: On multiple occasions I've wanted to add Service III limit states to this list.
   // This is a cross beam, reinforced concrete load rating. Service III isn't applicable.
   pgsTypes::LimitState limitStates[] = { pgsTypes::StrengthI_Inventory,
                                          pgsTypes::StrengthI_Operating,
                                          pgsTypes::StrengthI_LegalRoutine,
                                          pgsTypes::StrengthI_LegalSpecial,
                                          pgsTypes::StrengthI_LegalEmergency,
                                          pgsTypes::StrengthII_PermitRoutine,
                                          pgsTypes::ServiceI_PermitRoutine,
                                          pgsTypes::StrengthII_PermitSpecial,
                                          pgsTypes::ServiceI_PermitSpecial };
   
   RowIndexType row = pTable->GetNumberOfHeaderRows();
   IndexType nLimitStates = sizeof(limitStates)/sizeof(pgsTypes::LimitState);
   for ( IndexType limitStateIdx = 0; limitStateIdx < nLimitStates; limitStateIdx++, row++ )
   {
      col = 0;

      pgsTypes::LimitState limitState = limitStates[limitStateIdx];
      Float64 DC = pProject->GetDCLoadFactor(limitState);
      Float64 DW = pProject->GetDWLoadFactor(limitState);
      Float64 CR = pProject->GetCRLoadFactor(limitState);
      Float64 SH = pProject->GetSHLoadFactor(limitState);
      Float64 RE = pProject->GetRELoadFactor(limitState);
      Float64 PS = pProject->GetPSLoadFactor(limitState);

      pgsTypes::LoadRatingType ratingType = ::RatingTypeFromLimitState(limitState);

      CString strLimitState = ::GetLimitStateString(limitState);

      Float64 LL = pProject->GetLiveLoadFactor(pierID,limitState,INVALID_INDEX);

      (*pTable)(row,col++) << strLimitState;
      (*pTable)(row,col++) << DC;
      (*pTable)(row,col++) << DW;
      (*pTable)(row,col++) << CR;
      (*pTable)(row,col++) << SH;
      (*pTable)(row,col++) << RE;
      (*pTable)(row,col++) << PS;
      (*pTable)(row,col++) << LL;
   }
}

void write_dead_load_details(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID)
{
   rptParagraph* pPara = new rptParagraph(rptStyleManager::GetHeadingStyle());
   *pChapter << pPara;
   *pPara << _T("Cross Beam Dead Load") << rptNewLine;

   pPara = new rptParagraph;
   *pChapter << pPara;

   INIT_UV_PROTOTYPE( rptLengthUnitValue, length, pDisplayUnits->GetSpanLengthUnit(), false );
   INIT_UV_PROTOTYPE( rptForcePerLengthUnitValue, fpl, pDisplayUnits->GetForcePerLengthUnit(), false );

   rptRcTable* pTable = rptStyleManager::CreateDefaultTable(4,_T("Lower Cross Beam"));
   *pPara << pTable << rptNewLine;

   ColumnIndexType col = 0;
   (*pTable)(0,col++) << COLHDR(Sub2(_T("X"),_T("start")), rptLengthUnitTag, pDisplayUnits->GetSpanLengthUnit());
   (*pTable)(0,col++) << COLHDR(Sub2(_T("X"),_T("end")), rptLengthUnitTag, pDisplayUnits->GetSpanLengthUnit());
   (*pTable)(0,col++) << COLHDR(Sub2(_T("w"),_T("start")), rptForcePerLengthUnitTag, pDisplayUnits->GetForcePerLengthUnit());
   (*pTable)(0,col++) << COLHDR(Sub2(_T("w"),_T("end")), rptForcePerLengthUnitTag, pDisplayUnits->GetForcePerLengthUnit());

   GET_IFACE2(pBroker,IXBRProductForces,pProductForces);
   const std::vector<LowerXBeamLoad>& vLoads = pProductForces->GetLowerCrossBeamLoading(pierID);
   RowIndexType row = pTable->GetNumberOfHeaderRows();
   for (const auto& load : vLoads)
   {
      col = 0;

      (*pTable)(row,col++) << length.SetValue(load.Xs);
      (*pTable)(row,col++) << length.SetValue(load.Xe);
      (*pTable)(row,col++) << fpl.SetValue(load.Ws);
      (*pTable)(row,col++) << fpl.SetValue(load.We);

      row++;
   }

   fpl.ShowUnitTag(true);
   pPara = new rptParagraph;
   *pChapter << pPara;
   *pPara << _T("Upper Cross Beam Diaphragm, w = ") << fpl.SetValue(pProductForces->GetUpperCrossBeamLoading(pierID));
}

void write_bearing_layout_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID)
{
   rptParagraph* pPara = new rptParagraph(rptStyleManager::GetHeadingStyle());
   *pChapter << pPara;
   *pPara << _T("Bearing Layout and Reactions") << rptNewLine;

   INIT_UV_PROTOTYPE( rptLengthUnitValue, length, pDisplayUnits->GetSpanLengthUnit(), false );
   INIT_UV_PROTOTYPE( rptForceUnitValue, force, pDisplayUnits->GetGeneralForceUnit(), false );
   INIT_UV_PROTOTYPE( rptForcePerLengthUnitValue, fpl, pDisplayUnits->GetForcePerLengthUnit(), false );

   GET_IFACE2(pBroker,IXBRProject,pProject);
   IndexType nBearingLines = pProject->GetBearingLineCount(pierID);
   for ( IndexType brgLineIdx = 0; brgLineIdx < nBearingLines; brgLineIdx++ )
   {
      pPara = new rptParagraph;
      *pChapter << pPara;

      ColumnIndexType nColumns = 8;
      xbrTypes::ReactionLoadType bearingReactionType = pProject->GetBearingReactionType(pierID,brgLineIdx);
      if ( bearingReactionType == xbrTypes::rltUniform )
      {
         nColumns++;
      }

      rptRcTable* pTable;
      if ( nBearingLines == 1 )
      {
         pTable = rptStyleManager::CreateDefaultTable(nColumns);
      }
      else
      {
         CString strTitle;
         strTitle.Format(_T("%s Bearing Line"),(brgLineIdx == 0 ? _T("Back") : _T("Ahead")));
         pTable = rptStyleManager::CreateDefaultTable(nColumns,strTitle);
      }
      *pPara << pTable << rptNewLine;


      ColumnIndexType col = 0;
      (*pTable)(0,col++) << _T("Bearing");
      if ( bearingReactionType == xbrTypes::rltUniform )
      {
         (*pTable)(0,col++) << COLHDR(_T("DC"), rptForcePerLengthUnitTag, pDisplayUnits->GetForcePerLengthUnit());
         (*pTable)(0,col++) << COLHDR(_T("DW"), rptForcePerLengthUnitTag, pDisplayUnits->GetForcePerLengthUnit());
         (*pTable)(0,col++) << COLHDR(_T("CR"), rptForcePerLengthUnitTag, pDisplayUnits->GetForcePerLengthUnit());
         (*pTable)(0,col++) << COLHDR(_T("SH"), rptForcePerLengthUnitTag, pDisplayUnits->GetForcePerLengthUnit());
         (*pTable)(0,col++) << COLHDR(_T("PS"), rptForcePerLengthUnitTag, pDisplayUnits->GetForcePerLengthUnit());
         (*pTable)(0,col++) << COLHDR(_T("RE"), rptForcePerLengthUnitTag, pDisplayUnits->GetForcePerLengthUnit());
         (*pTable)(0,col++) << COLHDR(_T("Width"), rptLengthUnitTag, pDisplayUnits->GetSpanLengthUnit() );
      }
      else
      {
         (*pTable)(0,col++) << COLHDR(_T("DC"), rptForceUnitTag, pDisplayUnits->GetGeneralForceUnit());
         (*pTable)(0,col++) << COLHDR(_T("DW"), rptForceUnitTag, pDisplayUnits->GetGeneralForceUnit());
         (*pTable)(0,col++) << COLHDR(_T("CR"), rptForceUnitTag, pDisplayUnits->GetGeneralForceUnit());
         (*pTable)(0,col++) << COLHDR(_T("SH"), rptForceUnitTag, pDisplayUnits->GetGeneralForceUnit());
         (*pTable)(0,col++) << COLHDR(_T("PS"), rptForceUnitTag, pDisplayUnits->GetGeneralForceUnit());
         (*pTable)(0,col++) << COLHDR(_T("RE"), rptForceUnitTag, pDisplayUnits->GetGeneralForceUnit());
      }

      (*pTable)(0,col++) << COLHDR(_T("Spacing"), rptLengthUnitTag, pDisplayUnits->GetSpanLengthUnit() );

      RowIndexType row = pTable->GetNumberOfHeaderRows();

      length.ShowUnitTag(false);

      IndexType nBearings = pProject->GetBearingCount(pierID,brgLineIdx);
      for ( IndexType brgIdx = 0; brgIdx < nBearings; brgIdx++, row++ )
      {
         col = 0;
         (*pTable)(row,col++) << LABEL_INDEX(brgIdx);

   
         Float64 DC, DW, CR, SH, PS, RE, W;
         pProject->GetBearingReactions(pierID,brgLineIdx,brgIdx,&DC,&DW,&CR,&SH,&PS,&RE,&W);

         if ( bearingReactionType == xbrTypes::rltUniform )
         {
            (*pTable)(row,col++) << fpl.SetValue(DC);
            (*pTable)(row,col++) << fpl.SetValue(DW);
            (*pTable)(row,col++) << fpl.SetValue(CR);
            (*pTable)(row,col++) << fpl.SetValue(SH);
            (*pTable)(row,col++) << fpl.SetValue(PS);
            (*pTable)(row,col++) << fpl.SetValue(RE);
            (*pTable)(row,col++) << length.SetValue(W);
         }
         else
         {
            (*pTable)(row,col++) << force.SetValue(DC);
            (*pTable)(row,col++) << force.SetValue(DW);
            (*pTable)(row,col++) << force.SetValue(CR);
            (*pTable)(row,col++) << force.SetValue(SH);
            (*pTable)(row,col++) << force.SetValue(PS);
            (*pTable)(row,col++) << force.SetValue(RE);
         }

         if ( brgIdx < nBearings-1 )
         {
            Float64 S = pProject->GetBearingSpacing(pierID,brgLineIdx,brgIdx);
            (*pTable)(row,col++) << length.SetValue(S);
         }
         else
         {
            (*pTable)(row,col++) << _T("");
         }
      }

      length.ShowUnitTag(true);
      IndexType refBrgIdx;
      Float64 refBrgOffset;
      pgsTypes::OffsetMeasurementType refBearingDatum;
      pProject->GetReferenceBearing(pierID,brgLineIdx,&refBrgIdx,&refBrgOffset,&refBearingDatum);
      *pPara << _T("Bearing ") << (refBrgIdx+1) << _T(" is located ") << RPT_OFFSET(refBrgOffset,length) << _T(" of the ");
      if ( refBearingDatum == pgsTypes::omtAlignment )
      {
         *pPara << _T("Alignment") << rptNewLine;
      }
      else
      {
         *pPara << _T("Bridge Line") << rptNewLine;
      }
   }
}

void write_live_load_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID)
{
   rptParagraph* pPara = new rptParagraph(rptStyleManager::GetHeadingStyle());
   *pChapter << pPara;
   *pPara << _T("Live Load Reactions") << rptNewLine;

   pPara = new rptParagraph;
   *pChapter << pPara;
   *pPara << _T("Live load reactions are per lane.") << rptNewLine;

   GET_IFACE2(pBroker,IXBRProject,pProject);
   if ( pProject->GetReactionLoadApplicationType(pierID) == xbrTypes::rlaCrossBeam )
   {
      *pPara << _T("Reactions are applied directly to the cross beam.") << rptNewLine;
   }
   else
   {
      *pPara << _T("Reactions are applied through the bearings.") << rptNewLine;
   }

   pPara = new rptParagraph;
   *pChapter << pPara;

   INIT_UV_PROTOTYPE( rptLengthUnitValue, length, pDisplayUnits->GetSpanLengthUnit(), false );
   INIT_UV_PROTOTYPE( rptForceUnitValue, force, pDisplayUnits->GetGeneralForceUnit(), false );

   int n = (int)pgsTypes::lrLoadRatingTypeCount;
   for ( int i = 0; i < n; i++ )
   {
      pgsTypes::LoadRatingType ratingType = (pgsTypes::LoadRatingType)i;
      IndexType nVehicles = pProject->GetLiveLoadReactionCount(pierID,ratingType);
      if ( nVehicles == 0 || pProject->GetLiveLoadName(pierID,ratingType,0) == NO_LIVE_LOAD_DEFINED)
      {
         continue;
      }

      ColumnIndexType nColumns = 3;
      if ( ::IsLegalRatingType(ratingType) )
      {
         nColumns++;
      }

      rptParagraph* pPara = new rptParagraph;
      *pChapter << pPara;

      rptRcTable* pTable = rptStyleManager::CreateDefaultTable(nColumns,pProject->GetLiveLoadName(pierID,ratingType,INVALID_INDEX).c_str());
      pTable->SetColumnStyle(1, rptStyleManager::GetTableCellStyle( CB_NONE | CJ_LEFT) );
      pTable->SetStripeRowColumnStyle(1, rptStyleManager::GetTableStripeRowCellStyle( CB_NONE | CJ_LEFT) );
      *pPara << pTable << rptNewLine;

      ColumnIndexType col = 0;
      (*pTable)(0,col++) << _T("");
      (*pTable)(0,col++) << _T("Name");
      (*pTable)(0,col++) << COLHDR(_T("LL+IM"), rptForceUnitTag, pDisplayUnits->GetGeneralForceUnit() );
      if ( ::IsLegalRatingType(ratingType) )
      {
         (*pTable)(0,col++) << COLHDR(_T("Vehicle") << rptNewLine << _T("Weight"), rptForceUnitTag, pDisplayUnits->GetGeneralForceUnit() );
      }

      RowIndexType row = pTable->GetNumberOfHeaderRows();
      for (IndexType vehicleIdx = 0; vehicleIdx < nVehicles; vehicleIdx++, row++ )
      {
         col = 0;
         (*pTable)(row,col++) << LABEL_INDEX(vehicleIdx);
         (*pTable)(row,col++) << pProject->GetLiveLoadName(pierID,ratingType,vehicleIdx);
         (*pTable)(row,col++) << force.SetValue(pProject->GetLiveLoadReaction(pierID,ratingType,vehicleIdx));
         if ( ::IsLegalRatingType(ratingType) )
         {
            (*pTable)(row,col++) << force.SetValue(pProject->GetVehicleWeight(pierID,ratingType,vehicleIdx));
         }
      }
   }
}
