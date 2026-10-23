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



xbrPierData::xbrPierData()
{
   m_ID = INVALID_ID;
   m_ConnectionType = xbrTypes::pctIntegral;
   m_strSkew = _T("00 00 00.0 L");
   m_BridgeLineOffset = 0;
   m_CurbLineDatum = pgsTypes::omtAlignment;
   m_LeftCurbOffset  = -WBFL::Units::ConvertToSysUnits(10.0,WBFL::Units::Measure::Feet);
   m_RightCurbOffset = WBFL::Units::ConvertToSysUnits(10.0,WBFL::Units::Measure::Feet);

   m_tDeck = WBFL::Units::ConvertToSysUnits(8.0,WBFL::Units::Measure::Inch);

   m_DeckSurfaceType = Simplified;
   m_DeckElevation = 0;
   m_CrownPointOffset = 0;
   m_LeftCrownSlope = -0.02;
   m_RightCrownSlope = -0.02;

   m_H = WBFL::Units::ConvertToSysUnits(3,WBFL::Units::Measure::Feet);
   m_W = WBFL::Units::ConvertToSysUnits(5,WBFL::Units::Measure::Feet);

   m_H1 = WBFL::Units::ConvertToSysUnits(5,WBFL::Units::Measure::Feet);
   m_H2 = WBFL::Units::ConvertToSysUnits(0,WBFL::Units::Measure::Feet);
   m_H3 = WBFL::Units::ConvertToSysUnits(5,WBFL::Units::Measure::Feet);
   m_H4 = WBFL::Units::ConvertToSysUnits(0,WBFL::Units::Measure::Feet);
   m_X1 = WBFL::Units::ConvertToSysUnits(0,WBFL::Units::Measure::Feet);
   m_X2 = WBFL::Units::ConvertToSysUnits(0,WBFL::Units::Measure::Feet);
   m_X3 = WBFL::Units::ConvertToSysUnits(0,WBFL::Units::Measure::Feet);
   m_X4 = WBFL::Units::ConvertToSysUnits(0,WBFL::Units::Measure::Feet);
   m_XW = WBFL::Units::ConvertToSysUnits(5,WBFL::Units::Measure::Feet);

   CColumnData column;
   m_vColumnData.push_back(column);

   m_RefColumnIdx = 0;
   m_RefColumnOffset = 0;
   m_RefColumnDatum = pgsTypes::omtAlignment;

   m_X5 = WBFL::Units::ConvertToSysUnits(10,WBFL::Units::Measure::Feet);
   m_X6 = WBFL::Units::ConvertToSysUnits(10,WBFL::Units::Measure::Feet);

   // Load Rating Condition
   m_ConditionFactorType = pgsTypes::cfGood;
   m_ConditionFactor = 1.0;

   // Materials
   m_RebarType = WBFL::Materials::Rebar::Type::A615;
   m_RebarGrade = WBFL::Materials::Rebar::Grade::Grade60;

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

void xbrPierData::SetPierType(xbrTypes::PierType type)
{
   m_ConnectionType = type;
}

xbrTypes::PierType xbrPierData::GetPierType() const
{
   return m_ConnectionType;
}

xbrTypes::PierType& xbrPierData::GetPierType()
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

void xbrPierData::SetLowerXBeamDimensions(Float64 h1,Float64 h2,Float64 h3,Float64 h4,Float64 x1,Float64 x2,Float64 x3,Float64 x4,Float64 w)
{
   m_H1 = h1;
   m_H2 = h2;
   m_H3 = h3;
   m_H4 = h4;
   m_X1 = x1;
   m_X2 = x2;
   m_X3 = x3;
   m_X4 = x4;
   m_XW = w;
}

void xbrPierData::GetLowerXBeamDimensions(Float64* ph1,Float64* ph2,Float64* ph3,Float64* ph4,Float64* px1,Float64* px2,Float64* px3,Float64* px4,Float64* pw) const
{
   *ph1 = m_H1;
   *ph2 = m_H2;
   *ph3 = m_H3;
   *ph4 = m_H4;
   *px1 = m_X1;
   *px2 = m_X2;
   *px3 = m_X3;
   *px4 = m_X4;
   *pw  = m_XW;
}

Float64& xbrPierData::GetH1()
{
   return m_H1;
}

Float64& xbrPierData::GetH2()
{
   return m_H2;
}

Float64& xbrPierData::GetH3()
{
   return m_H3;
}

Float64& xbrPierData::GetH4()
{
   return m_H4;
}

Float64& xbrPierData::GetX1()
{
   return m_X1;
}

Float64& xbrPierData::GetX2()
{
   return m_X2;
}

Float64& xbrPierData::GetX3()
{
   return m_X3;
}

Float64& xbrPierData::GetX4()
{
   return m_X4;
}

Float64& xbrPierData::GetW()
{
   return m_XW;
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
   if ( m_vColumnData.size() != nColumns )
   {
      m_vColumnData.resize(nColumns,m_vColumnData.back());
      if ( m_vColumnSpacing.size() == 0 && nColumns != 1)
      {
         m_vColumnSpacing.push_back(WBFL::Units::ConvertToSysUnits(10.0,WBFL::Units::Measure::Feet));
      }
      m_vColumnSpacing.resize(nColumns-1,m_vColumnSpacing.back());
   }
   ATLASSERT(m_vColumnData.size() == m_vColumnSpacing.size()+1);
}

ColumnIndexType xbrPierData::GetColumnCount() const
{
   return m_vColumnData.size();
}

void xbrPierData::AddColumn(const CColumnData& columnData,Float64 spacing)
{
   m_vColumnData.push_back(columnData);
   m_vColumnSpacing.push_back(spacing);
   ATLASSERT(m_vColumnData.size() == m_vColumnSpacing.size()+1);
}

void xbrPierData::RemoveColumn(ColumnIndexType colIdx)
{
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

void xbrPierData::SetXBeamOverhangs(Float64 X5,Float64 X6)
{
   m_X5 = X5;
   m_X6 = X6;
}

void xbrPierData::GetXBeamOverhangs(Float64* pX5,Float64* pX6) const
{
   *pX5 = m_X5;
   *pX6 = m_X6;
}

Float64& xbrPierData::GetX5()
{
   return m_X5;
}

Float64& xbrPierData::GetX6()
{
   return m_X6;
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

void xbrPierData::SetRebarMaterial(WBFL::Materials::Rebar::Type type,WBFL::Materials::Rebar::Grade grade)
{
   m_RebarType = type;
   m_RebarGrade = grade;
}

void xbrPierData::GetRebarMaterial(WBFL::Materials::Rebar::Type* pType,WBFL::Materials::Rebar::Grade* pGrade) const
{
   *pType = m_RebarType;
   *pGrade = m_RebarGrade;
}

WBFL::Materials::Rebar::Type& xbrPierData::GetRebarType()
{
   return m_RebarType;
}

WBFL::Materials::Rebar::Grade& xbrPierData::GetRebarGrade()
{
   return m_RebarGrade;
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
   L += (m_X5-m_X2) + (m_X6-m_X4);
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

   pStrSave->BeginUnit(_T("LowerCrossBeam"),1.0);
      pStrSave->put_Property(_T("H1"),CComVariant(m_H1));
      pStrSave->put_Property(_T("H2"),CComVariant(m_H2));
      pStrSave->put_Property(_T("H3"),CComVariant(m_H3));
      pStrSave->put_Property(_T("H4"),CComVariant(m_H4));
      pStrSave->put_Property(_T("X1"),CComVariant(m_X1));
      pStrSave->put_Property(_T("X2"),CComVariant(m_X2));
      pStrSave->put_Property(_T("X3"),CComVariant(m_X3));
      pStrSave->put_Property(_T("X4"),CComVariant(m_X4));
      pStrSave->put_Property(_T("W"), CComVariant(m_XW));
   pStrSave->EndUnit(); // LowerCrossBeam

   pStrSave->BeginUnit(_T("Columns"),1.0);
      pStrSave->put_Property(_T("RefColumn"),CComVariant(m_RefColumnIdx));
      pStrSave->put_Property(_T("RefColumnOffset"),CComVariant(m_RefColumnOffset));
      pStrSave->put_Property(_T("RefColumnDatum"),CComVariant(m_RefColumnDatum));
      pStrSave->put_Property(_T("X5"),CComVariant(m_X5));
      pStrSave->put_Property(_T("X6"),CComVariant(m_X6));

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

   pStrSave->BeginUnit(_T("Reinforcement"),1.0);
      pStrSave->put_Property(_T("RebarType"),CComVariant(+m_RebarType));
      pStrSave->put_Property(_T("RebarGrade"),CComVariant(+m_RebarGrade));
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
         m_ConnectionType = (xbrTypes::PierType)(var.lVal);

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

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("H1"),&var);
         m_H1 = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("H2"),&var);
         m_H2 = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("H3"),&var);
         m_H3 = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("H4"),&var);
         m_H4 = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("X1"),&var);
         m_X1 = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("X2"),&var);
         m_X2 = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("X3"),&var);
         m_X3 = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("X4"),&var);
         m_X4 = var.dblVal;

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

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("X5"),&var);
         m_X5 = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("X6"),&var);
         m_X6 = var.dblVal;

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
      }

      {
         hr = m_Concrete.Load(pStrLoad,nullptr);
      }

      {
         hr = pStrLoad->BeginUnit(_T("Reinforcement"));

         var.vt = VT_I4;
         hr = pStrLoad->get_Property(_T("RebarType"),&var);
         m_RebarType = (WBFL::Materials::Rebar::Type)(var.lVal);

         hr = pStrLoad->get_Property(_T("RebarGrade"),&var);
         m_RebarGrade = (WBFL::Materials::Rebar::Grade)(var.lVal);

         hr = m_LongitudinalRebar.Load(pStrLoad,nullptr);
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

   m_H1 = rOther.m_H1;
   m_H2 = rOther.m_H2;
   m_H3 = rOther.m_H3;
   m_H4 = rOther.m_H4;
   m_X1 = rOther.m_X1;
   m_X2 = rOther.m_X2;
   m_X3 = rOther.m_X3;
   m_X4 = rOther.m_X4;
   m_XW = rOther.m_XW;

   m_vColumnData = rOther.m_vColumnData;
   m_vColumnSpacing = rOther.m_vColumnSpacing;

   m_RefColumnDatum  = rOther.m_RefColumnDatum;
   m_RefColumnIdx    = rOther.m_RefColumnIdx;
   m_RefColumnOffset = rOther.m_RefColumnOffset;

   m_X5              = rOther.m_X5;
   m_X6              = rOther.m_X6;

   m_RebarGrade = rOther.m_RebarGrade;
   m_RebarType = rOther.m_RebarType;

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
