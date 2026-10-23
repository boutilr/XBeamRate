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

#include <PGSuperTypes.h>

typedef struct xbrTypes
{
   typedef enum PierType
   {
      pctContinuous, // superstructure is continuous but hinged with substructure
      pctIntegral,   // superstructure is fully continuous with substructure
      pctExpansion   // no moment connectivity at pier
   } PierType;

   typedef enum TransverseDimensionMeasurementType
   {
      tdmNormalToAlignment, // transverse dimensions are normal to the alignment
      tdmPlaneOfPier        // transverse dimensions are in the plane of the pier
   } TransverseDimensionMeasurementType;

   typedef enum Stage
   {
      Stage1, // lower cross beam only
      Stage2  // upper and lower cross beam together
   } Stage;

   typedef enum LongitudinalRebarDatumType
   {
      Top,           // rebar cover is measured from top of upper cross beam
      TopLowerXBeam, // rebar cover is measured from top of lower cross beam
      Bottom,        // rebar cover is measured from bottom of cross beam
   } LongitudinalRebarDatumType;

   typedef enum LongitudinalRebarLayoutType
   {
      blLeftEnd,  // start location is measured from left end of cross beam
      blRightEnd,  // start location is measured from right end of cross beam
      blFullLength // bar is full length of cross beam (start and length parameters ignored)
   } LongitudinalRebarLayoutType;

   typedef enum ProductForceType
   {
      pftLowerXBeam,
      pftUpperXBeam,
      pftDCReactions,
      pftDWReactions,
      pftCRReactions,
      pftSHReactions,
      pftPSReactions,
      pftREReactions
   } ProductForceType;

   typedef enum CombinedForceType
   {
      lcDC,
      lcDW,
      lcCR,
      lcSH,
      lcRE,
      lcPS
   } CombinedForceType;

   typedef enum ReactionLoadType
   {
      rltConcentrated, // DC/DW reaction values are concentrated forces
      rltUniform // DC/DW are uniform load forces (FORCE/LENGTH) centered about the CL Bearing
   } ReactionLoadType;

   typedef enum ReactionLoadApplicationType
   {
      rlaCrossBeam,  // apply wheel line reactions directly to cross beam
      rlaBearings    // apply wheel line reactions to bearings through a load transfer model
   } ReactionLoadApplicationType;

   typedef enum PermitRatingMethod
   {
      prmAASHTO, // use AASHTO MBE Equation 6A.4.2.1-1
      prmWSDOT   // use WSDOT BDM Equation 13.1.1A-2
   } PermitRatingMethod;

   typedef enum EmergencyRatingMethod
   {
      ermAASHTO, // use AASHTO MBE Equation 6A.4.2.1-1
      ermWSDOT   // use WSDOT BDM Equation 13.1.1A-2
   } EmergencyRatingMethod;

   typedef enum LoadType
   {
      ltPermanent, // permanent loads 
      ltTransient  // transient loads
      // used for cracked section properties (See LRFD 5.7.1 and MBE example in appendix A2)
   } LoadType;

   typedef enum XBeamLocation
   {
      xblTopUpperXBeam,
      xblTopLowerXBeam,
      xblBottomXBeam
   } XBeamLocation;

} xbrTypes;