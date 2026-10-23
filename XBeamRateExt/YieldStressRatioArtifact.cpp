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
#include <XBeamRateExt\YieldStressRatioArtifact.h>


/****************************************************************************
CLASS
   xbrYieldStressRatioArtifact
****************************************************************************/
xbrYieldStressRatioArtifact::xbrYieldStressRatioArtifact()
{
   m_bRFComputed = false;

   m_RatingType = pgsTypes::lrDesign_Inventory;
   m_PermitRatingMethod = xbrTypes::prmAASHTO;

   m_VehicleIndex = INVALID_INDEX;
   m_VehicleWeight = -99999;
   m_strVehicleName = _T("Unknown");

   m_AllowableStressRatio = 0.9;
   m_Mdc = 0;
   m_Mdw = 0;
   m_Mcr = 0;
   m_Msh = 0;
   m_Mre = 0;
   m_Mps = 0;
   m_Mllim = 0;
   m_MllimAdj = 0;
   m_gDC = 1.0;
   m_gDW = 1.0;
   m_gCR = 1.0;
   m_gSH = 1.0;
   m_gRE = 1.0;
   m_gPS = 1.0;
   m_gLL = 1.0;

   m_Icr[xbrTypes::ltPermanent] = 99999;
   m_c[xbrTypes::ltPermanent] = 99999;
   m_n[xbrTypes::ltPermanent] = 99999;

   m_Icr[xbrTypes::ltTransient] = 99999;
   m_c[xbrTypes::ltTransient] = 99999;
   m_n[xbrTypes::ltTransient] = 99999;

   m_fy = 0;
   m_Ybar = 0;
}

xbrYieldStressRatioArtifact::xbrYieldStressRatioArtifact(const xbrYieldStressRatioArtifact& rOther)
{
   MakeCopy(rOther);
}

xbrYieldStressRatioArtifact::~xbrYieldStressRatioArtifact()
{
}

xbrYieldStressRatioArtifact& xbrYieldStressRatioArtifact::operator=(const xbrYieldStressRatioArtifact& rOther)
{
   if ( this != &rOther )
   {
      MakeAssignment(rOther);
   }

   return *this;
}

void xbrYieldStressRatioArtifact::SetPointOfInterest(const xbrPointOfInterest& poi)
{
   m_POI = poi;
   m_bRFComputed = false;
}

const xbrPointOfInterest& xbrYieldStressRatioArtifact::GetPointOfInterest() const
{
   return m_POI;
}

void xbrYieldStressRatioArtifact::SetRatingType(pgsTypes::LoadRatingType ratingType)
{
   m_RatingType = ratingType;
   m_bRFComputed = false;
}

pgsTypes::LoadRatingType xbrYieldStressRatioArtifact::GetLoadRatingType() const
{
   return m_RatingType;
}

void xbrYieldStressRatioArtifact::SetPermitRatingMethod(xbrTypes::PermitRatingMethod permitRatingMethod)
{
   m_PermitRatingMethod = permitRatingMethod;
   m_bRFComputed = false;
}

xbrTypes::PermitRatingMethod xbrYieldStressRatioArtifact::GetPermitRatingMethod() const
{
   return m_PermitRatingMethod;
}

void xbrYieldStressRatioArtifact::SetVehicleIndex(VehicleIndexType vehicleIdx)
{
   m_VehicleIndex = vehicleIdx;
   m_bRFComputed = false;
}

VehicleIndexType xbrYieldStressRatioArtifact::GetVehicleIndex() const
{
   return m_VehicleIndex;
}

