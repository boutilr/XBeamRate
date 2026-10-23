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

#include <XBeamRateExt\XBRExtExp.h>
#include <PsgLib\ColumnData.h>
#include <PsgLib\ConcreteMaterial.h>
#include <XBeamRateExt\LongitudinalRebarData.h>
#include <XBeamRateExt\StirrupData.h>
#include <XBeamRateExt\BearingLineData.h>

#include <WBFLGeometry.h>

class XBREXTCLASS xbrPierData
{
public:
   enum DeckSurfaceType
   {
      Simplified, // deck surface defined by elevation and crown slopes
      Profile // deck surface defined by a profile
   };

   xbrPierData();
   xbrPierData(const xbrPierData& rOther);
   virtual ~xbrPierData();

   xbrPierData& operator = (const xbrPierData& rOther);

   void SetID(PierIDType id);
   PierIDType GetID() const;

   // Set/Get the offset from the alignment to the bridge line
   void SetBridgeLineOffset(Float64 blo);
   Float64 GetBridgeLineOffset() const;
   Float64& GetBridgeLineOffset();

   // Sets/Get the pier skew angle as an input string (e.g. "15 01 14 L")
   void SetSkew(LPCTSTR strSkew);
   LPCTSTR GetSkew() const;

   // Set/Get the deck thickness
   void SetDeckThickness(Float64 tDeck);
   Float64 GetDeckThickness() const;
   Float64& GetDeckThickness();

   // Defines the deck surface description type
   void SetDeckSurfaceType(DeckSurfaceType surfaceType);
   DeckSurfaceType GetDeckSurfaceType() const;

   // Simplified Deck Surface Parameters

   // Set/Get the deck elevation at the alignment line
   void SetDeckElevation(Float64 elev);
   Float64 GetDeckElevation() const;
   Float64& GetDeckElevation();

   // Set/Get the offset from the alignment to the crown point
   void SetCrownPointOffset(Float64 cpo);
   Float64 GetCrownPointOffset() const;
   Float64& GetCrownPointOffset();

   // Set/Get the crown slope on either side of the crown point
   // Slope is measured in the plane of the pier
   void SetCrownSlope(Float64 sLeft,Float64 sRight);
   void GetCrownSlope(Float64* psLeft,Float64* psRight) const;
   Float64& GetLeftCrownSlope();
   Float64& GetRightCrownSlope();

   // Profile Deck Surface Parameters
   void SetDeckProfile(IPoint2dCollection* pProfile);
   void GetDeckProfile(IPoint2dCollection** ppProfile) const;

   // Defines the general superstructure connectivity at this pier
   void SetPierType(xbrTypes::PierType type);
   xbrTypes::PierType GetPierType() const;
   xbrTypes::PierType& GetPierType();

   // Defines the datum for measuring the curb line locations
   pgsTypes::OffsetMeasurementType GetCurbLineDatum() const;
   pgsTypes::OffsetMeasurementType& GetCurbLineDatum();
   void SetCurbLineDatum(pgsTypes::OffsetMeasurementType datumType);

   // Location of curb lines measured from the curb line datum
   void SetCurbLineOffset(Float64 leftCLO,Float64 rightCLO);
   void GetCurbLineOffset(Float64* pLeftCLO,Float64* pRightCLO) const;
   Float64& GetLeftCurbLineOffset();
   Float64& GetRightCurbLineOffset();

   // Set/Get the diaphragm dimensions
   // w = width of the diaphragm. at expansion piers, w is the sum of the left and right diaphragm
   // h = height of the diaphragm from the top of deck to the top of the lower cross beam
   void SetDiaphragmDimensions(Float64 h,Float64 w);
   void GetDiaphragmDimensions(Float64* ph,Float64* pw) const;
   Float64& GetDiaphragmHeight();
   Float64& GetDiaphragmWidth();

   void SetLowerXBeamDimensions(Float64 h1,Float64 h2,Float64 h3,Float64 h4,Float64 x1,Float64 x2,Float64 x3,Float64 x4,Float64 w);
   void GetLowerXBeamDimensions(Float64* ph1,Float64* ph2,Float64* ph3,Float64* ph4,Float64* px1,Float64* px2,Float64* px3,Float64* px4,Float64* pw) const;
   Float64& GetH1();
   Float64& GetH2();
   Float64& GetH3();
   Float64& GetH4();
   Float64& GetX1();
   Float64& GetX2();
   Float64& GetX3();
   Float64& GetX4();
   Float64& GetW();

   // Establishes the location of the columns with respect to the alignment/bridgeline.
   void SetRefColumnLocation(pgsTypes::OffsetMeasurementType refColumnDatum,IndexType refColumnIdx,Float64 refColumnOffset);
   void GetRefColumnLocation(pgsTypes::OffsetMeasurementType* prefColumnDatum,IndexType* prefColumnIdx,Float64* prefColumnOffset) const;
   pgsTypes::OffsetMeasurementType& GetColumnLayoutDatum();
   IndexType& GetRefColumnIndex();
   Float64& GetRefColumnOffset();

   void SetColumnCount(ColumnIndexType nColumns);
   ColumnIndexType GetColumnCount() const;
   void AddColumn(const CColumnData& columnData,Float64 spacing);
   void RemoveColumn(ColumnIndexType colIdx);
   void SetColumnData(ColumnIndexType colIdx,const CColumnData& columnData);
   const CColumnData& GetColumnData(ColumnIndexType colIdx) const;
   CColumnData& GetColumnData(ColumnIndexType colIdx);
   void SetColumnSpacing(SpacingIndexType spaceIdx,Float64 S);
   Float64 GetColumnSpacing(SpacingIndexType spaceIdx) const;
   Float64& GetColumnSpacing(SpacingIndexType spaceIdx);

