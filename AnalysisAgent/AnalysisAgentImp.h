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

// AnalysisAgentImp.h : Declaration of the CAnalysisAgentImp

// This agent is responsible for creating structural analysis models
// and providing analysis results

#pragma once

#include <EAF\Agent.h>
#include <AnalysisAgent.h>
#include "AnalysisAgentCLSID.h"

#include <IFace/AnalysisResults.h>

#include <IFace\Project.h>

#include <FEA2D/Model.h>

#include <..\..\PGSuper\Include\IFace\Project.h>

#if defined _USE_MULTITHREADING
#include <PgsExt\ThreadManager.h>
#endif

#define FIRST_LIVELOAD_ID 10000

#define MODEL_INIT_NONE    -1
#define MODEL_INIT_TOPOLOGY 0
#define MODEL_INIT_LOADS    1
#define MODEL_INIT_ALL      MODEL_INIT_LOADS

/////////////////////////////////////////////////////////////////////////////
// CAnalysisAgentImp
class CAnalysisAgentImp : public WBFL::EAF::Agent,
   public WBFL::EAF::IAgentPriority,
   public IXBRProductForces,
   public IXBRAnalysisResults,
   public IXBRProjectEventSink,
   public IBridgeDescriptionEventSink
{  
public:
   CAnalysisAgentImp() = default;

// Agent
public:
   std::_tstring GetName() const override { return _T("XBeam Rate Analysis Agent"); }
   bool RegisterInterfaces() override;
   bool Init() override;
   bool Reset() override;
   bool ShutDown() override;
   CLSID GetCLSID() const override;

   // IAgentPriority
public:
   IndexType GetPriority() const override;

// IXBRProductForces
public:
   const std::vector<LowerXBeamLoad>& GetLowerCrossBeamLoading(PierIDType pierID) const override;
   Float64 GetUpperCrossBeamLoading(PierIDType pierID) const override;

   IndexType GetLiveLoadConfigurationCount(PierIDType pierID,pgsTypes::LoadRatingType ratingType) const override;
   IndexType GetLoadedLaneCount(PierIDType pierID,IndexType liveLoadConfigIdx) const override;
   std::vector<Float64> GetWheelLineLocations(PierIDType pierID) const override;
   WheelLineConfiguration GetLiveLoadConfiguration(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType liveLoadConfigIdx,IndexType permitLaneIdx) const override;
   void GetGoverningMomentLiveLoadConfigurations(PierIDType pierID,const xbrPointOfInterest& poi,std::vector<IndexType>* pvMin,std::vector<IndexType>* pvMax) const override;
   void GetGoverningShearLiveLoadConfigurations(PierIDType pierID,const xbrPointOfInterest& poi,std::vector<IndexType>* pvLLConfig) const override;

// IXBRAnalysisResults
public:
   Float64 GetMoment(PierIDType pierID,xbrTypes::ProductForceType pfType,const xbrPointOfInterest& poi) const override;
   WBFL::System::SectionValue GetShear(PierIDType pierID,xbrTypes::ProductForceType pfType,const xbrPointOfInterest& poi) const override;
   std::vector<Float64> GetMoment(PierIDType pierID,xbrTypes::ProductForceType pfType,const std::vector<xbrPointOfInterest>& vPoi) const override;
   std::vector<WBFL::System::SectionValue> GetShear(PierIDType pierID,xbrTypes::ProductForceType pfType,const std::vector<xbrPointOfInterest>& vPoi) const override;

   Float64 GetMoment(PierIDType pierID,xbrTypes::CombinedForceType lcType,const xbrPointOfInterest& poi) const override;
   WBFL::System::SectionValue GetShear(PierIDType pierID,xbrTypes::CombinedForceType lcType,const xbrPointOfInterest& poi) const override;
   std::vector<Float64> GetMoment(PierIDType pierID,xbrTypes::CombinedForceType lcType,const std::vector<xbrPointOfInterest>& vPoi) const override;
   std::vector<WBFL::System::SectionValue> GetShear(PierIDType pierID,xbrTypes::CombinedForceType lcType,const std::vector<xbrPointOfInterest>& vPoi) const override;

   Float64 GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,const xbrPointOfInterest& poi) const override;
   WBFL::System::SectionValue GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,const xbrPointOfInterest& poi) const override;
   std::vector<Float64> GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,const std::vector<xbrPointOfInterest>& vPoi) const override;
   std::vector<WBFL::System::SectionValue> GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,const std::vector<xbrPointOfInterest>& vPoi) const override;

   void GetMoment(PierIDType pierID,pgsTypes::LoadRatingType permitRatingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx,const xbrPointOfInterest& poi,Float64* pMpermit,Float64* pMlegal) const override;
   void GetMoment(PierIDType pierID,pgsTypes::LoadRatingType permitRatingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx,const std::vector<xbrPointOfInterest>& vPoi,std::vector<Float64>* pvMpermit,std::vector<Float64>* pvMlegal) const override;
   void GetShear(PierIDType pierID,pgsTypes::LoadRatingType permitRatingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx,const xbrPointOfInterest& poi,WBFL::System::SectionValue* pVpermit,WBFL::System::SectionValue* pVlegal) const override;
   void GetShear(PierIDType pierID,pgsTypes::LoadRatingType permitRatingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx,const std::vector<xbrPointOfInterest>& vPoi,std::vector<WBFL::System::SectionValue>* pvVpermit,std::vector<WBFL::System::SectionValue>* pvVlegal) const override;

   void GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,const xbrPointOfInterest& poi,Float64* pMin,Float64* pMax,IndexType* pMinLLConfigIdx,IndexType* pMaxLLConfigIdx/*WheelLineConfiguration* pMinConfiguration,WheelLineConfiguration* pMaxConfiguration*/) const override;
   void GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,const xbrPointOfInterest& poi,WBFL::System::SectionValue* pMin,WBFL::System::SectionValue* pMax,IndexType* pMinLLConfigIdx,IndexType* pMaxLLConfigIdx/*,WheelLineConfiguration* pMinLeftConfiguration,WheelLineConfiguration* pMinRightConfiguration,WheelLineConfiguration* pMaxLeftConfiguration,WheelLineConfiguration* pMaxRightConfiguration*/) const override;
   void GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,const std::vector<xbrPointOfInterest>& vPoi,std::vector<Float64>* pvMin,std::vector<Float64>* pvMax,std::vector<IndexType>* pvMinLLConfigIdx,std::vector<IndexType>* pvMaxLLConfigIdx/*std::vector<WheelLineConfiguration>* pvMinConfiguration,std::vector<WheelLineConfiguration>* pvMaxConfiguration*/) const override;
   void GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,const std::vector<xbrPointOfInterest>& vPoi,std::vector<WBFL::System::SectionValue>* pvMin,std::vector<WBFL::System::SectionValue>* pvMax,std::vector<IndexType>* pvMinLLConfigIdx,std::vector<IndexType>* pvMaxLLConfigIdx/*std::vector<WheelLineConfiguration>* pvMinLeftConfiguration,std::vector<WheelLineConfiguration>* pvMinRightConfiguration,std::vector<WheelLineConfiguration>* pvMaxLeftConfiguration,std::vector<WheelLineConfiguration>* pvMaxRightConfiguration*/) const override;

   void GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,const xbrPointOfInterest& poi,Float64* pMin,Float64* pMax,VehicleIndexType* pMinVehicleIdx,VehicleIndexType* pMaxVehicleIdx) const override;
   void GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,const xbrPointOfInterest& poi,WBFL::System::SectionValue* pMin,WBFL::System::SectionValue* pMax,VehicleIndexType* pMinLeftVehicleIdx,VehicleIndexType* pMinRightVehicleIdx,VehicleIndexType* pMaxLeftVehicleIdx,VehicleIndexType* pMaxRightVehicleIdx) const override;
   void GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,const std::vector<xbrPointOfInterest>& vPoi,std::vector<Float64>* pvMin,std::vector<Float64>* pvMax,std::vector<VehicleIndexType>* pvMinVehicleIdx,std::vector<VehicleIndexType>* pvMaxVehicleIdx) const override;
   void GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,const std::vector<xbrPointOfInterest>& vPoi,std::vector<WBFL::System::SectionValue>* pvMin,std::vector<WBFL::System::SectionValue>* pvMax,std::vector<VehicleIndexType>* pvMinLeftVehicleIdx,std::vector<VehicleIndexType>* pvMinRightVehicleIdx,std::vector<VehicleIndexType>* pvMaxLeftVehicleIdx,std::vector<VehicleIndexType>* pvMaxRightVehicleIdx) const override;

   void GetMoment(PierIDType pierID,pgsTypes::LimitState limitState,const xbrPointOfInterest& poi,Float64* pMin,Float64* pMax) const override;
   void GetShear(PierIDType pierID,pgsTypes::LimitState limitState,const xbrPointOfInterest& poi,WBFL::System::SectionValue* pMin,WBFL::System::SectionValue* pMax) const override;
   void GetMoment(PierIDType pierID,pgsTypes::LimitState limitState,const std::vector<xbrPointOfInterest>& vPoi,std::vector<Float64>* pvMin,std::vector<Float64>* pvMax) const override;
   void GetShear(PierIDType pierID,pgsTypes::LimitState limitState,const std::vector<xbrPointOfInterest>& vPoi,std::vector<WBFL::System::SectionValue>* pvMin,std::vector<WBFL::System::SectionValue>* pvMax) const override;

