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


// ProjectAgentImp.h : Declaration of the CProjectAgentImp

// This agent provides everything related to the input data including the data itself,
// the UI for manipulating the data, and persistence of the data.

#pragma once

#include <EAF\Agent.h>
#include <IFace/Project.h>
#include <IFace/RatingSpecification.h>
#include <ProjectAgent.h>
#include "CPProjectAgent.h"
#include "ProjectAgentCLSID.h"


#include <EAF\EAFUIIntegration.h>
#include <XBeamRateExt\PierData.h>
#include <XBeamRateExt\XBeamRateUtilities.h>

#include <..\..\PGSuper\Include\IFace\Project.h>

/////////////////////////////////////////////////////////////////////////////
// CProjectAgentImp
class CProjectAgentImp : public CCmdTarget,
   public WBFL::EAF::Agent,
   public CProxyIXBRProjectPropertiesEventSink<CProjectAgentImp>,
   public CProxyIXBRProjectEventSink<CProjectAgentImp>,
   public CProxyIXBREventsEventSink<CProjectAgentImp>,
   public WBFL::EAF::IAgentPriority,
   public WBFL::EAF::IAgentUIIntegration,
   public WBFL::EAF::IAgentPersist,
   public IXBRProjectProperties,
   public IXBRProject,
   public IXBRRatingSpecification,
   public IXBRProjectEdit,
   public IXBREvents,
   public IXBRExport,
   public IBridgeDescriptionEventSink,
   public IEventsSink,
   public WBFL::EAF::ICommandCallback
{  
public:
	CProjectAgentImp(); 

   HRESULT SavePier(PierIndexType pierIdx,LPCTSTR lpszPathName);

// Agent
public:
   std::_tstring GetName() const override { return _T("XBeam Rate Project Agent"); }
   bool RegisterInterfaces() override;
   bool Init() override;
   bool Reset() override;
   bool ShutDown() override;
   CLSID GetCLSID() const override;

// IAgentPriority
public:
   IndexType GetPriority() const override;

// IAgentUIIntegration
public:
   bool IntegrateWithUI(bool bIntegrate) override;

// IAgentPersist
public:
   WBFL::EAF::Broker::LoadResult Load(WBFL::System::IStructuredLoad* pStrLoad) override;
   bool Save(WBFL::System::IStructuredSave* pStrSave) override;

// IXBRProjectProperties
public:
   LPCTSTR GetBridgeName() const override;
   void SetBridgeName(LPCTSTR name) override;
   LPCTSTR GetBridgeID() const override;
   void SetBridgeID(LPCTSTR bid) override;
   LPCTSTR GetJobNumber() const override;
   void SetJobNumber(LPCTSTR jid) override;
   LPCTSTR GetEngineer() const override;
   void SetEngineer(LPCTSTR eng) override;
   LPCTSTR GetCompany() const override;
   void SetCompany(LPCTSTR company) override;
   LPCTSTR GetComments() const override;
   void SetComments(LPCTSTR comments) override;
   void ShowProjectPropertiesOnNewProject(bool bShow) override;
   bool ShowProjectPropertiesOnNewProject() const override;
   void PromptForProjectProperties() override;

// IXBRProject
public:
   void SetPierData(const xbrPierData& pierData) override;
   const xbrPierData& GetPierData(PierIDType pierID) const override;

   xbrTypes::PierType GetPierType(PierIDType pierID) const override;
   void SetPierType(PierIDType pierID,xbrTypes::PierType pierType) override;

   void SetDeckElevation(PierIDType pierID,Float64 deckElevation) override;
   Float64 GetDeckElevation(PierIDType pierID) const override;
   void SetDeckThickness(PierIDType pierID,Float64 tDeck) override;
   Float64 GetDeckThickness(PierIDType pierID) const override;
   void SetCrownPointOffset(PierIDType pierID,Float64 cpo) override;
   Float64 GetCrownPointOffset(PierIDType pierID) const override;
   void SetBridgeLineOffset(PierIDType pierID,Float64 blo) override;
   Float64 GetBridgeLineOffset(PierIDType pierID) const override;

   void SetOrientation(PierIDType pierID,LPCTSTR strOrientation) override;
   LPCTSTR GetOrientation(PierIDType pierID) const override;

   pgsTypes::OffsetMeasurementType GetCurbLineDatum(PierIDType pierID) const override;
   void SetCurbLineDatum(PierIDType pierID,pgsTypes::OffsetMeasurementType datumType) override;

   void SetCurbLineOffset(PierIDType pierID,Float64 leftCLO,Float64 rightCLO) override;
   void GetCurbLineOffset(PierIDType pierID,Float64* pLeftCLO,Float64* pRightCLO) const override;

   void SetCrownSlopes(PierIDType pierID,Float64 sl,Float64 sr) override;
   void GetCrownSlopes(PierIDType pierID,Float64* psl,Float64* psr) const override;

   void GetDiaphragmDimensions(PierIDType pierID,Float64* pH,Float64* pW) const override;
   void SetDiaphragmDimensions(PierIDType pierID,Float64 H,Float64 W) override;

   IndexType GetBearingLineCount(PierIDType pierID) const override;
   void SetBearingLineCount(PierIDType pierID,IndexType nBearingLines) override;

   IndexType GetBearingCount(PierIDType pierID,IndexType brgLineIdx) const override;
   void SetBearingCount(PierIDType pierID,IndexType brgLineIdx,IndexType nBearings) override;

   Float64 GetBearingSpacing(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx) const override;
   void SetBearingSpacing(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx,Float64 spacing) override;

   void SetBearingReactionType(PierIDType pierID,IndexType brgLineIdx,xbrTypes::ReactionLoadType brgReactionType) override;
   xbrTypes::ReactionLoadType GetBearingReactionType(PierIDType pierID,IndexType brgLineIdx) const override;

   void SetBearingReactions(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx,Float64 DC,Float64 DW,Float64 CR,Float64 SH,Float64 PS,Float64 RE,Float64 W) override;
   void GetBearingReactions(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx,Float64* pDC,Float64* pDW,Float64* pCR,Float64* pSH,Float64* pPS,Float64* pRE,Float64* pW) const override;
   Float64 GetBearingWidth(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx) const override;

   void GetReferenceBearing(PierIDType pierID,IndexType brgLineIdx,IndexType* pRefIdx,Float64* pRefBearingOffset,pgsTypes::OffsetMeasurementType* pRefBearingDatum) const override;
   void SetReferenceBearing(PierIDType pierID,IndexType brgLineIdx,IndexType refIdx,Float64 refBearingOffset,pgsTypes::OffsetMeasurementType refBearingDatum) override;

   void SetReactionLoadApplicationType(PierIDType pierID,xbrTypes::ReactionLoadApplicationType applicationType) override;
   xbrTypes::ReactionLoadApplicationType GetReactionLoadApplicationType(PierIDType pierID) const override;

   IndexType GetLiveLoadReactionCount(PierIDType pierID,pgsTypes::LoadRatingType ratingType) const override;
   void SetLiveLoadReactions(PierIDType pierID,pgsTypes::LoadRatingType ratingType,const std::vector<xbrLiveLoadReactionData>& vLLIM) override;
   std::vector<xbrLiveLoadReactionData> GetLiveLoadReactions(PierIDType pierID,pgsTypes::LoadRatingType ratingType) const override;
   std::_tstring GetLiveLoadName(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const override;
   Float64 GetLiveLoadReaction(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const override;
   Float64 GetVehicleWeight(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const override;

   void SetRebarMaterial(PierIDType pierID,WBFL::Materials::Rebar::Type type,WBFL::Materials::Rebar::Grade grade) override;
   void GetRebarMaterial(PierIDType pierID,WBFL::Materials::Rebar::Type* pType,WBFL::Materials::Rebar::Grade* pGrade) const override;

   void SetConcrete(PierIDType pierID,const CConcreteMaterial& concrete) override;
   const CConcreteMaterial& GetConcrete(PierIDType pierID) const override;

   void SetLowerXBeamDimensions(PierIDType pierID,Float64 h1,Float64 h2,Float64 h3,Float64 h4,Float64 x1,Float64 x2,Float64 x3,Float64 x4,Float64 w) override;
   void GetLowerXBeamDimensions(PierIDType pierID,Float64* ph1,Float64* ph2,Float64* ph3,Float64* ph4,Float64* px1,Float64* px2,Float64* px3,Float64* px4,Float64* pw) const override;
   Float64 GetXBeamLeftOverhang(PierIDType pierID) const override;
   Float64 GetXBeamRightOverhang(PierIDType pierID) const override;
   Float64 GetXBeamWidth(PierIDType pierID) const override;

   void SetRefColumnLocation(PierIDType pierID,pgsTypes::OffsetMeasurementType refColumnDatum,IndexType refColumnIdx,Float64 refColumnOffset) override;
   void GetRefColumnLocation(PierIDType pierID,pgsTypes::OffsetMeasurementType* prefColumnDatum,IndexType* prefColumnIdx,Float64* prefColumnOffset) const override;
   IndexType GetColumnCount(PierIDType pierID) const override;
   Float64 GetColumnHeight(PierIDType pierID,ColumnIndexType colIdx) const override;
   CColumnData::ColumnHeightMeasurementType GetColumnHeightMeasurementType(PierIDType pierID,ColumnIndexType colIdx) const override;
   Float64 GetColumnSpacing(PierIDType pierID,SpacingIndexType spaceIdx) const override;
   pgsTypes::ColumnTransverseFixityType GetColumnFixity(PierIDType pierID,ColumnIndexType colIdx) const override;

   void SetColumnProperties(PierIDType pierID,ColumnIndexType colIdx,CColumnData::ColumnShapeType shapeType,Float64 D1,Float64 D2,CColumnData::ColumnHeightMeasurementType heightType,Float64 H) override;
   void GetColumnProperties(PierIDType pierID,ColumnIndexType colIdx,CColumnData::ColumnShapeType* pshapeType,Float64* pD1,Float64* pD2,CColumnData::ColumnHeightMeasurementType* pheightType,Float64* pH) const override;

   const xbrLongitudinalRebarData& GetLongitudinalRebar(PierIDType pierID) const override;
   void SetLongitudinalRebar(PierIDType pierID,const xbrLongitudinalRebarData& rebar) override;
   void SetLowerXBeamStirrups(PierIDType pierID,const xbrStirrupData& stirrups) override;
   const xbrStirrupData& GetLowerXBeamStirrups(PierIDType pierID) const override;
   void SetFullDepthStirrups(PierIDType pierID,const xbrStirrupData& stirrups) override;
   const xbrStirrupData& GetFullDepthStirrups(PierIDType pierID) const override;
   
   void SetFlexureResistanceFactors(Float64 phiC,Float64 phiT) override;
   void GetFlexureResistanceFactors(Float64* phiC,Float64* phiT) const override;
   void SetShearResistanceFactor(Float64 phi) override;
   Float64 GetShearResistanceFactor() const override;

   void SetSystemFactorFlexure(Float64 sysFactor) override;
   Float64 GetSystemFactorFlexure() const override;
   
   void SetSystemFactorShear(Float64 sysFactor) override;
   Float64 GetSystemFactorShear() const override;

   void SetConditionFactor(PierIDType pierID,pgsTypes::ConditionFactorType conditionFactorType,Float64 conditionFactor) override;
   void GetConditionFactor(PierIDType pierID,pgsTypes::ConditionFactorType* pConditionFactorType,Float64 *pConditionFactor) const override;
   Float64 GetConditionFactor(PierIDType pierID) const override;

   void SetDCLoadFactor(pgsTypes::LimitState limitState,Float64 dc) override;
   Float64 GetDCLoadFactor(pgsTypes::LimitState limitState) const override;

   void SetDWLoadFactor(pgsTypes::LimitState limitState,Float64 dw) override;
   Float64 GetDWLoadFactor(pgsTypes::LimitState limitState) const override;

   void SetCRLoadFactor(pgsTypes::LimitState limitState,Float64 cr) override;
   Float64 GetCRLoadFactor(pgsTypes::LimitState limitState) const override;

   void SetSHLoadFactor(pgsTypes::LimitState limitState,Float64 sh) override;
   Float64 GetSHLoadFactor(pgsTypes::LimitState limitState) const override;

   void SetRELoadFactor(pgsTypes::LimitState limitState,Float64 re) override;
   Float64 GetRELoadFactor(pgsTypes::LimitState limitState) const override;

   void SetPSLoadFactor(pgsTypes::LimitState limitState,Float64 ps) override;
   Float64 GetPSLoadFactor(pgsTypes::LimitState limitState) const override;

   void SetLiveLoadFactor(PierIDType pierID,pgsTypes::LimitState limitState,Float64 ll) override;
   Float64 GetLiveLoadFactor(PierIDType pierID,pgsTypes::LimitState limitState,VehicleIndexType vehicleIdx) const override;

   void SetMaxLiveLoadStepSize(Float64 stepSize) override;
   Float64 GetMaxLiveLoadStepSize() const override;

   void SetMaxLoadedLanes(IndexType nLanesMax) override;
   IndexType GetMaxLoadedLanes() const override;

   bool GetDoAnalyzeNegativeMomentBetweenFocOptions(Float64* minColumnWidth) override;
   void SetDoAnalyzeNegativeMomentBetweenFocOptions(bool bDoUseOption, Float64 minColumnWidth) override;

// IXBRRatingSpecification
public:
   bool IsRatingEnabled(pgsTypes::LoadRatingType ratingType) const override;
   void EnableRating(pgsTypes::LoadRatingType ratingType,bool bEnable) override;

   void RateForShear(pgsTypes::LoadRatingType ratingType,bool bRateForShear) override;
   bool RateForShear(pgsTypes::LoadRatingType ratingType) const override;

   // Evaluate yield stress in reinforcement MBE 6A.5.4.2.2b
   void CheckYieldStressLimit(bool bCheckYieldStress) override;
   bool CheckYieldStressLimit() const override;

   // returns fraction of yield stress that reinforcement can be stressed to during
   // a permit load rating evaluation MBE 6A.5.4.2.2b
   void SetYieldStressLimitCoefficient(Float64 x) override;
   Float64 GetYieldStressLimitCoefficient() const override;

   pgsTypes::AnalysisType GetAnalysisMethodForReactions() const override;
   void SetAnalysisMethodForReactions(pgsTypes::AnalysisType analysisType) override;

   xbrTypes::EmergencyRatingMethod GetEmergencyRatingMethod() const override;
   void SetEmergencyRatingMethod(xbrTypes::EmergencyRatingMethod emergencyRatingMethod) override;
   bool IsWSDOTEmergencyRating(pgsTypes::LoadRatingType ratingType) const override;

   xbrTypes::PermitRatingMethod GetPermitRatingMethod() const override;
   void SetPermitRatingMethod(xbrTypes::PermitRatingMethod permitRatingMethod) override;
   bool IsWSDOTPermitRating(pgsTypes::LoadRatingType ratingType) const override;

   bool DoCheckNegativeMomentBetweenFOCs(PierIDType pierID) const override;

// IXBRProjectEdit
public:
   void EditPier(int nPage) override;
   void EditOptions() override;

// IXBREvents
public:
   void HoldEvents() override;
   void FirePendingEvents() override;
   void CancelPendingEvents() override;

// IBridgeDescriptionEventSink
public:
   HRESULT OnBridgeChanged(CBridgeChangedHint* pHint) override;
   HRESULT OnGirderFamilyChanged() override;
   HRESULT OnGirderChanged(const CGirderKey& girderKey,Uint32 lHint) override;
   HRESULT OnLiveLoadChanged() override;
   HRESULT OnLiveLoadNameChanged(LPCTSTR strOldName,LPCTSTR strNewName) override;
   HRESULT OnConstructionLoadChanged() override;

// IEventsSink
public:
   HRESULT OnHoldEvents() override;
   HRESULT OnFirePendingEvents() override;
   HRESULT OnCancelPendingEvents() override;


// IXBRExport
public:
   HRESULT Export(PierIndexType pierIdx) override;
   HRESULT BatchExport() override;

   // ICommandCallback
   BOOL OnCommandMessage(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) override;
   BOOL GetStatusBarMessageString(UINT nID, CString& rMessage) const override;
   BOOL GetToolTipMessageString(UINT nID, CString& rMessage) const override;
   afx_msg void OnEditPier();
   afx_msg void OnEditOptions();
   afx_msg void OnProjectProperties();

private:
   EAF_DECLARE_AGENT_DATA;
   bool m_bShowProjectProperties = true;

   DECLARE_MESSAGE_MAP()

   PierIDType m_SavePierID; // contains the ID for the pier data that is being saved
   bool m_bExportingModel; // set to true, when exporting a pier model from PGS

   IDType m_dwBridgeDescCookie;
   IDType m_dwEventsCookie;

   StatusGroupIDType m_XBeamRateStatusGroupID; // ID used to identify status items created by this agent
   StatusCallbackIDType m_scidBridgeInfo;
   StatusCallbackIDType m_scidBridgeWarn;
   StatusCallbackIDType m_scidBridgeError;

   // Project Properties
   CString m_strBridgeName;
   CString m_strBridgeId;
   CString m_strJobNumber;
   CString m_strEngineer;
   CString m_strCompany;
   CString m_strComments;

   // The raw data for this project
   // The key to this map is the pier ID. INVALID_ID
   // will be the key for the stand alone case
   xbrPierData& GetPrivatePierData(PierIDType pierID) const;
   mutable std::map<PierIDType,xbrPierData> m_PierData;

   Float64 m_SysFactorFlexure;
   Float64 m_SysFactorShear;
   Float64 m_PhiC, m_PhiT; // resistance factors for flexure (compress,tension controlled section)
   Float64 m_PhiV; // resistance factor for shear
   std::array<bool, pgsTypes::lrLoadRatingTypeCount> m_bRatingEnabled; // array index is pgsTypes::LoadRatingType
   std::array<bool, pgsTypes::lrLoadRatingTypeCount> m_bRateForShear; // array index is pgsTypes::LoadRatingType
   bool m_bCheckYieldStress;
   Float64 m_YieldStressCoefficient;

   Float64 m_MaxLLStepSize; // maximum step size for moving live load
   IndexType m_MaxLoadedLanes; // maximum number of loaded lanes to consider (INVALID_INDEX means to consider all lanes)

   bool m_bDoAnalyzeNegativeMomentBetweenFOC;
   Float64 m_MinColumnWidthForNegMoment; // min column width before we have to analyze between FOC's

   // Load Factors
   Float64 m_gDC_StrengthI;
   Float64 m_gDW_StrengthI;
   Float64 m_gCR_StrengthI;
   Float64 m_gSH_StrengthI;
   Float64 m_gPS_StrengthI;
   Float64 m_gRE_StrengthI;

   Float64 m_gDC_StrengthII;
   Float64 m_gDW_StrengthII;
   Float64 m_gCR_StrengthII;
   Float64 m_gSH_StrengthII;
   Float64 m_gPS_StrengthII;
   Float64 m_gRE_StrengthII;

   Float64 m_gDC_ServiceI;
   Float64 m_gDW_ServiceI;
   Float64 m_gCR_ServiceI;
   Float64 m_gSH_ServiceI;
   Float64 m_gPS_ServiceI;
   Float64 m_gRE_ServiceI;

   mutable std::map<PierIDType,Float64> m_gLL[RATING_LIMIT_STATE_COUNT]; // use GET_INDEX macro to access the array
   // Bearing Reactions
   typedef struct BearingReactions
   {
      BearingReactions() { DC = 0, DW = 0; CR = 0; SH = 0; PS = 0; RE = 0; W = 0; }
      Float64 DC, DW, CR, SH, PS, RE;
      Float64 W; // width of reaction (only used of reaction type is rltUniform
   } BearingReactions;
   mutable std::map<PierIDType,std::vector<BearingReactions>> m_BearingReactions[2];
   std::vector<BearingReactions>& GetPrivateBearingReactions(PierIDType pierID,IndexType brgLineIdx) const;

   mutable std::map<PierIDType,xbrTypes::ReactionLoadType> m_BearingReactionType[2];
   xbrTypes::ReactionLoadType& GetPrivateBearingReactionType(PierIDType pierID,IndexType brgLineIdx) const;

   mutable std::map<PierIDType,std::vector<xbrLiveLoadReactionData>> m_LiveLoadReactions[pgsTypes::lrLoadRatingTypeCount]; // access with pgsTypes::LoadRatingType
   std::vector<xbrLiveLoadReactionData>& GetPrivateLiveLoadReactions(PierIDType pierID,pgsTypes::LoadRatingType ratingType) const;

   mutable std::map<PierIDType,xbrTypes::ReactionLoadApplicationType> m_ReactionApplication;
   xbrTypes::ReactionLoadApplicationType& GetPrivateReactionLoadApplication(PierIDType pierID) const;

   pgsTypes::AnalysisType m_AnalysisType; // use this analysis type when PGSuper is in Envelope model

   // Options
   xbrTypes::EmergencyRatingMethod m_EmergencyRatingMethod;
   xbrTypes::PermitRatingMethod m_PermitRatingMethod;

   // Events
   int m_EventHoldCount;
   Uint32 m_PendingEvents;

   friend CProxyIXBRProjectPropertiesEventSink<CProjectAgentImp>;
   friend CProxyIXBRProjectEventSink<CProjectAgentImp>;

   void CreateMenus();
   void RemoveMenus();

   void CreateToolbars();
   void RemoveToolbars();

   void UpdatePiers();
   void UpdatePierData(const CPierData2* pPier,xbrPierData& pierData);
   PierIndexType GetPierIndex(PierIDType pierID) const;
   CGirderKey GetGirderKey(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx) const;
   bool UseUniformLoads(PierIDType pierID,IndexType brgLineIdx) const;

   GirderIndexType GetLongestGirderLine() const;

   void Invalidate();
};

