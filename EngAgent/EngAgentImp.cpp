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

// EngAgentImp.cpp : Implementation of CEngAgentImp
#include "stdafx.h"
#include "EngAgent.h"
#include "EngAgentImp.h"

#include "CrackedSectionStressStrainModel.h"

#include <XBeamRateExt\PointOfInterest.h>
#include <XBeamRateExt\XBeamRateUtilities.h>
#include <IFace\Project.h>
#include <IFace\RatingSpecification.h>
#include <IFace\Pier.h>
#include <IFace\AnalysisResults.h>
#include <EAF/AutoProgress.h>

#include <IFace\Intervals.h>
#include <IFace\Bridge.h>

#include <EAF\EAFDisplayUnits.h>
#include <MFCTools\Format.h>

#include <Units\Convert.h>

#include "LoadRater.h"

#include <WBFLGenericBridge.h>
#include <WBFLSTL.h>

//////////////////////////////////////////////////////////////////////
// Agent
bool CEngAgentImp::RegisterInterfaces()
{
   EAF_AGENT_REGISTER_INTERFACES;

   REGISTER_INTERFACE(IXBRMomentCapacity);
   REGISTER_INTERFACE(IXBRShearCapacity);
   REGISTER_INTERFACE(IXBRCrackedSection);
   REGISTER_INTERFACE(IXBRArtifact);

   return true;
};

bool CEngAgentImp::Init()
{
   EAF_AGENT_INIT;

   CreateDataStructures();

   HRESULT hr;
   hr = m_MomentCapacitySolver.CoCreateInstance(CLSID_LRFDSolver2);
   ATLASSERT(SUCCEEDED(hr));

   CComQIPtr<ILRFDSolver2> solver(m_MomentCapacitySolver);
   if (WBFL::LRFD::BDSManager::GetUnits() == WBFL::LRFD::BDSManager::Units::US)
   {
      solver->put_UnitMode(suUS);
   }
   else
   {
      solver->put_UnitMode(suSI);
   }

   hr = m_CrackedSectionSolver.CoCreateInstance(CLSID_NLSolver);
   ATLASSERT(SUCCEEDED(hr));

   // use only one slice for cracked section analysis
   CComQIPtr<INLSolver> nlsolver(m_CrackedSectionSolver);
   nlsolver->put_Slices(1);


   //
   // Attach to connection points for interfaces this agent depends on
   //
   m_dwProjectCookie = REGISTER_EVENT_SINK(IXBRProjectEventSink);
   m_dwBridgeDescCookie = REGISTER_EVENT_SINK(IBridgeDescriptionEventSink);

   return true;
}

bool CEngAgentImp::Reset()
{
   EAF_AGENT_RESET;

   return true;
}

CLSID CEngAgentImp::GetCLSID() const
{
   return CLSID_XBeamRateEngAgent;
}

bool CEngAgentImp::ShutDown()
{
   EAF_AGENT_SHUTDOWN;

   Invalidate(false);

   //
   // Detach to connection points
   //
   UNREGISTER_EVENT_SINK(IXBRProjectEventSink, m_dwProjectCookie);
   UNREGISTER_EVENT_SINK(IBridgeDescriptionEventSink, m_dwBridgeDescCookie);

   //EAF_AGENT_CLEAR_INTERFACE_CACHE;
   return true;
}

