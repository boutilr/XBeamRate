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
#include "LoadRatingDetailsChapterBuilder.h"
#include <EAF\EAFDisplayUnits.h>

#include <IFace\LoadRating.h>
#include <IFace\Project.h>
#include <IFace\RatingSpecification.h>
#include <IFace\Pier.h>

#include "XBeamRateReportSpecification.h"

#include <PgsExt\CapacityToDemand.h>
#include <PsgLib\GirderLabel.h>
#include <PsgLib\Helpers.h>


CLoadRatingDetailsChapterBuilder::CLoadRatingDetailsChapterBuilder():
m_bReportEvenIncrements(true)
{
}

LPCTSTR CLoadRatingDetailsChapterBuilder::GetName() const
{
   return TEXT("Load Rating Details");
}

rptChapter* CLoadRatingDetailsChapterBuilder::Build(const std::shared_ptr<const WBFL::ReportMgr::ReportSpecification>& pRptSpec,Uint16 level) const
{
   auto pXBRRptSpec = std::dynamic_pointer_cast<const CXBeamRateReportSpecification>(pRptSpec);

   m_bReportEvenIncrements = pXBRRptSpec->GetDoReportEvenIncrements();

   // This report does not use the passed span and girder parameters
   rptChapter* pChapter = CXBeamRateChapterBuilder::Build(pRptSpec,level);

   auto pBroker = pXBRRptSpec->GetBroker();

   PierIDType pierID = pXBRRptSpec->GetPierID();

   GET_IFACE2(pBroker,IXBRArtifact,pArtifact);
   GET_IFACE2(pBroker,IXBRRatingSpecification,pRatingSpec);
   GET_IFACE2(pBroker,IXBRProject,pProject);

   xbrTypes::PermitRatingMethod permitRatingMethod = pRatingSpec->GetPermitRatingMethod();
   m_bReportNegativeMomentBetweenFOCs = pRatingSpec->DoCheckNegativeMomentBetweenFOCs(pierID);

   int n = (int)pgsTypes::lrLoadRatingTypeCount;
   for ( int i = 0; i < n; i++ )
   {
      pgsTypes::LoadRatingType ratingType = (pgsTypes::LoadRatingType)i;

      if ( !pRatingSpec->IsRatingEnabled(ratingType) )
      {
         continue;
      }

      CString strLiveLoadType = ::GetLiveLoadTypeName(ratingType);
      rptParagraph* pPara = new rptParagraph(rptStyleManager::GetHeadingStyle());
      *pChapter << pPara;
      pPara->SetName(strLiveLoadType);
      *pPara << pPara->GetName() << rptNewLine;

      std::_tstring strName = pProject->GetLiveLoadName(pierID,ratingType,0);
      if ( strName == NO_LIVE_LOAD_DEFINED )
      {
         pPara = new rptParagraph;
         *pChapter << pPara;
         *pPara << strName << rptNewLine;
         continue;
      }

      IndexType nVehicles = pProject->GetLiveLoadReactionCount(pierID,ratingType);
      VehicleIndexType firstVehicleIdx = 0;
      VehicleIndexType lastVehicleIdx  = (::IsDesignRatingType(ratingType) ? 0 : nVehicles-1);
      for ( VehicleIndexType vehicleIdx = firstVehicleIdx; vehicleIdx <= lastVehicleIdx; vehicleIdx++ )
      {
         if ( !::IsDesignRatingType(ratingType) )
         {
            std::_tstring strLiveLoadName = pProject->GetLiveLoadName(pierID,ratingType,vehicleIdx);
            pPara = new rptParagraph(rptStyleManager::GetSubheadingStyle());
            *pChapter << pPara;
            pPara->SetName(strLiveLoadName.c_str());
            *pPara << strLiveLoadName << rptNewLine;
         }

         const xbrRatingArtifact* pRatingArtifact = pArtifact->GetXBeamRatingArtifact(pierID,ratingType,(::IsDesignRatingType(ratingType) ? INVALID_INDEX : vehicleIdx));

         MomentRatingDetails(pChapter,pBroker,pierID,ratingType,true, pRatingArtifact);
         MomentRatingDetails(pChapter,pBroker,pierID,ratingType,false,pRatingArtifact);

         if ( pRatingSpec->RateForShear(ratingType) )
         {
            ShearRatingDetails(pChapter,pBroker,pierID,ratingType,pRatingArtifact);
         }

         if ( ::IsPermitRatingType(ratingType) && pRatingSpec->CheckYieldStressLimit() )
         {
            ReinforcementYieldingDetails(pChapter,pBroker,pierID,ratingType,true, pRatingArtifact);
            ReinforcementYieldingDetails(pChapter,pBroker,pierID,ratingType,false,pRatingArtifact);
         }

         if ( pRatingArtifact->IsLoadPostingRequired() )
         {
            LoadPostingDetails(pChapter,pBroker,pRatingArtifact);
         }
      } // next vehicle
   } // next rating type

   return pChapter;
}


