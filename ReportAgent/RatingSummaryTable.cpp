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
#include <RatingSummaryTable.h>

#include <PgsExt\RatingArtifact.h>
#include <PgsExt\CapacityToDemand.h>
#include <PsgLib\Helpers.h>

#include <EAF\EAFDisplayUnits.h>
#include <IFace\Project.h>
#include <IFace\RatingSpecification.h>
#include <IFace\LoadRating.h>


CRatingSummaryTable::CRatingSummaryTable()
{
}

CRatingSummaryTable::~CRatingSummaryTable()
{
}

rptRcTable* CRatingSummaryTable::BuildByLimitState(std::shared_ptr<WBFL::EAF::Broker> pBroker,PierIDType pierID,CRatingSummaryTable::RatingTableType ratingTableType) const
{
   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);
   GET_IFACE2(pBroker,IXBRRatingSpecification,pRatingSpec);

   rptCapacityToDemand rating_factor;

   INIT_UV_PROTOTYPE( rptXBRPointOfInterest, location, pDisplayUnits->GetSpanLengthUnit(),   true  );
   INIT_UV_PROTOTYPE( rptForceUnitValue,  force,    pDisplayUnits->GetGeneralForceUnit(), false );

   rptRcScalar scalar;
   scalar.SetFormat( WBFL::System::NumericFormatTool::Format::Fixed );
   scalar.SetWidth(6);
   scalar.SetPrecision(3);
   scalar.SetTolerance(1.0e-6);


   std::_tstring strRatingType;
   std::_tstring strRoutine, strSpecial, strEmergency;
   pgsTypes::LoadRatingType routine_rating_type, special_rating_type, emergency_rating_type;
   switch( ratingTableType )
   {
   case Design:
      strRoutine = _T("Inventory");
      strSpecial = _T("Operating");
      routine_rating_type = pgsTypes::lrDesign_Inventory;
      special_rating_type = pgsTypes::lrDesign_Operating;
      break;
   case Legal:
      strRoutine = _T("Routine Commercial Traffic");
      strSpecial = _T("Specialized Hauling Vehicles");
      strEmergency = _T("Emergency Vehicles");
      routine_rating_type = pgsTypes::lrLegal_Routine;
      special_rating_type = pgsTypes::lrLegal_Special;
      emergency_rating_type = pgsTypes::lrLegal_Emergency;
      break;
   case Permit:
      strRoutine = _T("Routine Permit");
      strSpecial = _T("Special Permit");
      routine_rating_type = pgsTypes::lrPermit_Routine;
      special_rating_type = pgsTypes::lrPermit_Special;
      break;
   }

   rptRcTable* table = rptStyleManager::CreateDefaultTable(8);
   table->SetColumnStyle(0,rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   table->SetStripeRowColumnStyle(0,rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));
   table->SetColumnStyle(1,rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   table->SetStripeRowColumnStyle(1,rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));

   table->SetNumberOfHeaderRows(2);

   table->SetRowSpan(0,0,2);
   (*table)(0,0) << _T("Limit State");

   table->SetRowSpan(0,1,2);
   (*table)(0,1) << _T("Type");

   table->SetColumnSpan(0,2,3);
   (*table)(0,2) << strRoutine;
   (*table)(1,2) << _T("RF");
   (*table)(1,3) << Sub2(symbol(gamma),_T("LL"));
   (*table)(1,4) << _T("Controlling Point");

   table->SetColumnSpan(0,5,3);
   (*table)(0,5) << strSpecial;
   (*table)(1,5) << _T("RF");
   (*table)(1,6) << Sub2(symbol(gamma),_T("LL"));
   (*table)(1,7) << _T("Controlling Point");
 
   RowIndexType row0 = 2; // row counter for column 0
   RowIndexType row1 = 2; // row counter for column 1
   // row labels
   if ( ratingTableType == Design || ratingTableType == Legal )
   {
      table->SetRowSpan(row0,0,3);
      (*table)(row0,0) << _T("Strength I");
      row0 += 3;

      (*table)(row1++,1) << _T("Flexure (+M)");
      (*table)(row1++,1) << _T("Flexure (-M)");
      (*table)(row1++,1) << _T("Shear");
   }

   if ( ratingTableType == Permit )
   {
      table->SetRowSpan(row0,0,3);
      (*table)(row0,0) << _T("Strength II");
      row0 += 3;

      (*table)(row1++,1) << _T("Flexure (+M)");
      (*table)(row1++,1) << _T("Flexure (-M)");
      (*table)(row1++,1) << _T("Shear");

      table->SetRowSpan(row0,0,2);
      (*table)(row0,0) << _T("Service I");
      row0 += 2;
      (*table)(row1++,1) << _T("Stress Ratio (+M)");
      (*table)(row1++,1) << _T("Stress Ratio (-M)");
   }

   GET_IFACE2(pBroker,IXBRArtifact,pArtifact);
   const xbrRatingArtifact* pRoutineRatingArtifact = pArtifact->GetXBeamRatingArtifact(pierID,routine_rating_type,INVALID_INDEX);
   const xbrRatingArtifact* pSpecialRatingArtifact = pArtifact->GetXBeamRatingArtifact(pierID, special_rating_type, INVALID_INDEX);
   const xbrRatingArtifact* pEmergencyRatingArtifact = (ratingTableType == Legal ? pArtifact->GetXBeamRatingArtifact(pierID, emergency_rating_type, INVALID_INDEX) : nullptr);

   bool bReportPostingAnalysis = false;

   ColumnIndexType col = 2;
   for (int i = 0; i < 3; i++)
   {
      RowIndexType row = table->GetNumberOfHeaderRows();

      const xbrRatingArtifact* pRatingArtifact = (i == 0 ? pRoutineRatingArtifact : (i == 1 ? pSpecialRatingArtifact : pEmergencyRatingArtifact));
      pgsTypes::LoadRatingType ratingType      = (i == 0 ? routine_rating_type    : (i == 1 ? special_rating_type : emergency_rating_type));
      if ( pRatingArtifact )
      {
         // Strength I
         if ( ratingTableType == Design || ratingTableType == Legal )
         {
            const xbrMomentRatingArtifact* pMomentArtifact;
            Float64 RF = pRatingArtifact->GetMomentRatingFactorEx(true, &pMomentArtifact);
            if ( RF < 1 )
            {
               (*table)(row,col) << RF_FAIL(rating_factor,RF);
            }
            else
            {
               (*table)(row,col) << RF_PASS(rating_factor,RF);
            }

            if ( pMomentArtifact )
            {
               (*table)(row,col+1) << scalar.SetValue(pMomentArtifact->GetLiveLoadFactor());
               const xbrPointOfInterest& poi = pMomentArtifact->GetPointOfInterest();
               (*table)(row,col+2) << location.SetValue(poi);
            }
            else
            {
               (*table)(row,col+1) << _T("");
               (*table)(row,col+2) << _T("");
            }
            row++;

            RF = pRatingArtifact->GetMomentRatingFactorEx(false, &pMomentArtifact);
            if ( RF < 1 )
            {
               (*table)(row,col) << RF_FAIL(rating_factor,RF);
            }
            else
            {
               (*table)(row,col) << RF_PASS(rating_factor,RF);
            }

            if ( pMomentArtifact )
            {
               (*table)(row,col+1) << scalar.SetValue(pMomentArtifact->GetLiveLoadFactor());

               const xbrPointOfInterest& poi = pMomentArtifact->GetPointOfInterest();
               (*table)(row,col+2) << location.SetValue(poi);
            }
            else
            {
               (*table)(row,col+1) << _T("");
               (*table)(row,col+2) << _T("");
            }
            row++;
         }

         if ( pRatingSpec->RateForShear(ratingType) )
         {
            const xbrShearRatingArtifact* pShearArtifact;
            Float64 RF = pRatingArtifact->GetShearRatingFactorEx(&pShearArtifact);
            if ( RF < 1 )
            {
               (*table)(row,col) << RF_FAIL(rating_factor,RF);
            }
            else
            {
               (*table)(row,col) << RF_PASS(rating_factor,RF);
            }

            if ( pShearArtifact )
            {
               (*table)(row,col+1) << scalar.SetValue(pShearArtifact->GetLiveLoadFactor());

               const xbrPointOfInterest& poi = pShearArtifact->GetPointOfInterest();
               (*table)(row,col+2) << location.SetValue(poi);
            }
            else
            {
               (*table)(row,col+1) << _T("");
               (*table)(row,col+2) << _T("");
            }
            row++;
         }
         else
         {
            (*table)(row, col + 0) << RPT_NA;
            (*table)(row, col + 1) << _T("");
            (*table)(row, col + 2) << _T("");
            row++;
         }

         // Strength II
         if ( ratingTableType == Permit )
         {
            const xbrMomentRatingArtifact* pMomentArtifact;
            Float64 RF = pRatingArtifact->GetMomentRatingFactorEx(true, &pMomentArtifact);
            if ( RF < 1 )
            {
               (*table)(row,col) << RF_FAIL(rating_factor,RF);
            }
            else
            {
               (*table)(row,col) << RF_PASS(rating_factor,RF);
            }

            if ( pMomentArtifact )
            {
               (*table)(row,col+1) << scalar.SetValue(pMomentArtifact->GetLiveLoadFactor());
               const xbrPointOfInterest& poi = pMomentArtifact->GetPointOfInterest();
               (*table)(row,col+2) << location.SetValue(poi);
            }
            else
            {
               (*table)(row,col+1) << _T("");
               (*table)(row,col+2) << _T("");
            }
            row++;

            RF = pRatingArtifact->GetMomentRatingFactorEx(false, &pMomentArtifact);
            if ( RF < 1 )
            {
               (*table)(row,col) << RF_FAIL(rating_factor,RF);
            }
            else
            {
               (*table)(row,col) << RF_PASS(rating_factor,RF);
            }

            if ( pMomentArtifact )
            {
               (*table)(row,col+1) << scalar.SetValue(pMomentArtifact->GetLiveLoadFactor());

               const xbrPointOfInterest& poi = pMomentArtifact->GetPointOfInterest();
               (*table)(row,col+2) << location.SetValue(poi);
            }
            else
            {
               (*table)(row,col+1) << _T("");
               (*table)(row,col+2) << _T("");
            }
            row++;

            if ( pRatingSpec->RateForShear(ratingType) )
            {
               const xbrShearRatingArtifact* pShearArtifact;
               Float64 RF = pRatingArtifact->GetShearRatingFactorEx(&pShearArtifact);
               if ( RF < 1 )
               {
                  (*table)(row,col) << RF_FAIL(rating_factor,RF);
               }
               else
               {
                  (*table)(row,col) << RF_PASS(rating_factor,RF);
               }

               if ( pShearArtifact )
               {
                  (*table)(row,col+1) << scalar.SetValue(pShearArtifact->GetLiveLoadFactor());

                  const xbrPointOfInterest& poi = pShearArtifact->GetPointOfInterest();
                  (*table)(row,col+2) << location.SetValue(poi);
               }
               else
               {
                  (*table)(row,col+1) << _T("");
                  (*table)(row,col+2) << _T("");
               }
               row++;
            }
            else
            {
               (*table)(row, col + 0) << RPT_NA;
               (*table)(row, col + 1) << _T("");
               (*table)(row, col + 2) << _T("");
               row++;
            }
         }

         // Service I
         if ( ratingTableType == Permit )
         {
            if ( pRatingSpec->CheckYieldStressLimit() )
            {
               const xbrYieldStressRatioArtifact* pYieldStressArtifact;
               Float64 RF = pRatingArtifact->GetYieldStressRatioEx(true, &pYieldStressArtifact);
               if ( RF < 1 )
               {
                  (*table)(row,col) << RF_FAIL(rating_factor,RF);
               }
               else
               {
                  (*table)(row,col) << RF_PASS(rating_factor,RF);
               }

               if ( pYieldStressArtifact )
               {
                  (*table)(row,col+1) << scalar.SetValue(pYieldStressArtifact->GetLiveLoadFactor());

                  const xbrPointOfInterest& poi = pYieldStressArtifact->GetPointOfInterest();
                  (*table)(row,col+2) << location.SetValue(poi);
               }
               else
               {
                  (*table)(row,col+1) << _T("");
                  (*table)(row,col+2) << _T("");
               }
               row++;

               RF = pRatingArtifact->GetYieldStressRatioEx(false, &pYieldStressArtifact);
               if ( RF < 1 )
               {
                  (*table)(row,col) << RF_FAIL(rating_factor,RF);
               }
               else
               {
                  (*table)(row,col) << RF_PASS(rating_factor,RF);
               }

               if ( pYieldStressArtifact )
               {
                  (*table)(row,col+1) << scalar.SetValue(pYieldStressArtifact->GetLiveLoadFactor());

                  const xbrPointOfInterest& poi = pYieldStressArtifact->GetPointOfInterest();
                  (*table)(row,col+2) << location.SetValue(poi);
               }
               else
               {
                  (*table)(row,col+1) << _T("");
                  (*table)(row,col+2) << _T("");
               }
               row++;
            }
            else
            {
               (*table)(row,col)   << RPT_NA;
               (*table)(row,col+1) << _T("");
               (*table)(row,col+2) << _T("");
               row++;
            }
         }
      }

      col += 3;
   }

   return table;
}

