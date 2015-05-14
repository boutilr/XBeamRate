#pragma once

#include <PgsExt\ColumnData.h>

/*****************************************************************************
INTERFACE
   IXBRProject

DESCRIPTION
   Interface for getting this application going.
*****************************************************************************/
// {2600A729-D7E6-44f6-9F9B-DF086FF9E53B}
DEFINE_GUID(IID_IXBRProject, 
0x2600a729, 0xd7e6, 0x44f6, 0x9f, 0x9b, 0xdf, 0x8, 0x6f, 0xf9, 0xe5, 0x3b);
interface IXBRProject : IUnknown
{
   virtual void SetProjectName(LPCTSTR strName) = 0;
   virtual LPCTSTR GetProjectName() = 0;

   virtual xbrTypes::PierConnectionType GetPierType() = 0;
   virtual void SetPierType(xbrTypes::PierConnectionType pierType) = 0;

   // Elevation of the deck on the alignment at the CL Pier
   virtual void SetDeckElevation(Float64 deckElevation) = 0;
   virtual Float64 GetDeckElevation() = 0;

   // Distance from alignment to crown point.
   virtual void SetCrownPointOffset(Float64 cpo) = 0;
   virtual Float64 GetCrownPointOffset() = 0;

   // Distance form alignemnt to bridge line
   virtual void SetBridgeLineOffset(Float64 blo) = 0;
   virtual Float64 GetBridgeLineOffset() = 0;

   // Orientation of the pier
   virtual void SetOrientation(LPCTSTR strOrientation) = 0;
   virtual LPCTSTR GetOrientation() = 0;

   // Sets the basis for the curb lines
   virtual pgsTypes::OffsetMeasurementType GetCurbLineDatum() = 0;
   virtual void SetCurbLineDatum(pgsTypes::OffsetMeasurementType datumType) = 0;

   virtual void SetCurbLineOffset(Float64 leftCLO,Float64 rightCLO) = 0;
   virtual void GetCurbLineOffset(Float64* pLeftCLO,Float64* pRightCLO) = 0;

   virtual void SetCrownSlopes(Float64 sl,Float64 sr) = 0;
   virtual void GetCrownSlopes(Float64* psl,Float64* psr) = 0;

   virtual void GetDiaphragmDimensions(Float64* pH,Float64* pW) = 0;
   virtual void SetDiaphragmDimensions(Float64 H,Float64 W) = 0;

   // Number of bearing lines at the pier. Valid values are 1 and 2.
   // Use 1 when girders are continuous (e.g. spliced girder, steel girders, etc)
   // Use 2 when simple span girders are made continuous (or are just simple spans)
   virtual IndexType GetBearingLineCount() = 0;
   virtual void SetBearingLineCount(IndexType nBearingLines) = 0;

   // Number of bearings on a bearing line. Some beam types, such as U-beams, use two
   // bearings and others, such as I-beams, use one bearing. This is the total number
   // of points of bearing along the bearing line
   virtual IndexType GetBearingCount(IndexType brgLineIdx) = 0;
   virtual void SetBearingCount(IndexType brgLineIdx,IndexType nBearings) = 0;

   // Spacing between the specified bearing and the bearing to its right
   virtual Float64 GetBearingSpacing(IndexType brgLineIdx,IndexType brgIdx) = 0;
   virtual void SetBearingSpacing(IndexType brgLineIdx,IndexType brgIdx,Float64 spacing) = 0;

   // Bearing reactions
   virtual void SetBearingReactions(IndexType brgLineIdx,IndexType brgIdx,Float64 DC,Float64 DW) = 0;
   virtual void GetBearingReactions(IndexType brgLineIdx,IndexType brgIdx,Float64* pDC,Float64* pDW) = 0;

   // Reference bearing
   virtual void GetReferenceBearing(IndexType brgLineIdx,IndexType* pRefIdx,Float64* pRefBearingOffset,pgsTypes::OffsetMeasurementType* pRefBearingDatum) = 0;
   virtual void SetReferenceBearing(IndexType brgLineIdx,IndexType refIdx,Float64 refBearingOffset,pgsTypes::OffsetMeasurementType refBearingDatum) = 0;

