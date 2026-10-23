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

interface IShape;
interface IPoint2d;
interface IPoint2dCollection;
interface IRebarSection;
interface IRebarSectionItem;

// {7F04A0B9-FD4E-4965-8F26-8BE78B063803}
DEFINE_GUID(IID_IXBRPier, 
0x7f04a0b9, 0xfd4e, 0x4965, 0x8f, 0x26, 0x8b, 0xe7, 0x8b, 0x6, 0x38, 0x3);
class IXBRPier
{
public:
   virtual Float64 GetSkewAngle(PierIDType pierID) const = 0;

   virtual IndexType GetBearingLineCount(PierIDType pierID) const = 0;
   virtual IndexType GetBearingCount(PierIDType pierID,IndexType brgLineIdx) const = 0;
   virtual Float64 GetBearingLocation(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx) const = 0;
   virtual Float64 GetBearingElevation(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx) const = 0;

   // Returns the number of columns in the pier
   virtual IndexType GetColumnCount(PierIDType pierID) const = 0;

   // Returns the location of the column in Cross Beam Coordinates
   virtual Float64 GetColumnLocation(PierIDType pierID,IndexType colIdx) const = 0;

   virtual Float64 GetColumnHeight(PierIDType pierID,IndexType colIdx) const = 0;
   virtual Float64 GetMaxColumnHeight(PierIDType pierID) const = 0;
   virtual Float64 GetTopColumnElevation(PierIDType pierID,IndexType colIdx) const = 0;
   virtual Float64 GetBottomColumnElevation(PierIDType pierID,IndexType colIdx) const = 0;
   virtual pgsTypes::ColumnTransverseFixityType GetColumnFixity(PierIDType pierID,IndexType colIdx) const = 0;

   // Returns the length of the cross beam measured at the bottom face.
   virtual Float64 GetXBeamLength(xbrTypes::XBeamLocation location,PierIDType pierID) const = 0;

   // Returns the cross beam profile in Pier Coordinates
   virtual void GetUpperXBeamProfile(PierIDType pierID,IShape** ppShape) const = 0;
   virtual void GetLowerXBeamProfile(PierIDType pierID,IShape** ppShape) const = 0;

   // Returns points that define the top/bottom surface of the cross beam
   virtual void GetTopSurface(PierIDType pierID,xbrTypes::Stage stage,IPoint2dCollection** ppPoints) const = 0;
   virtual void GetBottomSurface(PierIDType pierID,xbrTypes::Stage stage,IPoint2dCollection** ppPoints) const = 0;

   // Returns the offset of the crown point from the alignment
   virtual Float64 GetCrownPointOffset(PierIDType pierID) const = 0;

   // Returns the location of the crown point in curb line coordinates
   virtual Float64 GetCrownPointLocation(PierIDType pierID) const = 0;

   // Returns the deck elevation at the specified location in curb line coordinates
   virtual Float64 GetElevation(PierIDType pierID,Float64 Xcl) const = 0;

   // Returns the curb-to-curb width, normal to the alignment at the CL Pier
   virtual Float64 GetCurbToCurbWidth(PierIDType pierID) const = 0;

   virtual Float64 ConvertCrossBeamToCurbLineCoordinate(PierIDType pierID,Float64 Xxb) const = 0;
   virtual Float64 ConvertCurbLineToCrossBeamCoordinate(PierIDType pierID,Float64 Xcl) const = 0;
   virtual Float64 ConvertPierToCrossBeamCoordinate(PierIDType pierID,Float64 Xpier) const = 0;
   virtual Float64 ConvertCrossBeamToPierCoordinate(PierIDType pierID,Float64 Xxb) const = 0;
   virtual Float64 ConvertPierToCurbLineCoordinate(PierIDType pierID,Float64 Xpier) const = 0;
   virtual Float64 ConvertCurbLineToPierCoordinate(PierIDType pierID,Float64 Xcl) const = 0;
};

