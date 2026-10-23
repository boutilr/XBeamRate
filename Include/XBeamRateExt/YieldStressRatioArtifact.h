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

#pragma once

#include <XBeamRateExt\XBRExtExp.h>
#include <XBeamRateExt\PointOfInterest.h>

/*****************************************************************************
CLASS 
   xbrYieldStressRatioArtifact

   Artifact for permit rating yield stress ratio evaluation


DESCRIPTION
   Artifact for permit rating yield stress ratio evaluation. See LRFR 6A.5.4.2.2b

LOG
   rab : 01.07.2010 : Created file
*****************************************************************************/

class XBREXTCLASS xbrYieldStressRatioArtifact
{
public:
   xbrYieldStressRatioArtifact();
   xbrYieldStressRatioArtifact(const xbrYieldStressRatioArtifact& rOther);
   virtual ~xbrYieldStressRatioArtifact();

   xbrYieldStressRatioArtifact& operator = (const xbrYieldStressRatioArtifact& rOther);

   void SetPointOfInterest(const xbrPointOfInterest& poi);
   const xbrPointOfInterest& GetPointOfInterest() const;

   void SetRatingType(pgsTypes::LoadRatingType ratingType);
   pgsTypes::LoadRatingType GetLoadRatingType() const;

   void SetPermitRatingMethod(xbrTypes::PermitRatingMethod permitRatingMethod);
   xbrTypes::PermitRatingMethod GetPermitRatingMethod() const;

   void SetVehicleIndex(VehicleIndexType vehicleIdx);
   VehicleIndexType GetVehicleIndex() const;

   void SetVehicleWeight(Float64 W);
   Float64 GetVehicleWeight() const;

   void SetVehicleName(LPCTSTR str);
   const std::_tstring& GetVehicleName() const;

   void SetAllowableStressRatio(Float64 K);
   Float64 GetAllowableStressRatio() const;

   void SetDeadLoadFactor(Float64 gDC);
   Float64 GetDeadLoadFactor() const;

   void SetDeadLoadMoment(Float64 Mdc);
   Float64 GetDeadLoadMoment() const;

   void SetWearingSurfaceFactor(Float64 gDW);
   Float64 GetWearingSurfaceFactor() const;

   void SetWearingSurfaceMoment(Float64 Mdw);
   Float64 GetWearingSurfaceMoment() const;

   void SetCreepFactor(Float64 gCR);
   Float64 GetCreepFactor() const;

   void SetCreepMoment(Float64 Mcr);
   Float64 GetCreepMoment() const;

   void SetShrinkageFactor(Float64 gSH);
   Float64 GetShrinkageFactor() const;

   void SetShrinkageMoment(Float64 Msh);
   Float64 GetShrinkageMoment() const;

   void SetRelaxationFactor(Float64 gRE);
   Float64 GetRelaxationFactor() const;

   void SetRelaxationMoment(Float64 Mre);
   Float64 GetRelaxationMoment() const;

   void SetSecondaryEffectsFactor(Float64 gPS);
   Float64 GetSecondaryEffectsFactor() const;

   void SetSecondaryEffectsMoment(Float64 Mps);
   Float64 GetSecondaryEffectsMoment() const;

   void SetLiveLoadFactor(Float64 gLL);
   Float64 GetLiveLoadFactor() const;

   void SetLiveLoadMoment(Float64 Mllim);
   Float64 GetLiveLoadMoment() const;
   
   void SetAdjLiveLoadMoment(Float64 Mllim);
   Float64 GetAdjLiveLoadMoment() const;

   void SetIcr(xbrTypes::LoadType loadType,Float64 Icr);
   Float64 GetIcr(xbrTypes::LoadType loadType) const;

   void SetCrackDepth(xbrTypes::LoadType loadType,Float64 c);
   Float64 GetCrackDepth(xbrTypes::LoadType loadType) const;

   void SetModularRatio(xbrTypes::LoadType loadType,Float64 n);
   Float64 GetModularRatio(xbrTypes::LoadType loadType) const;

   void SetYbar(Float64 Ybar);
   Float64 GetYbar() const;

   void SetYieldStrength(Float64 fy);
   Float64 GetYieldStrength() const;

   Float64 GetAllowableStress() const;
   Float64 GetRebarStress() const;
   Float64 GetFdl() const;
   Float64 GetFll() const;
   Float64 GetStressRatio() const;
   Float64 GetRatingFactor() const { return GetStressRatio(); }

protected:
   void MakeCopy(const xbrYieldStressRatioArtifact& rOther);
   void MakeAssignment(const xbrYieldStressRatioArtifact& rOther);

   mutable bool m_bRFComputed;
   mutable Float64 m_StressRatio;
   mutable Float64 m_fdl;
   mutable Float64 m_fll;

   xbrPointOfInterest m_POI;

   pgsTypes::LoadRatingType m_RatingType;
   xbrTypes::PermitRatingMethod m_PermitRatingMethod;

   VehicleIndexType m_VehicleIndex;
   Float64 m_VehicleWeight;
   std::_tstring m_strVehicleName;

   Float64 m_AllowableStressRatio; // stress in reinforcement should not exceed this times the yield strength
   Float64 m_Mdc;
   Float64 m_Mdw;
   Float64 m_Mcr;
   Float64 m_Msh;
   Float64 m_Mre;
   Float64 m_Mps;
   Float64 m_Mllim;
   Float64 m_MllimAdj;
   Float64 m_gDC;
   Float64 m_gDW;
   Float64 m_gCR;
   Float64 m_gSH;
   Float64 m_gRE;
   Float64 m_gPS;
   Float64 m_gLL;

   // use xbrTypes::LoadType for array index
   Float64 m_Icr[2];
   Float64 m_c[2];
   Float64 m_n[2];

   Float64 m_Ybar;
   Float64 m_fy;
};
