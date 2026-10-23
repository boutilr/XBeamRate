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

#include <XBeamRateExt\PointOfInterest.h>

// {DC508193-9C4A-4047-9B5E-7EEFEC1E18B0}
DEFINE_GUID(IID_IXBRPointOfInterest, 
0xdc508193, 0x9c4a, 0x4047, 0x9b, 0x5e, 0x7e, 0xef, 0xec, 0x1e, 0x18, 0xb0);
class IXBRPointOfInterest
{
public:
   virtual std::vector<xbrPointOfInterest> GetXBeamPointsOfInterest(PierIDType pierID,PoiAttributeType attrib = 0) const = 0;
   virtual std::vector<xbrPointOfInterest> GetColumnPointsOfInterest(PierIDType pierID,ColumnIndexType colIdx) const = 0;
   virtual std::vector<xbrPointOfInterest> GetMomentRatingPointsOfInterest(PierIDType pierID, bool bNegativeMoment) const = 0;
   virtual std::vector<xbrPointOfInterest> GetShearRatingPointsOfInterest(PierIDType pierID) const = 0;

   virtual Float64 ConvertPoiToPierCoordinate(PierIDType pierID,const xbrPointOfInterest& poi) const = 0;
   virtual xbrPointOfInterest ConvertPierCoordinateToPoi(PierIDType pierID,Float64 Xp) const = 0;
   virtual xbrPointOfInterest GetNearestPointOfInterest(PierIDType pierID,const xbrPointOfInterest& poi) const = 0;
   virtual xbrPointOfInterest GetNextPointOfInterest(PierIDType pierID,PoiIDType poiID) const = 0;
   virtual xbrPointOfInterest GetPrevPointOfInterest(PierIDType pierID,PoiIDType poiID) const = 0;
};
