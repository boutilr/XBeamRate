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
#include <XBeamRateExt\PierData.h>
#include <WBFLGenericBridge.h>





xbrPierData::xbrPierData()
{
   m_ID = INVALID_ID;
   m_ConnectionType = pgsTypes::pctIntegral;
   m_strSkew = _T("00 00 00.0 L");
   m_BridgeLineOffset = 0;
   m_CurbLineDatum = pgsTypes::omtAlignment;
   m_LeftCurbOffset  = -WBFL::Units::ConvertToSysUnits(10.0,WBFL::Units::Measure::Feet);
   m_RightCurbOffset = WBFL::Units::ConvertToSysUnits(10.0,WBFL::Units::Measure::Feet);

   m_tDeck = WBFL::Units::ConvertToSysUnits(8.0,WBFL::Units::Measure::Inch);

   m_PierLayoutType = pgsTypes::pltCommon;

   m_DeckSurfaceType = Simplified;
   m_DeckElevation = 0;
   m_CrownPointOffset = 0;
   m_LeftCrownSlope = -0.02;
   m_RightCrownSlope = -0.02;

   m_H = WBFL::Units::ConvertToSysUnits(3,WBFL::Units::Measure::Feet);
   m_W = WBFL::Units::ConvertToSysUnits(5,WBFL::Units::Measure::Feet);

   m_H1L = WBFL::Units::ConvertToSysUnits(5,WBFL::Units::Measure::Feet);
   m_H1R = WBFL::Units::ConvertToSysUnits(5,WBFL::Units::Measure::Feet);
   m_H2L = WBFL::Units::ConvertToSysUnits(0,WBFL::Units::Measure::Feet);
   m_H2R = WBFL::Units::ConvertToSysUnits(0,WBFL::Units::Measure::Feet);
   m_X1L = WBFL::Units::ConvertToSysUnits(0,WBFL::Units::Measure::Feet);
   m_X1R = WBFL::Units::ConvertToSysUnits(0,WBFL::Units::Measure::Feet);
   m_X2L = WBFL::Units::ConvertToSysUnits(0,WBFL::Units::Measure::Feet);
   m_X2R = WBFL::Units::ConvertToSysUnits(0,WBFL::Units::Measure::Feet);
   m_XW = WBFL::Units::ConvertToSysUnits(5,WBFL::Units::Measure::Feet);

   m_OverhangRadius[pgsTypes::stLeft] = WBFL::Units::ConvertToSysUnits(10.0, WBFL::Units::Measure::Feet);
   m_OverhangRadius[pgsTypes::stRight] = WBFL::Units::ConvertToSysUnits(10.0, WBFL::Units::Measure::Feet);
   m_ScallopDepths.push_back(WBFL::Units::ConvertToSysUnits(5.0, WBFL::Units::Measure::Feet));

   CColumnData column;
   m_vColumnData.push_back(column);

   m_RefColumnIdx = 0;
   m_RefColumnOffset = 0;
   m_RefColumnDatum = pgsTypes::omtAlignment;

   m_ColumnFixity = pgsTypes::cftFixed;

   m_OHL = WBFL::Units::ConvertToSysUnits(10,WBFL::Units::Measure::Feet);
   m_OHR = WBFL::Units::ConvertToSysUnits(10,WBFL::Units::Measure::Feet);

   // Load Rating Condition
   m_ConditionFactorType = pgsTypes::cfGood;
   m_ConditionFactor = 1.0;

   // Materials
   m_StirrupRebarType = WBFL::Materials::Rebar::Type::A615;
   m_StirrupRebarGrade = WBFL::Materials::Rebar::Grade::Grade60;

   //// Rebar
   //xbrLongitudinalRebarData::RebarRow row;
   //row.Datum = xbrTypes::Bottom;
   //row.LayoutType = xbrTypes::blFullLength;
   //row.BarSize = WBFL::Materials::Rebar::Size::bs11;
   //row.BarSpacing = WBFL::Units::ConvertToSysUnits(6,WBFL::Units::Measure::Inch);
   //row.Cover = WBFL::Units::ConvertToSysUnits(2,WBFL::Units::Measure::Inch);
   //row.NumberOfBars = 5;
   //m_LongitudinalRebar.RebarRows.push_back(row);

   // Bearings
   xbrBearingLineData bearingLine;
   bearingLine.SetReferenceBearing(pgsTypes::omtAlignment,0,WBFL::Units::ConvertToSysUnits(0,WBFL::Units::Measure::Feet));
   bearingLine.SetBearingCount(1);
   m_vBearingLines.push_back(bearingLine);

   m_Concrete.StrengthDensity = WBFL::Units::ConvertToSysUnits(0.145, WBFL::Units::Measure::KipPerFeet3); // plan concrete LRFD Table 3.5.1-1
   m_Concrete.WeightDensity = WBFL::Units::ConvertToSysUnits(0.155, WBFL::Units::Measure::KipPerFeet3); // 10 kcf allowance for reinforcement (BDM Ch 3).
}

xbrPierData::xbrPierData(const xbrPierData& other)
{
   MakeCopy(other);
}

xbrPierData::~xbrPierData()
{
}

xbrPierData& xbrPierData::operator= (const xbrPierData& rOther)
{
   if( this != &rOther )
   {
      MakeAssignment(rOther);
   }

   return *this;
}

void xbrPierData::SetID(PierIDType id)
{
   m_ID = id;
}

PierIDType xbrPierData::GetID() const
{
   return m_ID;
}

pgsTypes::PierLayoutType xbrPierData::GetPierLayoutType() const
{
    return m_PierLayoutType;
}

void xbrPierData::SetPierLayoutType(pgsTypes::PierLayoutType layoutType)
{
    m_PierLayoutType = layoutType;
}

void xbrPierData::SetSkew(LPCTSTR strSkew)
{
   m_strSkew = strSkew;
}

LPCTSTR xbrPierData::GetSkew() const
{
   return m_strSkew.c_str();
}

void xbrPierData::SetDeckElevation(Float64 elev)
{
   m_DeckElevation = elev;
}

Float64 xbrPierData::GetDeckElevation() const
{
   return m_DeckElevation;
}

Float64& xbrPierData::GetDeckElevation()
{
   return m_DeckElevation;
}

void xbrPierData::SetDeckThickness(Float64 tDeck)
{
   m_tDeck = tDeck;
}

Float64 xbrPierData::GetDeckThickness() const
{
   return m_tDeck;
}

Float64& xbrPierData::GetDeckThickness()
{
   return m_tDeck;
}

void xbrPierData::SetDeckSurfaceType(xbrPierData::DeckSurfaceType surfaceType)
{
   m_DeckSurfaceType = surfaceType;
}

xbrPierData::DeckSurfaceType xbrPierData::GetDeckSurfaceType() const
{
   return m_DeckSurfaceType;
}

void xbrPierData::SetDeckProfile(IPoint2dCollection* pProfile)
{
   m_DeckProfile.Release();
   pProfile->Clone(&m_DeckProfile);
}

void xbrPierData::GetDeckProfile(IPoint2dCollection** ppProfile) const
{
   m_DeckProfile->Clone(ppProfile);
}

void xbrPierData::SetCrownPointOffset(Float64 cpo)
{
   m_CrownPointOffset = cpo;
}

Float64 xbrPierData::GetCrownPointOffset() const
{
   return m_CrownPointOffset;
}

Float64& xbrPierData::GetCrownPointOffset()
{
   return m_CrownPointOffset;
}

void xbrPierData::SetBridgeLineOffset(Float64 blo)
{
   m_BridgeLineOffset = blo;
}

