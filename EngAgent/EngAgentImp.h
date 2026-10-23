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

// EngAgentImp.h : Declaration of the CEngAgentImp

// This agent is responsible for creating structural analysis models
// and providing analysis results

#pragma once

#include <EAF\Agent.h>
#include <EngAgent.h>
#include "EngAgentCLSID.h"

#include <WBFLRCCapacity.h>

#include <IFace\LoadRating.h>
#include <IFace\Project.h>

#include <..\..\PGSuper\Include\IFace\Project.h>

#if defined _USE_MULTITHREADING
#include <PgsExt\ThreadManager.h>
#endif

/////////////////////////////////////////////////////////////////////////////
// CEngAgentImp
class CEngAgentImp : public WBFL::EAF::Agent,
   public IXBRMomentCapacity,
   public IXBRShearCapacity,
   public IXBRCrackedSection,
   public IXBRArtifact,
   public IXBRProjectEventSink,
   public IBridgeDescriptionEventSink
{  
public:
	CEngAgentImp() = default; 

// Agent
public:
   std::_tstring GetName() const override { return _T("XBeam Rate Eng Agent"); }
   bool RegisterInterfaces() override;
   bool Init() override;
   bool Reset() override;
   bool ShutDown() override;
   CLSID GetCLSID() const override;

// IXBRMomentCapacity
public:
   Float64 GetMomentCapacity(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const override;
   const MomentCapacityDetails& GetMomentCapacityDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const override;

   Float64 GetCrackingMoment(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const override;
   const CrackingMomentDetails& GetCrackingMomentDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const override;

   Float64 GetMinMomentCapacity(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const override;
   const MinMomentCapacityDetails& GetMinMomentCapacityDetails(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const override;
   MinMomentCapacityDetails GetMinMomentCapacityDetails(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx) const override;

// IXBRCrackedSection
public:
   Float64 GetIcrack(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,xbrTypes::LoadType loadType) const override;
   const CrackedSectionDetails& GetCrackedSectionDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,xbrTypes::LoadType loadType) const override;

// IXBRShearCapacity
public:
   Float64 GetShearCapacity(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;
   const ShearCapacityDetails& GetShearCapacityDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;
   const AvOverSDetails& GetAverageAvOverSDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;
   Float64 GetDv(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;
   const DvDetails& GetDvDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;

// IXBRArtifact
public:
   const xbrRatingArtifact* GetXBeamRatingArtifact(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const override;

// IXBRProjectEventSink
public:
   HRESULT OnProjectChanged() override;

   // IBridgeDescriptionEventSink
public:
   HRESULT OnBridgeChanged(CBridgeChangedHint* pHint) override;
   HRESULT OnGirderFamilyChanged() override;
   HRESULT OnGirderChanged(const CGirderKey& girderKey, Uint32 lHint) override;
   HRESULT OnLiveLoadChanged() override;
   HRESULT OnLiveLoadNameChanged(LPCTSTR strOldName, LPCTSTR strNewName) override;
   HRESULT OnConstructionLoadChanged() override;

private:
   EAF_DECLARE_AGENT_DATA;

   IDType m_dwProjectCookie;
   IDType m_dwBridgeDescCookie;


   // PierID is not used to store results because the POI ID is sufficient
   // POI IDs are not duplicated between piers
   std::unique_ptr<std::map<IDType,MomentCapacityDetails>> m_pPositiveMomentCapacity[2]; // key = POI ID, array index = xbrTypes::Stage
   std::unique_ptr<std::map<IDType,MomentCapacityDetails>> m_pNegativeMomentCapacity[2];

   std::unique_ptr<std::map<IDType,CrackingMomentDetails>> m_pPositiveCrackingMoment[2]; // key = POI ID, array index = xbrTypes::Stage
   std::unique_ptr<std::map<IDType,CrackingMomentDetails>> m_pNegativeCrackingMoment[2];

   std::unique_ptr<std::map<IDType,MinMomentCapacityDetails>> m_pPositiveMinMomentCapacity[2][pgsTypes::lrLoadRatingTypeCount]; // key = POI ID, array index = xbrTypes::Stage, second array index is based on limit state type.use GET_INDEX(limitState) macro
   std::unique_ptr<std::map<IDType,MinMomentCapacityDetails>> m_pNegativeMinMomentCapacity[2][pgsTypes::lrLoadRatingTypeCount];

   std::unique_ptr<std::map<IDType,CrackedSectionDetails>> m_pPositiveMomentCrackedSection[2][2]; // key = POI ID, array index = [xbrTypes::Stage][xbrTypes::LoadType]
   std::unique_ptr<std::map<IDType,CrackedSectionDetails>> m_pNegativeMomentCrackedSection[2][2];

   std::unique_ptr<std::map<IDType,ShearCapacityDetails>> m_pShearCapacity[2];
   std::unique_ptr<std::map<IDType,AvOverSDetails>> m_pShearFailurePlane[2];

   std::unique_ptr<std::map<IDType,DvDetails>> m_pDvDetails[2];

   MomentCapacityDetails ComputeMomentCapacity(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const;
   CrackingMomentDetails ComputeCrackingMoment(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const;
   MinMomentCapacityDetails ComputeMinMomentCapacity(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const;
   MinMomentCapacityDetails ComputeMinMomentCapacity(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx) const;
   void GetCrackingMomentFactors(PierIDType pierID,Float64* pG1,Float64* pG2,Float64* pG3) const;
   CrackedSectionDetails ComputeCrackedSectionProperties(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,xbrTypes::LoadType loadType) const;
   void BuildMomentCapacityModel(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,IRCBeam2** ppModel,Float64* pdt) const;

   DvDetails ComputeDv(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const;
   ShearCapacityDetails ComputeShearCapacity(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const;
   AvOverSDetails ComputeAverageAvOverS(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,Float64 theta) const;


   // rating artifacts for vehicleIdx == INVALID_INDEX are the governing artifacts for a load rating type
   typedef std::map<VehicleIndexType,xbrRatingArtifact> RatingArtifacts;
   std::unique_ptr<std::map<PierIDType,RatingArtifacts>> m_pRatingArtifacts[pgsTypes::lrLoadRatingTypeCount]; // array index is pgsTypes::LoadRatingType
   RatingArtifacts& GetPrivateRatingArtifacts(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const;
   void CreateRatingArtifact(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const;

   CComPtr<IRCSolver2> m_MomentCapacitySolver;
   CComPtr<IRCCrackedSectionSolver2> m_CrackedSectionSolver;


#if defined _USE_MULTITHREADING
   CThreadManager m_ThreadManager;
#endif

   void Invalidate(bool bCreateNewDataStructures = true);
   struct DataStructures
   {
      std::map<IDType,MomentCapacityDetails>* m_pPositiveMomentCapacity[2];
      std::map<IDType,MomentCapacityDetails>* m_pNegativeMomentCapacity[2];

      std::map<IDType,CrackingMomentDetails>* m_pPositiveCrackingMoment[2];
      std::map<IDType,CrackingMomentDetails>* m_pNegativeCrackingMoment[2];

      std::map<IDType,MinMomentCapacityDetails>* m_pPositiveMinMomentCapacity[2][pgsTypes::lrLoadRatingTypeCount];
      std::map<IDType,MinMomentCapacityDetails>* m_pNegativeMinMomentCapacity[2][pgsTypes::lrLoadRatingTypeCount];

      std::map<IDType,CrackedSectionDetails>* m_pPositiveMomentCrackedSection[2][2];
      std::map<IDType,CrackedSectionDetails>* m_pNegativeMomentCrackedSection[2][2];

      std::map<PierIDType,RatingArtifacts>* m_pRatingArtifacts[pgsTypes::lrLoadRatingTypeCount];

      std::map<IDType,ShearCapacityDetails>* m_pShearCapacity[2];
      std::map<IDType,AvOverSDetails>* m_pShearFailurePlane[2];

      std::map<IDType,DvDetails>* m_pDvDetails[2];
   };
   static UINT DeleteDataStructures(LPVOID pParam);
   void CreateDataStructures();
};

