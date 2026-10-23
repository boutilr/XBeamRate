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

/*****************************************************************************
CLASS 
   CLongitudinalRebarData

   Utility class for longitudinal rebar description data.

DESCRIPTION
   Utility class for longitudinal rebar. This class encapsulates 
   the input data a row of rebar and implements the IStructuredLoad 
   and IStructuredSave persistence interfaces.

LOG
   rab : 02.08.2007 : Created file
*****************************************************************************/

class XBREXTCLASS xbrLongitudinalRebarData
{
public:
   class XBREXTCLASS RebarRow 
   {
   public:
      xbrTypes::LongitudinalRebarDatumType Datum;
      xbrTypes::LongitudinalRebarLayoutType LayoutType;
      Float64 Start;
      Float64 Length;
      WBFL::Materials::Rebar::Size BarSize;
      IndexType NumberOfBars;
      Float64     Cover;
      Float64     BarSpacing;
      bool bHookStart;
      bool bHookEnd;

      RebarRow();
      bool operator==(const RebarRow&) const;
   };

   std::vector<RebarRow> RebarRows;

   xbrLongitudinalRebarData();
   xbrLongitudinalRebarData(const xbrLongitudinalRebarData& rOther);
   virtual ~xbrLongitudinalRebarData();

   xbrLongitudinalRebarData& operator = (const xbrLongitudinalRebarData& rOther);
   bool operator == (const xbrLongitudinalRebarData& rOther) const;
   bool operator != (const xbrLongitudinalRebarData& rOther) const;

	HRESULT Load(IStructuredLoad* pStrLoad,std::shared_ptr<IEAFProgress> pProgress);
	HRESULT Save(IStructuredSave* pStrSave,std::shared_ptr<IEAFProgress> pProgress);

protected:
   void MakeCopy(const xbrLongitudinalRebarData& rOther);
   void MakeAssignment(const xbrLongitudinalRebarData& rOther);
};