rptRcTable* CRatingSummaryTable::BuildByVehicle(std::shared_ptr<WBFL::EAF::Broker> pBroker,PierIDType pierID,pgsTypes::LoadRatingType ratingType) const
{
   GET_IFACE2(pBroker,IXBRProject,pProject);

   std::_tstring strName = pProject->GetLiveLoadName(pierID,ratingType,0);

   if ( strName == NO_LIVE_LOAD_DEFINED )
   {
      return nullptr;
   }

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);
   GET_IFACE2(pBroker,IXBRArtifact,pArtifact);

   rptCapacityToDemand rating_factor;

   rptRcScalar scalar;
   scalar.SetFormat( WBFL::System::NumericFormatTool::Format::Fixed );
   scalar.SetWidth(6);
   scalar.SetPrecision(3);
   scalar.SetTolerance(1.0e-6);

   INIT_UV_PROTOTYPE( rptXBRPointOfInterest, location, pDisplayUnits->GetSpanLengthUnit(), true );

   CString strTitle = ::GetLiveLoadTypeName(ratingType);
   rptRcTable* pTable = rptStyleManager::CreateDefaultTable(5,strTitle);

   pTable->SetColumnStyle(0,rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   pTable->SetStripeRowColumnStyle(0,rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));

   pTable->SetColumnStyle(4,rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   pTable->SetStripeRowColumnStyle(4,rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));

   (*pTable)(0,0) << _T("Truck");
   (*pTable)(0,1) << _T("RF");
   (*pTable)(0,2) << Sub2(symbol(gamma),_T("LL"));
   (*pTable)(0,3)  << _T("Controlling Point");
   (*pTable)(0,4) << _T("Cause");

   RowIndexType row = pTable->GetNumberOfHeaderRows();
   VehicleIndexType nVehicles = pProject->GetLiveLoadReactionCount(pierID,ratingType);
   for ( VehicleIndexType vehicleIdx = 0; vehicleIdx < nVehicles; vehicleIdx++ )
   {
      std::_tstring strName = pProject->GetLiveLoadName(pierID,ratingType,vehicleIdx);

      const xbrRatingArtifact* pRatingArtifact = pArtifact->GetXBeamRatingArtifact(pierID,ratingType,vehicleIdx);

      const xbrMomentRatingArtifact* pPositiveMoment;
      const xbrMomentRatingArtifact* pNegativeMoment;
      const xbrShearRatingArtifact* pShear;
      const xbrYieldStressRatioArtifact* pYieldStressPositiveMoment;
      const xbrYieldStressRatioArtifact* pYieldStressNegativeMoment;

      Float64 RF = pRatingArtifact->GetRatingFactorEx(&pPositiveMoment,&pNegativeMoment,&pShear,&pYieldStressPositiveMoment,&pYieldStressNegativeMoment);

      Float64 gLL;
      std::_tstring strControlling;
      xbrPointOfInterest poi;
      bool bIsStressRatio = false;
      if ( pPositiveMoment )
      {
         ATLASSERT(vehicleIdx == pPositiveMoment->GetVehicleIndex());
         gLL = pPositiveMoment->GetLiveLoadFactor();
         strControlling = _T("Positive Moment");
         poi = pPositiveMoment->GetPointOfInterest();
      }
      else if ( pNegativeMoment )
      {
         ATLASSERT(vehicleIdx == pNegativeMoment->GetVehicleIndex());
         gLL = pNegativeMoment->GetLiveLoadFactor();
         strControlling = _T("Negative Moment");
         poi = pNegativeMoment->GetPointOfInterest();
      }
      else if ( pShear )
      {
         ATLASSERT(vehicleIdx == pShear->GetVehicleIndex());
         gLL = pShear->GetLiveLoadFactor();
         strControlling = _T("Shear");
         poi = pShear->GetPointOfInterest();
      }
      else if ( pYieldStressPositiveMoment )
      {
         ATLASSERT(vehicleIdx == pYieldStressPositiveMoment->GetVehicleIndex());
         gLL = pYieldStressPositiveMoment->GetLiveLoadFactor();
         strControlling = _T("Yield Stress Positive Moment");
         poi = pYieldStressPositiveMoment->GetPointOfInterest();
         bIsStressRatio = true;
      }
      else if ( pYieldStressNegativeMoment )
      {
         ATLASSERT(vehicleIdx == pYieldStressNegativeMoment->GetVehicleIndex());
         gLL = pYieldStressNegativeMoment->GetLiveLoadFactor();
         strControlling = _T("Yield Stress Negative Moment");
         poi = pYieldStressNegativeMoment->GetPointOfInterest();
         bIsStressRatio = true;
      }
      else
      {
         ATLASSERT(false);
         gLL = 0;
         strControlling = _T("UNKNOWN");
      }

     (*pTable)(row,0) << strName;

     if ( RF < 1 )
     {
        (*pTable)(row,1) << RF_FAIL(rating_factor,RF);
     }
     else
     {
        (*pTable)(row,1) << RF_PASS(rating_factor,RF);
     }

     if ( bIsStressRatio )
     {
        (*pTable)(row,1) << rptNewLine << _T("(Stress Ratio)");
     }

     (*pTable)(row,2) << scalar.SetValue(gLL);
     (*pTable)(row,3) << location.SetValue(poi);
     (*pTable)(row,4) << strControlling;

     row++;
   }

   return pTable;
}

