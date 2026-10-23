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
#include "EngAgent.h"
#include "LoadRater.h"
#include <IFace\Project.h>
#include <IFace\PointOfInterest.h>
#include <IFace\AnalysisResults.h>
#include <IFace\RatingSpecification.h>
#include <IFace\LoadRating.h>
#include <IFace\Pier.h>
#include <EAF\EAFDisplayUnits.h>
#include <EAF/AutoProgress.h>

#include <WBFLGenericBridge.h>

#include <PGSuperUnits.h> // for FormatDimension


xbrLoadRater::xbrLoadRater(std::weak_ptr<WBFL::EAF::Broker> pBroker) :
   m_pBroker(pBroker)
{
   //CREATE_LOGFILE("LoadRating");
}

xbrLoadRater::~xbrLoadRater(void)
{
   //CLOSE_LOGFILE;
}

xbrRatingArtifact xbrLoadRater::RateXBeam(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx)
{
   GET_IFACE2(GetBroker(),IXBRPointOfInterest,pPOI);

   GET_IFACE2(GetBroker(),IXBRProject, pProject);
   xbrTypes::PierType pierType = pProject->GetPierType(pierID);
   xbrTypes::Stage stage = (pierType == xbrTypes::pctIntegral ? xbrTypes::Stage2 : xbrTypes::Stage1);

   xbrRatingArtifact ratingArtifact(ratingType);

   GET_IFACE2(GetBroker(),IXBRRatingSpecification, pRatingSpec);

   // Rate for flexure
   MomentRating(pierID,stage,ratingType,vehicleIdx,ratingArtifact);

   // Rate for yield stress ratio, if applicable
   if ( ::IsPermitRatingType(ratingType) && pRatingSpec->CheckYieldStressLimit() )
   {
      CheckReinforcementYielding(pierID,stage,ratingType,vehicleIdx,ratingArtifact);
   }

   // Rate for shear, if applicable
   if ( pRatingSpec->RateForShear(ratingType) )
   {
      std::vector<xbrPointOfInterest> vShearPoi( pPOI->GetShearRatingPointsOfInterest(pierID) );
      ShearRating(pierID,stage,vShearPoi,ratingType,vehicleIdx,ratingArtifact);
   }

   return ratingArtifact;
}