   // Live Load Reactions per lane
   virtual IndexType GetLiveLoadReactionCount(pgsTypes::LiveLoadType liveLoadType) = 0;
   virtual void SetLiveLoadReactions(pgsTypes::LiveLoadType liveLoadType,const std::vector<std::pair<std::_tstring,Float64>>& vLLIM) = 0;
   virtual std::vector<std::pair<std::_tstring,Float64>> GetLiveLoadReactions(pgsTypes::LiveLoadType liveLoadType) = 0;

   // Material properties of the sub-structure concrete
   virtual void SetModE(Float64 Ec) = 0;
   virtual Float64 GetModE() = 0;

   virtual void SetXBeamDimensions(pgsTypes::PierSideType side,Float64 height,Float64 taperHeight,Float64 taperLength) = 0;
   virtual void GetXBeamDimensions(pgsTypes::PierSideType side,Float64* pHeight,Float64* pTaperHeight,Float64* pTaperLength) = 0;
   virtual void SetXBeamWidth(Float64 width) = 0;
   virtual Float64 GetXBeamWidth() = 0;
   virtual void SetXBeamOverhang(pgsTypes::PierSideType side,Float64 overhang) = 0;
   virtual void SetXBeamOverhangs(Float64 leftOverhang,Float64 rightOverhang) = 0;
   virtual Float64 GetXBeamOverhang(pgsTypes::PierSideType side) = 0; 
   virtual void GetXBeamOverhangs(Float64* pLeftOverhang,Float64* pRightOverhang) = 0;

   virtual void SetColumns(IndexType nColumns,Float64 height,CColumnData::ColumnHeightMeasurementType heightMeasure,Float64 spacing) = 0;
   virtual IndexType GetColumnCount() = 0;
   virtual Float64 GetColumnHeight(IndexType colIdx) = 0;
   virtual CColumnData::ColumnHeightMeasurementType GetColumnHeightMeasurementType() = 0;
   virtual xbrTypes::ColumnBaseType GetColumnBaseType(IndexType colIdx) = 0;
   virtual Float64 GetSpacing(IndexType spaceIdx) = 0;
   virtual void SetColumnShape(CColumnData::ColumnShapeType shapeType,Float64 D1,Float64 D2) = 0;
   virtual void GetColumnShape(CColumnData::ColumnShapeType* pShapeType,Float64* pD1,Float64* pD2) = 0;

   virtual void SetTransverseLocation(ColumnIndexType refColIdx,Float64 offset,pgsTypes::OffsetMeasurementType measure) = 0;
   virtual void GetTransverseLocation(ColumnIndexType* pRefColIdx,Float64* pOffset,pgsTypes::OffsetMeasurementType* pMeasure) = 0;

   virtual Float64 GetXBeamLength() = 0;

   // Longitudinal Rebar
   virtual IndexType GetRebarRowCount() = 0;
   virtual void AddRebarRow(xbrTypes::LongitudinalRebarDatumType datum,Float64 cover,matRebar::Size barSize,Int16 nBars,Float64 spacing) = 0;
   virtual void SetRebarRow(IndexType rowIdx,xbrTypes::LongitudinalRebarDatumType datum,Float64 cover,matRebar::Size barSize,Int16 nBars,Float64 spacing) = 0;
   virtual void GetRebarRow(IndexType rowIdx,xbrTypes::LongitudinalRebarDatumType* pDatum,Float64* pCover,matRebar::Size* pBarSize,Int16* pnBars,Float64* pSpacing) = 0;
   virtual void RemoveRebarRow(IndexType rowIdx) = 0;
   virtual void RemoveRebarRows() = 0;
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
interface IXBRProjectEventSink : IUnknown
{
   virtual HRESULT OnProjectChanged() = 0;
};

/*****************************************************************************
INTERFACE
   IXBRProjectEdit

DESCRIPTION
   Interface to enable programatic activation of the editing UI
*****************************************************************************/
// {BBA7F95B-A5DE-4c2a-BB68-19983F308767}
DEFINE_GUID(IID_IXBRProjectEdit, 
0xbba7f95b, 0xa5de, 0x4c2a, 0xbb, 0x68, 0x19, 0x98, 0x3f, 0x30, 0x87, 0x67);
interface IXBRProjectEdit : IUnknown
{
   virtual void EditPier(int nPage) = 0;
};
