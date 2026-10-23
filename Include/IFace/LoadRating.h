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
#include <XBeamRateExt\RatingArtifact.h>

#include <WBFLRCCapacity.h>
typedef struct MomentCapacityDetails
{
   Float64 dt; // distance from compression face to extreme tensile reinforcement
   Float64 de; // distance from compression face to resultant tensile force
   Float64 dc; // distance from compression face to resultant compression force
   Float64 phi; // capacity reduction factor
   Float64 Mn; // nominal capacity
   Float64 Mr; // nominal resistance (phi*Mn)
   CComPtr<IRCBeam2> rcBeam; // need this for cracked section analysis, so cache it here
   CComPtr<ILRFDSolutionEx> solution;
} MomentCapacityDetails;


typedef struct CrackingMomentDetails
{
   Float64 Mcr;  // Cracking moment
   Float64 Mdnc; // Dead load moment on non-composite XBeam (lower x-beam dead load)
   Float64 fr;   // Rupture stress
   Float64 fcpe; // Stress at bottom of non-composite XBeam due to prestress
   Float64 Snc;  // Section modulus of non-composite XBeam
   Float64 Sc;   // Section modulus of composite XBeam
   Float64 McrLimit; // Limiting cracking moment ... per 2nd Edition + 2003 interims (changed in 2005 interims)

   Float64 g1,g2,g3; // gamma factors from LRFD 5.7.3.3.2 (LRFD 6th Edition, 2012)
} CrackingMomentDetails;

typedef struct MinMomentCapacityDetails
{
   Float64 Mr;     // Nominal resistance (phi*Mn)
   Float64 Mcr;
   Float64 MrMin;  // Minimum nominal resistance Max(MrMin1,MrMin2)
   Float64 MrMin1; // 1.2Mcr
   Float64 MrMin2; // 1.33Mu
   Float64 Mu;
} MinMomentCapacityDetails;

typedef struct CrackedSectionDetails
{
   Float64 n;   // modular ratio used to compute properties (LRFD 5.7.1)
   Float64 c; // distance from compression face to crack (crack depth)
   Float64 Ycr;   // distance from top of section to crack (crack location)
   Float64 Icr; // cracked section moment of inertia
   CComPtr<ICrackedSectionSolution> CrackedSectionSolution;
} CrackedSectionDetails;

typedef struct ShearCapacityDetails
{
   Float64 beta;
   Float64 theta;
   Float64 bv;
   Float64 dv1; // shear depth for stage 1 reinforcement
   Float64 dv2; // shear depth for stage 2 reinforcement
   Float64 dv;  // shear depth used to compute shear capacity (max of dv1,dv2)
   Float64 Av_over_S1; // average Av/S for stage 1 reinforcement
   Float64 Av_over_S2; // average Av/S for stage 2 reinforcement
   Float64 Vc;
   Float64 Vs;
   Float64 Vn1; // LRFD Eq 5.8.3.3-1
   Float64 Vn2; // LRFD Eq 5.8.3.3-2
   Float64 Vn;  // Min of Vn1 and Vn2 (LRFD 5.8.3.3)
   Float64 phi;
   Float64 Vr; // phi*Vn
} ShearCapacityDetails;

typedef struct AvOverSZone
{
   Float64 Start;
   Float64 End;
   Float64 Length;
   Float64 AvOverS;
} AvOverSZone;

typedef struct AvOverSDetails
{
   std::vector<AvOverSZone> Zones;
   Float64 ShearFailurePlaneLength;
   Float64 AvgAvOverS;
   AvOverSDetails() { ShearFailurePlaneLength = 0; AvgAvOverS = 0; }
} AvOverSDetails;

typedef struct DvDetails
{
   // NOTE: array index 0, positive moment, 1, negative moment
   Float64 h; // height of the section
   Float64 de[2]; // depth to resultant tensile force
   Float64 MomentArm[2]; // moment arm
   Float64 MomentDv[2]; // dv based on type of moment ([0] = dv for positive moment, [1] = dv for negative moment)
   Float64 dv; // dv, per LRFD 5.8.2.9 (minimum of MomentDv[0] and MomentDv[1])
} DvDetails;

/*****************************************************************************
INTERFACE
   IXBRMomentCapacity

DESCRIPTION
   Interface for getting moment capacity information
*****************************************************************************/
// {15281829-5F1E-4a78-9A9C-E619A7B1E0F6}
DEFINE_GUID(IID_IXBRMomentCapacity, 
0x15281829, 0x5f1e, 0x4a78, 0x9a, 0x9c, 0xe6, 0x19, 0xa7, 0xb1, 0xe0, 0xf6);
class IXBRMomentCapacity
{
public:
   virtual Float64 GetMomentCapacity(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const = 0;
   virtual const MomentCapacityDetails& GetMomentCapacityDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const = 0;

   virtual Float64 GetCrackingMoment(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const = 0;
   virtual const CrackingMomentDetails& GetCrackingMomentDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const = 0;

   virtual Float64 GetMinMomentCapacity(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const = 0;
   virtual const MinMomentCapacityDetails& GetMinMomentCapacityDetails(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment) const = 0;

   // used for WSDOT Permit rating type
   virtual MinMomentCapacityDetails GetMinMomentCapacityDetails(PierIDType pierID,pgsTypes::LimitState limitState,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx) const = 0;
};

/*****************************************************************************
INTERFACE
   IXBRShearCapacity

DESCRIPTION
   Interface for getting shear capacity information
*****************************************************************************/
// {BD0D6DBC-2127-4413-BB1D-16BBD33B054D}
DEFINE_GUID(IID_IXBRShearCapacity, 
0xbd0d6dbc, 0x2127, 0x4413, 0xbb, 0x1d, 0x16, 0xbb, 0xd3, 0x3b, 0x5, 0x4d);
class IXBRShearCapacity
{
public:
   virtual Float64 GetShearCapacity(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;
   virtual const ShearCapacityDetails& GetShearCapacityDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;

   virtual const AvOverSDetails& GetAverageAvOverSDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;

   virtual Float64 GetDv(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;
   virtual const DvDetails& GetDvDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi) const = 0;
};

/*****************************************************************************
INTERFACE
   IXBRCrackedSection

DESCRIPTION
   Interface for getting cracked section information
*****************************************************************************/
// {20128EE2-B293-49dc-A335-DA01574AC2A8}
DEFINE_GUID(IID_IXBRCrackedSection, 
0x20128ee2, 0xb293, 0x49dc, 0xa3, 0x35, 0xda, 0x1, 0x57, 0x4a, 0xc2, 0xa8);
class IXBRCrackedSection
{
public:
   virtual Float64 GetIcrack(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,xbrTypes::LoadType loadType) const = 0;
   virtual const CrackedSectionDetails& GetCrackedSectionDetails(PierIDType pierID,xbrTypes::Stage stage,const xbrPointOfInterest& poi,bool bPositiveMoment,xbrTypes::LoadType loadType) const = 0;
};

/*****************************************************************************
INTERFACE
   IXBRArtifact

DESCRIPTION
   Interface for getting analysis artifacts
*****************************************************************************/
// {E7B694B8-1B81-4b53-A737-F2153CCCB5CA}
DEFINE_GUID(IID_IXBRArtifact, 
0xe7b694b8, 0x1b81, 0x4b53, 0xa7, 0x37, 0xf2, 0x15, 0x3c, 0xcc, 0xb5, 0xca);
class IXBRArtifact
{
public:
   virtual const xbrRatingArtifact* GetXBeamRatingArtifact(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) const = 0;
};