void xbrLoadRater::MomentRating(PierIDType pierID, xbrTypes::Stage stage,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,xbrRatingArtifact& ratingArtifact)
{
   GET_IFACE2(GetBroker(),IEAFDisplayUnits,pDisplayUnits);
   GET_IFACE2(GetBroker(),IXBRPointOfInterest,pPOI);

   GET_IFACE2(GetBroker(),IEAFProgress, pProgress);
   WBFL::EAF::AutoProgress ap(pProgress);
   pProgress->UpdateMessage(_T("Load rating for moment"));

   GET_IFACE2(GetBroker(),IXBRMomentCapacity,pMomentCapacity);

   GET_IFACE2(GetBroker(),IXBRProject,pProject);
   Float64 system_factor    = pProject->GetSystemFactorFlexure();
   Float64 condition_factor = pProject->GetConditionFactor(pierID);

   pgsTypes::LimitState ls = ::GetStrengthLimitStateType(ratingType);

   Float64 gDC = pProject->GetDCLoadFactor(ls);
   Float64 gDW = pProject->GetDWLoadFactor(ls);
   Float64 gCR = pProject->GetCRLoadFactor(ls);
   Float64 gSH = pProject->GetSHLoadFactor(ls);
   Float64 gRE = pProject->GetRELoadFactor(ls);
   Float64 gPS = pProject->GetPSLoadFactor(ls);
   Float64 gLL = pProject->GetLiveLoadFactor(pierID,ls,vehicleIdx);

   std::_tstring strVehicleName = pProject->GetLiveLoadName(pierID,ratingType,vehicleIdx);

   Float64 W = (vehicleIdx == INVALID_INDEX ? 0 : pProject->GetVehicleWeight(pierID,ratingType,vehicleIdx));

   for ( int j = 0; j < 2; j++ )
   {
      bool bPositiveMoment = (j == 0 ? true : false);

      // Poi lists may be different for positive and negative moment
      std::vector<xbrPointOfInterest> vPoi( pPOI->GetMomentRatingPointsOfInterest(pierID, !bPositiveMoment) );
      IndexType nPOI = vPoi.size();

      std::vector<Float64> vDC, vDW, vCR, vSH, vRE, vPS;
      std::vector<Float64> vLLIMmin,vLLIMmax;
      std::vector<IndexType> vMinLLConfigIdx, vMaxLLConfigIdx;
      GetMoments(pierID, ratingType, vehicleIdx, vPoi, vDC, vDW, vCR, vSH, vRE, vPS, vLLIMmin, vLLIMmax, vMinLLConfigIdx, vMaxLLConfigIdx);

      for ( IndexType i = 0; i < nPOI; i++ )
      {
         const xbrPointOfInterest& poi = vPoi[i];

         Float64 DC   = vDC[i];
         Float64 DW   = vDW[i];
         Float64 CR   = vCR[i];
         Float64 SH   = vSH[i];
         Float64 RE   = vRE[i];
         Float64 PS   = vPS[i];

         Float64 LLIM          = (bPositiveMoment ? vLLIMmax[i] : vLLIMmin[i]); // Live load (includes mpf)
         IndexType llConfigIdx = (bPositiveMoment ? vMaxLLConfigIdx[i] : vMinLLConfigIdx[i]);

         CString strProgress;
         strProgress.Format(_T("Load rating %s for %s moment at %s"),
                            strVehicleName.c_str(),bPositiveMoment ? _T("positive") : _T("negative"),
                            ::FormatDimension(poi.GetDistFromStart(),pDisplayUnits->GetSpanLengthUnit()));
         pProgress->UpdateMessage(strProgress);

         const MomentCapacityDetails& momentCapacityDetails = pMomentCapacity->GetMomentCapacityDetails(pierID,stage,poi,bPositiveMoment);
         Float64 phi_moment = momentCapacityDetails.phi;
         Float64 Mn = momentCapacityDetails.Mn;

         Float64 K = 1.0;
         GET_IFACE2(GetBroker(),IXBRRatingSpecification, pSpec);
         if ( !pSpec->IsWSDOTEmergencyRating(ratingType) && !pSpec->IsWSDOTPermitRating(ratingType) )
         {
            // NOTE: For WSDOT Method - Permit cases, K has to be computed for each combination
            // of legal and permit loading... K will be computed in the moment rating artifact for the WSDOT/permit case.

            // NOTE: K can be less than zero when we are rating for negative moment and the minumum moment demand (Mu)
            // is positive. This happens near the simple ends of spans. For example Mr < 0 because we are rating for
            // negative moment and Mmin = min (1.2Mcr and 1.33Mu)... Mcr < 0 because we are looking at negative moment
            // and Mu > 0.... Since we are looking at the negative end of things, Mmin = 1.33Mu. +/- = -... it doesn't
            // make since for K to be negative... K < 0 indicates that the section is most definate NOT under reinforced.
            // No adjustment needs to be made for underreinforcement so take K = 1.0
            const MinMomentCapacityDetails& minCapacityDetails = pMomentCapacity->GetMinMomentCapacityDetails(pierID,ls,stage,poi,bPositiveMoment);
            Float64 Mr = minCapacityDetails.Mr;
            Float64 MrMin = minCapacityDetails.MrMin;
            Float64 K = (IsZero(MrMin) ? 1.0 : Mr/MrMin); // MBE 6A.5.6
            if ( K < 0.0 || 1.0 < K )
            {
               K = 1.0;
            }
         }

         xbrMomentRatingArtifact momentArtifact;
         momentArtifact.SetRatingType(ratingType);
         momentArtifact.SetPermitRatingMethod(pSpec->GetPermitRatingMethod());
         momentArtifact.SetEmergencyRatingMethod(pSpec->GetEmergencyRatingMethod());
         momentArtifact.SetPierID(pierID);
         momentArtifact.SetPointOfInterest(poi);
         momentArtifact.SetVehicleIndex(vehicleIdx == INVALID_INDEX ? llConfigIdx : vehicleIdx);
         momentArtifact.SetVehicleWeight(W);
         momentArtifact.SetVehicleName(strVehicleName.c_str());
         momentArtifact.SetSystemFactor(system_factor);
         momentArtifact.SetConditionFactor(condition_factor);
         momentArtifact.SetCapacityReductionFactor(phi_moment);
         momentArtifact.SetMinimumReinforcementFactor(K);
         momentArtifact.SetNominalMomentCapacity(Mn);
         momentArtifact.SetDeadLoadFactor(gDC);
         momentArtifact.SetDeadLoadMoment(DC);
         momentArtifact.SetWearingSurfaceFactor(gDW);
         momentArtifact.SetWearingSurfaceMoment(DW);
         momentArtifact.SetCreepFactor(gCR);
         momentArtifact.SetCreepMoment(CR);
         momentArtifact.SetShrinkageFactor(gSH);
         momentArtifact.SetShrinkageMoment(SH);
         momentArtifact.SetRelaxationFactor(gRE);
         momentArtifact.SetRelaxationMoment(RE);
         momentArtifact.SetSecondaryEffectsFactor(gPS);
         momentArtifact.SetSecondaryEffectsMoment(PS);
         momentArtifact.SetLiveLoadFactor(gLL);
         momentArtifact.SetLiveLoadMoment(LLIM);

         if ( vehicleIdx == INVALID_INDEX && llConfigIdx != INVALID_INDEX )
         {
            // llConfigIdx is actually the index of the vehicle that governs the load case... we now need to get the live load configuration
            GET_IFACE2(GetBroker(),IXBRAnalysisResults,pResults);
            Float64 Mmin, Mmax;
            IndexType minLLConfigIdx, maxLLConfigIdx;
            pResults->GetMoment(pierID,ratingType,llConfigIdx/*this is actually a vehicle index*/,poi,&Mmin,&Mmax,&minLLConfigIdx,&maxLLConfigIdx);
            if ( bPositiveMoment )
            {
               llConfigIdx = maxLLConfigIdx;
               ATLASSERT(IsEqual(LLIM,Mmax));
            }
            else
            {
               llConfigIdx = minLLConfigIdx;
               ATLASSERT(IsEqual(LLIM,Mmin));
            }
         }

         momentArtifact.SetLiveLoadConfigurationIndex(llConfigIdx);

         ratingArtifact.AddArtifact(poi,momentArtifact,bPositiveMoment);
      }
   }
}