rptRcTable* CRatingSummaryTable::BuildLoadPosting(std::shared_ptr<WBFL::EAF::Broker> pBroker,PierIDType pierID,pgsTypes::LoadRatingType ratingType, bool* pbMustCloseBridge) const
{
   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);
   GET_IFACE2(pBroker,IXBRProject,pProject);
   GET_IFACE2_NOCHECK(pBroker, IXBRArtifact, pArtifact); // this interface not used of nVehicles = 0

   rptCapacityToDemand rating_factor;
   INIT_UV_PROTOTYPE( rptForceUnitValue, tonnage, pDisplayUnits->GetTonnageUnit(), false );

   rptRcTable* table = rptStyleManager::CreateDefaultTable(5,_T("Load Posting (MBE 6A.8)"));

   table->SetColumnStyle(0,rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   table->SetStripeRowColumnStyle(0,rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));

   // across the top
   (*table)(0,0) << _T("Vehicle");
   (*table)(0,1) << COLHDR(_T("Vehicle Weight"), rptForceUnitTag, pDisplayUnits->GetTonnageUnit());
   (*table)(0,2) << _T("RF");
   (*table)(0,3) << COLHDR(_T("Safe Load Capacity (RT)"), rptForceUnitTag, pDisplayUnits->GetTonnageUnit());
   (*table)(0,4) << COLHDR(_T("Safe Posting Load"),  rptForceUnitTag, pDisplayUnits->GetTonnageUnit());

   bool bLoadPostingRequired = false;
   bool bMustCloseBridge = false; // MBE 6A.8.1 & .3 - bridges not capable of carrying a minimum gross live load of weight of three tons must be closed.
   Float64 RTmin = WBFL::Units::ConvertToSysUnits(3.0, WBFL::Units::Measure::Ton);

   RowIndexType row = table->GetNumberOfHeaderRows();
   VehicleIndexType nVehicles = pProject->GetLiveLoadReactionCount(pierID,ratingType);
   for ( VehicleIndexType vehicleIdx = 0; vehicleIdx < nVehicles; vehicleIdx++ )
   {
      ColumnIndexType col = 0;
      const xbrRatingArtifact* pRatingArtifact = pArtifact->GetXBeamRatingArtifact(pierID,ratingType,vehicleIdx);
      if ( pRatingArtifact )
      {
         Float64 postingLoad, W, RF;
         std::_tstring strName;
         pRatingArtifact->GetSafePostingLoad(&postingLoad,&W,&RF,&strName);

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

         Float64 RT = ::FloorOff(W*RF, 0.01);
         if (RT < RTmin || RF < 0.3) // RF < 0.3 is from MBE C6A.8.3
         {
            bMustCloseBridge = true;
         }
         (*table)(row, col++) << tonnage.SetValue(RT);

         if ( RF < 1 )
         {
            (*table)(row,col++) << tonnage.SetValue(postingLoad);
         }
         else
         {
            (*table)(row,col++) << _T("-");
         }

         if ( RF < 1 )
         {
            bLoadPostingRequired = true;
         }
      }
      else
      {
         (*table)(row,col++) << _T("");
         (*table)(row,col++) << _T("");
         (*table)(row,col++) << _T("");
         (*table)(row,col++) << _T("");
         (*table)(row,col++) << _T("");
      }

      row++;
   }

   if ( !bLoadPostingRequired )
   {
      delete table;
      table = nullptr;
   }

   *pbMustCloseBridge = bMustCloseBridge;
   return table;
}