Float64 xbrPierData::GetBridgeLineOffset() const
{
   return m_BridgeLineOffset;
}

Float64& xbrPierData::GetBridgeLineOffset()
{
   return m_BridgeLineOffset;
}

void xbrPierData::SetPierType(pgsTypes::PierType type)
{
   m_ConnectionType = type;
}

pgsTypes::PierType xbrPierData::GetPierType() const
{
   return m_ConnectionType;
}

pgsTypes::PierType& xbrPierData::GetPierType()
{
   return m_ConnectionType;
}

pgsTypes::OffsetMeasurementType xbrPierData::GetCurbLineDatum() const
{
   return m_CurbLineDatum;
}

pgsTypes::OffsetMeasurementType& xbrPierData::GetCurbLineDatum()
{
   return m_CurbLineDatum;
}

void xbrPierData::SetCurbLineDatum(pgsTypes::OffsetMeasurementType datumType)
{
   m_CurbLineDatum = datumType;
}

void xbrPierData::SetCurbLineOffset(Float64 leftCLO,Float64 rightCLO)
{
   m_LeftCurbOffset = leftCLO;
   m_RightCurbOffset = rightCLO;
}

void xbrPierData::GetCurbLineOffset(Float64* pLeftCLO,Float64* pRightCLO) const
{
   *pLeftCLO = m_LeftCurbOffset;
   *pRightCLO = m_RightCurbOffset;
}

Float64& xbrPierData::GetLeftCurbLineOffset()
{
   return m_LeftCurbOffset;
}

Float64& xbrPierData::GetRightCurbLineOffset()
{
   return m_RightCurbOffset;
}

void xbrPierData::SetCrownSlope(Float64 sLeft,Float64 sRight)
{
   m_LeftCrownSlope = sLeft;
   m_RightCrownSlope = sRight;
}

void xbrPierData::GetCrownSlope(Float64* psLeft,Float64* psRight) const
{
   *psLeft = m_LeftCrownSlope;
   *psRight = m_RightCrownSlope;
}

Float64& xbrPierData::GetLeftCrownSlope()
{
   return m_LeftCrownSlope;
}

Float64& xbrPierData::GetRightCrownSlope()
{
   return m_RightCrownSlope;
}

void xbrPierData::SetDiaphragmDimensions(Float64 h,Float64 w)
{
   m_H = h;
   m_W = w;
}

void xbrPierData::GetDiaphragmDimensions(Float64* ph,Float64* pw) const
{
   *ph = m_H;
   *pw = m_W;
}

Float64& xbrPierData::GetDiaphragmHeight()
{
   return m_H;
}

Float64& xbrPierData::GetDiaphragmWidth()
{
   return m_W;
}

void xbrPierData::SetLowerXBeamDimensions(Float64 h1l,Float64 h1r,Float64 h2l,Float64 h2r,Float64 x1l, Float64 x1r, Float64 x2l,Float64 x2r,Float64 w, std::vector<CPierPointData> vpp)
{
   m_H1L = h1l;
   m_H1R = h1r;
   m_H2L = h2l;
   m_H2R = h2r;

   m_X1L = x1l;
   m_X1R = x1r;
   m_X2L = x2l;
   m_X2R = x2r;

   m_XW = w;

   m_vPierPoints = vpp;
}

void xbrPierData::GetLowerXBeamDimensions(Float64* ph1l, Float64* ph1r, Float64* ph2l, Float64* ph2r, Float64* px1l, Float64* px1r, Float64* px2l, Float64* px2r, Float64* pw, std::vector<CPierPointData>* pvpp) const
{
   *ph1l = m_H1L;
   *ph1r = m_H1R;
   *ph2l = m_H2L;
   *ph2r = m_H2R;

   *px1l = m_X1L;
   *px1r = m_X1R;
   *px2l = m_X2L;
   *px2r = m_X2R;

   *pw  = m_XW;

   *pvpp = m_vPierPoints;
}

Float64& xbrPierData::GetH1L()
{
   return m_H1L;
}

Float64& xbrPierData::GetH1R()
{
   return m_H1R;
}

Float64& xbrPierData::GetH2L()
{
   return m_H2L;
}

Float64& xbrPierData::GetH2R()
{
   return m_H2R;
}

Float64& xbrPierData::GetX1L()
{
   return m_X1L;
}

Float64& xbrPierData::GetX1R()
{
   return m_X1R;
}

Float64& xbrPierData::GetX2L()
{
   return m_X2L;
}

Float64& xbrPierData::GetX2R()
{
   return m_X2R;
}

Float64& xbrPierData::GetW()
{
   return m_XW;
}

void xbrPierData::SetPierPointData(PierPointIndexType ppIdx, const CPierPointData& ppData)
{
    m_vPierPoints[ppIdx] = ppData;
}

const std::vector<CPierPointData>& xbrPierData::GetPierPointData() const
{
   return m_vPierPoints;
}

void xbrPierData::SetPierPointCount(PierPointIndexType nPierPoints)
{
    if (nPierPoints < m_vPierPoints.size())
    {
        m_vPierPoints.erase(m_vPierPoints.begin() + nPierPoints, m_vPierPoints.end());
    }
    else if (m_vPierPoints.size() < nPierPoints)
    {
        // the number of pier points is being increased... add pier points on the right side of the pier
        PierPointIndexType nPierPointsToAdd = nPierPoints - m_vPierPoints.size();
        CPierPointData pp;
        if (!m_vPierPoints.empty())
        {
            pp = m_vPierPoints.back(); // right-most pier point
            Float64 spacing = WBFL::Units::ConvertToSysUnits(4.0, WBFL::Units::Measure::Feet);
            pp.Set_X(pp.Get_X() + spacing); // set the new pier point to be 4 feet to the right of the last pier point
        }
        m_vPierPoints.insert(m_vPierPoints.end(), nPierPointsToAdd, pp);
    }
}

void xbrPierData::SetColumnFixity(pgsTypes::ColumnLongitudinalBaseFixityType fixityType)
{
    m_ColumnFixity = fixityType;
}

pgsTypes::ColumnLongitudinalBaseFixityType xbrPierData::GetColumnFixity() const
{
    return m_ColumnFixity;
}

PierPointIndexType xbrPierData::GetPierPointCount() const
{
    return m_vPierPoints.size();
}

void xbrPierData::SetRefColumnLocation(pgsTypes::OffsetMeasurementType refColumnDatum,IndexType refColumnIdx,Float64 refColumnOffset)
{
   m_RefColumnDatum  = refColumnDatum;
   m_RefColumnIdx    = refColumnIdx;
   m_RefColumnOffset = refColumnOffset;
}

void xbrPierData::GetRefColumnLocation(pgsTypes::OffsetMeasurementType* prefColumnDatum,IndexType* prefColumnIdx,Float64* prefColumnOffset) const
{
   *prefColumnDatum  = m_RefColumnDatum;
   *prefColumnIdx    = m_RefColumnIdx;
   *prefColumnOffset = m_RefColumnOffset;
}

pgsTypes::OffsetMeasurementType& xbrPierData::GetColumnLayoutDatum()
{
   return m_RefColumnDatum;
}

IndexType& xbrPierData::GetRefColumnIndex()
{
   return m_RefColumnIdx;
}

Float64& xbrPierData::GetRefColumnOffset()
{
   return m_RefColumnOffset;
}