void xbrLoadRater::ShearRating(PierIDType pierID, xbrTypes::Stage stage, const std::vector<xbrPointOfInterest>& vPoi,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,xbrRatingArtifact& ratingArtifact)
{
   GET_IFACE2(GetBroker(),IEAFDisplayUnits,pDisplayUnits);
   GET_IFACE2(GetBroker(),IEAFProgress, pProgress);
   WBFL::EAF::AutoProgress ap(pProgress);
   pProgress->UpdateMessage(_T("Load rating for shear"));

   GET_IFACE2(GetBroker(),IXBRAnalysisResults,pAnalysisResults);
   GET_IFACE2(GetBroker(),IXBRShearCapacity,pShearCapacity);

   GET_IFACE2(GetBroker(),IXBRProject,pProject);
   Float64 system_factor    = pProject->GetSystemFactorShear();
   Float64 condition_factor = pProject->GetConditionFactor(pierID);

   pgsTypes::LimitState ls = ::GetStrengthLimitStateType(ratingType);

   Float64 gDC = pProject->GetDCLoadFactor(ls);
   Float64 gDW = pProject->GetDWLoadFactor(ls);
   Float64 gCR = pProject->GetCRLoadFactor(ls);
   Float64 gSH = pProject->GetSHLoadFactor(ls);
   Float64 gRE = pProject->GetRELoadFactor(ls);
   Float64 gPS = pProject->GetPSLoadFactor(ls);
   Float64 gLL = pProject->GetLiveLoadFactor(pierID,ls,vehicleIdx);

   std::_tstring strVehicleName = pProject->GetLiveLoadName(pierID,ratingType,vehicleIdx);

   Float64 W = (vehicleIdx == INVALID_INDEX ? 0 : pProject->GetVehicleWeight(pierID,ratingType,vehicleIdx));

   GET_IFACE2(GetBroker(),IXBRRatingSpecification, pSpec);
   xbrTypes::PermitRatingMethod ratingMethod = pSpec->GetPermitRatingMethod();
   xbrTypes::EmergencyRatingMethod emergencyRatingMethod = pSpec->GetEmergencyRatingMethod();
   bool bIsWSDOTMethodApplicable = pSpec->IsWSDOTEmergencyRating(ratingType) || pSpec->IsWSDOTPermitRating(ratingType);


   for( const auto& poi : vPoi)
   {
      WBFL::System::SectionValue vDC = pAnalysisResults->GetShear(pierID,xbrTypes::lcDC,poi);
      WBFL::System::SectionValue vDW = pAnalysisResults->GetShear(pierID,xbrTypes::lcDW,poi);
      WBFL::System::SectionValue vCR = pAnalysisResults->GetShear(pierID,xbrTypes::lcCR,poi);
      WBFL::System::SectionValue vSH = pAnalysisResults->GetShear(pierID,xbrTypes::lcSH,poi);
      WBFL::System::SectionValue vRE = pAnalysisResults->GetShear(pierID,xbrTypes::lcRE,poi);
      WBFL::System::SectionValue vPS = pAnalysisResults->GetShear(pierID,xbrTypes::lcPS,poi);

      Float64 DC   = MaxMagnitude(vDC.Left(),vDC.Right());
      Float64 DW   = MaxMagnitude(vDW.Left(),vDW.Right());
      Float64 CR   = MaxMagnitude(vCR.Left(),vCR.Right());
      Float64 SH   = MaxMagnitude(vSH.Left(),vSH.Right());
      Float64 RE   = MaxMagnitude(vRE.Left(),vRE.Right());
      Float64 PS   = MaxMagnitude(vPS.Left(),vPS.Right());

      Float64 LLIM = 0;
      IndexType llConfigIdx;
      VehicleIndexType vehIdx;
      if (bIsWSDOTMethodApplicable)
      {
         // for WSDOT permit or emergency rating, the analysis is done in the rating artifact object
         LLIM = 0;
      }
      else
      {
         // moments include multiple presence factor
         if ( vehicleIdx == INVALID_INDEX )
         {
            WBFL::System::SectionValue vLLIMmin, vLLIMmax;
            IndexType minLLLeftConfigIdx, minLLRightConfigIdx, maxLLLeftConfigIdx, maxLLRightConfigIdx;
            pAnalysisResults->GetShear(pierID,ratingType,poi,&vLLIMmin,&vLLIMmax,&minLLLeftConfigIdx, &minLLRightConfigIdx, &maxLLLeftConfigIdx, &maxLLRightConfigIdx);
            LLIM = MaxMagnitude(vLLIMmin.Left(),vLLIMmin.Right(),vLLIMmax.Left(),vLLIMmax.Right());
            IndexType i = MaxMagnitudeIndex(vLLIMmin.Left(),vLLIMmin.Right(),vLLIMmax.Left(),vLLIMmax.Right());
            if ( i == 0 )
            {
               vehIdx = minLLLeftConfigIdx;
            }
            else if ( i == 1 )
            {
               vehIdx = minLLRightConfigIdx;
            }
            else if ( i == 2 )
            {
               vehIdx = maxLLLeftConfigIdx;
            }
            else if ( i == 3 )
            {
               vehIdx = maxLLRightConfigIdx;
            }
            else
            {
               ATLASSERT(false); // should never get here
            }

            WBFL::System::SectionValue Vmin, Vmax;
            IndexType minLLConfigIdx, maxLLConfigIdx;
            pAnalysisResults->GetShear(pierID,ratingType,vehIdx,poi,&Vmin,&Vmax,&minLLConfigIdx,&maxLLConfigIdx);
            if ( i == 0 || i == 1 )
            {
               llConfigIdx = minLLConfigIdx;
            }
            else
            {
               llConfigIdx = maxLLConfigIdx;
            }
         }
         else
         {
            WBFL::System::SectionValue vLLIMmin, vLLIMmax;
            IndexType minLLConfigIdx, maxLLConfigIdx;
            pAnalysisResults->GetShear(pierID,ratingType,vehicleIdx,poi,&vLLIMmin,&vLLIMmax,&minLLConfigIdx,&maxLLConfigIdx);
            LLIM = MaxMagnitude(vLLIMmin.Left(),vLLIMmin.Right(),vLLIMmax.Left(),vLLIMmax.Right());
            IndexType i = MaxMagnitudeIndex(vLLIMmin.Left(),vLLIMmin.Right(),vLLIMmax.Left(),vLLIMmax.Right());
            if ( i == 0 || i == 1 )
            {
               llConfigIdx = minLLConfigIdx;
            }
            else 
            {
               llConfigIdx = maxLLConfigIdx;
            }
         }
      }


      CString strProgress;
      strProgress.Format(_T("Load rating %s for shear at %s"),
                         strVehicleName.c_str(),
                         ::FormatDimension(poi.GetDistFromStart(),pDisplayUnits->GetSpanLengthUnit()));
      pProgress->UpdateMessage(strProgress);

      const ShearCapacityDetails& details = pShearCapacity->GetShearCapacityDetails(pierID,stage,poi);

      xbrShearRatingArtifact shearArtifact;
      shearArtifact.SetRatingType(ratingType);
      shearArtifact.SetPermitRatingMethod(ratingMethod);
      shearArtifact.SetEmergencyRatingMethod(emergencyRatingMethod);
      shearArtifact.SetPierID(pierID);
      shearArtifact.SetPointOfInterest(poi);
      shearArtifact.SetVehicleIndex(vehicleIdx == INVALID_INDEX ? vehIdx : vehicleIdx);
      shearArtifact.SetVehicleWeight(W);
      shearArtifact.SetVehicleName(strVehicleName.c_str());
      shearArtifact.SetSystemFactor(system_factor);
      shearArtifact.SetConditionFactor(condition_factor);
      shearArtifact.SetCapacityReductionFactor(details.phi);
      shearArtifact.SetNominalShearCapacity(details.Vn);
      shearArtifact.SetDeadLoadFactor(gDC);
      shearArtifact.SetDeadLoadShear(DC);
      shearArtifact.SetWearingSurfaceFactor(gDW);
      shearArtifact.SetWearingSurfaceShear(DW);
      shearArtifact.SetCreepFactor(gCR);
      shearArtifact.SetCreepShear(CR);
      shearArtifact.SetShrinkageFactor(gSH);
      shearArtifact.SetShrinkageShear(SH);
      shearArtifact.SetRelaxationFactor(gRE);
      shearArtifact.SetRelaxationShear(RE);
      shearArtifact.SetSecondaryEffectsFactor(gPS);
      shearArtifact.SetSecondaryEffectsShear(PS);
      shearArtifact.SetLiveLoadFactor(gLL);
      shearArtifact.SetLiveLoadShear(LLIM);
      shearArtifact.SetLiveLoadConfigurationIndex(llConfigIdx);

#pragma Reminder("add longitudinal reinforcement for shear check")
   //   // longitudinal steel check
   //   pgsLongReinfShearArtifact l_artifact;
   //   SHEARCAPACITYDETAILS scd;
   //   pgsDesigner2 designer;
   //   designer.SetBroker(m_pBroker);
   //   pShearCapacity->GetShearCapacityDetails(ls,loadRatingIntervalIdx,poi,&scd);
   //   designer.InitShearCheck(poi.GetSegmentKey(),loadRatingIntervalIdx,ls,nullptr);
   //   designer.CheckLongReinfShear(poi,loadRatingIntervalIdx,ls,scd,nullptr,&l_artifact);
   //   shearArtifact.SetLongReinfShearArtifact(l_artifact);

      ratingArtifact.AddArtifact(poi,shearArtifact);
   }
}

