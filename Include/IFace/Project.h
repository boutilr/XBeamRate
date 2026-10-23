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

#include <PsgLib\ColumnData.h>
#include <XBeamRateExt\PierData.h>
#include <XBeamRateExt\LiveLoadReactionData.h>
#include <memory>

/*****************************************************************************
INTERFACE
   IXBRProjectProperties

   Interface to edit project properties.

DESCRIPTION
   Interface to edit project properties.
*****************************************************************************/
// {E27FD663-261C-40de-BFAD-01B03D98324D}
DEFINE_GUID(IID_IXBRProjectProperties, 
0xe27fd663, 0x261c, 0x40de, 0xbf, 0xad, 0x1, 0xb0, 0x3d, 0x98, 0x32, 0x4d);
class IXBRProjectProperties
{
public:
   virtual LPCTSTR GetBridgeName() const = 0;
   virtual void SetBridgeName(LPCTSTR name) = 0;
   virtual LPCTSTR GetBridgeID() const = 0;
   virtual void SetBridgeID(LPCTSTR bid) = 0;
   virtual LPCTSTR GetJobNumber() const = 0;
   virtual void SetJobNumber(LPCTSTR jid) = 0;
   virtual LPCTSTR GetEngineer() const = 0;
   virtual void SetEngineer(LPCTSTR eng) = 0;
   virtual LPCTSTR GetCompany() const = 0;
   virtual void SetCompany(LPCTSTR company) = 0;
   virtual LPCTSTR GetComments() const = 0;
   virtual void SetComments(LPCTSTR comments) = 0;

   virtual void ShowProjectPropertiesOnNewProject(bool bShow) = 0;
   virtual bool ShowProjectPropertiesOnNewProject() const = 0;
   virtual void PromptForProjectProperties() = 0;
};

/*****************************************************************************
INTERFACE
   IProjectPropertiesEventSink

   Callback interface for project properties.

DESCRIPTION
   Callback interface for project properties.
*****************************************************************************/
// {50C5E910-0941-4135-8603-A8D7192F19CB}
DEFINE_GUID(IID_IXBRProjectPropertiesEventSink, 
0x50c5e910, 0x941, 0x4135, 0x86, 0x3, 0xa8, 0xd7, 0x19, 0x2f, 0x19, 0xcb);
class IXBRProjectPropertiesEventSink
{
public:
   virtual HRESULT OnProjectPropertiesChanged() = 0;
};

/*****************************************************************************
INTERFACE
   IXBRProject

DESCRIPTION
   Interface for getting this application going.
   PierIDType pierID is the pier identifier. For stand alone use INVALID_ID
*****************************************************************************/
// {2600A729-D7E6-44f6-9F9B-DF086FF9E53B}
DEFINE_GUID(IID_IXBRProject, 
0x2600a729, 0xd7e6, 0x44f6, 0x9f, 0x9b, 0xdf, 0x8, 0x6f, 0xf9, 0xe5, 0x3b);
class IXBRProject
{
public:
   virtual void SetPierData(const xbrPierData& pierData) = 0;
   virtual const xbrPierData& GetPierData(PierIDType pierID) const = 0;

   virtual xbrTypes::PierType GetPierType(PierIDType pierID) const = 0;
   virtual void SetPierType(PierIDType pierID,xbrTypes::PierType pierType) = 0;

   // Elevation of the deck on the alignment at the CL Pier
   virtual void SetDeckElevation(PierIDType pierID,Float64 deckElevation) = 0;
   virtual Float64 GetDeckElevation(PierIDType pierID) const = 0;

   // Gross thickness of the deck at the CL Pier
   virtual void SetDeckThickness(PierIDType pierID,Float64 tDeck) = 0;
   virtual Float64 GetDeckThickness(PierIDType pierID) const = 0;

   // Distance from alignment to crown point, measured normal to the alignment.
   virtual void SetCrownPointOffset(PierIDType pierID,Float64 cpo) = 0;
   virtual Float64 GetCrownPointOffset(PierIDType pierID) const = 0;

