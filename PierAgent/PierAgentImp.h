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


// PierAgentImp.h : Declaration of the CPierAgentImp

// This agent is responsible for creating structural analysis models
// and providing analysis results

#pragma once

#include <EAF\Agent.h>
#include <PierAgent.h>
#include "PierAgentCLSID.h"


#include <IFace\Project.h>
#include <IFace\Pier.h>
#include <IFace\PointOfInterest.h>

class CPierAgentImp : public WBFL::EAF::Agent,
   public IXBRPier,
   public IXBRSectionProperties,
   public IXBRMaterial,
   public IXBRRebar,
   public IXBRStirrups,
   public IXBRPointOfInterest,
   public IXBRProjectEventSink
{  
public:
	CPierAgentImp() = default; 

// Agent
public:
   std::_tstring GetName() const override { return _T("XBeam Rate Pier Agent"); }
   bool RegisterInterfaces() override;
   bool Init() override;
   bool Reset() override;
   bool ShutDown() override;
   CLSID GetCLSID() const override;

// IXBRPier
public:
   Float64 GetSkewAngle(PierIDType pierID) const override;
   IndexType GetBearingLineCount(PierIDType pierID) const override;
   IndexType GetBearingCount(PierIDType pierID,IndexType brgLineIdx) const override;
   Float64 GetBearingLocation(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx) const override;
   Float64 GetBearingElevation(PierIDType pierID,IndexType brgLineIdx,IndexType brgIdx) const override;
   IndexType GetColumnCount(PierIDType pierID) const override;
   Float64 GetColumnLocation(PierIDType pierID,IndexType colIdx) const override;
   Float64 GetColumnHeight(PierIDType pierID,IndexType colIdx) const override;
   Float64 GetTopColumnElevation(PierIDType pierID,IndexType colIdx) const override;
   Float64 GetBottomColumnElevation(PierIDType pierID,IndexType colIdx) const override;
   pgsTypes::ColumnTransverseFixityType GetColumnFixity(PierIDType pierID,IndexType colIdx) const override;
   Float64 GetMaxColumnHeight(PierIDType pierID) const override;
   Float64 GetXBeamLength(xbrTypes::XBeamLocation location,PierIDType pierID) const override;
   void GetUpperXBeamProfile(PierIDType pierID,IShape** ppShape) const override;
   void GetLowerXBeamProfile(PierIDType pierID,IShape** ppShape) const override;
   void GetTopSurface(PierIDType pierID,xbrTypes::Stage stage,IPoint2dCollection** ppPoints) const override;
   void GetBottomSurface(PierIDType pierID,xbrTypes::Stage stage,IPoint2dCollection** ppPoints) const override;

   Float64 GetCrownPointOffset(PierIDType pierID) const override;
   Float64 GetCrownPointLocation(PierIDType pierID) const override;

   Float64 GetElevation(PierIDType pierID,Float64 Xcl) const override;

   Float64 GetCurbToCurbWidth(PierIDType pierID) const override;

   Float64 ConvertCrossBeamToCurbLineCoordinate(PierIDType pierID,Float64 Xxb) const override;
   Float64 ConvertCurbLineToCrossBeamCoordinate(PierIDType pierID,Float64 Xcl) const override;
   Float64 ConvertPierToCrossBeamCoordinate(PierIDType pierID,Float64 Xpier) const override;
   Float64 ConvertCrossBeamToPierCoordinate(PierIDType pierID,Float64 Xxb) const override;
   Float64 ConvertPierToCurbLineCoordinate(PierIDType pierID,Float64 Xpier) const override;
   Float64 ConvertCurbLineToPierCoordinate(PierIDType pierID,Float64 Xcl) const override;

// IXBRSectionProperties
public:
   Float64 GetDepth(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;
   Float64 GetArea(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;
   Float64 GetIxx(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;
   Float64 GetIyy(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;
   Float64 GetYtop(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;
   Float64 GetYbot(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;
   Float64 GetStop(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;
   Float64 GetSbot(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;
   void GetXBeamShape(PierIDType pierID,const xbrPointOfInterest& poi,IShape** ppShape) const override;
   void GetXBeamShape(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,IShape** ppShape) const override;

// IXBRMaterial
public:
   Float64 GetXBeamDensity(PierIDType pierID) const override;
   Float64 GetXBeamFc(PierIDType pierID) const override;
   Float64 GetXBeamEc(PierIDType pierID) const override;
   Float64 GetXBeamModulusOfRupture(PierIDType pierID) const override;
   Float64 GetXBeamLambda(PierIDType pierID) const override;
   Float64 GetColumnEc(PierIDType pierID,IndexType colIdx) const override;
   void GetRebarProperties(PierIDType pierID,Float64* pE,Float64* pFy,Float64* pFu) const override;

// IXBRRebar
public:
   void GetRebarSection(PierIDType pierIdx,xbrTypes::Stage stage,const xbrPointOfInterest& poi,IRebarSection** ppRebarSection) const override;
   IndexType GetRebarRowCount(PierIDType pierID) const override;
   IndexType GetRebarCount(PierIDType pierID,IndexType rowIdx) const override;
   void GetRebarProfile(PierIDType pierID,IndexType rowIdx,IPoint2dCollection** ppPoints) const override;
   Float64 GetDevLengthFactor(PierIDType pierID,const xbrPointOfInterest& poi,IRebarSectionItem* pRebarSectionItem) const override;
   Float64 GetRebarRowLocation(PierIDType pierID,const xbrPointOfInterest& poi,IndexType rowIdx) const override;
   void GetRebarLocation(PierIDType pierID,const xbrPointOfInterest& poi,IndexType rowIdx,IndexType barIdx,IPoint2d** ppPoint) const override;
   Float64 GetRebarDepth(PierIDType pierID,const xbrPointOfInterest& poi,xbrTypes::Stage stage,IPoint2d* pRebarLocation) const override;

// IXBRStirrups
public:
   ZoneIndexType FindStirrupZone(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const override;
   ZoneIndexType GetStirrupZoneCount(PierIDType pierID,xbrTypes::Stage stage) const override;
   void GetStirrupZoneBoundary(PierIDType pierID,xbrTypes::Stage stage,ZoneIndexType zoneIdx,Float64* pXstart,Float64* pXend) const override;
   Float64 GetStirrupZoneLength(PierIDType pierID, xbrTypes::Stage stage, ZoneIndexType zoneIdx) const override;
   Float64 GetStirrupZoneSpacing(PierIDType pierID,xbrTypes::Stage stage,ZoneIndexType zoneIdx) const override;
   Float64 GetStirrupZoneReinforcement(PierIDType pierID,xbrTypes::Stage stage,ZoneIndexType zoneIdx) const override;
   Float64 GetStirrupLegCount(PierIDType pierID,xbrTypes::Stage stage,ZoneIndexType zoneIdx) const override;
   IndexType GetStirrupCount(PierIDType pierID,xbrTypes::Stage stage,ZoneIndexType zoneIdx) const override;

// IXBRPointOfInterest
public:
   std::vector<xbrPointOfInterest> GetXBeamPointsOfInterest(PierIDType pierID,PoiAttributeType attrib) const override;
   std::vector<xbrPointOfInterest> GetColumnPointsOfInterest(PierIDType pierID,ColumnIndexType colIdx) const override;
   std::vector<xbrPointOfInterest> GetMomentRatingPointsOfInterest(PierIDType pierID, bool bNegativeMoment) const override;
   std::vector<xbrPointOfInterest> GetShearRatingPointsOfInterest(PierIDType pierID) const override;
   Float64 ConvertPoiToPierCoordinate(PierIDType pierID,const xbrPointOfInterest& poi) const override;
   xbrPointOfInterest ConvertPierCoordinateToPoi(PierIDType pierID,Float64 Xp) const override;
   xbrPointOfInterest GetNearestPointOfInterest(PierIDType pierID,const xbrPointOfInterest& poi) const override;
   xbrPointOfInterest GetNextPointOfInterest(PierIDType pierID,PoiIDType poiID) const override;
   xbrPointOfInterest GetPrevPointOfInterest(PierIDType pierID,PoiIDType poiID) const override;

// IXBRProjectEventSink
public:
   HRESULT OnProjectChanged();

#ifdef _DEBUG
   bool AssertValid() const;
#endif//

private:
   EAF_DECLARE_AGENT_DATA;
   IDType m_dwProjectCookie;

   mutable std::map<PierIDType,CComPtr<IPier>> m_PierModels;
   void ValidatePierModel(PierIDType pierID) const;
   void GetPierModel(PierIDType pierID,IPier** ppPierModel) const;

   void Invalidate();

   mutable PoiIDType m_NextPoiID = 0;
   mutable std::map<PierIDType, std::vector<xbrPointOfInterest>> m_XBeamPoi;

   void ValidatePointsOfInterest(PierIDType pierID) const;
   void SimplifyPOIList(std::vector<xbrPointOfInterest>& vPoi) const;
   const std::vector<xbrPointOfInterest>& GetPointsOfInterest(PierIDType pierID) const;
   bool FindXBeamPoi(PierIDType pierID,Float64 Xxb,xbrPointOfInterest* pPoi) const;

   void GetCrownPoint(PierIDType pierID,IPoint2d** ppPoint) const;

   typedef struct
   {
      Float64 Xstart;
      Float64 Xend;
      Float64 Length;
      WBFL::Materials::Rebar::Size BarSize;
      Float64 S;
      Float64 Av_over_S;
      Float64 nLegs;
      IndexType nStirrups;
   } StirrupZone;
   mutable std::map<PierIDType,std::vector<StirrupZone>> m_StirrupZones[2]; // use xbrTypes::Stage to access array
   const std::vector<StirrupZone>& GetStirrupZones(PierIDType pierID,xbrTypes::Stage stage) const;
   void ValidateStirrupZones(PierIDType pierID,xbrTypes::Stage stage) const;
   void ValidateStirrupZones(PierIDType pierID,const xbrStirrupData& stirrupData,std::vector<StirrupZone>* pvStirrupZones) const;

   std::vector<xbrPointOfInterest> GetRatingPointsOfInterest(PierIDType pierID,bool bShear, bool bNegativeMoment) const;
};