void xbrPierData::SetColumnCount(ColumnIndexType nColumns)
{
   ATLASSERT(0 < nColumns && nColumns != INVALID_INDEX);
   if (nColumns == 0 || nColumns == INVALID_INDEX)
      return;

   const CColumnData column = m_vColumnData.empty() ? CColumnData() : m_vColumnData.back();
   const Float64 spacing = m_vColumnSpacing.empty()
      ? WBFL::Units::ConvertToSysUnits(10.0, WBFL::Units::Measure::Feet)
      : m_vColumnSpacing.back();
   m_vColumnData.resize(nColumns, column);
   m_vColumnSpacing.resize(nColumns - 1, spacing);
   m_ScallopDepths.resize(nColumns, WBFL::Units::ConvertToSysUnits(5.0, WBFL::Units::Measure::Feet));
   m_ScallopRadii.resize(nColumns - 1, WBFL::Units::ConvertToSysUnits(10.0, WBFL::Units::Measure::Feet));
}

ColumnIndexType xbrPierData::GetColumnCount() const
{
   return m_vColumnData.size();
}

void xbrPierData::AddColumn(const CColumnData& columnData,Float64 spacing)
{
   m_vColumnData.push_back(columnData);
   m_vColumnSpacing.push_back(spacing);
   m_ScallopDepths.push_back(WBFL::Units::ConvertToSysUnits(5.0, WBFL::Units::Measure::Feet));
   m_ScallopRadii.push_back(WBFL::Units::ConvertToSysUnits(10.0, WBFL::Units::Measure::Feet));
   ATLASSERT(m_vColumnData.size() == m_vColumnSpacing.size()+1);
}

void xbrPierData::RemoveColumn(ColumnIndexType colIdx)
{
   if (m_vColumnData.size() <= 1 || colIdx >= m_vColumnData.size())
      return;
   m_ScallopDepths.erase(m_ScallopDepths.begin() + colIdx);
   const ColumnIndexType bayIdx = colIdx == m_vColumnData.size() - 1 ? colIdx - 1 : colIdx;
   m_ScallopRadii.erase(m_ScallopRadii.begin() + bayIdx);

   // always remove the spacing to the right of the column, unless
   // this is the last column. If this is the last column remove
   // the left spacing
   if ( colIdx == m_vColumnData.size()-1 )
   {
      m_vColumnSpacing.pop_back();
   }
   else
   {
      m_vColumnSpacing.erase(m_vColumnSpacing.begin()+colIdx);
   }

   m_vColumnData.erase(m_vColumnData.begin()+colIdx);
   ATLASSERT(m_vColumnData.size() == m_vColumnSpacing.size()+1);
}

void xbrPierData::SetColumnData(ColumnIndexType colIdx,const CColumnData& columnData)
{
   m_vColumnData[colIdx] = columnData;
}

const CColumnData& xbrPierData::GetColumnData(ColumnIndexType colIdx) const
{
   return m_vColumnData[colIdx];
}

CColumnData& xbrPierData::GetColumnData(ColumnIndexType colIdx)
{
   return m_vColumnData[colIdx];
}

void xbrPierData::SetColumnScallopDepth(ColumnIndexType colIdx, Float64 depth)
{
   ATLASSERT(colIdx < m_ScallopDepths.size());
   m_ScallopDepths.at(colIdx) = depth;
}

Float64 xbrPierData::GetColumnScallopDepth(ColumnIndexType colIdx) const
{
   ATLASSERT(colIdx < m_ScallopDepths.size());
   return m_ScallopDepths.at(colIdx);
}

void xbrPierData::SetColumnScallopRadius(ColumnIndexType colIdx, Float64 radius)
{
   ATLASSERT(colIdx < m_ScallopRadii.size());
   m_ScallopRadii.at(colIdx) = radius;
}

Float64 xbrPierData::GetColumnScallopRadius(ColumnIndexType colIdx) const
{
   ATLASSERT(colIdx < m_ScallopRadii.size());
   return m_ScallopRadii.at(colIdx);
}

void xbrPierData::SetXBeamOverhangRadius(pgsTypes::SideType side, Float64 radius)
{
   ATLASSERT(side == pgsTypes::stLeft || side == pgsTypes::stRight);
   m_OverhangRadius[side] = radius;
}

Float64 xbrPierData::GetXBeamOverhangRadius(pgsTypes::SideType side) const
{
   ATLASSERT(side == pgsTypes::stLeft || side == pgsTypes::stRight);
   return m_OverhangRadius[side];
}

void xbrPierData::SetColumnSpacing(SpacingIndexType spaceIdx,Float64 S)
{
   m_vColumnSpacing[spaceIdx] = S;
}

Float64 xbrPierData::GetColumnSpacing(SpacingIndexType spaceIdx) const
{
   return m_vColumnSpacing[spaceIdx];
}

Float64& xbrPierData::GetColumnSpacing(SpacingIndexType spaceIdx)
{
   return m_vColumnSpacing[spaceIdx];
}

void xbrPierData::SetXBeamOverhangs(Float64 OHL,Float64 OHR)
{
   m_OHL = OHL;
   m_OHR = OHR;
}

void xbrPierData::GetXBeamOverhangs(Float64* pOHL,Float64* pOHR) const
{
   *pOHL = m_OHL;
   *pOHR = m_OHR;
}

Float64& xbrPierData::GetOHL()
{
   return m_OHL;
}

Float64& xbrPierData::GetOHR()
{
   return m_OHR;
}

pgsTypes::ConditionFactorType xbrPierData::GetConditionFactorType() const
{
   return m_ConditionFactorType;
}

pgsTypes::ConditionFactorType& xbrPierData::GetConditionFactorType()
{
   return m_ConditionFactorType;
}

void xbrPierData::SetConditionFactorType(pgsTypes::ConditionFactorType conditionFactorType)
{
   m_ConditionFactorType = conditionFactorType;
}

Float64 xbrPierData::GetConditionFactor() const
{
   return m_ConditionFactor;
}

Float64& xbrPierData::GetConditionFactor()
{
   return m_ConditionFactor;
}

void xbrPierData::SetConditionFactor(Float64 conditionFactor)
{
   m_ConditionFactor = conditionFactor;
}

void xbrPierData::SetStirrupMaterial(WBFL::Materials::Rebar::Type type, WBFL::Materials::Rebar::Grade grade)
{
    m_StirrupRebarType = type;
    m_StirrupRebarGrade = grade;
}

void xbrPierData::GetStirrupMaterial(WBFL::Materials::Rebar::Type* pType, WBFL::Materials::Rebar::Grade* pGrade) const
{
    *pType = m_StirrupRebarType;
    *pGrade = m_StirrupRebarGrade;
}

WBFL::Materials::Rebar::Type& xbrPierData::GetStirrupRebarType()
{
    return m_StirrupRebarType;
}

WBFL::Materials::Rebar::Grade& xbrPierData::GetStirrupRebarGrade()
{
    return m_StirrupRebarGrade;
}

void xbrPierData::SetConcreteMaterial(const CConcreteMaterial& concrete)
{
   m_Concrete = concrete;
}

CConcreteMaterial& xbrPierData::GetConcreteMaterial()
{
   return m_Concrete;
}

const CConcreteMaterial& xbrPierData::GetConcreteMaterial() const
{
   return m_Concrete;
}

void xbrPierData::SetLongitudinalRebar(const xbrLongitudinalRebarData& rebarData)
{
   m_LongitudinalRebar = rebarData;
}

xbrLongitudinalRebarData& xbrPierData::GetLongitudinalRebar()
{
   return m_LongitudinalRebar;
}

const xbrLongitudinalRebarData& xbrPierData::GetLongitudinalRebar() const
{
   return m_LongitudinalRebar;
}

void xbrPierData::SetLowerXBeamStirrups(const xbrStirrupData& stirrupData)
{
   m_LowerXBeamStirrups = stirrupData;
}

