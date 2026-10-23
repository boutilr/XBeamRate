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

class XBREXTCLASS xbrBearingLineData
{
public:
   xbrBearingLineData();
   xbrBearingLineData(const xbrBearingLineData& rOther);
   virtual ~xbrBearingLineData();

   xbrBearingLineData& operator = (const xbrBearingLineData& rOther);

   void SetBearingLineOffset(Float64 offset);
   Float64 GetBearingLineOffset() const;
   Float64& GetBearingLineOffset();

   void SetReferenceBearing(pgsTypes::OffsetMeasurementType datum,IndexType refBrgIdx,Float64 refBrgOffset);
   void GetReferenceBearing(pgsTypes::OffsetMeasurementType* pdatum,IndexType* prefBrgIdx,Float64* prefBrgOffset) const;
   pgsTypes::OffsetMeasurementType& GetRefBearingDatum();
   IndexType& GetRefBearingIndex();
   Float64& GetRefBearingOffset();

   void SetSpacing(const std::vector<Float64>& vSpacing);
   const std::vector<Float64>& GetSpacing() const;

   void SetSpacing(IndexType spaceIdx,Float64 s);
   Float64 GetSpacing(IndexType spaceIdx) const;

   void SetBearingCount(IndexType nBearings);
   IndexType GetBearingCount() const;

   HRESULT Load(IStructuredLoad* pStrLoad);
	HRESULT Save(IStructuredSave* pStrSave) const;

protected:
   void MakeCopy(const xbrBearingLineData& rOther);
   void MakeAssignment(const xbrBearingLineData& rOther);

protected:
   // Offset from CL Pier to the bearing line, measure normal to the pier
   Float64 m_BrgLineOffset;

   // dimensions are measured in the plane of the pier, along the CL pier
   pgsTypes::OffsetMeasurementType m_RefBearingDatum;
   IndexType m_RefBearingIndex;
   Float64 m_RefBearingOffset; // offset from the datum point ot the reference bearing
   std::vector<Float64> m_vSpacing; // spacing between bearings
};