   // Cross beam overhangs
   void SetXBeamOverhangs(Float64 X5,Float64 X6);
   void GetXBeamOverhangs(Float64* pX5,Float64* pX6) const;
   Float64& GetX5();
   Float64& GetX6();

   // Cross beam condition information
   pgsTypes::ConditionFactorType GetConditionFactorType() const;
   pgsTypes::ConditionFactorType& GetConditionFactorType();
   void SetConditionFactorType(pgsTypes::ConditionFactorType conditionFactorType);

   Float64 GetConditionFactor() const;
   Float64& GetConditionFactor();
   void SetConditionFactor(Float64 conditionFactor);

   // Material
   void SetRebarMaterial(WBFL::Materials::Rebar::Type type,WBFL::Materials::Rebar::Grade grade);
   void GetRebarMaterial(WBFL::Materials::Rebar::Type* pType,WBFL::Materials::Rebar::Grade* pGrade) const;
   WBFL::Materials::Rebar::Type& GetRebarType();
   WBFL::Materials::Rebar::Grade& GetRebarGrade();

   void SetConcreteMaterial(const CConcreteMaterial& concrete);
   CConcreteMaterial& GetConcreteMaterial();
   const CConcreteMaterial& GetConcreteMaterial() const;

   void SetLongitudinalRebar(const xbrLongitudinalRebarData& rebarData);
   xbrLongitudinalRebarData& GetLongitudinalRebar();
   const xbrLongitudinalRebarData& GetLongitudinalRebar() const;

   void SetLowerXBeamStirrups(const xbrStirrupData& stirrupData);
   xbrStirrupData& GetLowerXBeamStirrups();
   const xbrStirrupData& GetLowerXBeamStirrups() const;

   void SetFullDepthStirrups(const xbrStirrupData& stirrupData);
   xbrStirrupData& GetFullDepthStirrups();
   const xbrStirrupData& GetFullDepthStirrups() const;

   void SetBearingLineCount(IndexType nBearingLines);
   IndexType GetBearingLineCount() const;

   void SetBearingLineData(IndexType brgLineIdx,const xbrBearingLineData& bearingLineData);
   xbrBearingLineData& GetBearingLineData(IndexType brgLineIdx);
   const xbrBearingLineData& GetBearingLineData(IndexType brgLineIdx) const;

   void SetBearingSpacing(IndexType brgLineIdx,IndexType brgIdx,Float64 s);
   Float64 GetBearingSpacing(IndexType brgLineIdx,IndexType brgIdx) const;
   
   void SetBearingCount(IndexType brgLineIdx,IndexType nBearings);
   IndexType GetBearingCount(IndexType brgLineIdx) const;

   Float64 GetXBeamLength() const;

   HRESULT Load(IStructuredLoad* pStrLoad,std::shared_ptr<IEAFProgress> pProgress);
	HRESULT Save(IStructuredSave* pStrSave,std::shared_ptr<IEAFProgress> pProgress);

protected:
   void MakeCopy(const xbrPierData& rOther);
   void MakeAssignment(const xbrPierData& rOther);

protected:
   PierIDType m_ID;
   std::_tstring m_strSkew;
   Float64 m_BridgeLineOffset; // offset from alignment to bridge line
   xbrTypes::PierType m_ConnectionType;
   pgsTypes::OffsetMeasurementType m_CurbLineDatum;
   Float64 m_LeftCurbOffset, m_RightCurbOffset; 
   Float64 m_H, m_W; // diaphragm dimensions

   Float64 m_tDeck; // gross thickness of the deck. Locates top of upper diaphragm relative to the roadway surface

   DeckSurfaceType m_DeckSurfaceType;
   // Simplified Deck Surface
   Float64 m_DeckElevation; // elevation of the top of deck at the alignment line
   Float64 m_CrownPointOffset; // offset from alignment to crown point
   Float64 m_LeftCrownSlope, m_RightCrownSlope;
   // Deck Surface Profile
   CComPtr<IPoint2dCollection> m_DeckProfile; // x = offset from alignment, y = elevation

   // Lower X-Beam Dimensions
   Float64 m_H1, m_H2, m_H3, m_H4;
   Float64 m_X1, m_X2, m_X3, m_X4;
   Float64 m_XW;

   // Column Layout
   std::vector<CColumnData> m_vColumnData;
   std::vector<Float64> m_vColumnSpacing;

   IndexType m_RefColumnIdx;
   Float64 m_RefColumnOffset;
   pgsTypes::OffsetMeasurementType m_RefColumnDatum;
   Float64 m_X5, m_X6;

   // Load Rating Condition
   pgsTypes::ConditionFactorType m_ConditionFactorType;
   Float64 m_ConditionFactor;

   // Materials
   WBFL::Materials::Rebar::Type m_RebarType;
   WBFL::Materials::Rebar::Grade m_RebarGrade;
   CConcreteMaterial m_Concrete;

   // Rebar
   xbrLongitudinalRebarData m_LongitudinalRebar;
   xbrStirrupData m_LowerXBeamStirrups;
   xbrStirrupData m_FullDepthStirrups;

   // Bearings
   mutable std::vector<xbrBearingLineData> m_vBearingLines;
};