// IXBRProjectEventSink
public:
   HRESULT OnProjectChanged();

// IBridgeDescriptionEventSink
public:
   HRESULT OnBridgeChanged(CBridgeChangedHint* pHint) override;
   HRESULT OnGirderFamilyChanged() override;
   HRESULT OnGirderChanged(const CGirderKey& girderKey,Uint32 lHint) override;
   HRESULT OnLiveLoadChanged() override;
   HRESULT OnLiveLoadNameChanged(LPCTSTR strOldName,LPCTSTR strNewName) override;
   HRESULT OnConstructionLoadChanged() override;

#ifdef _DEBUG
   bool AssertValid() const;
#endif//

private:
   EAF_DECLARE_AGENT_DATA;
   StatusCallbackIDType m_scidBridgeWarning;
   StatusCallbackIDType m_scidBridgeError;

   void Invalidate(bool bCreateNewDataStructures = true);

   LoadCaseIDType GetLoadCaseID(xbrTypes::ProductForceType pfType) const;
   std::vector<xbrTypes::ProductForceType> GetLoads(xbrTypes::CombinedForceType lcType) const;

   IDType m_dwProjectCookie;
   IDType m_dwBridgeDescCookie;

   struct BeamMember
   {
      Float64 Xs; // start of member location
      Float64 Xe; // end of member location
      MemberIDType mbrID;
   };

   struct LaneConfiguration
   {
      // locations are measured from the left curb line
      Float64 Xleft;  // location of left wheel line
      Float64 Xright; // location of right wheel line
   };

   struct LiveLoadConfiguration
   {
      // a live load configuration consists of one or more live load cases
      // each live load case models a single lane (two wheel lines)

      std::vector<LoadCaseIDType> m_LoadCases; // FEM load case IDs that make up the loads for this configuration.
   };

   typedef struct ModelData
   {
      std::unique_ptr<WBFL::FEA2D::Model> m_Model;
      
      std::vector<BeamMember> m_XBeamMembers; // these members make up the cap beam
      std::vector<BeamMember> m_SuperstructureMembers; // these are the members where the live load is applied
      
      std::vector<LowerXBeamLoad> m_LowerXBeamLoads;

      // key is the product model poi ID
      // value is the FEM model poi ID
      std::map<PoiIDType,PoiIDType> m_PoiMap;

      LoadCaseIDType m_NextLiveLoadCaseID; // ID of the next live load case

      // this vector contains all of the live load configurations
      // each configuration represents the same number of loaded lanes,
      // the position of the loaded lanes with respect to one another, 
      // and the position of the loaded lanes transversely on the cross beam
      std::vector<LiveLoadConfiguration> m_LiveLoadConfigurations;

      IndexType m_LastSingleLaneLLConfigIdx; // Index of the last live load configuration where a single lane is loaded
                                             // This is needed because for MBE permit ratings, only the one loaded lane case is needed
                                             // This is the index of the last entry in m_LiveLoadConfigruations that is for a single loaded lane case

      // this map keeps track of the live load cases
      // the key is the location of the center of the lane
      std::map<Float64,LoadCaseIDType,Float64_less> m_LiveLoadCases;

      // this map keeps track of the lane configurations corresponding
      // to each live load case.
      std::map<LoadCaseIDType,LaneConfiguration> m_LaneConfigurations;

      int m_InitLevel;

      Float64 m_Lmax;

      ModelData() { m_InitLevel = MODEL_INIT_NONE; 
                    m_NextLiveLoadCaseID = FIRST_LIVELOAD_ID; }
   } ModelData;
   std::unique_ptr<std::map<PierIDType,ModelData>> m_pModelData;
   ModelData* GetModelData(PierIDType pierID,int level = MODEL_INIT_ALL) const;
   void BuildModel(PierIDType pierID,int level) const;
   void GetFemModelLocation(ModelData* pModelData,const xbrPointOfInterest& poi,MemberIDType* pMbrID,Float64* pMbrLocation) const;
   void GetXBeamFemModelLocation(ModelData* pModelData,Float64 X,MemberIDType* pMbrID,Float64* pMbrLocation) const;
   void GetSuperstructureFemModelLocation(ModelData* pModelData,Float64 X,MemberIDType* pMbrID,Float64* pMbrLocation) const;

   void ApplyDeadLoad(PierIDType pierID,ModelData* pModelData) const;
   void ApplyLowerXBeamDeadLoad(PierIDType pierID,ModelData* pModelData) const;
   void ApplyUpperXBeamDeadLoad(PierIDType pierID,ModelData* pModelData) const;
   void ApplySuperstructureDeadLoadReactions(PierIDType pierID,ModelData* pModelData) const;
   void ValidateLowerXBeamDeadLoad(PierIDType pierID,ModelData* pModelData) const;

   void ComputeLiveLoadLocations(PierIDType pierID,ModelData* pModelData) const;
   void GetLaneGapConfiguration(IndexType nTotalSteps,IndexType nLaneGaps,std::vector<std::vector<IndexType>>& vGapPositions) const;
   std::vector<Float64> GetWheelLinePositions(Float64 skew,Float64 stepSize,Float64 wLoadedLane,const std::vector<IndexType>& vGapPosition) const;
   std::vector<LoadCaseIDType> InitializeWheelLineLoads(ModelData* pModelData,Float64 Xoffset,const std::vector<Float64>& vLoadPositions) const;
   void ApplyWheelLineLoadsToFemModel(ModelData* pModelData) const;


   typedef struct UnitLiveLoadResult
   {
      // unit load results are moment/shear for a unit lane reaction in each loaded lane
      // multiplied by the multiple presence factor.
      // to get actual moment/shear for a lane reaction case, simply scale the stored values
      // by a single lane reaction.
      bool operator<(const UnitLiveLoadResult& other)const {return m_idPOI < other.m_idPOI;}
      PoiIDType m_idPOI;

      // controlling moment for one loaded lane
      IndexType m_llConfigIdx_MzMin_SingleLane; // live load configuration for the controlling min moment
      IndexType m_llConfigIdx_MzMax_SingleLane; // live load configuration for the controlling max moment
      Float64 m_MzMin_SingleLane;
      Float64 m_MzMax_SingleLane;

      // controlling moment for multiple loaded lanes
      IndexType m_llConfigIdx_MzMin; // live load configuration for the controlling min moment
      IndexType m_llConfigIdx_MzMax; // live load configuration for the controlling max moment
      Float64 m_MzMin;
      Float64 m_MzMax;

      // Live load configuration index for the configurations that cause the N max/min moments
      // Used only when permit rating factors are computed by the WSDOT method.
      // WSDOT permit rating factor method requires that multiple combinations
      // of permit load in one lane and legal load in all remaining lanes be
      // evaluated. In theory, all loading conditions should be evaluated, however
      // this is considerably time-consuming. Instead, the rating factor
      // will be computed based the N load cases that cause the max/min
      // moments at this section.
      std::vector<IndexType> m_MzMaxLiveLoadConfigs;
      std::vector<IndexType> m_MzMinLiveLoadConfigs;

      // controlling shear for one loaded lane
      IndexType m_llConfigIdx_FyLeftMin_SingleLane, m_llConfigIdx_FyRightMin_SingleLane; // live load configuration for the controlling min shear
      IndexType m_llConfigIdx_FyLeftMax_SingleLane, m_llConfigIdx_FyRightMax_SingleLane; // live load configuration for the controlling max shear
      WBFL::System::SectionValue m_FyMin_SingleLane;
      WBFL::System::SectionValue m_FyMax_SingleLane;

      // controlling shear for multiple loaded lanes
      IndexType m_llConfigIdx_FyLeftMin, m_llConfigIdx_FyRightMin; // live load configuration for the controlling min shear
      IndexType m_llConfigIdx_FyLeftMax, m_llConfigIdx_FyRightMax; // live load configuration for the controlling max shear
      WBFL::System::SectionValue m_FyMin;
      WBFL::System::SectionValue m_FyMax;

      // Live load configuration index for the configurations that cause the N max shears
      // Max shear is based on the magnitude of the shear.
      // Used only when permit rating factors are computed by the WSDOT method.
      // WSDOT permit rating factor method requires that multiple combinations
      // of permit load in one lane and legal load in all remaining lanes be
      // evaluated. In theory, all loading conditions should be evaluated, however
      // this is considerably time-consuming. Instead, the rating factor
      // will be computed based the N load cases that cause the max
      // shears at this section.
      std::vector<IndexType> m_FyLiveLoadConfigs;
   } UnitLiveLoadResult;
   std::unique_ptr<std::map<PierIDType,std::set<UnitLiveLoadResult>>> m_pUnitLiveLoadResults;
   std::set<UnitLiveLoadResult>& GetUnitLiveLoadResults(PierIDType pierID) const;
   void ComputeUnitLiveLoadResult(PierIDType pierID,const xbrPointOfInterest& poi) const;
   UnitLiveLoadResult& GetUnitLiveLoadResult(PierIDType pierID,const xbrPointOfInterest& poi) const;

   Float64 GetMaxLegalReaction(PierIDType pierID) const;
   std::vector<Float64> GetWheelLineLocations(ModelData* pModelData) const;


#if defined _USE_MULTITHREADING
   CThreadManager m_ThreadManager;
#endif

   void InvalidateModels(bool bCreateNewDataStructures = true);
   static UINT DeleteModels(LPVOID pParam);

   void InvalidateResults(bool bCreateNewDataStructures = true);
   static UINT DeleteResults(LPVOID pParam);

};