void xbrYieldStressRatioArtifact::SetVehicleWeight(Float64 W)
{
   m_VehicleWeight = W;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetVehicleWeight() const
{
   return m_VehicleWeight;
}

void xbrYieldStressRatioArtifact::SetVehicleName(LPCTSTR str)
{
   m_strVehicleName = str;
   m_bRFComputed = false;
}

const std::_tstring& xbrYieldStressRatioArtifact::GetVehicleName() const
{
   return m_strVehicleName;
}

void xbrYieldStressRatioArtifact::SetAllowableStressRatio(Float64 K)
{
   m_AllowableStressRatio = K;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetAllowableStressRatio() const
{
   return m_AllowableStressRatio;
}

void xbrYieldStressRatioArtifact::SetDeadLoadFactor(Float64 gDC)
{
   m_gDC = gDC;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetDeadLoadFactor() const
{
   return m_gDC;
}

void xbrYieldStressRatioArtifact::SetDeadLoadMoment(Float64 Mdc)
{
   m_Mdc = Mdc;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetDeadLoadMoment() const
{
   return m_Mdc;
}

void xbrYieldStressRatioArtifact::SetWearingSurfaceFactor(Float64 gDW)
{
   m_gDW = gDW;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetWearingSurfaceFactor() const
{
   return m_gDW;
}

void xbrYieldStressRatioArtifact::SetWearingSurfaceMoment(Float64 Mdw)
{
   m_Mdw = Mdw;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetWearingSurfaceMoment() const
{
   return m_Mdw;
}

void xbrYieldStressRatioArtifact::SetCreepFactor(Float64 gCR)
{
   m_gCR = gCR;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetCreepFactor() const
{
   return m_gCR;
}

void xbrYieldStressRatioArtifact::SetCreepMoment(Float64 Mcr)
{
   m_Mcr = Mcr;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetCreepMoment() const
{
   return m_Mcr;
}

void xbrYieldStressRatioArtifact::SetShrinkageFactor(Float64 gSH)
{
   m_gSH = gSH;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetShrinkageFactor() const
{
   return m_gSH;
}

void xbrYieldStressRatioArtifact::SetShrinkageMoment(Float64 Msh)
{
   m_Msh = Msh;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetShrinkageMoment() const
{
   return m_Msh;
}

void xbrYieldStressRatioArtifact::SetRelaxationFactor(Float64 gRE)
{
   m_gRE = gRE;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetRelaxationFactor() const
{
   return m_gRE;
}

void xbrYieldStressRatioArtifact::SetRelaxationMoment(Float64 Mre)
{
   m_Mre = Mre;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetRelaxationMoment() const
{
   return m_Mre;
}

void xbrYieldStressRatioArtifact::SetSecondaryEffectsFactor(Float64 gPS)
{
   m_gPS = gPS;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetSecondaryEffectsFactor() const
{
   return m_gPS;
}

void xbrYieldStressRatioArtifact::SetSecondaryEffectsMoment(Float64 Mps)
{
   m_Mps = Mps;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetSecondaryEffectsMoment() const
{
   return m_Mps;
}

void xbrYieldStressRatioArtifact::SetLiveLoadFactor(Float64 gLL)
{
   m_gLL = gLL;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetLiveLoadFactor() const
{
   return m_gLL;
}

void xbrYieldStressRatioArtifact::SetLiveLoadMoment(Float64 Mllim)
{
   m_Mllim = Mllim;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetLiveLoadMoment() const
{
   return m_Mllim;
}
   
void xbrYieldStressRatioArtifact::SetAdjLiveLoadMoment(Float64 Mllim)
{
   m_MllimAdj = Mllim;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetAdjLiveLoadMoment() const
{
   return m_MllimAdj;
}

void xbrYieldStressRatioArtifact::SetIcr(xbrTypes::LoadType loadType,Float64 Icr)
{
   m_Icr[loadType] = Icr;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetIcr(xbrTypes::LoadType loadType) const
{
   return m_Icr[loadType];
}

void xbrYieldStressRatioArtifact::SetCrackDepth(xbrTypes::LoadType loadType,Float64 c)
{
   m_c[loadType] = c;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetCrackDepth(xbrTypes::LoadType loadType) const
{
   return m_c[loadType];
}

void xbrYieldStressRatioArtifact::SetModularRatio(xbrTypes::LoadType loadType,Float64 n)
{
   m_n[loadType] = n;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetModularRatio(xbrTypes::LoadType loadType) const
{
   return m_n[loadType];
}

void xbrYieldStressRatioArtifact::SetYbar(Float64 Ybar)
{
   m_Ybar = Ybar;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetYbar() const
{
   return m_Ybar;
}

void xbrYieldStressRatioArtifact::SetYieldStrength(Float64 fy)
{
   m_fy = fy;
   m_bRFComputed = false;
}

Float64 xbrYieldStressRatioArtifact::GetYieldStrength() const
{
   return m_fy;
}

Float64 xbrYieldStressRatioArtifact::GetAllowableStress() const
{
   return m_AllowableStressRatio*m_fy;
}

Float64 xbrYieldStressRatioArtifact::GetFdl() const
{
   if (!m_bRFComputed)
   {
      GetStressRatio();
   }
   return m_fdl;
}

Float64 xbrYieldStressRatioArtifact::GetFll() const
{
   if (!m_bRFComputed)
   {
      GetStressRatio();
   }
   return m_fll;
}

Float64 xbrYieldStressRatioArtifact::GetStressRatio() const
{
   // NOTE: Calculation procedure based on MBE A2A.13.2.2
   if ( m_bRFComputed )
   {
      return m_StressRatio;
   }

   Float64 SR = -Float64_Max;
   Float64 fdl, fll;

   if ( IsZero(m_Mllim) || IsZero(m_gLL) )
   {
      fdl = 0;
      fll = 0;
      SR = Float64_Max;
   }
   else
   {
      Float64 kp = (m_Ybar - m_c[xbrTypes::ltPermanent])/m_Icr[xbrTypes::ltPermanent];

      Float64 fDC = m_gDC*m_Mdc*kp;
      Float64 fDW = m_gDW*m_Mdw*kp;
      Float64 fCR = m_gCR*m_Mcr*kp;
      Float64 fSH = m_gSH*m_Msh*kp;
      Float64 fRE = m_gRE*m_Mre*kp;
      Float64 fPS = m_gPS*m_Mps*kp;

      Float64 kt = (m_Ybar - m_c[xbrTypes::ltTransient])/m_Icr[xbrTypes::ltTransient];

      Float64 fLLIM = m_gLL*m_Mllim*kt;
      Float64 fLLIM_Adj = m_gLL*m_MllimAdj*kt;

      Float64 fr = GetAllowableStress();

      fdl = m_n[xbrTypes::ltPermanent]*(fDC + fDW + fCR + fSH + fRE + fPS);

      // WSDOT rating equation changed in July 2019 BDM
      //if ( ::IsPermitRatingType(m_RatingType) && m_PermitRatingMethod == xbrTypes::prmWSDOT )
      //{
      //   fdl += m_n[xbrTypes::ltTransient]*fLLIM_Adj; // WSDOT BDM Eqn. 13.1.1A-2
      //}

      fll = m_n[xbrTypes::ltTransient]*fLLIM;

      // WSDOT rating equation changed in July 2019 BDM
      if (::IsPermitRatingType(m_RatingType) && m_PermitRatingMethod == xbrTypes::prmWSDOT)
      {
         fll += m_n[xbrTypes::ltTransient] * fLLIM_Adj; // WSDOT BDM Eqn. 13.1.1A-2
      }

      SR = IsZero(fll) ? Float64_Max : (fr - fdl) / fll;

      if ( IsZero(fr) || (0 < fr && fr-fdl < 0) || (fr < 0 && 0 < fr-fdl) )
      {
         // There isn't any capacity remaining for live load
         SR = 0;
      }
      else if ( ::BinarySign(fr-fdl) != ::BinarySign(fll) && !IsZero(fr-fdl) )
      {
         // (C - DL) and LL have opposite signs
         // this case probably shouldn't happen, but if does,
         // the rating is great
         SR = Float64_Max;
      }
   }

   m_fdl = fdl;
   m_fll = fll;
   m_StressRatio = SR;;
   m_bRFComputed = true;
   return m_StressRatio;
}

void xbrYieldStressRatioArtifact::MakeCopy(const xbrYieldStressRatioArtifact& rOther)
{
   m_POI            = rOther.m_POI;
   m_RatingType     = rOther.m_RatingType;
   m_PermitRatingMethod         = rOther.m_PermitRatingMethod;
   m_VehicleIndex   = rOther.m_VehicleIndex;
   m_bRFComputed    = rOther.m_bRFComputed;
   m_StressRatio    = rOther.m_StressRatio;
   m_VehicleWeight  = rOther.m_VehicleWeight;
   m_strVehicleName = rOther.m_strVehicleName;
   m_AllowableStressRatio = rOther.m_AllowableStressRatio;
   m_Mdc            = rOther.m_Mdc;
   m_Mdw            = rOther.m_Mdw;
   m_Mcr            = rOther.m_Mcr;
   m_Msh            = rOther.m_Msh;
   m_Mre            = rOther.m_Mre;
   m_Mps            = rOther.m_Mps;
   m_Mllim          = rOther.m_Mllim;
   m_MllimAdj       = rOther.m_MllimAdj;
   m_gDC            = rOther.m_gDC;
   m_gDW            = rOther.m_gDW;
   m_gCR            = rOther.m_gCR;
   m_gSH            = rOther.m_gSH;
   m_gRE            = rOther.m_gRE;
   m_gPS            = rOther.m_gPS;
   m_gLL            = rOther.m_gLL;

   m_Icr[xbrTypes::ltPermanent] = rOther.m_Icr[xbrTypes::ltPermanent];
   m_c[xbrTypes::ltPermanent]   = rOther.m_c[xbrTypes::ltPermanent];
   m_n[xbrTypes::ltPermanent]   = rOther.m_n[xbrTypes::ltPermanent];

   m_Icr[xbrTypes::ltTransient] = rOther.m_Icr[xbrTypes::ltTransient];
   m_c[xbrTypes::ltTransient]   = rOther.m_c[xbrTypes::ltTransient];
   m_n[xbrTypes::ltTransient]   = rOther.m_n[xbrTypes::ltTransient];

   m_fy             = rOther.m_fy;
   m_Ybar           = rOther.m_Ybar;
}

void xbrYieldStressRatioArtifact::MakeAssignment(const xbrYieldStressRatioArtifact& rOther)
{
   MakeCopy( rOther );
}