rptRcTable* CRatingSummaryTable::BuildEmergencyVehicleLoadPosting(std::shared_ptr<WBFL::EAF::Broker> pBroker, PierIDType pierID) const
{
   GET_IFACE2(pBroker, IXBRArtifact, pArtifact);

   const xbrRatingArtifact* pEV2Artifact = pArtifact->GetXBeamRatingArtifact(pierID, pgsTypes::lrLegal_Emergency, 0);
   const xbrRatingArtifact* pEV3Artifact = pArtifact->GetXBeamRatingArtifact(pierID, pgsTypes::lrLegal_Emergency, 1);

   Float64 RF2 = pEV2Artifact->GetRatingFactor();
   Float64 RF3 = pEV3Artifact->GetRatingFactor();

   if (1.0 <= RF3)
   {
      // if RF for Type EV3 is OK, no load posting required
      return nullptr;
   }


   Float64 wgtEV2 = WBFL::Units::ConvertToSysUnits(57.5, WBFL::Units::Measure::Kip);
   Float64 wgtEV3 = WBFL::Units::ConvertToSysUnits(86.0, WBFL::Units::Measure::Kip);

   Float64 axleEV2 = WBFL::Units::ConvertToSysUnits(33.5, WBFL::Units::Measure::Kip);
   Float64 tandemEV3 = WBFL::Units::ConvertToSysUnits(62.0, WBFL::Units::Measure::Kip);

   ATLASSERT(RF3 < 1.0);

   Float64 W2, W3, GVW;
   if (RF2 < 1.0)
   {
      W2 = RF2*axleEV2;
      W3 = RF3*tandemEV3;
      GVW = Min(RF2*wgtEV2, RF3*wgtEV3);
   }
   else
   {
      W2 = axleEV2;
      W3 = RF3*tandemEV3;
      GVW = RF3*wgtEV3;
   }


   GET_IFACE2(pBroker, IEAFDisplayUnits, pDisplayUnits);

   INIT_UV_PROTOTYPE(rptForceUnitValue, tonnage, pDisplayUnits->GetTonnageUnit(), false);
   rptCapacityToDemand rating_factor;

   rptRcTable* table = rptStyleManager::CreateDefaultTable(7, _T("Emergency Vehicle Load Posting"));

   table->SetColumnStyle(0, rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   table->SetStripeRowColumnStyle(0, rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));

   ColumnIndexType col = 0;
   RowIndexType row = 0;

   table->SetNumberOfHeaderRows(2);

   table->SetColumnSpan(0, col, 2);
   (*table)(0, col) << _T("Type EV2");
   (*table)(1, col++) << COLHDR(_T("Weight"), rptForceUnitTag, pDisplayUnits->GetTonnageUnit());
   (*table)(1, col++) << _T("RF");

   table->SetColumnSpan(0, col, 2);
   (*table)(0, col) << _T("Type EV3");
   (*table)(1, col++) << COLHDR(_T("Weight"), rptForceUnitTag, pDisplayUnits->GetTonnageUnit());
   (*table)(1, col++) << _T("RF");

   table->SetRowSpan(0, col, 2);
   (*table)(0, col++) << COLHDR(_T("Single Axle"), rptForceUnitTag, pDisplayUnits->GetTonnageUnit());

   table->SetRowSpan(0, col, 2);
   (*table)(0, col++) << COLHDR(_T("Tandem"), rptForceUnitTag, pDisplayUnits->GetTonnageUnit());

   table->SetRowSpan(0, col, 2);
   (*table)(0, col++) << COLHDR(_T("Gross"), rptForceUnitTag, pDisplayUnits->GetTonnageUnit());

   row = table->GetNumberOfHeaderRows();
   col = 0;
   (*table)(row, col++) << tonnage.SetValue(wgtEV2);
   if (RF2 < 1.0)
   {
      (*table)(row, col++) << RF_FAIL(rating_factor, RF2);
   }
   else
   {
      (*table)(row, col++) << RF_PASS(rating_factor, RF2);
   }

   (*table)(row, col++) << tonnage.SetValue(wgtEV3);
   if (RF3 < 1.0)
   {
      (*table)(row, col++) << RF_FAIL(rating_factor, RF3);
   }
   else
   {
      (*table)(row, col++) << RF_PASS(rating_factor, RF3);
   }

   (*table)(row, col++) << tonnage.SetValue(W2);
   (*table)(row, col++) << tonnage.SetValue(W3);
   (*table)(row, col++) << tonnage.SetValue(GVW);

   return table;
}