// {7F260544-5BBC-4be3-87E7-5DF89A45F35D}
DEFINE_GUID(IID_IXBRSectionProperties, 
0x7f260544, 0x5bbc, 0x4be3, 0x87, 0xe7, 0x5d, 0xf8, 0x9a, 0x45, 0xf3, 0x5d);
class IXBRSectionProperties
{
public:
   virtual Float64 GetDepth(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;
   virtual Float64 GetArea(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;
   virtual Float64 GetIxx(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;
   virtual Float64 GetIyy(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;
   virtual Float64 GetYtop(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;
   virtual Float64 GetYbot(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;
   virtual Float64 GetStop(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;
   virtual Float64 GetSbot(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;

   // Returns the total shape of the cross beam cross section at a POI.
   virtual void GetXBeamShape(PierIDType pierID,const xbrPointOfInterest& poi,IShape** ppShape) const = 0;

   // Returns the shape of the cross beam cross section at a POI, taking into account the pier type
   // and the stage.
   virtual void GetXBeamShape(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,IShape** ppShape) const = 0;
};

// {BE372349-0F8D-48e4-90F2-536AC90BEBBE}
DEFINE_GUID(IID_IXBRMaterial, 
0xbe372349, 0xf8d, 0x48e4, 0x90, 0xf2, 0x53, 0x6a, 0xc9, 0xb, 0xeb, 0xbe);
class IXBRMaterial
{
public:
   virtual Float64 GetXBeamDensity(PierIDType pierID) const = 0;
   virtual Float64 GetXBeamFc(PierIDType pierID) const = 0;
   virtual Float64 GetXBeamEc(PierIDType pierID) const = 0;
   virtual Float64 GetXBeamModulusOfRupture(PierIDType pierID) const = 0;
   virtual Float64 GetXBeamLambda(PierIDType pierID) const = 0;
   virtual Float64 GetColumnEc(PierIDType pierID,IndexType colIdx) const = 0;

   virtual void GetRebarProperties(PierIDType pierID,Float64* pE,Float64* pFy,Float64* pFu) const = 0;
};

// {80B9F943-F0BF-4c4b-BCE9-70BBB3A55188}
DEFINE_GUID(IID_IXBRRebar, 
0x80b9f943, 0xf0bf, 0x4c4b, 0xbc, 0xe9, 0x70, 0xbb, 0xb3, 0xa5, 0x51, 0x88);
class IXBRRebar
{
public:
   // The rebar points obtained through this method are in global cross section coordinates.
   virtual void GetRebarSection(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,IRebarSection** ppRebarSection) const = 0;

   virtual IndexType GetRebarRowCount(PierIDType pierID) const = 0;
   virtual IndexType GetRebarCount(PierIDType pierID,IndexType rowIdx) const = 0;
   virtual void GetRebarProfile(PierIDType pierID,IndexType rowIdx,IPoint2dCollection** ppPoints) const = 0;

   virtual Float64 GetDevLengthFactor(PierIDType pierID,const xbrPointOfInterest& poi,IRebarSectionItem* pRebarSectionItem) const = 0;

   // Returns the vertical location of a rebar row, measured in global cross section coordinates.
   virtual Float64 GetRebarRowLocation(PierIDType pierID,const xbrPointOfInterest& poi,IndexType rowIdx) const = 0;
   virtual void GetRebarLocation(PierIDType pierID,const xbrPointOfInterest& poi,IndexType rowIdx,IndexType barIdx,IPoint2d** ppPoint) const = 0;

   // Returns the depth of the rebar, measured down from top of the cross beam
   // for stage 1, measures from the top of the lower cross beam
   virtual Float64 GetRebarDepth(PierIDType pierID,const xbrPointOfInterest& poi,xbrTypes::Stage stage,IPoint2d* pRebarLocation) const = 0;
};

// {025A63FF-9FE0-4733-8AB9-B1B6B96E0F7B}
DEFINE_GUID(IID_IXBRStirrups, 
0x25a63ff, 0x9fe0, 0x4733, 0x8a, 0xb9, 0xb1, 0xb6, 0xb9, 0x6e, 0xf, 0x7b);
class IXBRStirrups
{
public:
   // Stage 1 = Lower cross beam
   // Stage 2 = Full depth cross beam

   virtual ZoneIndexType FindStirrupZone(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;
   virtual ZoneIndexType GetStirrupZoneCount(PierIDType pierID,xbrTypes::Stage stage) const = 0;
   virtual void GetStirrupZoneBoundary(PierIDType pierID,xbrTypes::Stage stage,ZoneIndexType zoneIdx,Float64* pXstart,Float64* pXend) const = 0;
   virtual Float64 GetStirrupZoneLength(PierIDType pierID, xbrTypes::Stage stage, ZoneIndexType zoneIdx) const = 0;
   virtual Float64 GetStirrupZoneSpacing(PierIDType pierID,xbrTypes::Stage stage,ZoneIndexType zoneIdx) const = 0;
   virtual Float64 GetStirrupZoneReinforcement(PierIDType pierID,xbrTypes::Stage stage,ZoneIndexType zoneIdx) const = 0;
   virtual Float64 GetStirrupLegCount(PierIDType pierID,xbrTypes::Stage stage,ZoneIndexType zoneIdx) const = 0;
   virtual IndexType GetStirrupCount(PierIDType pierID,xbrTypes::Stage stage,ZoneIndexType zoneIdx) const = 0;
};