xbrStirrupData& xbrPierData::GetLowerXBeamStirrups()
{
   return m_LowerXBeamStirrups;
}

const xbrStirrupData& xbrPierData::GetLowerXBeamStirrups() const
{
   return m_LowerXBeamStirrups;
}

void xbrPierData::SetFullDepthStirrups(const xbrStirrupData& stirrupData)
{
   m_FullDepthStirrups = stirrupData;
}

xbrStirrupData& xbrPierData::GetFullDepthStirrups()
{
   return m_FullDepthStirrups;
}

const xbrStirrupData& xbrPierData::GetFullDepthStirrups() const
{
   return m_FullDepthStirrups;
}

void xbrPierData::SetBearingLineCount(IndexType nBearingLines)
{
   ATLASSERT(1 <= nBearingLines && nBearingLines <= 2);
   m_vBearingLines.resize(nBearingLines);
}

IndexType xbrPierData::GetBearingLineCount() const
{
   return m_vBearingLines.size();
}

void xbrPierData::SetBearingLineData(IndexType brgLineIdx,const xbrBearingLineData& bearingLineData)
{
   ATLASSERT(brgLineIdx < 2);
   if ( m_vBearingLines.size() <= brgLineIdx )
   {
      xbrBearingLineData brgLineData;
      m_vBearingLines.push_back(brgLineData);
   }

   m_vBearingLines[brgLineIdx] = bearingLineData;
}

xbrBearingLineData& xbrPierData::GetBearingLineData(IndexType brgLineIdx)
{
   ATLASSERT(brgLineIdx < 2);
   if ( m_vBearingLines.size() <= brgLineIdx )
   {
      xbrBearingLineData brgLineData;
      m_vBearingLines.push_back(brgLineData);
   }
   return m_vBearingLines[brgLineIdx];
}

const xbrBearingLineData& xbrPierData::GetBearingLineData(IndexType brgLineIdx) const
{
   ATLASSERT(brgLineIdx < 2);
   if ( m_vBearingLines.size() <= brgLineIdx )
   {
      xbrBearingLineData brgLineData;
      m_vBearingLines.push_back(brgLineData);
   }
   return m_vBearingLines[brgLineIdx];
}
   
void xbrPierData::SetBearingSpacing(IndexType brgLineIdx,IndexType brgIdx,Float64 s)
{
   m_vBearingLines[brgLineIdx].SetSpacing(brgIdx,s);
}

Float64 xbrPierData::GetBearingSpacing(IndexType brgLineIdx,IndexType brgIdx) const
{
   return m_vBearingLines[brgLineIdx].GetSpacing(brgIdx);
}

void xbrPierData::SetBearingCount(IndexType brgLineIdx,IndexType nBearings)
{
   m_vBearingLines[brgLineIdx].SetBearingCount(nBearings);
}

IndexType xbrPierData::GetBearingCount(IndexType brgLineIdx) const
{
   return m_vBearingLines[brgLineIdx].GetBearingCount();
}

Float64 xbrPierData::GetXBeamLength() const
{
   Float64 L = 0;
   for (const auto& s : m_vColumnSpacing)
   {
      L += s;
   }
   L += (m_OHL-m_X1L) + (m_OHR-m_X1R);
   return L;
}

HRESULT xbrPierData::Save(IStructuredSave* pStrSave,std::shared_ptr<IEAFProgress> pProgress)
{
   pStrSave->BeginUnit(_T("Pier"),1.0);
   pStrSave->put_Property(_T("ID"),CComVariant(m_ID));

   pStrSave->BeginUnit(_T("Layout"),1.0);
      pStrSave->put_Property(_T("SuperstructureConnectionType"),CComVariant(m_ConnectionType));
      pStrSave->put_Property(_T("Skew"),CComVariant(m_strSkew.c_str()));
      pStrSave->put_Property(_T("BridgeLineOffset"),CComVariant(m_BridgeLineOffset));
      pStrSave->put_Property(_T("CurbLineDatum"),CComVariant(m_CurbLineDatum));
      pStrSave->put_Property(_T("LeftCurbOffset"),CComVariant(m_LeftCurbOffset));
      pStrSave->put_Property(_T("RightCurbOffset"),CComVariant(m_RightCurbOffset));
      pStrSave->put_Property(_T("DeckThickness"),CComVariant(m_tDeck));

      pStrSave->put_Property(_T("DeckSurfaceType"),CComVariant(m_DeckSurfaceType));
      if ( m_DeckSurfaceType == Simplified )
      {
         pStrSave->put_Property(_T("DeckElevation"),CComVariant(m_DeckElevation));
         pStrSave->put_Property(_T("CrownPointOffset"),CComVariant(m_CrownPointOffset));
         pStrSave->put_Property(_T("LeftCrownSlope"),CComVariant(m_LeftCrownSlope));
         pStrSave->put_Property(_T("RightCrownSlope"),CComVariant(m_RightCrownSlope));
      }
      else
      {
         pStrSave->BeginUnit(_T("DeckProfile"),1.0);
         CComPtr<IEnumPoint2d> enumPoints;
         m_DeckProfile->get__Enum(&enumPoints);
         CComPtr<IPoint2d> pnt;
         while ( enumPoints->Next(1,&pnt,nullptr) != S_FALSE )
         {
            Float64 x,y;
            pnt->Location(&x,&y);
            pStrSave->BeginUnit(_T("Point"),1.0);
               pStrSave->put_Property(_T("X"),CComVariant(x));
               pStrSave->put_Property(_T("Y"),CComVariant(y));
            pStrSave->EndUnit(); // Point
            pnt.Release();
         }
         pStrSave->EndUnit(); // DeckProfile
      }

   pStrSave->EndUnit(); // Layout

   pStrSave->BeginUnit(_T("Diaphragm"),1.0);
      pStrSave->put_Property(_T("H"),CComVariant(m_H));
      pStrSave->put_Property(_T("W"),CComVariant(m_W));
   pStrSave->EndUnit(); // Diaphragm

   pStrSave->BeginUnit(_T("LowerCrossBeam"),2.0);
      pStrSave->put_Property(_T("PierLayoutType"),CComVariant(m_PierLayoutType));
      pStrSave->put_Property(_T("H1L"),CComVariant(m_H1L));
      pStrSave->put_Property(_T("H1R"),CComVariant(m_H1R));
      pStrSave->put_Property(_T("H2L"),CComVariant(m_H2L));
      pStrSave->put_Property(_T("H2R"),CComVariant(m_H2R));
      pStrSave->put_Property(_T("X1L"),CComVariant(m_X1L));
      pStrSave->put_Property(_T("X1R"),CComVariant(m_X1R));
      pStrSave->put_Property(_T("X2L"),CComVariant(m_X2L));
      pStrSave->put_Property(_T("X2R"),CComVariant(m_X2R));
      pStrSave->put_Property(_T("RL"), CComVariant(m_OverhangRadius[pgsTypes::stLeft]));
      pStrSave->put_Property(_T("RR"), CComVariant(m_OverhangRadius[pgsTypes::stRight]));
      pStrSave->put_Property(_T("ScallopColumnCount"), CComVariant(GetColumnCount()));
      for (Float64 depth : m_ScallopDepths)
         pStrSave->put_Property(_T("ScallopDepth"), CComVariant(depth));
      for (Float64 radius : m_ScallopRadii)
         pStrSave->put_Property(_T("ScallopRadius"), CComVariant(radius));

      pStrSave->put_Property(_T("PierPointCount"), CComVariant(m_vPierPoints.size()));
      std::vector<CPierPointData>::iterator ppIterBegin = m_vPierPoints.begin();
      std::vector<CPierPointData>::iterator ppIter = ppIterBegin;
      std::vector<CPierPointData>::iterator ppIterEnd = m_vPierPoints.end();
      for (; ppIter != ppIterEnd; ppIter++)
      {
          CPierPointData& pierPointData = *ppIter;
          pierPointData.Save(pStrSave, pProgress);
      }

      pStrSave->put_Property(_T("W"), CComVariant(m_XW));

   pStrSave->EndUnit(); // LowerCrossBeam

   pStrSave->BeginUnit(_T("Columns"),2.0);
      pStrSave->put_Property(_T("RefColumn"),CComVariant(m_RefColumnIdx));
      pStrSave->put_Property(_T("RefColumnOffset"),CComVariant(m_RefColumnOffset));
      pStrSave->put_Property(_T("RefColumnDatum"),CComVariant(m_RefColumnDatum));
      pStrSave->put_Property(_T("OHL"),CComVariant(m_OHL));
      pStrSave->put_Property(_T("OHR"),CComVariant(m_OHR));

      std::vector<Float64>::iterator spacingIter = m_vColumnSpacing.begin();
      std::vector<CColumnData>::iterator columnIterBegin = m_vColumnData.begin();
      std::vector<CColumnData>::iterator columnIter = columnIterBegin;
      std::vector<CColumnData>::iterator columnIterEnd = m_vColumnData.end();
      for ( ; columnIter != columnIterEnd; columnIter++ )
      {
         if ( columnIter != columnIterBegin )
         {
            Float64 spacing = *spacingIter;
            pStrSave->put_Property(_T("Spacing"),CComVariant(spacing));
            spacingIter++;
         }
         CColumnData& columnData = *columnIter;
         columnData.Save(pStrSave,pProgress);
      }
   pStrSave->EndUnit(); // Columns

   m_Concrete.Save(pStrSave,nullptr);

   pStrSave->BeginUnit(_T("Reinforcement"),2.0);
      pStrSave->put_Property(_T("StirrupRebarType"),CComVariant(+m_StirrupRebarType)); //modified to stirrup rebar type in version 2.0
      pStrSave->put_Property(_T("StirrupRebarGrade"),CComVariant(+m_StirrupRebarGrade)); //modified to stirrup rebar grade in version 2.0
      m_LongitudinalRebar.Save(pStrSave,nullptr);
      m_LowerXBeamStirrups.Save(pStrSave);
      m_FullDepthStirrups.Save(pStrSave);
   pStrSave->EndUnit(); // Reinforcement

   pStrSave->BeginUnit(_T("Bearings"),1.0);
   for (const auto& bearingLine : m_vBearingLines)
      {
         bearingLine.Save(pStrSave);
      }
   pStrSave->EndUnit(); // Bearings

   pStrSave->BeginUnit(_T("Condition"),1.0);
      pStrSave->put_Property(_T("ConditionFactorType"),CComVariant(m_ConditionFactorType));
      pStrSave->put_Property(_T("ConditionFactor"),CComVariant(m_ConditionFactor));
   pStrSave->EndUnit(); // Condition

   pStrSave->EndUnit(); // Pier
   return S_OK;
}