//////////////////////////////////////////////////////////////////////
// IXBRMomentCapacity
Float64 CEngAgentImp::GetMomentCapacity(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const
{
   const MomentCapacityDetails& capacityDetails = GetMomentCapacityDetails(pierID,stage,poi,bPositiveMoment);
   return capacityDetails.Mr;
}

const MomentCapacityDetails& CEngAgentImp::GetMomentCapacityDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const
{
   std::map<IDType,MomentCapacityDetails>* pCapacity = (bPositiveMoment ? m_pPositiveMomentCapacity[stage].get() : m_pNegativeMomentCapacity[stage].get());
   std::map<IDType,MomentCapacityDetails>::iterator found(pCapacity->find(poi.GetID()));
   if ( found != pCapacity->end() )
   {
      return found->second;
   }

   MomentCapacityDetails capacityDetails = ComputeMomentCapacity(pierID,stage,poi,bPositiveMoment);
   ATLASSERT(poi.GetID() != INVALID_ID);

   std::pair<std::map<IDType,MomentCapacityDetails>::iterator,bool> result = pCapacity->insert(std::make_pair(poi.GetID(),capacityDetails));
   ATLASSERT(result.second == true);

   std::map<IDType,MomentCapacityDetails>::iterator iter = result.first;
   MomentCapacityDetails& details = iter->second;
   return details;
}

Float64 CEngAgentImp::GetCrackingMoment(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const
{
   const CrackingMomentDetails& McrDetails = GetCrackingMomentDetails(pierID,stage,poi,bPositiveMoment);
   Float64 Mcr = McrDetails.Mcr;

   bool bAfter2002 = ( WBFL::LRFD::BDSManager::Edition::SecondEditionWith2003Interims <= WBFL::LRFD::BDSManager::GetEdition() ? true : false );
   bool bBefore2012 = ( WBFL::LRFD::BDSManager::GetEdition() <  WBFL::LRFD::BDSManager::Edition::SixthEdition2012 ? true : false );
   if ( bAfter2002 && bBefore2012 )
   {
      Mcr = (bPositiveMoment ? Max(McrDetails.Mcr,McrDetails.McrLimit) : Min(McrDetails.Mcr,McrDetails.McrLimit));
   }
   else
   {
      Mcr = McrDetails.Mcr;
   }

   return Mcr;
}

const CrackingMomentDetails& CEngAgentImp::GetCrackingMomentDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const
{
   std::map<IDType,CrackingMomentDetails>* pCapacity = (bPositiveMoment ? m_pPositiveCrackingMoment[stage].get() : m_pNegativeCrackingMoment[stage].get());
   std::map<IDType,CrackingMomentDetails>::iterator found(pCapacity->find(poi.GetID()));
   if ( found != pCapacity->end() )
   {
      return found->second;
   }

   CrackingMomentDetails McrDetails = ComputeCrackingMoment(pierID,stage,poi,bPositiveMoment);
   ATLASSERT(poi.GetID() != INVALID_ID);

   std::pair<std::map<IDType,CrackingMomentDetails>::iterator,bool> result = pCapacity->insert(std::make_pair(poi.GetID(),McrDetails));
   ATLASSERT(result.second == true);

   std::map<IDType,CrackingMomentDetails>::iterator iter = result.first;
   CrackingMomentDetails& details = iter->second;
   return details;
}

Float64 CEngAgentImp::GetMinMomentCapacity(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const
{
   const MinMomentCapacityDetails& MminDetails = GetMinMomentCapacityDetails(pierID,limitState,stage,poi,bPositiveMoment);
   return MminDetails.MrMin;
}

const MinMomentCapacityDetails& CEngAgentImp::GetMinMomentCapacityDetails(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const
{
   ATLASSERT(::IsRatingLimitState(limitState));// must be a load rating limit state
   std::map<IDType,MinMomentCapacityDetails>* pCapacity = (bPositiveMoment ? m_pPositiveMinMomentCapacity[stage][GET_INDEX(limitState)].get() : m_pNegativeMinMomentCapacity[stage][GET_INDEX(limitState)].get());
   std::map<IDType,MinMomentCapacityDetails>::iterator found(pCapacity->find(poi.GetID()));
   if ( found != pCapacity->end() )
   {
      return found->second;
   }

   MinMomentCapacityDetails MminDetails = ComputeMinMomentCapacity(pierID,limitState,stage,poi,bPositiveMoment);
   ATLASSERT(poi.GetID() != INVALID_ID);

   std::pair<std::map<IDType,MinMomentCapacityDetails>::iterator,bool> result = pCapacity->insert(std::make_pair(poi.GetID(),MminDetails));
   ATLASSERT(result.second == true);

   std::map<IDType,MinMomentCapacityDetails>::iterator iter = result.first;
   MinMomentCapacityDetails& details = iter->second;
   return details;
}

MinMomentCapacityDetails CEngAgentImp::GetMinMomentCapacityDetails(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx) const
{
   return ComputeMinMomentCapacity(pierID,limitState,stage,poi,bPositiveMoment,vehicleIdx,llConfigIdx,permitLaneIdx);
}

//////////////////////////////////////////////////////////////////////
// IXBRCrackedSection
Float64 CEngAgentImp::GetIcrack(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,xbrTypes::LoadType loadType) const
{
   const CrackedSectionDetails& details = GetCrackedSectionDetails(pierID,stage,poi,bPositiveMoment,loadType);
   return details.Icr;
}

const CrackedSectionDetails& CEngAgentImp::GetCrackedSectionDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,xbrTypes::LoadType loadType) const
{
   std::map<IDType,CrackedSectionDetails>* pDetails = (bPositiveMoment ? m_pPositiveMomentCrackedSection[stage][loadType].get() : m_pNegativeMomentCrackedSection[stage][loadType].get());
   std::map<IDType,CrackedSectionDetails>::iterator found(pDetails->find(poi.GetID()));
   if ( found != pDetails->end() )
   {
      return found->second;
   }

   CrackedSectionDetails details = ComputeCrackedSectionProperties(pierID,stage,poi,bPositiveMoment,loadType);
   ATLASSERT(poi.GetID() != INVALID_ID);

   std::pair<std::map<IDType,CrackedSectionDetails>::iterator,bool> result = pDetails->insert(std::make_pair(poi.GetID(),details));
   ATLASSERT(result.second == true);

   std::map<IDType,CrackedSectionDetails>::iterator iter = result.first;
   return iter->second;
}

//////////////////////////////////////////////////////////////////////
// IXBRShearCapacity
Float64 CEngAgentImp::GetShearCapacity(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const
{
   const ShearCapacityDetails& details = GetShearCapacityDetails(pierID,stage,poi);
   return details.Vr;
}

const ShearCapacityDetails& CEngAgentImp::GetShearCapacityDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const
{
   std::map<IDType,ShearCapacityDetails>* pDetails = m_pShearCapacity[stage].get();
   std::map<IDType,ShearCapacityDetails>::iterator found(pDetails->find(poi.GetID()));
   if ( found != pDetails->end() )
   {
      return found->second;
   }

   ShearCapacityDetails details = ComputeShearCapacity(pierID,stage,poi);
   ATLASSERT(poi.GetID() != INVALID_ID);

   std::pair<std::map<IDType,ShearCapacityDetails>::iterator,bool> result = pDetails->insert(std::make_pair(poi.GetID(),details));
   ATLASSERT(result.second == true);

   std::map<IDType,ShearCapacityDetails>::iterator iter = result.first;
   return iter->second;
}

const AvOverSDetails& CEngAgentImp::GetAverageAvOverSDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const
{
   std::map<IDType,AvOverSDetails>* pDetails = m_pShearFailurePlane[stage].get();
   std::map<IDType,AvOverSDetails>::iterator found(pDetails->find(poi.GetID()));
   if ( found != pDetails->end() )
   {
      return found->second;
   }

   Float64 theta = M_PI/4; // 45 deg
   AvOverSDetails details = ComputeAverageAvOverS(pierID,stage,poi,theta);
   ATLASSERT(poi.GetID() != INVALID_ID);

   std::pair<std::map<IDType,AvOverSDetails>::iterator,bool> result = pDetails->insert(std::make_pair(poi.GetID(),details));
   ATLASSERT(result.second == true);

   std::map<IDType,AvOverSDetails>::iterator iter = result.first;
   return iter->second;
}

Float64 CEngAgentImp::GetDv(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const
{
   const DvDetails& details = GetDvDetails(pierID,stage,poi);
   return details.dv;
}

const DvDetails& CEngAgentImp::GetDvDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const
{
   std::map<IDType,DvDetails>* pDetails = m_pDvDetails[stage].get();
   std::map<IDType,DvDetails>::iterator found(pDetails->find(poi.GetID()));
   if ( found != pDetails->end() )
   {
      return found->second;
   }

   DvDetails details = ComputeDv(pierID,stage,poi);
   ATLASSERT(poi.GetID() != INVALID_ID);

   std::pair<std::map<IDType,DvDetails>::iterator,bool> result = pDetails->insert(std::make_pair(poi.GetID(),details));
   ATLASSERT(result.second == true);

   std::map<IDType,DvDetails>::iterator iter = result.first;
   return iter->second;
}

//////////////////////////////////////////////////////////////////////
// IXBRArtifactCapacity
const xbrRatingArtifact* CEngAgentImp::GetXBeamRatingArtifact(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const
{
   RatingArtifacts& ratingArtifacts = GetPrivateRatingArtifacts(pierID,ratingType,vehicleIdx);
   std::map<VehicleIndexType,xbrRatingArtifact>::iterator found = ratingArtifacts.find(vehicleIdx);
   if ( found == ratingArtifacts.end() )
   {
      CreateRatingArtifact(pierID,ratingType,vehicleIdx);
      found = ratingArtifacts.find(vehicleIdx);
   }

   if ( found == ratingArtifacts.end() )
   {
      return nullptr;
   }
   else
   {
      return &(found->second);
   }
}

//////////////////////////////////////////////////
HRESULT CEngAgentImp::OnProjectChanged()
{
   Invalidate();

   return S_OK;
}

//////////////////////////////////////////////////////////
// IBridgeDescriptionEventSink
HRESULT CEngAgentImp::OnBridgeChanged(CBridgeChangedHint* pHint)
{
   Invalidate();
   return S_OK;
}

HRESULT CEngAgentImp::OnGirderFamilyChanged()
{
   Invalidate();
   return S_OK;
}

HRESULT CEngAgentImp::OnGirderChanged(const CGirderKey& girderKey, Uint32 lHint)
{
   Invalidate();
   return S_OK;
}

HRESULT CEngAgentImp::OnLiveLoadChanged()
{
   Invalidate();
   return S_OK;
}

HRESULT CEngAgentImp::OnLiveLoadNameChanged(LPCTSTR strOldName, LPCTSTR strNewName)
{
   Invalidate();
   return S_OK;
}

HRESULT CEngAgentImp::OnConstructionLoadChanged()
{
   Invalidate();
   return S_OK;
}

//////////////////////////////////////////////////
MomentCapacityDetails CEngAgentImp::ComputeMomentCapacity(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const
{
   CComPtr<IRCBeam2> rcBeam;
   Float64 dt;
   BuildMomentCapacityModel(pierID,stage,poi,bPositiveMoment,&rcBeam,&dt);

   CComPtr<IRCSolutionEx> solution;
   HRESULT hr = m_MomentCapacitySolver->Solve(rcBeam,&solution); 
   ATLASSERT(SUCCEEDED(hr));

   Float64 Mn;
   solution->get_Mn(&Mn);
   ATLASSERT(0 <= Mn);
   if ( !bPositiveMoment )
   {
      Mn *= -1;
   }

   Float64 dc = 0;
   Float64 de = 0;

   GET_IFACE(IXBRProject, pProject);
   Float64 PhiC, PhiT;
   pProject->GetFlexureResistanceFactors(&PhiC, &PhiT);
   Float64 phi = PhiT;

   if ( !IsZero(Mn) )
   {
      Float64 c;
      solution->get_NeutralAxisDepth(&c);

      WBFL::Materials::Rebar::Type rebarType;
      WBFL::Materials::Rebar::Grade rebarGrade;
      pProject->GetRebarMaterial(pierID,&rebarType,&rebarGrade);

      Float64 et  = (dt - c)*0.003/c;
      Float64 ecl = WBFL::LRFD::Rebar::GetCompressionControlledStrainLimit(rebarGrade);
      Float64 etl = WBFL::LRFD::Rebar::GetTensionControlledStrainLimit(rebarGrade);
      phi = PhiC + (PhiT - PhiC)*(et - ecl)/(etl - ecl);
      phi = ::ForceIntoRange(PhiC,phi,PhiT);

      Float64 Cweb, Cflange, Yweb, Yflange;
      solution->get_Cweb(&Cweb);
      solution->get_Cflange(&Cflange);
      solution->get_Yweb(&Yweb);
      solution->get_Yflange(&Yflange);
      dc = (Cweb*Yweb + Cflange*Yflange)/(Cweb + Cflange);

      // compute de... distance from extreme compression fiber to
      // centroid of tensile force in tension reinforcement
      //
      // The solution has the location of the resultant tension force, but this
      // includes compression reinforcement. Find the resultant location for
      // only the reinforcement in tension
      Float64 sumT = 0;
      Float64 sumTd = 0;
      CComPtr<IDblArray> vfs;
      solution->get_fs(&vfs);
      IndexType nRebarLayers;
      rcBeam->get_RebarLayerCount(&nRebarLayers);
      for (IndexType idx = 0; idx < nRebarLayers; idx++)
      {
         Float64 ds, As, devFactor;
         rcBeam->GetRebarLayer(idx, &ds, &As, &devFactor);
         Float64 f;
         vfs->get_Item(idx, &f);
         if (0 < f)
         {
            Float64 T = devFactor*As*f;
            sumT += T;
            sumTd += T*ds;
         }
      }

      CComPtr<IDblArray> vfps;
      solution->get_fps(&vfps);
      IndexType nStrandLayers;
      rcBeam->get_StrandLayerCount(&nStrandLayers);
      for (IndexType idx = 0; idx < nStrandLayers; idx++)
      {
         Float64 ds, As, devFactor;
         rcBeam->GetStrandLayer(idx, &ds, &As, &devFactor);
         Float64 f;
         vfps->get_Item(idx, &f);
         if (0 < f)
         {
            Float64 T = devFactor*As*f;
            sumT += T;
            sumTd += T*ds;
         }
      }

      de = IsZero(sumT) ? 0 : sumTd / sumT;
   }

   MomentCapacityDetails capacityDetails;
   capacityDetails.rcBeam = rcBeam;
   hr = solution.QueryInterface(&capacityDetails.solution);
   ATLASSERT(SUCCEEDED(hr));
   capacityDetails.solution = solution;
   capacityDetails.dc = dc;
   capacityDetails.de = de;
   capacityDetails.dt = dt;
   capacityDetails.phi = phi;
   capacityDetails.Mn = Mn;
   capacityDetails.Mr = phi*Mn;

   return capacityDetails;
}

void CEngAgentImp::GetCrackingMomentFactors(PierIDType pierID,Float64* pG1,Float64* pG2,Float64* pG3) const
{
   // gamma factors from LRFD 5.7.3.3.2 (LRFD 6th Edition, 2012)
   if ( WBFL::LRFD::BDSManager::Edition::SixthEdition2012 <= WBFL::LRFD::BDSManager::GetEdition() )
   {
      *pG1 = 1.6; // all other concrete structures (not-segmental)
      *pG2 = 1.1; // bonded strand/tendon

      GET_IFACE(IXBRMaterial,pMaterial);
      Float64 E,fy,fu;
      pMaterial->GetRebarProperties(pierID,&E,&fy,&fu);
      *pG3 = fy/fu;
   }
   else
   {
      *pG1 = 1.0;
      *pG2 = 1.0;
      *pG3 = 1.0;
   }
}

CrackingMomentDetails CEngAgentImp::ComputeCrackingMoment(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const
{
   CrackingMomentDetails McrDetails;

   McrDetails.McrLimit; // Limiting cracking moment ... per 2nd Edition + 2003 interims (changed in 2005 interims)

   GET_IFACE(IXBRMaterial,pMaterial);
   Float64 fr = pMaterial->GetXBeamModulusOfRupture(pierID);

   Float64 fcpe = 0; // There is no prestressing in the cross beams

   // gamma factors from LRFD 5.7.3.3.2 (LRFD 6th Edition, 2012)
   Float64 g1, g2, g3;
   GetCrackingMomentFactors(pierID,&g1,&g2,&g3);

   GET_IFACE(IXBRAnalysisResults,pAnalysisResults);
   Float64 Mdnc = pAnalysisResults->GetMoment(pierID,xbrTypes::pftLowerXBeam,poi);

   GET_IFACE(IXBRSectionProperties,pSectProps);
   Float64 Sc  = (bPositiveMoment ? pSectProps->GetSbot(pierID,stage,poi)            : pSectProps->GetStop(pierID,stage,poi));
   Float64 Snc = (bPositiveMoment ? pSectProps->GetSbot(pierID,xbrTypes::Stage1,poi) : pSectProps->GetStop(pierID,xbrTypes::Stage1,poi));

   Float64 Mcr = g3*((g1*fr + g2*fcpe)*Sc - Mdnc*(Sc/Snc-1));

   if ( WBFL::LRFD::BDSManager::Edition::SecondEditionWith2003Interims <= WBFL::LRFD::BDSManager::GetEdition() )
   {
      Float64 McrLimit = Sc*fr;
      McrDetails.McrLimit = McrLimit;
   }

   McrDetails.Snc = Snc;
   McrDetails.Sc  = Sc;
   McrDetails.Mdnc = Mdnc;
   McrDetails.fr = fr;
   McrDetails.fcpe = fcpe;
   McrDetails.g1 = g1;
   McrDetails.g2 = g2;
   McrDetails.g3 = g3;

   McrDetails.Mcr = Mcr;

   return McrDetails;
}

MinMomentCapacityDetails CEngAgentImp::ComputeMinMomentCapacity(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const
{
   Float64 Mr;     // Nominal resistance (phi*Mn)
   Float64 Mcr;    // Cracking moment
   Float64 MrMin;  // Minimum nominal resistance - Min(MrMin1,MrMin2)
   Float64 MrMin1; // 1.2Mcr
   Float64 MrMin2; // 1.33Mu
   Float64 Mu;

   const MomentCapacityDetails& MnDetails  = GetMomentCapacityDetails(pierID,stage,poi,bPositiveMoment);
   const CrackingMomentDetails& McrDetails = GetCrackingMomentDetails(pierID,stage,poi,bPositiveMoment);

   bool bAfter2002  = ( WBFL::LRFD::BDSManager::Edition::SecondEditionWith2003Interims <= WBFL::LRFD::BDSManager::GetEdition() ? true : false );
   bool bBefore2012 = ( WBFL::LRFD::BDSManager::GetEdition() <  WBFL::LRFD::BDSManager::Edition::SixthEdition2012 ? true : false );
   if ( bAfter2002 && bBefore2012 )
   {
      Mcr = (bPositiveMoment ? Max(McrDetails.Mcr,McrDetails.McrLimit) : Min(McrDetails.Mcr,McrDetails.McrLimit));
   }
   else
   {
      Mcr = McrDetails.Mcr;
   }

   Mr = MnDetails.phi * MnDetails.Mn;

   GET_IFACE(IXBRAnalysisResults,pAnalysisResults);

   Float64 MuMin, MuMax;
   pAnalysisResults->GetMoment(pierID,limitState,poi,&MuMin,&MuMax);
   Mu = (bPositiveMoment ? MuMax : MuMin);

   if ( WBFL::LRFD::BDSManager::Edition::SixthEdition2012 <= WBFL::LRFD::BDSManager::GetEdition() )
   {
      MrMin1 = Mcr;
   }
   else
   {
      MrMin1 = 1.20*Mcr;
   }

   MrMin2 = 1.33*Mu;

   MrMin = (bPositiveMoment ? Min(MrMin1,MrMin2) : Max(MrMin1,MrMin2));

   MinMomentCapacityDetails MminDetails;
   MminDetails.Mr     = Mr;
   MminDetails.Mcr    = Mcr;
   MminDetails.MrMin  = MrMin;
   MminDetails.MrMin1 = MrMin1;
   MminDetails.MrMin2 = MrMin2;
   MminDetails.Mu     = Mu;

   return MminDetails;
}

MinMomentCapacityDetails CEngAgentImp::ComputeMinMomentCapacity(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx) const
{
   Float64 Mr;     // Nominal resistance (phi*Mn)
   Float64 Mcrack; // Cracking moment
   Float64 MrMin;  // Minimum nominal resistance - Min(MrMin1,MrMin2)
   Float64 MrMin1; // 1.Mcrack
   Float64 MrMin2; // 1.33Mu
   Float64 Mu;

   const MomentCapacityDetails& MnDetails  = GetMomentCapacityDetails(pierID,stage,poi,bPositiveMoment);
   const CrackingMomentDetails& McrDetails = GetCrackingMomentDetails(pierID,stage,poi,bPositiveMoment);

   bool bAfter2002  = ( WBFL::LRFD::BDSManager::Edition::SecondEditionWith2003Interims <= WBFL::LRFD::BDSManager::GetEdition() ? true : false );
   bool bBefore2012 = ( WBFL::LRFD::BDSManager::GetEdition() <  WBFL::LRFD::BDSManager::Edition::SixthEdition2012 ? true : false );
   if ( bAfter2002 && bBefore2012 )
   {
      Mcrack = (bPositiveMoment ? Max(McrDetails.Mcr,McrDetails.McrLimit) : Min(McrDetails.Mcr,McrDetails.McrLimit));
   }
   else
   {
      Mcrack = McrDetails.Mcr;
   }

   Mr = MnDetails.phi * MnDetails.Mn;

   GET_IFACE(IXBRAnalysisResults,pAnalysisResults);

   Float64 Mdc = pAnalysisResults->GetMoment(pierID,xbrTypes::lcDC,poi);
   Float64 Mdw = pAnalysisResults->GetMoment(pierID,xbrTypes::lcDW,poi);
   Float64 Mcr = pAnalysisResults->GetMoment(pierID,xbrTypes::lcCR,poi);
   Float64 Msh = pAnalysisResults->GetMoment(pierID,xbrTypes::lcSH,poi);
   Float64 Mre = pAnalysisResults->GetMoment(pierID,xbrTypes::lcRE,poi);
   Float64 Mps = pAnalysisResults->GetMoment(pierID,xbrTypes::lcPS,poi);

   pgsTypes::LoadRatingType ratingType = ::RatingTypeFromLimitState(limitState);

   Float64 Mpermit, Mlegal;
   pAnalysisResults->GetMoment(pierID,ratingType,vehicleIdx,llConfigIdx,permitLaneIdx,poi,&Mpermit,&Mlegal);

   GET_IFACE(IXBRProject,pProject);
   Float64 gDC = pProject->GetDCLoadFactor(limitState);
   Float64 gDW = pProject->GetDWLoadFactor(limitState);
   Float64 gCR = pProject->GetCRLoadFactor(limitState);
   Float64 gSH = pProject->GetSHLoadFactor(limitState);
   Float64 gRE = pProject->GetRELoadFactor(limitState);
   Float64 gPS = pProject->GetPSLoadFactor(limitState);
   Float64 gLL = pProject->GetLiveLoadFactor(pierID,limitState,vehicleIdx);

   Mu = gDC*Mdc + gDW*Mdw + gCR*Mcr + gSH*Msh + gRE*Mre + gPS*Mps + gLL*(Mpermit + Mlegal);

   if ( WBFL::LRFD::BDSManager::Edition::SixthEdition2012 <= WBFL::LRFD::BDSManager::GetEdition() )
   {
      MrMin1 = Mcrack;
   }
   else
   {
      MrMin1 = 1.20*Mcrack;
   }

   MrMin2 = 1.33*Mu;

   MrMin = (bPositiveMoment ? Min(MrMin1,MrMin2) : Max(MrMin1,MrMin2));

   MinMomentCapacityDetails MminDetails;
   MminDetails.Mr     = Mr;
   MminDetails.Mcr    = Mcrack;
   MminDetails.MrMin  = MrMin;
   MminDetails.MrMin1 = MrMin1;
   MminDetails.MrMin2 = MrMin2;
   MminDetails.Mu     = Mu;

   return MminDetails;
}

CrackedSectionDetails CEngAgentImp::ComputeCrackedSectionProperties(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,xbrTypes::LoadType loadType) const
{
   GET_IFACE(IXBRMaterial,pMaterial);

   Float64 Ec = pMaterial->GetXBeamEc(pierID);

   Float64 Es, fy, fu;
   pMaterial->GetRebarProperties(pierID,&Es,&fy,&fu);

   Float64 n = ::Round(Es/Ec); // modular ratio is rounded to nearest integer (LRFD 5.7.1)
   if ( loadType == xbrTypes::ltPermanent )
   {
      n *= 2; // use 2n for permanent loads (LRFD 5.7.1)
   }

   // The cracked section solver is going to use whatever Es and Ec we give to it.
   // We have to manipulate Ec so that the solver uses the correct modular ratio
   // (The solver computes y = Sum(EAy)/Sum(EA) = (Ec*Ac*Yc + Es*As*Yb)/(Ec*Ac + Es*As)
   // Below, Ec is used to transform the elastic properties to transformed section properties
   // and this is where the modular ratio comes into play. We will divide through by
   // Ec so in the concrete terms Ec/Ec = 1 and Ec/Es = n. We have to alter Ec here
   // so that when we divide through by Ec below we get the correct modular ratio.
   Ec = Es/n;


   // The cracked section solver only uses the modulus of elasticity part of the
   // the IStressStrain object. We have a special implementation for the exact purpose.
   CComObject<CCrackedSectionStressStrainModel>* pSSModel;
   CComObject<CCrackedSectionStressStrainModel>::CreateInstance(&pSSModel);
   pSSModel->SetXBeamModE(Ec);

   CComPtr<IStressStrain> ss;
   pSSModel->QueryInterface(&ss);

   // tell the solver to use our special stress/strain model for the beam and slab concrete
   CComQIPtr<INLSolver> solver(m_CrackedSectionSolver);
   solver->putref_SlabConcreteModel(ss);
   solver->putref_BeamConcreteModel(ss);


   // The "Capacity Problem" is the same as we used for moment capacity analysis.
   // Get it from the cache
   const MomentCapacityDetails& mcd = GetMomentCapacityDetails(pierID,stage,poi,bPositiveMoment);

   // solve the cracked section problem
   CComPtr<ICrackedSectionSolution> solution;
   HRESULT hr = m_CrackedSectionSolver->Solve(mcd.rcBeam,&solution); 
   ATLASSERT(SUCCEEDED(hr));

   CrackedSectionDetails csd;
   csd.n = n;
   csd.CrackedSectionSolution = solution;
  
   ///////////////////////////////////////////
   // Compute I-crack
   ///////////////////////////////////////////
   CComPtr<IElasticProperties> elastic_properties;
   solution->get_ElasticProperties(&elastic_properties);

   // transform properties into concrete matieral
   // Divides through by Ec
   CComPtr<IShapeProperties> shape_properties;
   elastic_properties->TransformProperties(Ec,&shape_properties);

   // Icrack for girder
   Float64 Icr;
   shape_properties->get_Ixx(&Icr);
   csd.Icr = Icr;

   // distance from top face to the cracked centroid
   Float64 c;
   shape_properties->get_Ytop(&c);
   csd.c = c;
   if ( bPositiveMoment )
   {
      csd.Ycr = c;
   }
   else
   {
      GET_IFACE(IXBRSectionProperties,pSectProps);
      Float64 h = pSectProps->GetDepth(pierID,stage,poi);
      csd.Ycr = h - c;
   }

   return csd;
}

ShearCapacityDetails CEngAgentImp::ComputeShearCapacity(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const
{
   // LRFD 5.8.3.4.1
   Float64 beta = 2.0;
   Float64 theta = M_PI/4; // 45 deg

   GET_IFACE(IXBRProject,pProject);
   GET_IFACE(IXBRMaterial,pMaterial);

   WBFL::Materials::Rebar::Type type;
   WBFL::Materials::Rebar::Grade grade;
   pProject->GetRebarMaterial(pierID,&type,&grade);
   Float64 fy = WBFL::Materials::Rebar::GetYieldStrength(type,grade);

   if (WBFL::LRFD::BDSManager::GetEdition() < WBFL::LRFD::BDSManager::Edition::ThirdEditionWith2005Interims)
   {
      // LRFD 3rd Edition 2004 and earlier, fy is limited to 60 ksi
      // See pre2017 Section 5.8.2.8
      fy = min(fy, WBFL::Units::ConvertToSysUnits(60.0, WBFL::Units::Measure::KSI));
   }
   else
   {
      // LRFD 3rd Edition 2004 + 2005 Interims, for fy > 60 ksi materials, fy is limited to
      // fy = Es*0.0035 but not to exceed 75 ksi
      if (WBFL::Units::ConvertToSysUnits(60.0, WBFL::Units::Measure::KSI) < fy)
      {
         auto Es = WBFL::Materials::Rebar::GetE(type, grade);
         auto fy_ = min(0.0035 * Es, WBFL::Units::ConvertToSysUnits(75.0, WBFL::Units::Measure::KSI));
         fy = min(fy, fy_);
      }
   }

   // added with LRFD 7th Edition 2014
   // fy <= 100 ksi
   // See LRFD 5.7.2.5 (pre2017: 5.8.2.5)
   // Also see UHPC GS 1.7.3.4.1
   if (WBFL::LRFD::BDSManager::Edition::SeventhEdition2014 <= WBFL::LRFD::BDSManager::GetEdition())
   {
      fy = min(fy, WBFL::Units::ConvertToSysUnits(100.0, WBFL::Units::Measure::KSI));
   }

   Float64 dv1 = GetDv(pierID,xbrTypes::Stage1,poi);
   Float64 dv2 = GetDv(pierID,stage,poi);
   
   const AvOverSDetails& avs1 = GetAverageAvOverSDetails(pierID,xbrTypes::Stage1,poi);
   const AvOverSDetails& avs2 = GetAverageAvOverSDetails(pierID,stage,poi);
   Float64 Av_over_S1 = avs1.AvgAvOverS;
   Float64 Av_over_S2 = (stage == xbrTypes::Stage1 ? 0 : avs2.AvgAvOverS);

   // if non-integral pier, dv2 is zero so dv1 will be the max, 
   // otherwise dv2 will be the max
   Float64 dv = Max(dv1,dv2);

   // Also need to account for x-beam type (integral, continuous, expansion... only integral has upper diaphragm)
   Float64 fc = pProject->GetConcrete(pierID).Fc;
   Float64 bv = pProject->GetXBeamWidth(pierID);
   Float64 fc_us = WBFL::Units::ConvertFromSysUnits(fc,WBFL::Units::Measure::KSI);
   Float64 lambda = pMaterial->GetXBeamLambda(pierID);
   Float64 Vc_us = 0.0316*lambda*beta*sqrt(fc_us)*bv*dv;
   Float64 Vc = WBFL::Units::ConvertToSysUnits(Vc_us,WBFL::Units::Measure::KSI);
   Float64 Vs1 = Av_over_S1*fy*dv1/(tan(theta)); // lower x-beam reinforcement capacity
   Float64 Vs2 = Av_over_S2*fy*dv2/(tan(theta)); // full x-beam reinforcement capacity
   Float64 Vs = Vs1 + Vs2; // total capacity due to reinforcement

   Float64 Vn1 = Vc + Vs;
   Float64 Vn2 = 0.25*fc*bv*dv;

   Float64 Vn = Min(Vn1,Vn2);
   Float64 phi = pProject->GetShearResistanceFactor();
   Float64 Vr = phi*Vn;

   ShearCapacityDetails details;
   details.beta = beta;
   details.theta = theta;
   details.bv = bv;
   details.dv1 = dv1;
   details.dv2 = dv2;
   details.dv = dv;
   details.Vc = Vc;
   details.Av_over_S1 = Av_over_S1;
   details.Av_over_S2 = Av_over_S2;
   details.Vs = Vs;
   details.Vn1 = Vn1;
   details.Vn2 = Vn2;
   details.Vn = Vn;
   details.phi = phi;
   details.Vr = Vr;

   return details;
}

void CEngAgentImp::BuildMomentCapacityModel(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,IRCBeam2** ppModel,Float64* pdt) const
{
   CComPtr<IRCBeam2> rcBeam;
   HRESULT hr = rcBeam.CoCreateInstance(CLSID_RCBeam2);
   ATLASSERT(SUCCEEDED(hr));

   GET_IFACE(IXBRSectionProperties,pSectProps);
   GET_IFACE(IXBRProject,pProject);

   // only using the cross beam height for capacity
   // the top of the cross beam is at the bottom of the slab
   Float64 h = pSectProps->GetDepth(pierID,stage,poi);
   rcBeam->put_h(h);
   rcBeam->put_hf(0);

   Float64 w = pProject->GetXBeamWidth(pierID);
   rcBeam->put_bw(w);
   rcBeam->put_b(w); // model the top flange width equal to the web (maybe change this in the future for T-section capacity)

   xbrTypes::PierType connectionType = pProject->GetPierType(pierID);
   Float64 d;
   pProject->GetDiaphragmDimensions(pierID,&d,&w); // dimensions of upper diaphragm/xbeam

   GET_IFACE(IXBRMaterial,pMaterial);
   Float64 fc = pMaterial->GetXBeamFc(pierID);
   rcBeam->put_FcBeam(fc);
   rcBeam->put_FcSlab(fc);

   Float64 Es, fy, fu;
   pMaterial->GetRebarProperties(pierID,&Es,&fy,&fu);
   rcBeam->put_fy(fy);
   rcBeam->put_Es(Es);

   GET_IFACE(IXBRRebar,pRebar);
   CComPtr<IRebarSection> rebarSection;
   pRebar->GetRebarSection(pierID,stage,poi,&rebarSection);

   CComPtr<IEnumRebarSectionItem> enumRebar;
   rebarSection->get__EnumRebarSectionItem(&enumRebar);

   // the capacity analysis goes faster if we lump all the rebar
   // at a single elevation into one bar.
   // this map keeps track of Ybar and As*devFactor
   std::map<Float64,Float64,Float64_less> rebarMap;

   // This loop accumulates As for each unique Ybar... it doesn't
   // put the bar in the capacity model
   CComPtr<IRebarSectionItem> rebarSectionItem;
   while ( enumRebar->Next(1,&rebarSectionItem,nullptr) != S_FALSE )
   {
      CComPtr<IPoint2d> pntRebar;
      rebarSectionItem->get_Location(&pntRebar);

      Float64 Ybar = pRebar->GetRebarDepth(pierID,poi,stage,pntRebar); // depth from top of cross beam to rebar

      if ( Ybar < 0 )
      {
         // rebar is not in the cross section (not applicable in this stage)
         rebarSectionItem.Release();
         continue;
      }

      // if continous or expansion pier, the upper cross beam doesn't contribute
      // to capacity. but the rebar are measured from the top down of the entire
      // section. deduct the height of the upper cross beam to get the depth of
      // the rebar relative to the top of the lower cross beam
      if ( connectionType != xbrTypes::pctIntegral && stage == xbrTypes::Stage2 )
      {
         Ybar -= d;
      }

      ATLASSERT(0 < Ybar && Ybar < h); // if this fires, the rebar is not in the cross section

      if ( !bPositiveMoment )
      {
         // for negative moment we have to build the section "upside down"
         // since the solver only solves for bending in one direction (compression top - tension bottom)
         // for neg moment, compression and tension faces are reversed so we put the top rebar
         // at the bottom and the bottom rebar at the top for negative moment capacity.
         Ybar = h - Ybar; // Ybar is measured from the bottom up for negative moment
      }

      CComPtr<IRebar> rebar;
      rebarSectionItem->get_Rebar(&rebar);

      Float64 As;
      rebar->get_NominalArea(&As);

      Float64 devFactor = pRebar->GetDevLengthFactor(pierID,poi,rebarSectionItem);
      ATLASSERT(::InRange(0.0,devFactor,1.0));

      As *= devFactor;

      // do we already have bars at this elevation ?
      std::map<Float64,Float64,Float64_less>::iterator found = rebarMap.find(Ybar);
      if ( found == rebarMap.end() )
      {
         // no... add a new record
         rebarMap.insert(std::make_pair(Ybar,As));
      }
      else
      {
         // yes... increment the area of bar
         found->second += As;
      }

      rebarSectionItem.Release();
   }

   // add the "lumped" bars to the capacity problem model
   Float64 dt = 0; // location of the extreme tension steel, measured from the top of the deck
   std::map<Float64,Float64,Float64_less>::iterator iter(rebarMap.begin());
   std::map<Float64,Float64,Float64_less>::iterator end(rebarMap.end());
   for ( ; iter != end; iter++ )
   {
      Float64 Ybar = iter->first;
      Float64 As   = iter->second;

      rcBeam->AddRebarLayer(Ybar,As,1.0);

      dt = Max(dt,Ybar);
   }

   rcBeam.CopyTo(ppModel);
   *pdt = dt;
}

DvDetails CEngAgentImp::ComputeDv(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const
{
   GET_IFACE(IXBRProject,pProject);
   if ( pProject->GetPierType(pierID) != xbrTypes::pctIntegral && stage == xbrTypes::Stage2 )
   {
      // there isn't stage 2 for non-integral cross beams
      DvDetails details;
      details.h = 0;
      details.de[0] = 0;
      details.MomentArm[0] = 0;
      details.de[1] = 0;
      details.MomentArm[1] = 0;
      details.dv = 0;
   }

   GET_IFACE(IXBRSectionProperties,pSectProps);
   Float64 h = pSectProps->GetDepth(pierID,stage,poi);
   const MomentCapacityDetails& posCapacityDetails = GetMomentCapacityDetails(pierID,stage,poi,true);
   const MomentCapacityDetails& negCapacityDetails = GetMomentCapacityDetails(pierID,stage,poi,false);
   Float64 posMomentArm = posCapacityDetails.de - posCapacityDetails.dc;
   Float64 posDv = Max(posMomentArm,0.9*posCapacityDetails.de,0.72*h);
   Float64 negMomentArm = negCapacityDetails.de - negCapacityDetails.dc;
   Float64 negDv = Max(negMomentArm,0.9*negCapacityDetails.de,0.72*h);
   Float64 dv = Min(posDv,negDv);

   DvDetails details;
   details.h = h;
   details.de[0] = posCapacityDetails.de;
   details.de[1] = negCapacityDetails.de;
   details.MomentArm[0] = posMomentArm;
   details.MomentArm[1] = negMomentArm;
   details.MomentDv[0] = posDv;
   details.MomentDv[1] = negDv;
   details.dv = dv;
   return details;
}

AvOverSDetails CEngAgentImp::ComputeAverageAvOverS(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,Float64 theta) const
{
   AvOverSDetails details;

   GET_IFACE(IXBRProject,pProject);
   if ( pProject->GetPierType(pierID) != xbrTypes::pctIntegral && stage == xbrTypes::Stage2 )
   {
      // there isn't stage 2 for non-integral cross beams
      details.ShearFailurePlaneLength = 0;
      details.AvgAvOverS = 0;
      return details;
   }

   if ( WBFL::LRFD::MBEManager::GetEdition() < WBFL::LRFD::MBEManager::Edition::SecondEditionWith2015Interims )
   {
      // before 2015, shear capacity was based strictly on stirrups at a section
      GET_IFACE(IXBRStirrups,pStirrups);
      ZoneIndexType zoneIdx = pStirrups->FindStirrupZone(pierID,stage,poi);
      if (zoneIdx == INVALID_INDEX)
      {
         // POI is not in a zone (this can happen if there aren't any stirrups defined)
         details.AvgAvOverS = 0.0;
      }
      else
      {
         details.AvgAvOverS = pStirrups->GetStirrupZoneReinforcement(pierID, stage, zoneIdx);
      }
      return details;
   }
   else
   {
      // This procedure is based on MBE 6A.5.8 (2nd Edition, 2015 interims).
      GET_IFACE(IXBRPier,pPier);
      Float64 L = pPier->GetXBeamLength(xbrTypes::xblBottomXBeam, pierID);

      // Get start/end of the shear failure plane at the poi
      Float64 dv = GetDv(pierID,stage,poi);
      Float64 sfpStart = Max(poi.GetDistFromStart() - dv/(2*tan(theta)),0.0);
      Float64 sfpEnd   = Min(poi.GetDistFromStart() + dv/(2*tan(theta)),L);

      Float64 Avg_Av_over_S = 0;
      GET_IFACE(IXBRStirrups,pStirrups);
      ZoneIndexType nZones = pStirrups->GetStirrupZoneCount(pierID,stage);
      for ( ZoneIndexType zoneIdx = 0; zoneIdx < nZones; zoneIdx++ )
      {
         Float64 szStart, szEnd;
         pStirrups->GetStirrupZoneBoundary(pierID,stage,zoneIdx,&szStart,&szEnd);

         if ( szEnd <= sfpStart )
         {
            // The shear failure plane starts after this zone ends
            // This zone does not contribute stirrups to average Av/S
            // Continue with the next zone
            continue;
         }

         if ( sfpEnd <= szStart )
         {
            // The shear failure plane ends before this zone starts
            // This zone, and all zones that come after it, does not contribute stirrups to average Av/S
            // Break here so we don't have to process the remaining zones
            break;
         }

         Float64 start = Max(szStart,sfpStart,0.0);
         Float64 end   = Min(szEnd,  sfpEnd,  L);
         Float64 Av_over_S = pStirrups->GetStirrupZoneReinforcement(pierID,stage,zoneIdx);

         Float64 length = end - start;

         AvOverSZone zone;
         zone.Start = start;
         zone.End = end;
         zone.Length = length;
         zone.AvOverS = Av_over_S;
         details.Zones.push_back(zone);

         Avg_Av_over_S += Av_over_S*(length);
      }

      // average Av/S over the length of the shear failure plane
      // (if we are near the ends, it can be less than dv)
      Float64 Lsfp = sfpEnd - sfpStart;
      ATLASSERT( !IsZero(Lsfp) );
      ATLASSERT(::IsLE(Lsfp,dv/tan(theta)));
      Avg_Av_over_S /= Lsfp;

      details.ShearFailurePlaneLength = Lsfp;
      details.AvgAvOverS = Avg_Av_over_S;
   }
   return details;
}

CEngAgentImp::RatingArtifacts& CEngAgentImp::GetPrivateRatingArtifacts(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const
{
   std::map<PierIDType,RatingArtifacts>::iterator found = m_pRatingArtifacts[ratingType]->find(pierID);
   if ( found == m_pRatingArtifacts[ratingType]->end() )
   {
      RatingArtifacts artifacts;
      std::pair<std::map<PierIDType,RatingArtifacts>::iterator,bool> result = m_pRatingArtifacts[ratingType]->insert(std::make_pair(pierID,artifacts));
      ATLASSERT(result.second == true);
      found = result.first;
   }
   return found->second;
}

void CEngAgentImp::CreateRatingArtifact(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const
{
   GET_IFACE(IEAFProgress, pProgress);
   WBFL::EAF::AutoProgress ap(pProgress,0);
   pProgress->UpdateMessage(_T("Load rating cross beam"));

   RatingArtifacts& ratingArtifacts = GetPrivateRatingArtifacts(pierID,ratingType,vehicleIdx);
   xbrLoadRater loadRater(m_pBroker);
   xbrRatingArtifact artifact = loadRater.RateXBeam(pierID,ratingType,vehicleIdx);
   std::pair<std::map<VehicleIndexType,xbrRatingArtifact>::iterator,bool> result = ratingArtifacts.insert(std::make_pair(vehicleIdx,artifact));
   ATLASSERT(result.second == true);
}

void CEngAgentImp::Invalidate(bool bCreateNewDataStructures)
{
   DataStructures* pDataStructures = new DataStructures;

   CComQIPtr<ILRFDSolver2> solver(m_MomentCapacitySolver);
   solver->putref_RebarModel(nullptr);
   solver->putref_StrandModel(nullptr);

   CComQIPtr<INLSolver> nlsolver(m_CrackedSectionSolver);
   nlsolver->putref_RebarModel(nullptr);
   nlsolver->putref_StrandModel(nullptr);

   for ( int i = 0; i < 2; i++ )
   {
      pDataStructures->m_pPositiveMomentCapacity[i] = m_pPositiveMomentCapacity[i].release();
      pDataStructures->m_pNegativeMomentCapacity[i] = m_pNegativeMomentCapacity[i].release();

      pDataStructures->m_pPositiveCrackingMoment[i] = m_pPositiveCrackingMoment[i].release();
      pDataStructures->m_pNegativeCrackingMoment[i] = m_pNegativeCrackingMoment[i].release();

      pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Inventory)] = m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Inventory)].release();
      pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Inventory)] = m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Inventory)].release();

      pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Operating)] = m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Operating)].release();
      pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Operating)] = m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Operating)].release();

      pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalRoutine)] = m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalRoutine)].release();
      pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalRoutine)] = m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalRoutine)].release();

      pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalSpecial)] = m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalSpecial)].release();
      pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalSpecial)] = m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalSpecial)].release();

      pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalEmergency)] = m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalEmergency)].release();
      pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalEmergency)] = m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalEmergency)].release();

      pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitRoutine)] = m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitRoutine)].release();
      pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitRoutine)] = m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitRoutine)].release();

      pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitSpecial)] = m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitSpecial)].release();
      pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitSpecial)] = m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitSpecial)].release();

      pDataStructures->m_pPositiveMomentCrackedSection[i][xbrTypes::ltPermanent] = m_pPositiveMomentCrackedSection[i][xbrTypes::ltPermanent].release();
      pDataStructures->m_pNegativeMomentCrackedSection[i][xbrTypes::ltPermanent] = m_pNegativeMomentCrackedSection[i][xbrTypes::ltPermanent].release();

      pDataStructures->m_pPositiveMomentCrackedSection[i][xbrTypes::ltTransient] = m_pPositiveMomentCrackedSection[i][xbrTypes::ltTransient].release();
      pDataStructures->m_pNegativeMomentCrackedSection[i][xbrTypes::ltTransient] = m_pNegativeMomentCrackedSection[i][xbrTypes::ltTransient].release();

      pDataStructures->m_pShearCapacity[i] = m_pShearCapacity[i].release();
      pDataStructures->m_pShearFailurePlane[i] = m_pShearFailurePlane[i].release();

      pDataStructures->m_pDvDetails[i] = m_pDvDetails[i].release();
   }

   int n = (int)pgsTypes::lrLoadRatingTypeCount;
   for ( int i = 0; i < n; i++ )
   {
      pDataStructures->m_pRatingArtifacts[i] = m_pRatingArtifacts[i].release();
   }


