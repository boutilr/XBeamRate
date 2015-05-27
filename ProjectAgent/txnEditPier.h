///////////////////////////////////////////////////////////////////////
// PGSuper - Prestressed Girder SUPERstructure Design and Analysis
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
#pragma once

#include <WBFLCore.h>
#include <PgsExt\ColumnData.h>

class txnBearingData
{
public:
   txnBearingData();
   Float64 m_S; // spacing to next bearing (ignored for last bearing)
   Float64 m_DC;
   Float64 m_DW;
};

class txnLiveLoadData
{
public:
   txnLiveLoadData();
   std::vector<std::pair<std::_tstring,Float64>> m_LLIM;
};

class txnLongutindalRebarData
{
public:
   txnLongutindalRebarData();
   xbrTypes::LongitudinalRebarDatumType datum;
   Float64 cover;
   matRebar::Size barSize;
   Int16 nBars;
   Float64 spacing;
};

class txnEditPierData
{
public:
   txnEditPierData();

   xbrTypes::PierConnectionType m_PierType;
   Float64 m_DeckElevation;
   Float64 m_CrownPointOffset;
   Float64 m_BridgeLineOffset;
   CString m_strOrientation;

   pgsTypes::OffsetMeasurementType m_CurbLineDatum;
   Float64 m_LeftCLO;
   Float64 m_RightCLO;
   Float64 m_SL;
   Float64 m_SR;

   Float64 m_DiaphragmHeight;
   Float64 m_DiaphragmWidth;

   IndexType m_nBearingLines;
   std::vector<txnBearingData> m_BearingLines[2];
   IndexType m_RefBearingIdx[2];
   Float64 m_RefBearingLocation[2];
   pgsTypes::OffsetMeasurementType m_RefBearingDatum[2];


   Float64 m_Ec;
   std::vector<txnLongutindalRebarData> m_Rebar;

   ColumnIndexType m_nColumns;
   ColumnIndexType m_RefColumnIdx;
   Float64 m_TransverseOffset;
   pgsTypes::OffsetMeasurementType m_TransverseOffsetMeasurement;
   Float64 m_XBeamWidth;
   Float64 m_XBeamHeight[2];
   Float64 m_XBeamTaperHeight[2];
   Float64 m_XBeamTaperLength[2];
   Float64 m_XBeamOverhang[2];

   CColumnData::ColumnHeightMeasurementType m_ColumnHeightMeasurementType;
   Float64 m_ColumnHeight;
   Float64 m_ColumnSpacing;

   CColumnData::ColumnShapeType m_ColumnShape;
   Float64 m_B;
   Float64 m_D;

   pgsTypes::ConditionFactorType m_ConditionFactorType;
   Float64 m_ConditionFactor;

   Float64 m_gDC;
   Float64 m_gDW;
   Float64 m_gLL[6]; // use pgsTypes::LoadRatingType to access array

   txnLiveLoadData m_DesignLiveLoad;
   txnLiveLoadData m_LegalRoutineLiveLoad;
   txnLiveLoadData m_LegalSpecialLiveLoad;
   txnLiveLoadData m_PermitRoutineLiveLoad;
   txnLiveLoadData m_PermitSpecialLiveLoad;
};

class txnEditPier :
   public txnTransaction
{
public:
   txnEditPier(const txnEditPierData& oldPierData,const txnEditPierData& newPierData);
   ~txnEditPier(void);

   virtual bool Execute();
   virtual void Undo();
   virtual txnTransaction* CreateClone() const;
   virtual std::_tstring Name() const;
   virtual bool IsUndoable();
   virtual bool IsRepeatable();

private:
   void Execute(int i);

   txnEditPierData m_PierData[2];

};