HRESULT xbrPierData::Load(IStructuredLoad* pStrLoad,std::shared_ptr<IEAFProgress> pProgress)
{
   USES_CONVERSION;
   Float64 lowerCrossBeamVersion = 1.0;
   CHRException hr;
   try
   {
      CComVariant var;
      HRESULT hres = pStrLoad->BeginUnit(_T("Pier"));
      // don't want to throw an exception if this fails... let the caller deal with the failure
      // (we want to loop over pier objects until fail.. fail indicates there aren't any more pier objects
      if (FAILED(hres) )
      {
         return hres;
      }

      var.vt = VT_ID;
      hr = pStrLoad->get_Property(_T("ID"),&var);
      m_ID = VARIANT2ID(var);

      {
         hr = pStrLoad->BeginUnit(_T("Layout"));
         var.vt = VT_I4;
         hr = pStrLoad->get_Property(_T("SuperstructureConnectionType"),&var);
         m_ConnectionType = (pgsTypes::PierType)(var.lVal);

         var.vt = VT_BSTR;
         hr = pStrLoad->get_Property(_T("Skew"),&var);
         m_strSkew = OLE2T(var.bstrVal);

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("BridgeLineOffset"),&var);
         m_BridgeLineOffset = var.dblVal;

         var.vt = VT_I4;
         hr = pStrLoad->get_Property(_T("CurbLineDatum"),&var);
         m_CurbLineDatum = (pgsTypes::OffsetMeasurementType)(var.lVal);
         
         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("LeftCurbOffset"),&var);
         m_LeftCurbOffset = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("RightCurbOffset"),&var);
         m_RightCurbOffset = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("DeckThickness"),&var);
         m_tDeck = var.dblVal;

         var.vt = VT_I4;
         hr = pStrLoad->get_Property(_T("DeckSurfaceType"),&var);
         m_DeckSurfaceType = (DeckSurfaceType)var.lVal;

         if ( m_DeckSurfaceType == Simplified )
         {
            var.vt = VT_R8;
            hr = pStrLoad->get_Property(_T("DeckElevation"),&var);
            m_DeckElevation = var.dblVal;

            var.vt = VT_R8;
            hr = pStrLoad->get_Property(_T("CrownPointOffset"),&var);
            m_CrownPointOffset = var.dblVal;

            var.vt = VT_R8;
            hr = pStrLoad->get_Property(_T("LeftCrownSlope"),&var);
            m_LeftCrownSlope = var.dblVal;

            var.vt = VT_R8;
            hr = pStrLoad->get_Property(_T("RightCrownSlope"),&var);
            m_RightCrownSlope = var.dblVal;
         }
         else
         {
            if (m_DeckProfile)
            {
               m_DeckProfile->Clear();
            }
            else
            {
               m_DeckProfile.CoCreateInstance(CLSID_Point2dCollection);
            }

            hr = pStrLoad->BeginUnit(_T("DeckProfile"));
            while ( SUCCEEDED(pStrLoad->BeginUnit(_T("Point"))) )
            {
               var.vt = VT_R8;
               hr = pStrLoad->get_Property(_T("X"),&var);
               Float64 x = var.dblVal;

               var.vt = VT_R8;
               hr = pStrLoad->get_Property(_T("Y"),&var);
               Float64 y = var.dblVal;

               hr = pStrLoad->EndUnit(); // Point

               CComPtr<IPoint2d> pnt;
               pnt.CoCreateInstance(CLSID_Point2d);
               pnt->Move(x,y);
               m_DeckProfile->Add(pnt);
            }
            hr = pStrLoad->EndUnit(); // DeckProfile
         }

         hr = pStrLoad->EndUnit(); 
      }

      {
         hr = pStrLoad->BeginUnit(_T("Diaphragm"));

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("H"),&var);
         m_H = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("W"),&var);
         m_W = var.dblVal;

         hr = pStrLoad->EndUnit();
      }

      {
         hr = pStrLoad->BeginUnit(_T("LowerCrossBeam"));
         pStrLoad->get_Version(&lowerCrossBeamVersion);

         Float64 version;
         pStrLoad->get_Version(&version);

         if (version < 2.0)
         {
             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("H1"), &var);
             m_H1L = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("H2"), &var);
             m_H2L = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("H3"), &var);
             m_H1R = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("H4"), &var);
             m_H2R = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("X1"), &var);
             m_X2L = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("X2"), &var);
             m_X1L = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("X3"), &var);
             m_X2R = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("X4"), &var);
             m_X1R = var.dblVal;
         }
         else
         {
             var.vt = VT_I4;
             hr = pStrLoad->get_Property(_T("PierLayoutType"), &var);
             m_PierLayoutType = (pgsTypes::PierLayoutType)(var.lVal);

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("H1L"), &var);
             m_H1L = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("H1R"), &var);
             m_H1R = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("H2L"), &var);
             m_H2L = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("H2R"), &var);
             m_H2R = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("X1L"), &var);
             m_X1L = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("X1R"), &var);
             m_X1R = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("X2L"), &var);
             m_X2L = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("X2R"), &var);
             m_X2R = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("RL"), &var);
             m_OverhangRadius[pgsTypes::stLeft] = var.dblVal;
             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("RR"), &var);
             m_OverhangRadius[pgsTypes::stRight] = var.dblVal;
             var.vt = VT_INDEX;
             hr = pStrLoad->get_Property(_T("ScallopColumnCount"), &var);
             const ColumnIndexType count = VARIANT2INDEX(var);
             if (count == 0 || count == INVALID_INDEX)
                 throw E_FAIL;
             m_ScallopDepths.resize(count);
             m_ScallopRadii.resize(count - 1);
             for (auto& depth : m_ScallopDepths)
             {
                 var.vt = VT_R8;
                 hr = pStrLoad->get_Property(_T("ScallopDepth"), &var);
                 depth = var.dblVal;
             }
             for (auto& radius : m_ScallopRadii)
             {
                 var.vt = VT_R8;
                 hr = pStrLoad->get_Property(_T("ScallopRadius"), &var);
                 radius = var.dblVal;
             }

             m_vPierPoints.clear();
             var.vt = VT_INDEX;
             hr = pStrLoad->get_Property(_T("PierPointCount"), &var);
             PierPointIndexType nPierPoints = VARIANT2INDEX(var);

             m_vPierPoints.clear();

             m_vPierPoints.reserve(nPierPoints);

             for (PierPointIndexType ppIdx = 0; ppIdx < nPierPoints; ++ppIdx)
             {
                 CPierPointData pierPoint;

                 hr = pierPoint.Load(pStrLoad, pProgress);

                 m_vPierPoints.push_back(std::move(pierPoint));
             }

         }

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("W"),&var);
         m_XW = var.dblVal;

         hr = pStrLoad->EndUnit(); // LowerCrossBeam
      }

      {
         hr = pStrLoad->BeginUnit(_T("Columns"));
         var.vt = VT_INDEX;
         hr = pStrLoad->get_Property(_T("RefColumn"),&var);
         m_RefColumnIdx = VARIANT2INDEX(var);
         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("RefColumnOffset"),&var);
         m_RefColumnOffset = var.dblVal;
         var.vt = VT_I4;
         hr = pStrLoad->get_Property(_T("RefColumnDatum"),&var);
         m_RefColumnDatum = (pgsTypes::OffsetMeasurementType)var.lVal;

         Float64 version;
         pStrLoad->get_Version(&version);
		 if (version < 2.0)
         {
             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("X5"), &var);
             m_OHL = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("X6"), &var);
             m_OHR = var.dblVal;
         }
         else
         {
             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("OHL"), &var);
             m_OHL = var.dblVal;

             var.vt = VT_R8;
             hr = pStrLoad->get_Property(_T("OHR"), &var);
             m_OHR = var.dblVal;
         }

         m_vColumnData.clear();
         m_vColumnSpacing.clear();
         int i = 0;
         HRESULT hrSpacing = S_OK;
         // Reading Column-Spacing-Column-Spacing-Column....
         do
         {
            if ( i != 0 )
            {
               // don't read spacing first time through because we start with column
               var.vt = VT_R8;
               hrSpacing = pStrLoad->get_Property(_T("Spacing"),&var);
               if ( SUCCEEDED(hrSpacing) )
               {
                  Float64 spacing = var.dblVal;
                  m_vColumnSpacing.push_back(spacing);
               }
            }

            if ( SUCCEEDED(hrSpacing) )
            {
               // only read the column data if the spacing data was successfully read
               CColumnData columnData;
               hr = columnData.Load(pStrLoad,pProgress);
               m_vColumnData.push_back(columnData);
            }
            i++;
         } while (SUCCEEDED(hrSpacing)); // read data until we don't get the spacing data we expect
         ATLASSERT(m_vColumnData.size() == m_vColumnSpacing.size()+1);

         hr = pStrLoad->EndUnit(); // Columns
         if (lowerCrossBeamVersion < 2.0)
         {
             // Files predating bottom profiles have no scallop inputs.
             m_ScallopDepths.assign(m_vColumnData.size(), WBFL::Units::ConvertToSysUnits(5.0, WBFL::Units::Measure::Feet));
             m_ScallopRadii.assign(m_vColumnSpacing.size(), WBFL::Units::ConvertToSysUnits(10.0, WBFL::Units::Measure::Feet));
         }
         else if (m_ScallopDepths.size() != m_vColumnData.size() ||
                  m_ScallopRadii.size() != m_vColumnSpacing.size())
         {
             throw E_FAIL;
         }
      }

      {
         hr = m_Concrete.Load(pStrLoad,nullptr);
      }

      {
         hr = pStrLoad->BeginUnit(_T("Reinforcement"));
         Float64 version;
         pStrLoad->get_Version(&version);

		 if (version < 2.0)
         {
             var.vt = VT_I4;
             hr = pStrLoad->get_Property(_T("RebarType"), &var);
             WBFL::Materials::Rebar::Type type = (WBFL::Materials::Rebar::Type)(var.lVal);
             m_StirrupRebarType = (WBFL::Materials::Rebar::Type)(var.lVal);
             hr = pStrLoad->get_Property(_T("RebarGrade"), &var);
             WBFL::Materials::Rebar::Grade grade = (WBFL::Materials::Rebar::Grade)(var.lVal);
             m_StirrupRebarGrade = (WBFL::Materials::Rebar::Grade)(var.lVal);

         }
         else
         {
             var.vt = VT_I4;
             hr = pStrLoad->get_Property(_T("StirrupRebarType"), &var);
             WBFL::Materials::Rebar::Type type = (WBFL::Materials::Rebar::Type)(var.lVal);
             m_StirrupRebarType = (WBFL::Materials::Rebar::Type)(var.lVal);
             hr = pStrLoad->get_Property(_T("StirrupRebarGrade"), &var);
             WBFL::Materials::Rebar::Grade grade = (WBFL::Materials::Rebar::Grade)(var.lVal);
             m_StirrupRebarGrade = (WBFL::Materials::Rebar::Grade)(var.lVal);
         }

         hr = m_LongitudinalRebar.Load(pStrLoad,nullptr);
         if (version < 2.0)
         {
             // Legacy rows have no material fields. Apply the enclosing
             // reinforcement material after Load has populated the rows.
             for (auto& row : m_LongitudinalRebar.RebarRows)
                 row.SetBarMaterial(m_StirrupRebarType, m_StirrupRebarGrade);
         }
         hr = m_LowerXBeamStirrups.Load(pStrLoad);
         hr = m_FullDepthStirrups.Load(pStrLoad);
         hr = pStrLoad->EndUnit(); // Reinforcement
      }

      {
         hr = pStrLoad->BeginUnit(_T("Bearings"));
         m_vBearingLines.clear();
         xbrBearingLineData bearingLine;
         while ( SUCCEEDED(bearingLine.Load(pStrLoad)) )
         {
            m_vBearingLines.push_back(bearingLine);
         }
         hr = pStrLoad->EndUnit(); // Bearings
      }

      {
         hr = pStrLoad->BeginUnit(_T("Condition"));

         var.vt = VT_I4;
         hr = pStrLoad->get_Property(_T("ConditionFactorType"),&var);
         m_ConditionFactorType = (pgsTypes::ConditionFactorType)var.lVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("ConditionFactor"),&var);
         m_ConditionFactor = var.dblVal;

         hr = pStrLoad->EndUnit(); // Condition
      }

      hr = pStrLoad->EndUnit();
   }
   catch (HRESULT)
   {
      ATLASSERT(false);
      THROW_LOAD(InvalidFileFormat,pStrLoad);
   }
   return S_OK;
}