#if defined _USE_MULTITHREADING
   m_ThreadManager.CreateThread(CEngAgentImp::DeleteDataStructures,(LPVOID)(pDataStructures));
#else
   CEngAgentImp::DeleteDataStructures((LPVOID)(pDataStructures));
#endif


   if ( bCreateNewDataStructures )
   {
      CreateDataStructures();
   }
}

void CEngAgentImp::CreateDataStructures()
{
   for ( int i = 0; i < 2; i++ )
   {
      m_pPositiveMomentCapacity[i] = std::make_unique<std::map<IDType,MomentCapacityDetails>>();
      m_pNegativeMomentCapacity[i] = std::make_unique<std::map<IDType,MomentCapacityDetails>>();

      m_pPositiveCrackingMoment[i] = std::make_unique<std::map<IDType,CrackingMomentDetails>>();
      m_pNegativeCrackingMoment[i] = std::make_unique<std::map<IDType,CrackingMomentDetails>>();

      m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Inventory)] = std::make_unique<std::map<IDType,MinMomentCapacityDetails>>();
      m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Inventory)] = std::make_unique<std::map<IDType,MinMomentCapacityDetails>>();

      m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Operating)] = std::make_unique<std::map<IDType,MinMomentCapacityDetails>>();
      m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Operating)] = std::make_unique<std::map<IDType,MinMomentCapacityDetails>>();

      m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalRoutine)] = std::make_unique<std::map<IDType,MinMomentCapacityDetails>>();
      m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalRoutine)] = std::make_unique<std::map<IDType,MinMomentCapacityDetails>>();

      m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalSpecial)] = std::make_unique<std::map<IDType, MinMomentCapacityDetails>>();
      m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalSpecial)] = std::make_unique<std::map<IDType, MinMomentCapacityDetails>>();

      m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalEmergency)] = std::make_unique<std::map<IDType, MinMomentCapacityDetails>>();
      m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalEmergency)] = std::make_unique<std::map<IDType, MinMomentCapacityDetails>>();

      m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitRoutine)] = std::make_unique<std::map<IDType,MinMomentCapacityDetails>>();
      m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitRoutine)] = std::make_unique<std::map<IDType,MinMomentCapacityDetails>>();

      m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitSpecial)] = std::make_unique<std::map<IDType,MinMomentCapacityDetails>>();
      m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitSpecial)] = std::make_unique<std::map<IDType,MinMomentCapacityDetails>>();

      m_pPositiveMomentCrackedSection[i][xbrTypes::ltPermanent] = std::make_unique<std::map<IDType,CrackedSectionDetails>>();
      m_pNegativeMomentCrackedSection[i][xbrTypes::ltPermanent] = std::make_unique<std::map<IDType,CrackedSectionDetails>>();

      m_pPositiveMomentCrackedSection[i][xbrTypes::ltTransient] = std::make_unique<std::map<IDType,CrackedSectionDetails>>();
      m_pNegativeMomentCrackedSection[i][xbrTypes::ltTransient] = std::make_unique<std::map<IDType,CrackedSectionDetails>>();

      m_pShearCapacity[i] = std::make_unique<std::map<IDType,ShearCapacityDetails>>();

      m_pShearFailurePlane[i] = std::make_unique<std::map<IDType,AvOverSDetails>>();

      m_pDvDetails[i] = std::make_unique<std::map<IDType,DvDetails>>();
   }

   int n = (int)pgsTypes::lrLoadRatingTypeCount;
   for ( int i = 0; i < n; i++ )
   {
      m_pRatingArtifacts[i] = std::make_unique<std::map<PierIDType,RatingArtifacts>>();
   }
}