void xbrLoadRater::CheckReinforcementYielding(PierIDType pierID, xbrTypes::Stage stage,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,xbrRatingArtifact& ratingArtifact)
{
   GET_IFACE2(GetBroker(),IEAFDisplayUnits,pDisplayUnits);
   GET_IFACE2(GetBroker(),IEAFProgress, pProgress);
   WBFL::EAF::AutoProgress ap(pProgress);

   ATLASSERT(::IsPermitRatingType(ratingType));

   pgsTypes::LimitState ls = (ratingType == pgsTypes::lrPermit_Routine ? pgsTypes::ServiceI_PermitRoutine : pgsTypes::ServiceI_PermitSpecial);
   ATLASSERT(::IsServiceLimitState(ls));

   GET_IFACE2(GetBroker(),IXBRCrackedSection,pCrackedSection);
   GET_IFACE2(GetBroker(),IXBRProject,pProject);
   GET_IFACE2(GetBroker(),IXBRSectionProperties,pSectProps);
   GET_IFACE2(GetBroker(),IXBRRebar,pRebar);
   GET_IFACE2(GetBroker(),IXBRPointOfInterest,pPOI);

   GET_IFACE2(GetBroker(),IXBRRatingSpecification,pRatingSpec);
   Float64 K = pRatingSpec->GetYieldStressLimitCoefficient();

   // Get load factors
   Float64 gDC = pProject->GetDCLoadFactor(ls);
   Float64 gDW = pProject->GetDWLoadFactor(ls);
   Float64 gCR = pProject->GetCRLoadFactor(ls);
   Float64 gSH = pProject->GetSHLoadFactor(ls);
   Float64 gRE = pProject->GetRELoadFactor(ls);
   Float64 gPS = pProject->GetPSLoadFactor(ls);
   Float64 gLL = pProject->GetLiveLoadFactor(pierID,ls,vehicleIdx);


   GET_IFACE2(GetBroker(),IXBRMaterial,pMaterial);
   Float64 Es, fy, fu;
   pMaterial->GetRebarProperties(pierID,&Es,&fy,&fu);

   // Create artifacts
   for ( int i = 0; i < 2; i++ )
   {
      bool bPositiveMoment = (i == 0 ? true : false);

      // Poi lists may be different for positive and negative moment
      std::vector<xbrPointOfInterest> vPoi( pPOI->GetMomentRatingPointsOfInterest(pierID, !bPositiveMoment) );
      IndexType nPOI = vPoi.size();

      for (const auto& poi : vPoi)
      {
         Float64 Hxb = pSectProps->GetDepth(pierID,stage,poi);

         // this also gives us accees to Mlegal and Mpermit for the controlling case (WSDOT method)
         const xbrMomentRatingArtifact* pMomentRatingArtifact = ratingArtifact.GetMomentRatingArtifact(poi,bPositiveMoment);

         CString strProgress;
         strProgress.Format(_T("Checking for reinforcement yielding %s for %s moment at %s"),
                            pMomentRatingArtifact->GetVehicleName().c_str(),bPositiveMoment ? _T("positive") : _T("negative"),
                            ::FormatDimension(poi.GetDistFromStart(),pDisplayUnits->GetSpanLengthUnit()));
         pProgress->UpdateMessage(strProgress);

         // use this to get all the loading information
         Float64 DC = pMomentRatingArtifact->GetDeadLoadMoment();
         Float64 DW = pMomentRatingArtifact->GetWearingSurfaceMoment();
         Float64 CR = pMomentRatingArtifact->GetCreepMoment();
         Float64 SH = pMomentRatingArtifact->GetShrinkageMoment();
         Float64 RE = pMomentRatingArtifact->GetRelaxationMoment();
         Float64 PS = pMomentRatingArtifact->GetSecondaryEffectsMoment();

         // NOTE: live load moments include multiple presence factor
         Float64 LLIMpermit = 0;
         Float64 LLIMlegal = 0;
         if ( pMomentRatingArtifact->GetPermitRatingMethod() == xbrTypes::prmAASHTO )
         {
            LLIMpermit = pMomentRatingArtifact->GetLiveLoadMoment(); // this is for AASHTO method
         }
         else
         {
            IndexType llConfigIdx;
            IndexType permitLaneIdx;
            IndexType vehIdx;
            Float64 K;
            pMomentRatingArtifact->GetWSDOTPermitConfiguration(&llConfigIdx,&permitLaneIdx,&vehIdx,&LLIMpermit,&LLIMlegal,&K);
         }

         // Get distance to reinforcement from extreme compression face
         CComPtr<IRebarSection> rebarSection;
         pRebar->GetRebarSection(pierID,stage,poi,&rebarSection);
         Float64 Yb = (bPositiveMoment ? -DBL_MAX : DBL_MAX);
         CComPtr<IEnumRebarSectionItem> enumRebarSectionItem;
         rebarSection->get__EnumRebarSectionItem(&enumRebarSectionItem);

         CComPtr<IRebarSectionItem> rebarSectionItem;
         while ( enumRebarSectionItem->Next(1,&rebarSectionItem,nullptr) != S_FALSE )
         {
           CComPtr<IPoint2d> location;
           rebarSectionItem->get_Location(&location);
           Float64 y = pRebar->GetRebarDepth(pierID,poi,stage,location);
           ATLASSERT(0 <= y);
           Yb = (bPositiveMoment ? Max(Yb,y) : Min(Yb,y));
           rebarSectionItem.Release();
        }


         const CrackedSectionDetails& crackedSectionDetails_PermanentLoads = pCrackedSection->GetCrackedSectionDetails(pierID,stage,poi,bPositiveMoment,xbrTypes::ltPermanent);
         const CrackedSectionDetails& crackedSectionDetails_TransientLoads = pCrackedSection->GetCrackedSectionDetails(pierID,stage,poi,bPositiveMoment,xbrTypes::ltTransient);


         // Get allowable
         xbrYieldStressRatioArtifact stressRatioArtifact;
         stressRatioArtifact.SetRatingType(ratingType);
         stressRatioArtifact.SetPermitRatingMethod(pMomentRatingArtifact->GetPermitRatingMethod());
         stressRatioArtifact.SetPointOfInterest(poi);
         stressRatioArtifact.SetVehicleIndex(pMomentRatingArtifact->GetVehicleIndex());
         stressRatioArtifact.SetVehicleWeight(pMomentRatingArtifact->GetVehicleWeight());
         stressRatioArtifact.SetVehicleName(pMomentRatingArtifact->GetVehicleName().c_str());
         stressRatioArtifact.SetAllowableStressRatio(K);
         stressRatioArtifact.SetDeadLoadFactor(gDC);
         stressRatioArtifact.SetDeadLoadMoment(DC);
         stressRatioArtifact.SetWearingSurfaceFactor(gDW);
         stressRatioArtifact.SetWearingSurfaceMoment(DW);
         stressRatioArtifact.SetCreepFactor(gCR);
         stressRatioArtifact.SetCreepMoment(CR);
         stressRatioArtifact.SetShrinkageFactor(gSH);
         stressRatioArtifact.SetShrinkageMoment(SH);
         stressRatioArtifact.SetRelaxationFactor(gRE);
         stressRatioArtifact.SetRelaxationMoment(RE);
         stressRatioArtifact.SetSecondaryEffectsFactor(gPS);
         stressRatioArtifact.SetSecondaryEffectsMoment(PS);
         stressRatioArtifact.SetLiveLoadFactor(gLL);
         stressRatioArtifact.SetLiveLoadMoment(LLIMpermit);
         stressRatioArtifact.SetAdjLiveLoadMoment(LLIMlegal);
         stressRatioArtifact.SetIcr(xbrTypes::ltPermanent,crackedSectionDetails_PermanentLoads.Icr);
         stressRatioArtifact.SetIcr(xbrTypes::ltTransient,crackedSectionDetails_TransientLoads.Icr);
         stressRatioArtifact.SetCrackDepth(xbrTypes::ltPermanent,crackedSectionDetails_PermanentLoads.Ycr);
         stressRatioArtifact.SetCrackDepth(xbrTypes::ltTransient,crackedSectionDetails_TransientLoads.Ycr);
         stressRatioArtifact.SetModularRatio(xbrTypes::ltPermanent,crackedSectionDetails_PermanentLoads.n);
         stressRatioArtifact.SetModularRatio(xbrTypes::ltTransient,crackedSectionDetails_TransientLoads.n);
         stressRatioArtifact.SetYbar(Yb);
         stressRatioArtifact.SetYieldStrength(fy);
         ratingArtifact.AddArtifact(poi,stressRatioArtifact,bPositiveMoment);
      }
   } // next poi
}