void xbrPierData::MakeCopy(const xbrPierData& rOther)
{
   m_ID = rOther.m_ID;

   m_strSkew = rOther.m_strSkew;
   m_BridgeLineOffset = rOther.m_BridgeLineOffset;

   m_ConnectionType  = rOther.m_ConnectionType;
   m_CurbLineDatum   = rOther.m_CurbLineDatum;
   m_LeftCurbOffset  = rOther.m_LeftCurbOffset;
   m_RightCurbOffset = rOther.m_RightCurbOffset;

   m_DeckSurfaceType  = rOther.m_DeckSurfaceType;
   m_DeckElevation    = rOther.m_DeckElevation;
   m_CrownPointOffset = rOther.m_CrownPointOffset;
   m_LeftCrownSlope   = rOther.m_LeftCrownSlope;
   m_RightCrownSlope  = rOther.m_RightCrownSlope;
   m_DeckProfile.Release();
   if ( rOther.m_DeckProfile )
   {
      rOther.m_DeckProfile->Clone(&m_DeckProfile);
   }

   m_H = rOther.m_H;
   m_W = rOther.m_W;

   m_tDeck = rOther.m_tDeck;

   m_PierLayoutType = rOther.m_PierLayoutType;

   m_H1L = rOther.m_H1L;
   m_H1R = rOther.m_H1R;
   m_H2L = rOther.m_H2L;
   m_H2R = rOther.m_H2R;
   m_X1L = rOther.m_X1L;
   m_X1R = rOther.m_X1R;
   m_X2L = rOther.m_X2L;
   m_X2R = rOther.m_X2R;
   m_XW = rOther.m_XW;
   m_OverhangRadius[pgsTypes::stLeft] = rOther.m_OverhangRadius[pgsTypes::stLeft];
   m_OverhangRadius[pgsTypes::stRight] = rOther.m_OverhangRadius[pgsTypes::stRight];
   m_ScallopDepths = rOther.m_ScallopDepths;
   m_ScallopRadii = rOther.m_ScallopRadii;
   m_vPierPoints = rOther.m_vPierPoints;

   m_vColumnData = rOther.m_vColumnData;
   m_vColumnSpacing = rOther.m_vColumnSpacing;

   m_RefColumnDatum  = rOther.m_RefColumnDatum;
   m_RefColumnIdx    = rOther.m_RefColumnIdx;
   m_RefColumnOffset = rOther.m_RefColumnOffset;

   m_OHL              = rOther.m_OHL;
   m_OHR              = rOther.m_OHR;

   m_StirrupRebarGrade = rOther.m_StirrupRebarGrade;
   m_StirrupRebarType = rOther.m_StirrupRebarType;

   m_Concrete = rOther.m_Concrete;

   m_ConditionFactorType = rOther.m_ConditionFactorType;
   m_ConditionFactor = rOther.m_ConditionFactor;

   m_LongitudinalRebar = rOther.m_LongitudinalRebar;
   m_LowerXBeamStirrups = rOther.m_LowerXBeamStirrups;
   m_FullDepthStirrups = rOther.m_FullDepthStirrups;

   m_vBearingLines = rOther.m_vBearingLines;
}

