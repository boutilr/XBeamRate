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
#include <AgentTools.h>
#include <XBeamRateExt\ShearRatingArtifact.h>
#include <IFace\AnalysisResults.h>
#include <IFace\Project.h>


/****************************************************************************
CLASS
   xbrShearRatingArtifact
****************************************************************************/
xbrShearRatingArtifact::xbrShearRatingArtifact() :
m_strVehicleName(_T("Unknown"))
{
   m_bRFComputed = false;
   m_RF = 0;
   m_LLConfigIdx = INVALID_INDEX;
   m_PermitLaneIdx = INVALID_INDEX;
   m_PermitVehicleIdx = INVALID_INDEX;
   m_Vpermit = -DBL_MAX;
   m_Vlegal  = -DBL_MAX;

   m_PierID = INVALID_ID;

   m_RatingType = pgsTypes::lrDesign_Inventory;
   m_PermitRatingMethod = xbrTypes::prmAASHTO;
   m_EmergencyRatingMethod = xbrTypes::ermAASHTO;

   m_VehicleIdx = INVALID_INDEX;
   m_VehicleWeight = -99999;

   m_SystemFactor = 1.0;
   m_ConditionFactor = 1.0;
   m_CapacityRedutionFactor = 0.9;
   m_Vn = 0;
   m_gDC = 1;
   m_gDW = 1;
   m_gCR = 1;
   m_gSH = 1;
   m_gRE = 1;
   m_gPS = 1;
   m_gLL = 1;
   m_Vdc = 0;
   m_Vdw = 0;
   m_Vcr = 0;
   m_Vsh = 0;
   m_Vre = 0;
   m_Vps = 0;
   m_Vllim = 0;
}

xbrShearRatingArtifact::xbrShearRatingArtifact(const xbrShearRatingArtifact& rOther)
{
   MakeCopy(rOther);
}

xbrShearRatingArtifact::~xbrShearRatingArtifact()
{
}

xbrShearRatingArtifact& xbrShearRatingArtifact::operator=(const xbrShearRatingArtifact& rOther)
{
   if ( this != &rOther )
   {
      MakeAssignment(rOther);
   }

   return *this;
}

void xbrShearRatingArtifact::SetPierID(PierIDType pierID)
{
   m_PierID = pierID;
   m_bRFComputed = false;
}

PierIDType xbrShearRatingArtifact::GetPierID() const
{
   return m_PierID;
}

void xbrShearRatingArtifact::SetPointOfInterest(const xbrPointOfInterest& poi)
{
   m_POI = poi;
}

const xbrPointOfInterest& xbrShearRatingArtifact::GetPointOfInterest() const
{
   return m_POI;
}

void xbrShearRatingArtifact::SetRatingType(pgsTypes::LoadRatingType ratingType)
{
   m_RatingType = ratingType;
   m_bRFComputed = false;
}

pgsTypes::LoadRatingType xbrShearRatingArtifact::GetLoadRatingType() const
{
   return m_RatingType;
}

void xbrShearRatingArtifact::SetEmergencyRatingMethod(xbrTypes::EmergencyRatingMethod emergencyRatingMethod)
{
   m_EmergencyRatingMethod = emergencyRatingMethod;
   m_bRFComputed = false;
}

xbrTypes::EmergencyRatingMethod xbrShearRatingArtifact::GetEmergencyRatingMethod() const
{
   return m_EmergencyRatingMethod;
}

void xbrShearRatingArtifact::SetPermitRatingMethod(xbrTypes::PermitRatingMethod permitRatingMethod)
{
   m_PermitRatingMethod = permitRatingMethod;
   m_bRFComputed = false;
}

xbrTypes::PermitRatingMethod xbrShearRatingArtifact::GetPermitRatingMethod() const
{
   return m_PermitRatingMethod;
}

void xbrShearRatingArtifact::SetVehicleIndex(VehicleIndexType vehicleIdx)
{
   m_VehicleIdx = vehicleIdx;
}

VehicleIndexType xbrShearRatingArtifact::GetVehicleIndex() const
{
   return m_VehicleIdx;
}

void xbrShearRatingArtifact::SetVehicleWeight(Float64 W)
{
   m_VehicleWeight = W;
}

Float64 xbrShearRatingArtifact::GetVehicleWeight() const
{
   return m_VehicleWeight;
}

void xbrShearRatingArtifact::SetVehicleName(LPCTSTR str)
{
   m_strVehicleName = str;
}