void xbrLoadRater::GetMoments(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx, const std::vector<xbrPointOfInterest>& vPoi, std::vector<Float64>& vDC,std::vector<Float64>& vDW,std::vector<Float64>& vCR,std::vector<Float64>& vSH,std::vector<Float64>& vRE,std::vector<Float64>& vPS, std::vector<Float64>& vLLIMmin, std::vector<Float64>& vLLIMmax,std::vector<IndexType>& vMinLLConfigIdx,std::vector<IndexType>& vMaxLLConfigIdx)
{
   pgsTypes::LiveLoadType llType = ::GetLiveLoadType(ratingType);

   GET_IFACE2(GetBroker(),IXBRAnalysisResults,pResults);
   vDC = pResults->GetMoment(pierID,xbrTypes::lcDC,vPoi);
   vDW = pResults->GetMoment(pierID,xbrTypes::lcDW,vPoi);
   vCR = pResults->GetMoment(pierID,xbrTypes::lcCR,vPoi);
   vSH = pResults->GetMoment(pierID,xbrTypes::lcSH,vPoi);
   vRE = pResults->GetMoment(pierID,xbrTypes::lcRE,vPoi);
   vPS = pResults->GetMoment(pierID,xbrTypes::lcPS,vPoi);

   GET_IFACE2(GetBroker(),IXBRRatingSpecification, pSpec);
   if ( pSpec->IsWSDOTEmergencyRating(ratingType) || pSpec->IsWSDOTPermitRating(ratingType))
   {
      // for WSDOT permit/emergency rating, the analysis is done in the rating artifact object
      vLLIMmin.resize(vPoi.size(),0);
      vLLIMmax.resize(vPoi.size(),0);
      vMinLLConfigIdx.resize(vPoi.size(),INVALID_INDEX);
      vMaxLLConfigIdx.resize(vPoi.size(),INVALID_INDEX);
   }
   else
   {
      // moments include multiple presence factor
      if (vehicleIdx == INVALID_INDEX)
      {
         pResults->GetMoment(pierID, ratingType, vPoi, &vLLIMmin, &vLLIMmax, &vMinLLConfigIdx, &vMaxLLConfigIdx);
      }
      else
      {
         pResults->GetMoment(pierID, ratingType, vehicleIdx, vPoi, &vLLIMmin, &vLLIMmax, &vMinLLConfigIdx, &vMaxLLConfigIdx);
      }
   }
}