UINT CEngAgentImp::DeleteDataStructures(LPVOID pParam)
{
   DataStructures* pDataStructures = (DataStructures*)(pParam);

   for ( int i = 0; i < 2; i++ )
   {
      delete pDataStructures->m_pPositiveMomentCapacity[i];
      delete pDataStructures->m_pNegativeMomentCapacity[i];

      delete pDataStructures->m_pPositiveCrackingMoment[i];
      delete pDataStructures->m_pNegativeCrackingMoment[i];

      delete pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Inventory)];
      delete pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Inventory)];

      delete pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Operating)];
      delete pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_Operating)];

      delete pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalRoutine)];
      delete pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalRoutine)];

      delete pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalSpecial)];
      delete pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalSpecial)];

      delete pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalEmergency)];
      delete pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthI_LegalEmergency)];

      delete pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitRoutine)];
      delete pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitRoutine)];

      delete pDataStructures->m_pPositiveMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitSpecial)];
      delete pDataStructures->m_pNegativeMinMomentCapacity[i][GET_INDEX(pgsTypes::StrengthII_PermitSpecial)];

      delete pDataStructures->m_pPositiveMomentCrackedSection[i][xbrTypes::ltPermanent];
      delete pDataStructures->m_pNegativeMomentCrackedSection[i][xbrTypes::ltPermanent];

      delete pDataStructures->m_pPositiveMomentCrackedSection[i][xbrTypes::ltTransient];
      delete pDataStructures->m_pNegativeMomentCrackedSection[i][xbrTypes::ltTransient];

      delete pDataStructures->m_pShearCapacity[i];

      delete pDataStructures->m_pShearFailurePlane[i];

      delete pDataStructures->m_pDvDetails[i];
   }

   int n = (int)pgsTypes::lrLoadRatingTypeCount;
   for ( int i = 0; i < n; i++ )
   {
      delete pDataStructures->m_pRatingArtifacts[i];
   }

   delete pDataStructures;

   return 0;
}
