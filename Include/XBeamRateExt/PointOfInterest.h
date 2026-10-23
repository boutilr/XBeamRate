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


typedef Uint64 PoiAttributeType;

#define POI_SECTIONCHANGE 0x00000001
#define POI_COLUMN_LEFT   0x00000002 // POI in XBeam just left of CL of Column
#define POI_COLUMN_RIGHT  0x00000004 // POI in XBeam just right from CL Column
#define POI_FACEOFCOLUMN  0x00000008 // POI in XBeam at face of column
#define POI_BRG           0x00000010 // POI in XBeam at CL Girder Bearing Point
#define POI_WHEELLINE     0x00000020 // POI in XBeam at the location of a wheel line load
#define POI_GRID          0x00000040 // POI in XBeam at a one foot grid location
#define POI_MIDPOINT      0x00000080 // POI in XBeam mid-way between columns
#define POI_FOC_DV        0x00000100 // POI in XBeam at distance Dv from face of column
#define POI_FOC_DV2       0x00000200 // POI in XBeam at distance Dv/2 from face of column

class XBREXTCLASS xbrPointOfInterest
{
public:
   xbrPointOfInterest(PoiIDType id,ColumnIndexType colIdx,Float64 Xpoi,PoiAttributeType attribute=0);
   xbrPointOfInterest(PoiIDType id=INVALID_ID,Float64 Xpoi=0,PoiAttributeType attribute=0);
   xbrPointOfInterest(const xbrPointOfInterest& other) = default;
   ~xbrPointOfInterest() = default;
   xbrPointOfInterest& operator=(const xbrPointOfInterest& other) = default;


   bool operator==(const xbrPointOfInterest& other) const;
   bool operator<(const xbrPointOfInterest& other) const;

   PoiIDType GetID() const;

   bool IsColumnPOI() const;
   ColumnIndexType GetColumnIndex() const;
   void SetColumnIndex(ColumnIndexType colIdx);

   void SetDistFromStart(Float64 Xpoi);
   Float64 GetDistFromStart() const;

   void SetAttributes(PoiAttributeType attribute);
   PoiAttributeType GetAttributes() const;

   bool HasAttributes() const;
   bool HasAttribute(PoiAttributeType attribute) const;

   void ClearAttributes();

   // Returns a string of attribute codes.
   std::_tstring GetAttributes(bool bIncludeMarkup) const;

protected:
   PoiIDType m_ID;
   ColumnIndexType m_ColumnIndex;
   Float64 m_Xpoi;
   PoiAttributeType m_Attributes;

#if defined _DEBUG
   std::_tstring m_strAttributes;
   void UpdateAttributeString();
#endif // _DEBUG
};