   // Distance form alignment to bridge line, measured normal to the alignment
   virtual void SetBridgeLineOffset(PierIDType pierID,Float64 blo) = 0;
   virtual Float64 GetBridgeLineOffset(PierIDType pierID) const = 0;

   // Orientation of the pier
   virtual void SetOrientation(PierIDType pierID,LPCTSTR strOrientation) = 0;
   virtual LPCTSTR GetOrientation(PierIDType pierID) const = 0;

   // Sets the basis for the curb lines
   virtual pgsTypes::OffsetMeasurementType GetCurbLineDatum(PierIDType pierID) const = 0;
   virtual void SetCurbLineDatum(PierIDType pierID,pgsTypes::OffsetMeasurementType datumType) = 0;

   // Location of the curb lines, measured from and normal to the alignment
   virtual void SetCurbLineOffset(PierIDType pierID,Float64 leftCLO,Float64 rightCLO) = 0;
   virtual void GetCurbLineOffset(PierIDType pierID,Float64* pLeftCLO,Float64* pRightCLO) const = 0;

   // Crown slopes measured normal to the alignment
   virtual void SetCrownSlopes(PierIDType pierID,Float64 sl,Float64 sr) = 0;
   virtual void GetCrownSlopes(PierIDType pierID,Float64* psl,Float64* psr) const = 0;

   virtual void GetDiaphragmDimensions(PierIDType pierID,Float64* pH,Float64* pW) const = 0;
   virtual void SetDiaphragmDimensions(PierIDType pierID,Float64 H,Float64 W) = 0;

   // Number of bearing lines at the pier. Valid values are 1 and 2.
   // Use 1 when girders are continuous (e.g. spliced girder, steel girders, etc)
   // Use 2 when simple span girders are made continuous (or are just simple spans)
   virtual IndexType GetBearingLineCount(PierIDType pierID) const = 0;
   virtual void SetBearingLineCount(PierIDType pierID,IndexType nBearingLines) = 0;

   // Number of bearings on a bearing line. Some beam types, such as U-beams, use two
   // bearings and others, such as I-beams, use one bearing. This is the total number
   // of points of bearing along the bearing line
   virtual IndexType GetBearingCount(PierIDType pierID,IndexType brgLineIdx) const = 0;
   virtual void SetBearingCount(PierIDType pierID,IndexType brgLineIdx,IndexType nBearings) = 0;

   // Spacing between the specified bearing and the bearing to its right
   virtual Float64 GetBearingSpacing(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx) const = 0;
   virtual void SetBearingSpacing(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx,Float64 spacing) = 0;

   // Bearing reactions
   virtual void SetBearingReactionType(PierIDType pierID,IndexType brgLineIdx,xbrTypes::ReactionLoadType brgReactionType) = 0;
   virtual xbrTypes::ReactionLoadType GetBearingReactionType(PierIDType pierID,IndexType brgLineIdx) const = 0;
   // W = bearing reaction width if uniform load (e.g. slabs) ... use W = 0 for point load reactions
   virtual void SetBearingReactions(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx,Float64 DC,Float64 DW,Float64 CR,Float64 SH,Float64 PS,Float64 RE,Float64 W) = 0;
   virtual void GetBearingReactions(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx,Float64* pDC,Float64* pDW,Float64* pCR,Float64* pSH,Float64* pPS,Float64* pRE,Float64* pW) const = 0;

   // returns the width of the bearing reaction, without having to compute the reaction values when extending PGS
   virtual Float64 GetBearingWidth(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx) const = 0;

   // Reference bearing
   virtual void GetReferenceBearing(PierIDType pierID,IndexType brgLineIdx,IndexType* pRefIdx,Float64* pRefBearingOffset,pgsTypes::OffsetMeasurementType* pRefBearingDatum) const = 0;
   virtual void SetReferenceBearing(PierIDType pierID,IndexType brgLineIdx,IndexType refIdx,Float64 refBearingOffset,pgsTypes::OffsetMeasurementType refBearingDatum) = 0;

