///////////////////////////////////////////////////////////////////////
// XBeamRate - Cross Beam Load Rating
// Copyright � 1999-2015  Washington State Department of Transportation
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

#include "resource.h"       // main symbols
#include <EngAgent.h>
#include "EngAgentCLSID.h"

#include <WBFLRCCapacity.h>

#include <EAF\EAFInterfaceCache.h>
#include <IFace\Project.h>

/////////////////////////////////////////////////////////////////////////////
// CEngAgentImp
class ATL_NO_VTABLE CEngAgentImp : 
	public CComObjectRootEx<CComSingleThreadModel>,
   //public CComRefCountTracer<CEngAgentImp,CComObjectRootEx<CComSingleThreadModel> >,
	public CComCoClass<CEngAgentImp, &CLSID_EngAgent>,
   public IAgentEx,
   public IXBRMomentCapacity,
   public IXBRShearCapacity,
   public IXBRArtifact,
   public IXBRProjectEventSink
{  
public:
	CEngAgentImp(); 
   virtual ~CEngAgentImp();

   DECLARE_PROTECT_FINAL_CONSTRUCT();

   HRESULT FinalConstruct();
   void FinalRelease();

DECLARE_REGISTRY_RESOURCEID(IDR_ENGAGENT)

BEGIN_COM_MAP(CEngAgentImp)
	COM_INTERFACE_ENTRY(IAgent)
   COM_INTERFACE_ENTRY(IAgentEx)
	COM_INTERFACE_ENTRY(IXBRMomentCapacity)
	COM_INTERFACE_ENTRY(IXBRShearCapacity)
   COM_INTERFACE_ENTRY(IXBRArtifact)
   COM_INTERFACE_ENTRY(IXBRProjectEventSink)
	//COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

//BEGIN_CONNECTION_POINT_MAP(CEngAgentImp)
//   CONNECTION_POINT_ENTRY( IID_IProjectEventSink )
//END_CONNECTION_POINT_MAP()

// IAgentEx
public:
	STDMETHOD(SetBroker)(/*[in]*/ IBroker* pBroker);
   STDMETHOD(RegInterfaces)();
	STDMETHOD(Init)();
	STDMETHOD(Reset)();
	STDMETHOD(ShutDown)();
   STDMETHOD(Init2)();
   STDMETHOD(GetClassID)(CLSID* pCLSID);

// IXBRMomentCapacity
public:
   virtual Float64 GetMomentCapacity(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment);
   virtual const MomentCapacityDetails& GetMomentCapacityDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment);

   virtual Float64 GetCrackingMoment(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment);
   virtual const CrackingMomentDetails& GetCrackingMomentDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment);

   virtual Float64 GetMinMomentCapacity(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment);
   virtual const MinMomentCapacityDetails& GetMinMomentCapacityDetails(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment);
   virtual MinMomentCapacityDetails GetMinMomentCapacityDetails(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx);

// IXBRShearCapacity
public:
   virtual Float64 GetShearCapacity(PierIDType pierID,const xbrPointOfInterest& poi);

// IXBRArtifact
public:
   virtual const xbrRatingArtifact* GetXBeamRatingArtifact(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx);

// IXBRProjectEventSink
public:
   virtual HRESULT OnProjectChanged();

private:
   DECLARE_EAF_AGENT_DATA;

   DWORD m_dwProjectCookie;


   // PierID is not used to store moment capacity because the POI ID is sufficent
   // POI IDs are not duplicated between piers
   std::map<IDType,MomentCapacityDetails> m_PositiveMomentCapacity[2]; // key = POI ID, array index = xbrTypes::Stage
   std::map<IDType,MomentCapacityDetails> m_NegativeMomentCapacity[2];

   std::map<IDType,CrackingMomentDetails> m_PositiveCrackingMoment[2]; // key = POI ID, array index = xbrTypes::Stage
   std::map<IDType,CrackingMomentDetails> m_NegativeCrackingMoment[2];

   std::map<IDType,MinMomentCapacityDetails> m_PositiveMinMomentCapacity[2][6]; // key = POI ID, array index = xbrTypes::Stage, second array index is based on limit state type.use GET_INDEX(limitState) macro
   std::map<IDType,MinMomentCapacityDetails> m_NegativeMinMomentCapacity[2][6]; 

#pragma Reminder("WORKING HERE: need to have shear capacity by pier")
   // need to cache shear capacity

   MomentCapacityDetails ComputeMomentCapacity(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment);
   CrackingMomentDetails ComputeCrackingMoment(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment);
   MinMomentCapacityDetails ComputeMinMomentCapacity(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment);
   MinMomentCapacityDetails ComputeMinMomentCapacity(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx);
   void GetCrackingMomentFactors(PierIDType pierID,Float64* pG1,Float64* pG2,Float64* pG3);

   Float64 GetDv(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi);
   Float64 GetAverageAvOverS(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,Float64 theta);


   // rating artifacts for vehicleIdx == INVALID_INDEX are the governing artifacts for a load rating type
   typedef std::map<VehicleIndexType,xbrRatingArtifact> RatingArtifacts;
   std::map<PierIDType,RatingArtifacts> m_RatingArtifacts[6]; // array index is pgsTypes::LoadRatingType
   RatingArtifacts& GetPrivateRatingArtifacts(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx);
   void CreateRatingArtifact(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx);
};

OBJECT_ENTRY_AUTO(CLSID_EngAgent, CEngAgentImp)