void CLoadRatingDetailsChapterBuilder::MomentRatingDetails(rptChapter* pChapter,std::shared_ptr<WBFL::EAF::Broker> pBroker,PierIDType pierID,pgsTypes::LoadRatingType ratingType,bool bPositiveMoment,const xbrRatingArtifact* pRatingArtifact) const
{
   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);
   INIT_UV_PROTOTYPE( rptXBRPointOfInterest, location, pDisplayUnits->GetSpanLengthUnit(), false );
   INIT_UV_PROTOTYPE( rptMomentUnitValue, moment, pDisplayUnits->GetMomentUnit(), false );
   rptCapacityToDemand rating_factor;

   GET_IFACE2_NOCHECK(pBroker,IXBRProject,pProject);


   rptParagraph* pPara = new rptParagraph(rptStyleManager::GetSubheadingStyle());
   *pChapter << pPara;

   if ( bPositiveMoment )
   {
      *pPara << _T("Rating for Positive Moment") << rptNewLine;
   }
   else
   {
      *pPara << _T("Rating for Negative Moment") << rptNewLine;
   }

   GET_IFACE2(pBroker, IXBRRatingSpecification, pSpec);
   bool bIsWSDOTRating = (pSpec->IsWSDOTEmergencyRating(ratingType) || pSpec->IsWSDOTPermitRating(ratingType) ? true : false);

   pPara = new rptParagraph;
   *pChapter << pPara;

   if ( bIsWSDOTRating )
   {
      *pPara << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("XBeamMomentRatingEquation_WSDOT.png") ) << rptNewLine;
   }
   else
   {
      *pPara << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("XBeamMomentRatingEquation_LRFD.png") ) << rptNewLine;
   }

   ColumnIndexType nColumns = (bIsWSDOTRating ? 23 : 22);

   rptRcTable* pTable = rptStyleManager::CreateDefaultTable(nColumns);
   pTable->SetColumnStyle(0,rptStyleManager::GetTableCellStyle(CB_NONE | CJ_RIGHT));
   pTable->SetStripeRowColumnStyle(0,rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_RIGHT));

   *pPara << pTable << rptNewLine;

   pPara = new rptParagraph(rptStyleManager::GetFootnoteStyle());
   *pChapter << pPara;
   std::_tstring strVehicle;
   if (bIsWSDOTRating)
   {
      if (::IsEmergencyRatingType(ratingType))
      {
         strVehicle = _T("E");
         *pPara << _T("LCn, Em = Live Load Configuration n, with Emergency Vehicle in Lane m") << rptNewLine;
      }
      else
      {
         strVehicle = _T("P");
         *pPara << _T("LCn, Pm = Live Load Configuration n, with Permit Vehicle in Lane m") << rptNewLine;
      }
   }
   else
   {
      *pPara << _T("LCn = Live Load Configuration n") << rptNewLine;
   }

   if ( ::IsDesignRatingType(ratingType) )
   {
      GET_IFACE2(pBroker,IXBRProject,pProject);
      IndexType nVehicles = pProject->GetLiveLoadReactionCount(pierID,ratingType);
      for ( VehicleIndexType vehIdx = 0; vehIdx < nVehicles; vehIdx++ )
      {
         std::_tstring strName = pProject->GetLiveLoadName(pierID,ratingType,vehIdx);
         *pPara << _T("D") << LABEL_INDEX(vehIdx) << _T(" = ") << strName.c_str() << rptNewLine;
      }
   }


   ColumnIndexType col = 0;
   (*pTable)(0,col++) << COLHDR(_T("Location"), rptLengthUnitTag, pDisplayUnits->GetSpanLengthUnit());
   (*pTable)(0,col++) << Sub2(symbol(phi),_T("c"));
   (*pTable)(0,col++) << Sub2(symbol(phi),_T("s"));
   (*pTable)(0,col++) << Sub2(symbol(phi),_T("n"));
   (*pTable)(0,col++) << _T("K");
   (*pTable)(0,col++) << COLHDR(Sub2(_T("M"),_T("n")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("DC"));
   (*pTable)(0,col++) << COLHDR(Sub2(_T("M"),_T("DC")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("DW"));
   (*pTable)(0,col++) << COLHDR(Sub2(_T("M"),_T("DW")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("CR"));
   (*pTable)(0,col++) << COLHDR(Sub2(_T("M"),_T("CR")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("SR"));
   (*pTable)(0,col++) << COLHDR(Sub2(_T("M"),_T("SR")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("RE"));
   (*pTable)(0,col++) << COLHDR(Sub2(_T("M"),_T("RE")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("PS"));
   (*pTable)(0,col++) << COLHDR(Sub2(_T("M"),_T("PS")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("LL"));
   if (bIsWSDOTRating)
   {
      if (::IsEmergencyRatingType(ratingType))
      {
         (*pTable)(0, col++) << COLHDR(Sub2(_T("M"), _T("LL+IM")) << rptNewLine << _T("Emergency"), rptMomentUnitTag, pDisplayUnits->GetMomentUnit());
      }
      else
      {
         (*pTable)(0, col++) << COLHDR(Sub2(_T("M"), _T("LL+IM")) << rptNewLine << _T("Permit"), rptMomentUnitTag, pDisplayUnits->GetMomentUnit());
      }
      (*pTable)(0,col++) << COLHDR(Sub2(_T("M"),_T("LL+IM")) << rptNewLine << _T("Legal"),  rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   }
   else
   {
      (*pTable)(0,col++) << COLHDR(Sub2(_T("M"),_T("LL+IM")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   }
   (*pTable)(0,col++) << _T("Live") << rptNewLine << _T("Load") << rptNewLine << _T("Case");
   (*pTable)(0,col++) << _T("RF");

   const xbrMomentRatingArtifact* pControllingRating;
   pRatingArtifact->GetMomentRatingFactorEx(bPositiveMoment,&pControllingRating);
   xbrPointOfInterest controllingPoi = pControllingRating->GetPointOfInterest();

   const xbrRatingArtifact::MomentRatings& momentRatings = pRatingArtifact->GetMomentRatings(bPositiveMoment);

   RowIndexType row = pTable->GetNumberOfHeaderRows();
   xbrRatingArtifact::MomentRatings::const_iterator iter(momentRatings.begin());
   xbrRatingArtifact::MomentRatings::const_iterator end(momentRatings.end());
   for ( ; iter != end; iter++ )
   {
      col = 0;
      const xbrPointOfInterest& poi = iter->first;
      const xbrMomentRatingArtifact& artifact = iter->second;
      Float64 RF = artifact.GetRatingFactor();

      if ( 1.0 <= RF && !ReportAtThisPoi(poi,controllingPoi, !bPositiveMoment) )
      {
         continue;
      }

      ATLASSERT(poi == artifact.GetPointOfInterest());

      IndexType llConfigIdx;
      IndexType permitLaneIdx;
      VehicleIndexType permitVehicleIdx;
      Float64 Mpermit;
      Float64 Mlegal;
      Float64 K;
      if ( bIsWSDOTRating )
      {
         artifact.GetWSDOTPermitConfiguration(&llConfigIdx,&permitLaneIdx,&permitVehicleIdx,&Mpermit,&Mlegal,&K);
      }

      (*pTable)(row,col++) << location.SetValue(poi);
      (*pTable)(row,col++) << artifact.GetConditionFactor();
      (*pTable)(row,col++) << artifact.GetSystemFactor();
      (*pTable)(row,col++) << artifact.GetCapacityReductionFactor();
       if ( bIsWSDOTRating )
       {
         (*pTable)(row,col++) << K;
       }
       else
       {
         (*pTable)(row,col++) << artifact.GetMinimumReinforcementFactor();
       }
      (*pTable)(row,col++) << moment.SetValue(artifact.GetNominalMomentCapacity());
      (*pTable)(row,col++) << artifact.GetDeadLoadFactor();
      (*pTable)(row,col++) << moment.SetValue(artifact.GetDeadLoadMoment());
      (*pTable)(row,col++) << artifact.GetWearingSurfaceFactor();
      (*pTable)(row,col++) << moment.SetValue(artifact.GetWearingSurfaceMoment());
      (*pTable)(row,col++) << artifact.GetCreepFactor();
      (*pTable)(row,col++) << moment.SetValue(artifact.GetCreepMoment());
      (*pTable)(row,col++) << artifact.GetShrinkageFactor();
      (*pTable)(row,col++) << moment.SetValue(artifact.GetShrinkageMoment());
      (*pTable)(row,col++) << artifact.GetRelaxationFactor();
      (*pTable)(row,col++) << moment.SetValue(artifact.GetRelaxationMoment());
      (*pTable)(row,col++) << artifact.GetSecondaryEffectsFactor();
      (*pTable)(row,col++) << moment.SetValue(artifact.GetSecondaryEffectsMoment());
      (*pTable)(row,col++) << artifact.GetLiveLoadFactor();

      if ( bIsWSDOTRating )
      {
         (*pTable)(row,col++) << moment.SetValue(Mpermit);
         (*pTable)(row,col++) << moment.SetValue(Mlegal);
         (*pTable)(row,col++) << _T("LC") << LABEL_INDEX(llConfigIdx) << _T(", ") << strVehicle  << LABEL_INDEX(permitLaneIdx);
      }
      else
      {
         (*pTable)(row,col++) << moment.SetValue(artifact.GetLiveLoadMoment());
         (*pTable)(row,col++) << _T("LC") << LABEL_INDEX(artifact.GetLiveLoadConfigurationIndex());
         if ( ::IsDesignRatingType(ratingType) )
         {
            (*pTable)(row,col-1) << _T(", D") << LABEL_INDEX(artifact.GetVehicleIndex());
         }
      }

      if ( RF < 1 )
      {
         (*pTable)(row,col++) << RF_FAIL(rating_factor,RF);
      }
      else
      {
         (*pTable)(row,col++) << RF_PASS(rating_factor,RF);
      }

      row++;
   } // next poi
}

void CLoadRatingDetailsChapterBuilder::ShearRatingDetails(rptChapter* pChapter,std::shared_ptr<WBFL::EAF::Broker> pBroker,PierIDType pierID,pgsTypes::LoadRatingType ratingType,const xbrRatingArtifact* pRatingArtifact) const
{
   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);
   INIT_UV_PROTOTYPE( rptXBRPointOfInterest, location, pDisplayUnits->GetSpanLengthUnit(), false );
   INIT_UV_PROTOTYPE( rptForceUnitValue, shear, pDisplayUnits->GetShearUnit(), false );
   rptCapacityToDemand rating_factor;

   GET_IFACE2_NOCHECK(pBroker,IXBRProject,pProject);

   rptParagraph* pPara = new rptParagraph(rptStyleManager::GetSubheadingStyle());
   *pChapter << pPara;

   *pPara << _T("Rating for Shear") << rptNewLine;

   GET_IFACE2(pBroker, IXBRRatingSpecification, pSpec);
   bool bIsWSDOTRating = (pSpec->IsWSDOTEmergencyRating(ratingType) || pSpec->IsWSDOTPermitRating(ratingType) ? true : false);

   pPara = new rptParagraph;
   *pChapter << pPara;

   if ( bIsWSDOTRating )
   {
      *pPara << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("XBeamShearRatingEquation_WSDOT.png") ) << rptNewLine;
   }
   else
   {
      *pPara << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("XBeamShearRatingEquation_LRFD.png") ) << rptNewLine;
   }

   ColumnIndexType nColumns = (bIsWSDOTRating ? 22 : 21);

   rptRcTable* pTable = rptStyleManager::CreateDefaultTable(nColumns);
   pTable->SetColumnStyle(0,rptStyleManager::GetTableCellStyle(CB_NONE | CJ_RIGHT));
   pTable->SetStripeRowColumnStyle(0,rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_RIGHT));

   *pPara << pTable << rptNewLine;

   pPara = new rptParagraph(rptStyleManager::GetFootnoteStyle());
   *pChapter << pPara;
   std::_tstring strVehicle;
   if ( bIsWSDOTRating )
   {
      if (::IsEmergencyRatingType(ratingType))
      {
         strVehicle = _T("E");
         *pPara << _T("LCn, Em = Live Load Configuration n, with Emergency Vehicle in Lane m") << rptNewLine;
      }
      else
      {
         strVehicle = _T("P");
         *pPara << _T("LCn, Pm = Live Load Configuration n, with Permit Vehicle in Lane m") << rptNewLine;
      }
   }
   else
   {
      *pPara << _T("LCn = Live Load Configuration n") << rptNewLine;
   }
 
   if ( ::IsDesignRatingType(ratingType) )
   {
      GET_IFACE2(pBroker,IXBRProject,pProject);
      IndexType nVehicles = pProject->GetLiveLoadReactionCount(pierID,ratingType);
      for ( VehicleIndexType vehIdx = 0; vehIdx < nVehicles; vehIdx++ )
      {
         std::_tstring strName = pProject->GetLiveLoadName(pierID,ratingType,vehIdx);
         *pPara << _T("D") << LABEL_INDEX(vehIdx) << _T(" = ") << strName.c_str() << rptNewLine;
      }
   }


   ColumnIndexType col = 0;
   (*pTable)(0,col++) << COLHDR(_T("Location"), rptLengthUnitTag, pDisplayUnits->GetSpanLengthUnit());
   (*pTable)(0,col++) << Sub2(symbol(phi),_T("c"));
   (*pTable)(0,col++) << Sub2(symbol(phi),_T("s"));
   (*pTable)(0,col++) << Sub2(symbol(phi),_T("n"));
   (*pTable)(0,col++) << COLHDR(Sub2(_T("V"),_T("n")), rptForceUnitTag, pDisplayUnits->GetShearUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("DC"));
   (*pTable)(0,col++) << COLHDR(Sub2(_T("V"),_T("DC")), rptForceUnitTag, pDisplayUnits->GetShearUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("DW"));
   (*pTable)(0,col++) << COLHDR(Sub2(_T("V"),_T("DW")), rptForceUnitTag, pDisplayUnits->GetShearUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("CR"));
   (*pTable)(0,col++) << COLHDR(Sub2(_T("V"),_T("CR")), rptForceUnitTag, pDisplayUnits->GetShearUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("SR"));
   (*pTable)(0,col++) << COLHDR(Sub2(_T("V"),_T("SR")), rptForceUnitTag, pDisplayUnits->GetShearUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("RE"));
   (*pTable)(0,col++) << COLHDR(Sub2(_T("V"),_T("RE")), rptForceUnitTag, pDisplayUnits->GetShearUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("PS"));
   (*pTable)(0,col++) << COLHDR(Sub2(_T("V"),_T("PS")), rptForceUnitTag, pDisplayUnits->GetShearUnit() );
   (*pTable)(0,col++) << Sub2(symbol(gamma),_T("LL"));
   if ( bIsWSDOTRating )
   {
      if (::IsEmergencyRatingType(ratingType))
      {
         (*pTable)(0, col++) << COLHDR(Sub2(_T("V"), _T("LL+IM")) << rptNewLine << _T("Emergency"), rptForceUnitTag, pDisplayUnits->GetShearUnit());
      }
      else
      {
         (*pTable)(0, col++) << COLHDR(Sub2(_T("V"), _T("LL+IM")) << rptNewLine << _T("Permit"), rptForceUnitTag, pDisplayUnits->GetShearUnit());
      }
      (*pTable)(0,col++) << COLHDR(Sub2(_T("V"),_T("LL+IM")) << rptNewLine << _T("Legal"),  rptForceUnitTag, pDisplayUnits->GetShearUnit() );
   }
   else
   {
      (*pTable)(0,col++) << COLHDR(Sub2(_T("V"),_T("LL+IM")), rptForceUnitTag, pDisplayUnits->GetShearUnit() );
   }
   (*pTable)(0,col++) << _T("Live") << rptNewLine << _T("Load") << rptNewLine << _T("Case");
   (*pTable)(0,col++) << _T("RF");


   const xbrShearRatingArtifact* pControllingRating;
   pRatingArtifact->GetShearRatingFactorEx(&pControllingRating);
   xbrPointOfInterest controllingPoi = pControllingRating->GetPointOfInterest();

   const xbrRatingArtifact::ShearRatings& shearRatings = pRatingArtifact->GetShearRatings();

   RowIndexType row = pTable->GetNumberOfHeaderRows();
   xbrRatingArtifact::ShearRatings::const_iterator iter(shearRatings.begin());
   xbrRatingArtifact::ShearRatings::const_iterator end(shearRatings.end());
   for ( ; iter != end; iter++ )
   {
      col = 0;
      const xbrPointOfInterest& poi = iter->first;
      const xbrShearRatingArtifact& artifact = iter->second;

      Float64 RF = artifact.GetRatingFactor();

      if ( 1.0 <= RF && !ReportAtThisPoi(poi,controllingPoi,false) )
      {
         continue;
      }

      ATLASSERT(poi == artifact.GetPointOfInterest());

      IndexType llConfigIdx;
      IndexType permitLaneIdx;
      VehicleIndexType permitVehicleIdx;
      Float64 Vpermit;
      Float64 Vlegal;
      if ( bIsWSDOTRating )
      {
         artifact.GetWSDOTPermitConfiguration(&llConfigIdx,&permitLaneIdx,&permitVehicleIdx,&Vpermit,&Vlegal);
      }

      (*pTable)(row,col++) << location.SetValue(poi);
      (*pTable)(row,col++) << artifact.GetConditionFactor();
      (*pTable)(row,col++) << artifact.GetSystemFactor();
      (*pTable)(row,col++) << artifact.GetCapacityReductionFactor();
      (*pTable)(row,col++) << shear.SetValue(artifact.GetNominalShearCapacity());
      (*pTable)(row,col++) << artifact.GetDeadLoadFactor();
      (*pTable)(row,col++) << shear.SetValue(artifact.GetDeadLoadShear());
      (*pTable)(row,col++) << artifact.GetWearingSurfaceFactor();
      (*pTable)(row,col++) << shear.SetValue(artifact.GetWearingSurfaceShear());
      (*pTable)(row,col++) << artifact.GetCreepFactor();
      (*pTable)(row,col++) << shear.SetValue(artifact.GetCreepShear());
      (*pTable)(row,col++) << artifact.GetShrinkageFactor();
      (*pTable)(row,col++) << shear.SetValue(artifact.GetShrinkageShear());
      (*pTable)(row,col++) << artifact.GetRelaxationFactor();
      (*pTable)(row,col++) << shear.SetValue(artifact.GetRelaxationShear());
      (*pTable)(row,col++) << artifact.GetSecondaryEffectsFactor();
      (*pTable)(row,col++) << shear.SetValue(artifact.GetSecondaryEffectsShear());
      (*pTable)(row,col++) << artifact.GetLiveLoadFactor();

      if ( bIsWSDOTRating )
      {
         (*pTable)(row,col++) << shear.SetValue(Vpermit);
         (*pTable)(row,col++) << shear.SetValue(Vlegal);
         (*pTable)(row,col++) << _T("LC") << LABEL_INDEX(llConfigIdx) << _T(", ") << strVehicle << LABEL_INDEX(permitLaneIdx);
      }
      else
      {
         (*pTable)(row,col++) << shear.SetValue(artifact.GetLiveLoadShear());
         (*pTable)(row,col++) << _T("LC") << LABEL_INDEX(artifact.GetLiveLoadConfigurationIndex());
         if ( ::IsDesignRatingType(ratingType) )
         {
            (*pTable)(row,col-1) << _T(", D") << LABEL_INDEX(artifact.GetVehicleIndex());
         }
      }

      if ( RF < 1 )
      {
         (*pTable)(row,col++) << RF_FAIL(rating_factor,RF);
      }
      else
      {
         (*pTable)(row,col++) << RF_PASS(rating_factor,RF);
      }

      row++;
   } // next poi
}

void CLoadRatingDetailsChapterBuilder::ReinforcementYieldingDetails(rptChapter* pChapter, std::shared_ptr<WBFL::EAF::Broker> pBroker,PierIDType pierID,pgsTypes::LoadRatingType ratingType,bool bPositiveMoment,const xbrRatingArtifact* pRatingArtifact) const
{
   GET_IFACE2(pBroker, IXBRRatingSpecification, pSpec);

   bool bIsWSDOTPermitRating = (pSpec->IsWSDOTPermitRating(ratingType) ? true : false);

   const xbrRatingArtifact::YieldStressRatios& artifacts = pRatingArtifact->GetYieldStressRatios(bPositiveMoment);
   if ( artifacts.size() == 0 )
   {
      return;
   }

   rptParagraph* pPara = new rptParagraph(rptStyleManager::GetSubheadingStyle());
   *pChapter << pPara;
   if ( bPositiveMoment )
   {
      *pPara << _T("Check Reinforcement Yielding for Positive Moment") << rptNewLine;
   }
   else
   {
      *pPara << _T("Check Reinforcement Yielding for Negative Moment") << rptNewLine;
   }

   pPara = new rptParagraph;
   *pChapter << pPara;

   if ( bIsWSDOTPermitRating )
   {
      *pPara << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("XBeamReinforcementYieldingEquation_WSDOT.png") ) << rptNewLine;
   }
   else
   {
      *pPara << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("XBeamReinforcementYieldingEquation_LRFD.png") ) << rptNewLine;
   }

   ColumnIndexType nColumns = (bIsWSDOTPermitRating ? 17 : 16);
   rptRcTable* table = rptStyleManager::CreateDefaultTable(nColumns);
   
   table->SetColumnStyle(0,rptStyleManager::GetTableCellStyle(CB_NONE | CJ_RIGHT));
   table->SetStripeRowColumnStyle(0,rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_RIGHT));

   //*pPara << table << rptNewLine; // don't add table here... see below

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   INIT_UV_PROTOTYPE( rptXBRPointOfInterest,  location, pDisplayUnits->GetSpanLengthUnit(),      false );
   INIT_UV_PROTOTYPE( rptLengthUnitValue,  dim,      pDisplayUnits->GetComponentDimUnit(),    false );
   INIT_UV_PROTOTYPE( rptStressUnitValue,  stress,   pDisplayUnits->GetStressUnit(),          false );
   INIT_UV_PROTOTYPE( rptStressUnitValue,  modE,     pDisplayUnits->GetModEUnit(),            false );
   INIT_UV_PROTOTYPE( rptMomentUnitValue,  moment,   pDisplayUnits->GetMomentUnit(),          false );
   INIT_UV_PROTOTYPE( rptLength4UnitValue, mom_i,    pDisplayUnits->GetMomentOfInertiaUnit(), false );

   INIT_SCALAR_PROTOTYPE(rptRcScalar, scalar, pDisplayUnits->GetScalarFormat());

   rptCapacityToDemand rating_factor;

   ColumnIndexType col = 0;

   (*table)(0,col++) << COLHDR(_T("Location"), rptLengthUnitTag, pDisplayUnits->GetSpanLengthUnit());
   (*table)(0,col++) << COLHDR(Sub2(_T("M"),_T("DC")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   (*table)(0,col++) << COLHDR(Sub2(_T("M"),_T("DW")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   (*table)(0,col++) << COLHDR(Sub2(_T("M"),_T("CR")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   (*table)(0,col++) << COLHDR(Sub2(_T("M"),_T("SH")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   (*table)(0,col++) << COLHDR(Sub2(_T("M"),_T("RE")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   (*table)(0,col++) << COLHDR(Sub2(_T("M"),_T("PS")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   if ( bIsWSDOTPermitRating )
   {
      (*table)(0,col++) << COLHDR(Sub2(_T("M"),_T("LL+IM")) << rptNewLine << _T("Permit"), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
      (*table)(0,col++) << COLHDR(Sub2(_T("M"),_T("LL+IM")) << rptNewLine << _T("Legal"),  rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   }
   else
   {
      (*table)(0,col++) << COLHDR(Sub2(_T("M"),_T("LL+IM")), rptMomentUnitTag, pDisplayUnits->GetMomentUnit() );
   }
   (*table)(0,col++) << COLHDR(Sub2(_T("c"),_T("p")), rptLengthUnitTag, pDisplayUnits->GetComponentDimUnit() );
   (*table)(0,col++) << COLHDR(Sub2(_T("I"),_T("crp")), rptLength4UnitTag, pDisplayUnits->GetMomentOfInertiaUnit() );
   (*table)(0,col++) << COLHDR(Sub2(_T("c"),_T("t")), rptLengthUnitTag, pDisplayUnits->GetComponentDimUnit() );
   (*table)(0,col++) << COLHDR(Sub2(_T("I"),_T("crt")), rptLength4UnitTag, pDisplayUnits->GetMomentOfInertiaUnit() );
   (*table)(0,col++) << COLHDR(RPT_STRESS(_T("r")), rptStressUnitTag, pDisplayUnits->GetStressUnit() );
   (*table)(0, col++) << COLHDR(RPT_STRESS(_T("dl")), rptStressUnitTag, pDisplayUnits->GetStressUnit());
   (*table)(0, col++) << COLHDR(RPT_STRESS(_T("ll")), rptStressUnitTag, pDisplayUnits->GetStressUnit());
   (*table)(0,col++) << _T("SR");

   const xbrYieldStressRatioArtifact* pControllingRating;
   pRatingArtifact->GetYieldStressRatioEx(bPositiveMoment,&pControllingRating);
   xbrPointOfInterest controllingPoi = pControllingRating->GetPointOfInterest();

   pPara = new rptParagraph;
   *pChapter << pPara;

   GET_IFACE2(pBroker,IXBRMaterial,pMaterial);
   Float64 Es, fy, fu;
   pMaterial->GetRebarProperties(pierID,&Es,&fy,&fu);


   dim.ShowUnitTag(true);
   modE.ShowUnitTag(true);
   
   *pPara << Sub2(_T("f"),_T("r")) << _T(" = ") << _T("k") << Sub2(_T("f"),_T("y")) << _T(" = ") << artifacts.begin()->second.GetAllowableStressRatio() << Sub2(_T("f"),_T("y")) << rptNewLine;
   *pPara << Sub2(symbol(gamma),_T("DC")) << _T(" = ") << scalar.SetValue(artifacts.begin()->second.GetDeadLoadFactor()) << rptNewLine;
   *pPara << Sub2(symbol(gamma),_T("DW")) << _T(" = ") << scalar.SetValue(artifacts.begin()->second.GetWearingSurfaceFactor()) << rptNewLine;
   *pPara << Sub2(symbol(gamma),_T("CR")) << _T(" = ") << scalar.SetValue(artifacts.begin()->second.GetCreepFactor()) << rptNewLine;
   *pPara << Sub2(symbol(gamma),_T("SH")) << _T(" = ") << scalar.SetValue(artifacts.begin()->second.GetShrinkageFactor()) << rptNewLine;
   *pPara << Sub2(symbol(gamma),_T("RE")) << _T(" = ") << scalar.SetValue(artifacts.begin()->second.GetRelaxationFactor()) << rptNewLine;
   *pPara << Sub2(symbol(gamma),_T("PS")) << _T(" = ") << scalar.SetValue(artifacts.begin()->second.GetSecondaryEffectsFactor()) << rptNewLine;
   *pPara << Sub2(symbol(gamma),_T("LL")) << _T(" = ") << scalar.SetValue(artifacts.begin()->second.GetLiveLoadFactor()) << rptNewLine;
   *pPara << RPT_EC << _T(" = ") << modE.SetValue(pMaterial->GetXBeamEc(pierID)) << rptNewLine;
   *pPara << RPT_ES << _T(" = ") << modE.SetValue(Es) << rptNewLine;
   *pPara << Sub2(_T("n"),_T("t")) << _T(" = modular ratio, transient loads = ") << artifacts.begin()->second.GetModularRatio(xbrTypes::ltTransient) << _T(" (LRFD 5.7.1, n = ") << RPT_ES << _T("/") << RPT_EC << _T(", rounded to the nearest integer number)") << rptNewLine;
   *pPara << Sub2(_T("n"),_T("p")) << _T(" = modular ratio, permanent loads = ") << artifacts.begin()->second.GetModularRatio(xbrTypes::ltPermanent) << _T(" (LRFD 5.7.1, an effective modular ratio of 2n is applicable to permanent loads)") << rptNewLine;
   *pPara << Sub2(_T("Y"),_T("bar")) << _T(" = depth to extreme tension reinforcement = ") << dim.SetValue(artifacts.begin()->second.GetYbar()) << rptNewLine;
   *pPara << Sub2(_T("c"),_T("p")) << _T(" = depth to neutral axis, permanent loads") << rptNewLine;
   *pPara << Sub2(_T("c"),_T("t")) << _T(" = depth to neutral axis, transient loads") << rptNewLine;
   *pPara << Sub2(_T("I"),_T("crp")) << _T(" = cracked section moment of inertia, permanent loads") << rptNewLine;
   *pPara << Sub2(_T("I"),_T("crt")) << _T(" = cracked section moment of inertia, transient loads") << rptNewLine;

   
   dim.ShowUnitTag(false);
   modE.ShowUnitTag(false);

   // Add table here
   *pPara << table << rptNewLine;

   RowIndexType row = table->GetNumberOfHeaderRows();
   xbrRatingArtifact::YieldStressRatios::const_iterator iter(artifacts.begin());
   xbrRatingArtifact::YieldStressRatios::const_iterator end(artifacts.end());
   for ( ; iter != end; iter++ )
   {
      col = 0;
      const xbrPointOfInterest& poi = iter->first;
      const xbrYieldStressRatioArtifact& artifact = iter->second;

      Float64 SR = artifact.GetStressRatio();

      if ( 1.0 <= SR && !ReportAtThisPoi(poi,controllingPoi, !bPositiveMoment) )
      {
         continue;
      }

      (*table)(row,col++) << location.SetValue(poi);
      (*table)(row,col++) << moment.SetValue(artifact.GetDeadLoadMoment());
      (*table)(row,col++) << moment.SetValue(artifact.GetWearingSurfaceMoment());
      (*table)(row,col++) << moment.SetValue(artifact.GetCreepMoment());
      (*table)(row,col++) << moment.SetValue(artifact.GetShrinkageMoment());
      (*table)(row,col++) << moment.SetValue(artifact.GetRelaxationMoment());
      (*table)(row,col++) << moment.SetValue(artifact.GetSecondaryEffectsMoment());
      (*table)(row,col++) << moment.SetValue(artifact.GetLiveLoadMoment());

      if ( bIsWSDOTPermitRating ) 
      {
         (*table)(row,col++) << moment.SetValue(artifact.GetAdjLiveLoadMoment());
      }

      (*table)(row,col++) << dim.SetValue(artifact.GetCrackDepth(xbrTypes::ltPermanent));
      (*table)(row,col++) << mom_i.SetValue(artifact.GetIcr(xbrTypes::ltPermanent));

      (*table)(row,col++) << dim.SetValue(artifact.GetCrackDepth(xbrTypes::ltTransient));
      (*table)(row,col++) << mom_i.SetValue(artifact.GetIcr(xbrTypes::ltTransient));

      (*table)(row, col++) << stress.SetValue(artifact.GetAllowableStress());
      (*table)(row, col++) << stress.SetValue(artifact.GetFdl());
      (*table)(row, col++) << stress.SetValue(artifact.GetFll());

      if ( SR < 1 )
      {
         (*table)(row,col++) << RF_FAIL(rating_factor,SR);
      }
      else
      {
         (*table)(row,col++) << RF_PASS(rating_factor,SR);
      }

      row++;
   }
}

void CLoadRatingDetailsChapterBuilder::LoadPostingDetails(rptChapter* pChapter, std::shared_ptr<WBFL::EAF::Broker> pBroker,const xbrRatingArtifact* pRatingArtifact) const
{
   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   rptParagraph* pPara = new rptParagraph(rptStyleManager::GetHeadingStyle());
   *pChapter << pPara;
   *pPara << _T("Load Posting Analysis Details [MBE 6A.8]") << rptNewLine;

   *pPara << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("SafePostingLoad.png") ) << rptNewLine;

   INIT_UV_PROTOTYPE( rptForceUnitValue, tonnage, pDisplayUnits->GetTonnageUnit(), false );
   rptCapacityToDemand rating_factor;

   rptRcTable* table = rptStyleManager::CreateDefaultTable(5,_T(""));
   
   table->SetColumnStyle(0,rptStyleManager::GetTableCellStyle(CB_NONE | CJ_RIGHT));
   table->SetStripeRowColumnStyle(0,rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_RIGHT));

   *pPara << table << rptNewLine;

   ColumnIndexType col = 0;
   RowIndexType row = 0;

   (*table)(row,col++) << _T("Truck");
   (*table)(row,col++) << COLHDR(_T("Weight"), rptForceUnitTag, pDisplayUnits->GetTonnageUnit() );
   (*table)(row,col++) << _T("RF");
   (*table)(row,col++) << COLHDR(_T("Safe") << rptNewLine << _T("Load") << rptNewLine << _T("Capacity"), rptForceUnitTag, pDisplayUnits->GetTonnageUnit() );
   (*table)(row,col++) << COLHDR(_T("Safe") << rptNewLine << _T("Posting") << rptNewLine << _T("Load"), rptForceUnitTag, pDisplayUnits->GetTonnageUnit() );

   row++;
   col = 0;

   Float64 posting_load, W, RF;
   std::_tstring strName;
   pRatingArtifact->GetSafePostingLoad(&posting_load,&W,&RF,&strName);
   (*table)(row,col++) << strName;
   (*table)(row,col++) << tonnage.SetValue(W);

   if ( RF < 1 )
   {
      (*table)(row,col++) << RF_FAIL(rating_factor,RF);
   }
   else
   {
      (*table)(row,col++) << RF_PASS(rating_factor,RF);
   }

   (*table)(row,col++) << tonnage.SetValue(::FloorOff(W*RF,0.01));
   if ( RF < 1 )
   {
      (*table)(row,col++) << tonnage.SetValue(posting_load);
   }
   else
   {
      (*table)(row,col++) << _T("-");
   }
}

bool CLoadRatingDetailsChapterBuilder::ReportAtThisPoi(const xbrPointOfInterest& poi,const xbrPointOfInterest& controllingPoi, bool IsNegativeMoment) const
{
   if (!m_bReportEvenIncrements)
   {
      return true; // report all
   }
   else
   {
      if (IsNegativeMoment && !m_bReportNegativeMomentBetweenFOCs)
      {
         if (poi == controllingPoi ||
             poi.HasAttribute(POI_GRID) ||
             poi.HasAttribute(POI_FACEOFCOLUMN))
         {
            return true;
         }
      }
      else if (poi == controllingPoi ||
               poi.HasAttribute(POI_GRID) ||
               poi.HasAttribute(POI_COLUMN_LEFT) ||
               poi.HasAttribute(POI_COLUMN_RIGHT))
      {
         return true;
      }
      else
      {
         return false;
      }
   }

   return false;
}