const std::_tstring& xbrShearRatingArtifact::GetVehicleName() const
{
   return m_strVehicleName;
}

void xbrShearRatingArtifact::SetSystemFactor(Float64 systemFactor)
{
   m_SystemFactor = systemFactor;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetSystemFactor() const
{
   return m_SystemFactor;
}

void xbrShearRatingArtifact::SetConditionFactor(Float64 conditionFactor)
{
   m_ConditionFactor = conditionFactor;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetConditionFactor() const
{
   return m_ConditionFactor;
}

void xbrShearRatingArtifact::SetCapacityReductionFactor(Float64 phi)
{
   m_CapacityRedutionFactor = phi;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetCapacityReductionFactor() const
{
   return m_CapacityRedutionFactor;
}

void xbrShearRatingArtifact::SetNominalShearCapacity(Float64 Vn)
{
   m_Vn = Vn;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetNominalShearCapacity() const
{
   return m_Vn;
}

void xbrShearRatingArtifact::SetDeadLoadFactor(Float64 gDC)
{
   m_gDC = gDC;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetDeadLoadFactor() const
{
   return m_gDC;
}

void xbrShearRatingArtifact::SetDeadLoadShear(Float64 Vdc)
{
   m_Vdc = Vdc;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetDeadLoadShear() const
{
   return m_Vdc;
}

void xbrShearRatingArtifact::SetWearingSurfaceFactor(Float64 gDW)
{
   m_gDW = gDW;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetWearingSurfaceFactor() const
{
   return m_gDW;
}

void xbrShearRatingArtifact::SetWearingSurfaceShear(Float64 Vdw)
{
   m_Vdw = Vdw;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetWearingSurfaceShear() const
{
   return m_Vdw;
}

void xbrShearRatingArtifact::SetCreepFactor(Float64 gCR)
{
   m_gCR = gCR;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetCreepFactor() const
{
   return m_gCR;
}

void xbrShearRatingArtifact::SetCreepShear(Float64 Vcr)
{
   m_Vcr = Vcr;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetCreepShear() const
{
   return m_Vcr;
}

void xbrShearRatingArtifact::SetShrinkageFactor(Float64 gSH)
{
   m_gSH = gSH;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetShrinkageFactor() const
{
   return m_gSH;
}

void xbrShearRatingArtifact::SetShrinkageShear(Float64 Vsh)
{
   m_Vsh = Vsh;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetShrinkageShear() const
{
   return m_Vsh;
}

void xbrShearRatingArtifact::SetRelaxationFactor(Float64 gRE)
{
   m_gRE = gRE;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetRelaxationFactor() const
{
   return m_gRE;
}

void xbrShearRatingArtifact::SetRelaxationShear(Float64 Vre)
{
   m_Vre = Vre;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetRelaxationShear() const
{
   return m_Vre;
}

void xbrShearRatingArtifact::SetSecondaryEffectsFactor(Float64 gPS)
{
   m_gPS = gPS;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetSecondaryEffectsFactor() const
{
   return m_gPS;
}

void xbrShearRatingArtifact::SetSecondaryEffectsShear(Float64 Vps)
{
   m_Vps = Vps;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetSecondaryEffectsShear() const
{
   return m_Vps;
}

void xbrShearRatingArtifact::SetLiveLoadFactor(Float64 gLL)
{
   m_gLL = gLL;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetLiveLoadFactor() const
{
   return m_gLL;
}

void xbrShearRatingArtifact::SetLiveLoadShear(Float64 Vllim)
{
   m_Vllim = Vllim;
   m_bRFComputed = false;
}

Float64 xbrShearRatingArtifact::GetLiveLoadShear() const
{
   return m_Vllim;
}

void xbrShearRatingArtifact::SetLiveLoadConfigurationIndex(IndexType llConfigIdx)
{
   m_LLConfigIdx = llConfigIdx;
   m_bRFComputed = false;
}

IndexType xbrShearRatingArtifact::GetLiveLoadConfigurationIndex() const
{
   return m_LLConfigIdx;
}

Float64 xbrShearRatingArtifact::GetRatingFactor() const
{
   if ( m_bRFComputed )
   {
      return m_RF;
   }


   if ( 
      (::IsPermitRatingType(m_RatingType) && m_PermitRatingMethod == xbrTypes::prmWSDOT) 
      ||
      (::IsEmergencyRatingType(m_RatingType) && m_EmergencyRatingMethod == xbrTypes::ermWSDOT)
      )
   {
      // we don't know which combination of permit and legal loads cause the minimum rating factor
      // so we have to check them all.... the min rating factor could happen when the legal load
      // effect is greatest, leaving the least reserve capacity for the permit vehicle, or when
      // the legal load effect is least coupled with a very large permit load response, or something
      // in between.
      Float64 RFmin = DBL_MAX;
      bool bFirst = true;
      pgsTypes::LimitState ls = ::GetStrengthLimitStateType(m_RatingType);

#if defined COMPARE_WITH_FULL_ANALYSIS
      Float64 _RFmin = DBL_MAX;
      bool _bFirst = true;
      IndexType _LLConfigIdx;
      IndexType _PermitLaneIdx;
      VehicleIndexType _PermitVehicleIdx;
      Float64 _Vpermit;
      Float64 _Vlegal;
#endif

      auto pBroker = EAFGetBroker();

      GET_IFACE2(pBroker,IXBRProductForces,pProductForces);
      GET_IFACE2(pBroker,IXBRAnalysisResults,pAnalysisResults);
      GET_IFACE2_NOCHECK(pBroker,IXBRProject,pProject);

      VehicleIndexType nVehicles = (m_VehicleIdx == INVALID_INDEX ? pProject->GetLiveLoadReactionCount(m_PierID,m_RatingType) : 1);
      VehicleIndexType firstVehicleIdx = (m_VehicleIdx == INVALID_INDEX ? 0 : m_VehicleIdx);
      VehicleIndexType lastVehicleIdx  = (m_VehicleIdx == INVALID_INDEX ? nVehicles-1 : firstVehicleIdx);
      if ( nVehicles == 0 )
      {
         lastVehicleIdx = firstVehicleIdx;
      }

      std::vector<IndexType> vLLConfigIdx;
      pProductForces->GetGoverningShearLiveLoadConfigurations(m_PierID,m_POI,&vLLConfigIdx);
      for (const auto& llConfigIdx : vLLConfigIdx)
      {
         IndexType nLoadedLanes = pProductForces->GetLoadedLaneCount(m_PierID,llConfigIdx);
         for ( IndexType permitLaneIdx = 0; permitLaneIdx < nLoadedLanes; permitLaneIdx++ )
         {
            for ( VehicleIndexType vehicleIdx = firstVehicleIdx; vehicleIdx <= lastVehicleIdx; vehicleIdx++ )
            {
               Float64 rf;

               WBFL::System::SectionValue Vpermit, Vlegal; // Shear include multiple presence factor
               pAnalysisResults->GetShear(m_PierID,m_RatingType,vehicleIdx,llConfigIdx,permitLaneIdx,m_POI,&Vpermit,&Vlegal);
   
               Float64 vp = MaxMagnitude(Vpermit.Left(),Vpermit.Right());
               Float64 vl = MaxMagnitude(Vlegal.Left(),Vlegal.Right());
               rf = GetRatingFactor(vp,vl);

               if ( rf < RFmin || bFirst )
               {
                  RFmin              = rf;
                  m_LLConfigIdx      = llConfigIdx;
                  m_PermitLaneIdx    = permitLaneIdx;
                  m_PermitVehicleIdx = vehicleIdx;
                  m_Vpermit          = vp;
                  m_Vlegal           = vl;

                  bFirst = false;
               }
            } // next vehicle
         } // permit truck in next position
      } // next live load configuration

#if defined COMPARE_WITH_FULL_ANALYSIS
      IndexType nLiveLoadConfigurations = pProductForces->GetLiveLoadConfigurationCount(m_PierID,m_RatingType);
      for ( IndexType llConfigIdx = 0; llConfigIdx < nLiveLoadConfigurations; llConfigIdx++ )
      {
         IndexType nLoadedLanes = pProductForces->GetLoadedLaneCount(m_PierID,llConfigIdx);
         for ( IndexType permitLaneIdx = 0; permitLaneIdx < nLoadedLanes; permitLaneIdx++ )
         {
            for ( VehicleIndexType vehicleIdx = firstVehicleIdx; vehicleIdx <= lastVehicleIdx; vehicleIdx++ )
            {
               Float64 rf;

               WBFL::System::SectionValue Vpermit, Vlegal;
               pAnalysisResults->GetShear(m_PierID,m_RatingType,vehicleIdx,llConfigIdx,permitLaneIdx,m_POI,&Vpermit,&Vlegal);

               Float64 vp = MaxMagnitude(Vpermit.Left(),Vpermit.Right());
               Float64 vl = MaxMagnitude(Vlegal.Left(),Vlegal.Right());
               rf = GetRatingFactor(vp,vl);

               if ( rf < _RFmin || _bFirst )
               {
                  _RFmin            = rf;
                  _LLConfigIdx      = llConfigIdx;
                  _PermitLaneIdx    = permitLaneIdx;
                  _PermitVehicleIdx = vehicleIdx;
                  _Vpermit          = vp;
                  _Vlegal           = vl;

                  _bFirst = false;
               }
            } // next vehicle
         } // permit truck in next position
      } // next live load configuration

#if defined _DEBUG
      if ( _RFmin != DBL_MAX )
      {
         ATLASSERT(IsEqual(_RFmin,RFmin));
         ATLASSERT(_LLConfigIdx == m_LLConfigIdx);
         ATLASSERT(_PermitLaneIdx == m_PermitLaneIdx);
         ATLASSERT(_PermitVehicleIdx == m_PermitVehicleIdx);
         ATLASSERT(IsEqual(_Vpermit,m_Vpermit));
         ATLASSERT(IsEqual(_Vlegal,m_Vlegal));
      }
#else
      if ( _RFmin != DBL_MAX &&
            (!IsEqual(_RFmin,RFmin) ||
            _LLConfigIdx != m_LLConfigIdx ||
            _PermitLaneIdx != m_PermitLaneIdx ||
            _PermitVehicleIdx != m_PermitVehicleIdx ||
            !IsEqual(_Vpermit,m_Vpermit) ||
            !IsEqual(_Vlegal,m_Vlegal))
            )
      {
         CString strMsg1;
         strMsg1.Format(_T("Full and simplified analysis results don't match.\r\nPOI %d @ %f\r\nRF %f (Full), %f (Simplified)"),m_POI.GetID(),m_POI.GetDistFromStart(),_RFmin,RFmin);

         CString strMsg2;
         strMsg2.Format(_T("LLConfigIdx %d (Full), %d (Simplified)"),_LLConfigIdx,m_LLConfigIdx);

         CString strMsg3;
         strMsg3.Format(_T("PermitLaneIdx %d (Full), %d (Simplified)"),_PermitLaneIdx,m_PermitLaneIdx);

         CString strMsg4;
         strMsg4.Format(_T("PermitVehicleIdx %d (Full), %d (Simplified)"),_PermitVehicleIdx,m_PermitVehicleIdx);

         CString strMsg5;
         strMsg5.Format(_T("Vpermit %f (Full), %f (Simplified)"),_Vpermit,m_Vpermit);

         CString strMsg6;
         strMsg6.Format(_T("Vlegal %f (Full), %f (Simplified)"),_Vlegal,m_Vlegal);

         CString strMsg = strMsg1 + _T("\r\n") + strMsg2 + _T("\r\n") + strMsg3 + _T("\r\n") + strMsg4 + _T("\r\n") + strMsg5 + _T("\r\n") + strMsg6;
         AfxMessageBox(strMsg);
      }
#endif
#endif // COMPARE_WITH_FULL_ANALYSIS

      m_RF = RFmin;
   }
   else
   {
      m_RF = GetRatingFactor(m_Vllim,0);
   }

   m_bRFComputed = true;
   return m_RF;
}

void xbrShearRatingArtifact::GetWSDOTPermitConfiguration(IndexType* pLLConfigIdx,IndexType* pPermitLaneIdx,VehicleIndexType* pVehicleIdx,Float64 *pVpermit,Float64* pVlegal) const
{
   Float64 RF = GetRatingFactor(); // causes the rating factor analysis to happen

   *pLLConfigIdx   = m_LLConfigIdx;
   *pPermitLaneIdx = m_PermitLaneIdx;
   *pVehicleIdx    = m_PermitVehicleIdx;
   *pVpermit       = m_Vpermit;
   *pVlegal        = m_Vlegal;
}

void xbrShearRatingArtifact::MakeCopy(const xbrShearRatingArtifact& rOther)
{
   m_PierID                     = rOther.m_PierID;
   m_POI                        = rOther.m_POI;
   m_RatingType                 = rOther.m_RatingType;
   m_PermitRatingMethod         = rOther.m_PermitRatingMethod;
   m_EmergencyRatingMethod      = rOther.m_EmergencyRatingMethod;
   m_VehicleIdx                 = rOther.m_VehicleIdx;
   m_VehicleWeight              = rOther.m_VehicleWeight;
   m_strVehicleName             = rOther.m_strVehicleName;
   m_bRFComputed                = rOther.m_bRFComputed;
   m_RF                         = rOther.m_RF;
   m_LLConfigIdx                = rOther.m_LLConfigIdx;
   m_PermitLaneIdx              = rOther.m_PermitLaneIdx;
   m_PermitVehicleIdx           = rOther.m_PermitVehicleIdx;
   m_Vpermit                    = rOther.m_Vpermit;
   m_Vlegal                     = rOther.m_Vlegal;
   m_SystemFactor               = rOther.m_SystemFactor;
   m_ConditionFactor            = rOther.m_ConditionFactor;
   m_CapacityRedutionFactor     = rOther.m_CapacityRedutionFactor;
   m_Vn                         = rOther.m_Vn;
   m_gDC                        = rOther.m_gDC;
   m_gDW                        = rOther.m_gDW;
   m_gCR                        = rOther.m_gCR;
   m_gSH                        = rOther.m_gSH;
   m_gRE                        = rOther.m_gRE;
   m_gPS                        = rOther.m_gPS;
   m_gLL                        = rOther.m_gLL;
   m_Vdc                        = rOther.m_Vdc;
   m_Vdw                        = rOther.m_Vdw;
   m_Vcr                        = rOther.m_Vcr;
   m_Vsh                        = rOther.m_Vsh;
   m_Vre                        = rOther.m_Vre;
   m_Vps                        = rOther.m_Vps;
   m_Vllim                      = rOther.m_Vllim;
}

void xbrShearRatingArtifact::MakeAssignment(const xbrShearRatingArtifact& rOther)
{
   MakeCopy( rOther );
}

Float64 xbrShearRatingArtifact::GetRatingFactor(Float64 Vllim,Float64 VllimAdj) const
{
   Float64 RF = -Float64_Max;

   Float64 p = m_SystemFactor * m_ConditionFactor;
   if ( p < 0.85 )
   {
      p = 0.85; // 6A.4.2.1-3)
   }

   Float64 C = p * m_CapacityRedutionFactor * m_Vn;
   Float64 RFtop = C - fabs(m_gDC*m_Vdc + m_gDW*m_Vdw + m_gCR*m_Vcr + m_gSH*m_Vsh + m_gRE*m_Vre + m_gPS*m_Vps);

   // WSDOT rating equation changed in July 2019 BDM
   //if (
   //   (::IsPermitRatingType(m_RatingType) && m_PermitRatingMethod == xbrTypes::prmWSDOT)
   //   ||
   //   (::IsEmergencyRatingType(m_RatingType) && m_EmergencyRatingMethod == xbrTypes::ermWSDOT)
   //   )
   //{
   //   RFtop -= fabs(m_gLL*VllimAdj); // WSDOT BDM Eqn. 13.1.1A-2
   //}

   Float64 RFbot = fabs(m_gLL*Vllim);
   
   // WSDOT rating equation changed in July 2019 BDM
   if (
      (::IsPermitRatingType(m_RatingType) && m_PermitRatingMethod == xbrTypes::prmWSDOT)
      ||
      (::IsEmergencyRatingType(m_RatingType) && m_EmergencyRatingMethod == xbrTypes::ermWSDOT)
      )
   {
      RFbot -= fabs(m_gLL*VllimAdj); // WSDOT BDM Eqn. 13.1.1A-2
   }

   if (IsZero(C) && IsZero(RFtop) && IsZero(RFbot))
   {
      RF = Float64_Max;
   }
   else if ( IsZero(C) || (0 < C && RFtop < 0) || (C < 0 && 0 < RFtop) )
   {
      // There isn't any capacity remaining for live load
      RF = 0;
   }
   else if ( ::BinarySign(RFtop) != ::BinarySign(RFbot) && !IsZero(RFtop) )
   {
      // (C - DL) and LL have opposite signs
      // this case probably shouldn't happen, but if does,
      // the rating is great
      RF = Float64_Max;
   }
   else if (IsZero(Vllim) || IsZero(m_gLL))
   {
      RF = Float64_Max;
   }
   else
   {
      RF = IsZero(RFbot) ? Float64_Max : RFtop / RFbot;
   }

   return RF;
}