   // Reaction load application type
   virtual void SetReactionLoadApplicationType(PierIDType pierID,xbrTypes::ReactionLoadApplicationType applicationType) = 0;
   virtual xbrTypes::ReactionLoadApplicationType GetReactionLoadApplicationType(PierIDType pierID) const = 0;

   // Live Load Reactions per lane
   virtual IndexType GetLiveLoadReactionCount(PierIDType pierID,pgsTypes::LoadRatingType ratingType) const = 0;
   virtual void SetLiveLoadReactions(PierIDType pierID,pgsTypes::LoadRatingType ratingType,const std::vector<xbrLiveLoadReactionData>& vLLIM) = 0;
   virtual std::vector<xbrLiveLoadReactionData> GetLiveLoadReactions(PierIDType pierID,pgsTypes::LoadRatingType ratingType) const = 0;
   virtual std::_tstring GetLiveLoadName(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const = 0;
   virtual Float64 GetLiveLoadReaction(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const = 0;
   virtual Float64 GetVehicleWeight(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const = 0;

   // Material properties
   virtual void SetRebarMaterial(PierIDType pierID,WBFL::Materials::Rebar::Type type,WBFL::Materials::Rebar::Grade grade) = 0;
   virtual void GetRebarMaterial(PierIDType pierID,WBFL::Materials::Rebar::Type* pType,WBFL::Materials::Rebar::Grade* pGrade) const = 0;

   virtual void SetConcrete(PierIDType pierID,const CConcreteMaterial& concrete) = 0;
   virtual const CConcreteMaterial& GetConcrete(PierIDType pierID) const = 0;

   virtual void SetLowerXBeamDimensions(PierIDType pierID,Float64 h1,Float64 h2,Float64 h3,Float64 h4,Float64 x1,Float64 x2,Float64 x3,Float64 x4,Float64 w) = 0;
   virtual void GetLowerXBeamDimensions(PierIDType pierID,Float64* ph1,Float64* ph2,Float64* ph3,Float64* ph4,Float64* px1,Float64* px2,Float64* px3,Float64* px4,Float64* pw) const = 0;

   virtual Float64 GetXBeamLeftOverhang(PierIDType pierID) const = 0;
   virtual Float64 GetXBeamRightOverhang(PierIDType pierID) const = 0;
   virtual Float64 GetXBeamWidth(PierIDType pierID) const = 0;

   virtual void SetRefColumnLocation(PierIDType pierID,pgsTypes::OffsetMeasurementType refColumnDatum,IndexType refColumnIdx,Float64 refColumnOffset) = 0;
   virtual void GetRefColumnLocation(PierIDType pierID,pgsTypes::OffsetMeasurementType* prefColumnDatum,IndexType* prefColumnIdx,Float64* prefColumnOffset) const = 0;
   virtual IndexType GetColumnCount(PierIDType pierID) const = 0;
   virtual Float64 GetColumnHeight(PierIDType pierID,ColumnIndexType colIdx) const = 0;
   virtual CColumnData::ColumnHeightMeasurementType GetColumnHeightMeasurementType(PierIDType pierID,ColumnIndexType colIdx) const = 0;
   virtual Float64 GetColumnSpacing(PierIDType pierID,SpacingIndexType spaceIdx) const = 0;
   virtual pgsTypes::ColumnTransverseFixityType GetColumnFixity(PierIDType pierID,ColumnIndexType colIdx) const = 0;

   virtual void SetColumnProperties(PierIDType pierID,ColumnIndexType colIdx,CColumnData::ColumnShapeType shapeType,Float64 D1,Float64 D2,CColumnData::ColumnHeightMeasurementType heightType,Float64 H) = 0;
   virtual void GetColumnProperties(PierIDType pierID,ColumnIndexType colIdx,CColumnData::ColumnShapeType* pshapeType,Float64* pD1,Float64* pD2,CColumnData::ColumnHeightMeasurementType* pheightType,Float64* pH) const = 0;

   // Longitudinal Rebar
   virtual const xbrLongitudinalRebarData& GetLongitudinalRebar(PierIDType pierID) const = 0;
   virtual void SetLongitudinalRebar(PierIDType pierID,const xbrLongitudinalRebarData& rebar) = 0;

   // Stirrups
   virtual void SetLowerXBeamStirrups(PierIDType pierID,const xbrStirrupData& stirrups) = 0;
   virtual const xbrStirrupData& GetLowerXBeamStirrups(PierIDType pierID) const = 0;
   virtual void SetFullDepthStirrups(PierIDType pierID,const xbrStirrupData& stirrups) = 0;
   virtual const xbrStirrupData& GetFullDepthStirrups(PierIDType pierID) const = 0;

   // Resistance factors for compression controlled and tension controlled sections
   // LRFD 5.5.4.2.1
   virtual void SetFlexureResistanceFactors(Float64 phiC,Float64 phiT) = 0;
   virtual void GetFlexureResistanceFactors(Float64* phiC,Float64* phiT) const = 0;

   // Resistance factors for shear LRFD 5.5.4.2.1
   virtual void SetShearResistanceFactor(Float64 phi) = 0;
   virtual Float64 GetShearResistanceFactor() const = 0;

   virtual void SetSystemFactorFlexure(Float64 sysFactor) = 0;
   virtual Float64 GetSystemFactorFlexure() const = 0;
   
   virtual void SetSystemFactorShear(Float64 sysFactor) = 0;
   virtual Float64 GetSystemFactorShear() const = 0;

   virtual void SetConditionFactor(PierIDType pierID,pgsTypes::ConditionFactorType conditionFactorType,Float64 conditionFactor) = 0;
   virtual void GetConditionFactor(PierIDType pierID,pgsTypes::ConditionFactorType* pConditionFactorType,Float64 *pConditionFactor) const = 0;
   virtual Float64 GetConditionFactor(PierIDType pierID) const = 0;

   virtual void SetDCLoadFactor(pgsTypes::LimitState limitState,Float64 dc) = 0;
   virtual Float64 GetDCLoadFactor(pgsTypes::LimitState limitState) const = 0;

   virtual void SetDWLoadFactor(pgsTypes::LimitState limitState,Float64 dw) = 0;
   virtual Float64 GetDWLoadFactor(pgsTypes::LimitState limitState) const = 0;

   virtual void SetCRLoadFactor(pgsTypes::LimitState limitState,Float64 cr) = 0;
   virtual Float64 GetCRLoadFactor(pgsTypes::LimitState limitState) const = 0;

   virtual void SetSHLoadFactor(pgsTypes::LimitState limitState,Float64 sh) = 0;
   virtual Float64 GetSHLoadFactor(pgsTypes::LimitState limitState) const = 0;

   virtual void SetRELoadFactor(pgsTypes::LimitState limitState,Float64 re) = 0;
   virtual Float64 GetRELoadFactor(pgsTypes::LimitState limitState) const = 0;

   virtual void SetPSLoadFactor(pgsTypes::LimitState limitState,Float64 ps) = 0;
   virtual Float64 GetPSLoadFactor(pgsTypes::LimitState limitState) const = 0;

   virtual void SetLiveLoadFactor(PierIDType pierID,pgsTypes::LimitState limitState,Float64 ll) = 0;
   virtual Float64 GetLiveLoadFactor(PierIDType pierID,pgsTypes::LimitState limitState,VehicleIndexType vehicleIdx) const = 0;

   virtual void SetMaxLiveLoadStepSize(Float64 stepSize) = 0;
   virtual Float64 GetMaxLiveLoadStepSize() const = 0;

   virtual void SetMaxLoadedLanes(IndexType nLanesMax) = 0;
   virtual IndexType GetMaxLoadedLanes() const = 0;

   // returns true if negative moment not to be anlyzed between FOC if column width .gt. minimum value
   virtual bool GetDoAnalyzeNegativeMomentBetweenFocOptions(Float64* minColumnWidth) = 0;
   virtual void SetDoAnalyzeNegativeMomentBetweenFocOptions(bool bDoUseOption, Float64 minColumnWidth) = 0;
};

/*****************************************************************************
INTERFACE
   IXBRProjectEventSink

DESCRIPTION
   Callback interface for changes to the project data
*****************************************************************************/
// {9DD03140-A788-4e46-A283-0B343956A619}
DEFINE_GUID(IID_IXBRProjectEventSink, 
0x9dd03140, 0xa788, 0x4e46, 0xa2, 0x83, 0xb, 0x34, 0x39, 0x56, 0xa6, 0x19);
class IXBRProjectEventSink
{
public:
   virtual HRESULT OnProjectChanged() = 0;
};

/*****************************************************************************
INTERFACE
   IXBRProjectEdit

DESCRIPTION
   Interface to enable programmatic activation of the editing UI
*****************************************************************************/
// {BBA7F95B-A5DE-4c2a-BB68-19983F308767}
DEFINE_GUID(IID_IXBRProjectEdit, 
0xbba7f95b, 0xa5de, 0x4c2a, 0xbb, 0x68, 0x19, 0x98, 0x3f, 0x30, 0x87, 0x67);
class IXBRProjectEdit
{
public:
   virtual void EditPier(int nPage) = 0;
   virtual void EditOptions() = 0;
};


/*****************************************************************************
INTERFACE
   IXBREvents

   Interface to control events

DESCRIPTION
   Interface to control events
*****************************************************************************/
// {F0674DBA-E867-4692-B214-FDB23F04685B}
DEFINE_GUID(IID_IXBREvents, 
0xf0674dba, 0xe867, 0x4692, 0xb2, 0x14, 0xfd, 0xb2, 0x3f, 0x4, 0x68, 0x5b);
class IXBREvents
{
public:
   virtual void HoldEvents() = 0;
   virtual void FirePendingEvents() = 0;
   virtual void CancelPendingEvents() = 0;
};

/*****************************************************************************
INTERFACE
   IXBREventsSink

   Interface to control events

DESCRIPTION
   Interface to control events
*****************************************************************************/
// {46E15C3D-F822-4b97-A7E3-6ED0CE5FF37E}
DEFINE_GUID(IID_IXBREventsSink, 
0x46e15c3d, 0xf822, 0x4b97, 0xa7, 0xe3, 0x6e, 0xd0, 0xce, 0x5f, 0xf3, 0x7e);
class IXBREventsSink
{
public:
   virtual HRESULT OnHoldEvents() = 0;
   virtual HRESULT OnFirePendingEvents() = 0;
   virtual HRESULT OnCancelPendingEvents() = 0;
};

/*****************************************************************************
INTERFACE
   IXBRUIEvents

   Interface to control events in the user interface

DESCRIPTION
   Interface to control events in the user interface
*****************************************************************************/
DEFINE_GUID(IID_IXBRUIEvents, 
0xb2734352, 0xc900, 0x4c92, 0x9b, 0x68, 0x94, 0x84, 0xfc, 0x58, 0x1a, 0x1b);
class IXBRUIEvents
{
public:
   virtual void HoldEvents(bool bHold=true) = 0;
   virtual void FirePendingEvents() = 0;
   virtual void CancelPendingEvents() = 0;
   virtual void FireEvent(CView* pSender = nullptr,LPARAM lHint = 0,std::shared_ptr<CObject> pHint = nullptr) = 0;
};

/*****************************************************************************
INTERFACE
   IXBRExport

   Interface to export a PGS pier model to a stand alone file
*****************************************************************************/
// {DD3B518E-1CFE-45b7-AA7B-426ADE3138D6}
DEFINE_GUID(IID_IXBRExport, 
0xdd3b518e, 0x1cfe, 0x45b7, 0xaa, 0x7b, 0x42, 0x6a, 0xde, 0x31, 0x38, 0xd6);
class IXBRExport
{
public:
   // Exports the pier model, for the specified pier, into a stand alone XBRate file
   // if pierIdx is INVALID_INDEX, the user is prompted to select the pier model to export
   virtual HRESULT Export(PierIndexType pierIdx) = 0;

   // Exports several pier models into stand alone XBRate files
   // the user is prompted to select one or more piers to export
   virtual HRESULT BatchExport() = 0;
};