void xbrPierData::MakeAssignment(const xbrPierData& rOther)
{
   MakeCopy( rOther );
}

// Build geometry from a snapshot, without updating project data or analysis caches.
void xbrPierData::CreatePierModel(IPier** ppPier) const
{
   CComPtr<IPierEx> pierModel;
   pierModel.CoCreateInstance(CLSID_Pier);

   const xbrPierData& pierData = *this;

   Float64 H1L, H1R, H2L, H2R, X1L, X1R, X2L, X2R, W;
   Float64 HU, W2;
   pierData.GetDiaphragmDimensions(&HU, &W2);
   std::vector<CPierPointData> vPoints;
   pierData.GetLowerXBeamDimensions(&H1L, &H1R, &H2L, &H2R, &X1L, &X1R, &X2L, &X2R, &W, &vPoints);

   // Create Cross Beam
   CComPtr<IBasicCrossBeam> bxbeam;
   CComPtr<IScallopedCrossBeam> sxbeam;
   CComPtr<IPointDefinedCrossBeam> uxbeam;

   HRESULT hr;
   if (pierData.GetPierLayoutType() == pgsTypes::pltUserDefined)
   {
       hr = uxbeam.CoCreateInstance(CLSID_PointDefinedCrossBeam);
       pierModel->putref_CrossBeam(uxbeam);
   }
   else if (pierData.GetPierLayoutType() == pgsTypes::pltScalloped)
   {
       hr = sxbeam.CoCreateInstance(CLSID_ScallopedCrossBeam);
       pierModel->putref_CrossBeam(sxbeam);
   }
   else
   {
       hr = bxbeam.CoCreateInstance(CLSID_BasicCrossBeam);
       pierModel->putref_CrossBeam(bxbeam);
   }

   pgsTypes::PierType pierType = pierData.GetPierType();
   pierModel->put_Type((PierType)pierType);

   CComPtr<IAngle> skew;
   skew.CoCreateInstance(CLSID_Angle);
   skew->FromString(CComBSTR(pierData.GetSkew()));
   pierModel->putref_SkewAngle(skew);
   Float64 skew_angle;
   skew->get_Value(&skew_angle);

   // Superstructure Information
   pgsTypes::OffsetMeasurementType curbLineDatum = pierData.GetCurbLineDatum();

   Float64 leftCLO, rightCLO;
   pierData.GetCurbLineOffset(&leftCLO,&rightCLO);
   if ( curbLineDatum == pgsTypes::omtBridge )
   {
      leftCLO  += pierData.GetBridgeLineOffset();
      rightCLO += pierData.GetBridgeLineOffset();
   }

   pgsTypes::OffsetMeasurementType refColMeasure;
   ColumnIndexType refColIdx;
   Float64 refColOffset;
   pierData.GetRefColumnLocation(&refColMeasure, &refColIdx, &refColOffset);

   pierModel->put_CurbLineOffset(qcbLeft, leftCLO);
   pierModel->put_CurbLineOffset(qcbRight,rightCLO);

   if ( pierData.GetDeckSurfaceType() == xbrPierData::Simplified )
   {
      CComPtr<IPoint2dCollection> deckProfile;
      deckProfile.CoCreateInstance(CLSID_Point2dCollection);

      Float64 elevDeck = pierData.GetDeckElevation();

      Float64 sl,sr;
      pierData.GetCrownSlope(&sl,&sr);

      Float64 cpo = pierData.GetCrownPointOffset();

      Float64 elevCP;
      if ( IsZero(cpo) )
      {
         elevCP = elevDeck;
      }
      else
      {
         if ( 0 < cpo )
         {
            elevCP = cpo*sr + elevDeck;
         }
         else
         {
            ATLASSERT(0 < cpo);
            elevCP = elevDeck - sl*cpo;
         }
      }

      // left curb line point
      Float64 elevLCL = elevCP - sl*(leftCLO - cpo);
      CComPtr<IPoint2d> pntLCL;
      pntLCL.CoCreateInstance(CLSID_Point2d);
      pntLCL->Move(leftCLO/cos(skew_angle), elevLCL);
      deckProfile->Add(pntLCL);

      // Crown Point
      CComPtr<IPoint2d> pntCP;
      pntCP.CoCreateInstance(CLSID_Point2d);
      pntCP->Move(cpo / cos(skew_angle),elevCP);
      deckProfile->Add(pntCP);

      // right curb line point
      Float64 elevRCL = elevCP + sr*(rightCLO - cpo);
      CComPtr<IPoint2d> pntRCL;
      pntRCL.CoCreateInstance(CLSID_Point2d);
      pntRCL->Move(rightCLO / cos(skew_angle),elevRCL);
      deckProfile->Add(pntRCL);

      pierModel->putref_DeckProfile(deckProfile);
   }
   else
   {
      CComPtr<IPoint2dCollection> deckProfile;
      pierData.GetDeckProfile(&deckProfile);
      pierModel->putref_DeckProfile(deckProfile);
   }

   pierModel->put_DeckThickness(pierData.GetDeckThickness());

   // Create Column Layout
   CComPtr<IColumnLayout> columnLayout;
   columnLayout.CoCreateInstance(CLSID_ColumnLayout);

   ColumnIndexType nColumns = pierData.GetColumnCount();
   SpacingIndexType nSpaces = nColumns - 1;
   columnLayout->put_Uniform(VARIANT_FALSE);
   columnLayout->put_ColumnCount(nColumns);
   for (SpacingIndexType spaceIdx = 0; spaceIdx < nSpaces; spaceIdx++)
   {
       Float64 space = pierData.GetColumnSpacing(spaceIdx);
       columnLayout->put_Spacing(spaceIdx, space);
   }

   Float64 OHL = m_OHL;
   Float64 OHR = m_OHR;
   columnLayout->put_Overhang(qcbLeft, OHL);
   columnLayout->put_Overhang(qcbRight, OHR);


   for (ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++)
   {
       CColumnData::ColumnHeightMeasurementType colMeasurementType = pierData.GetColumnData(colIdx).GetColumnHeightMeasurementType();
       Float64 h = pierData.GetColumnData(colIdx).GetColumnHeight();

       CComPtr<IColumn> column;
       columnLayout->get_Column(colIdx, &column);

       if (colMeasurementType == CColumnData::chtHeight)
       {
           column->put_Height(h);
       }
       else
       {
           column->put_BaseElevation(h);
       }
   }

   // Set the reference column.
   if (refColMeasure == pgsTypes::omtBridge)
   {
       // the reference column needs to be measured from the alignment
       // for the product model
       Float64 blo = pierData.GetBridgeLineOffset();
       refColOffset += blo;
   }
   columnLayout->SetReferenceColumn(refColIdx, refColOffset);

   //
   // Create the bearing layout
   //
   CComPtr<IBearingLayout> bearingLayout;
   bearingLayout.CoCreateInstance(CLSID_BearingLayout);
   IndexType nBearingLines = pierData.GetBearingLineCount();
   bearingLayout->put_BearingLineCount(nBearingLines);
   for (IndexType brgLineIdx = 0; brgLineIdx < nBearingLines; brgLineIdx++)
   {
       const xbrBearingLineData& brgLineData = pierData.GetBearingLineData(brgLineIdx);

       bearingLayout->put_BearingLineOffset(brgLineIdx, brgLineData.GetBearingLineOffset());

       pgsTypes::OffsetMeasurementType datum;
       IndexType refBrgIdx;
       Float64 refBrgOffset;
       brgLineData.GetReferenceBearing(&datum, &refBrgIdx, &refBrgOffset);

       if (datum == pgsTypes::omtBridge)
       {
           // the reference bearing needs to be measured from the alignment
           // for the product model
           refBrgOffset += pierData.GetBridgeLineOffset();
       }
       bearingLayout->SetReferenceBearing(brgLineIdx, refBrgIdx, refBrgOffset);

       IndexType nBearings = brgLineData.GetBearingCount();
       bearingLayout->put_BearingCount(brgLineIdx, nBearings);
       for (IndexType spaceIdx = 0; spaceIdx < nBearings - 1; spaceIdx++)
       {
           Float64 s = brgLineData.GetSpacing(spaceIdx);
           bearingLayout->put_Spacing(brgLineIdx, spaceIdx, s);
       }
   }

   // finish the pier model (need pier to be complete because we need its geometry to layout the rebar)

   pierModel->putref_BearingLayout(bearingLayout);
   pierModel->putref_ColumnLayout(columnLayout);

   if (pierData.GetPierLayoutType() == pgsTypes::pltUserDefined)
   {
	   uxbeam->put_H1L(H1L);
	   uxbeam->put_H1R(H1R);
	   uxbeam->put_X1L(X1L);
	   uxbeam->put_X1R(X1R);
	   uxbeam->put_W1(W);
	   uxbeam->put_HU(HU);
	   uxbeam->put_W2(W2);
       CComPtr<IPoint2dCollection> points;
       points.CoCreateInstance(CLSID_Point2dCollection);

	   for (const auto& pointData : vPoints)
	   {
		   CComPtr<IPoint2d> point;
		   point.CoCreateInstance(CLSID_Point2d);
           Float64 Xxb = pointData.Get_X();

           Float64 Xp;
           pierModel->ConvertCrossBeamToPierCoordinate(Xxb, &Xp);

		   Float64 Xcl;
           pierModel->ConvertCrossBeamToCurbLineCoordinate(Xxb, &Xcl);

           Float64 Ydeck;
           pierModel->get_Elevation(Xcl, &Ydeck);

           point->Move(Xp,
               Ydeck - pointData.Get_Y()
               - pierData.GetDeckThickness()
               - HU);

		   points->Add(point);
	   }
	   uxbeam->SetPoints(points);
   }
   else if (pierData.GetPierLayoutType() == pgsTypes::pltScalloped)
   {
	   sxbeam->put_H1L(H1L);
	   sxbeam->put_H1R(H1R);
	   sxbeam->put_X1L(X1L);
	   sxbeam->put_X1R(X1R);
	   sxbeam->put_W1(W);
	   sxbeam->put_HU(HU);
	   sxbeam->put_W2(W2);
       sxbeam->put_RL(pierData.GetXBeamOverhangRadius(pgsTypes::stLeft));
       sxbeam->put_RR(pierData.GetXBeamOverhangRadius(pgsTypes::stRight));
       for (ColumnIndexType colIdx = 0; colIdx < nColumns; ++colIdx)
       {
           hr = sxbeam->put_ColumnScallopDepth(colIdx, pierData.GetColumnScallopDepth(colIdx));
           if (FAILED(hr))
               throw hr;
           if (colIdx + 1 < nColumns)
           {
               hr = sxbeam->put_ColumnScallopRadius(colIdx, pierData.GetColumnScallopRadius(colIdx));
               if (FAILED(hr))
                   throw hr;
           }
       }
   }
   else
   {
       bxbeam->put_H1L(H1L);
       bxbeam->put_H2L(H2L);
       bxbeam->put_H1R(H1R);
       bxbeam->put_H2R(H2R);
       bxbeam->put_X2L(X2L);
       bxbeam->put_X1L(X1L);
       bxbeam->put_X2R(X2R);
       bxbeam->put_X1R(X1R);
       bxbeam->put_W1(W);

       bxbeam->put_HU(HU);
       bxbeam->put_W2(W2);
   }

   CComQIPtr<IPier> pier(pierModel);
   pier.CopyTo(ppPier);
}
