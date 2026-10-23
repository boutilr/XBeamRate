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

#include <WbflTypes.h>

/*****************************************************************************
INTERFACE
   IRatingSpecification

   Interface to manipulating the rating specification input

DESCRIPTION
   Interface to manipulating the rating specification input
*****************************************************************************/
// {34BA2439-C0A2-4171-9FDC-B4307036C07C}
DEFINE_GUID(IID_IXBRRatingSpecification, 
0x34ba2439, 0xc0a2, 0x4171, 0x9f, 0xdc, 0xb4, 0x30, 0x70, 0x36, 0xc0, 0x7c);
class IXBRRatingSpecification
{
public:
   virtual bool IsRatingEnabled(pgsTypes::LoadRatingType ratingType) const = 0;
   virtual void EnableRating(pgsTypes::LoadRatingType ratingType,bool bEnable) = 0;

   virtual void RateForShear(pgsTypes::LoadRatingType ratingType,bool bRateForShear) = 0;
   virtual bool RateForShear(pgsTypes::LoadRatingType ratingType) const = 0;

   // Evaluate yield stress in reinforcement MBE 6A.5.4.2.2b
   virtual void CheckYieldStressLimit(bool bCheckYieldStress) = 0;
   virtual bool CheckYieldStressLimit() const = 0;

   // returns fraction of yield stress that reinforcement can be stressed to during
   // a permit load rating evaluation MBE 6A.5.4.2.2b
   virtual void SetYieldStressLimitCoefficient(Float64 x) = 0;
   virtual Float64 GetYieldStressLimitCoefficient() const = 0;

   // Controls the analysis type to be used for superstructure reactions obtained
   // from PGSuper when PGSuper is in Envelope mode
   virtual pgsTypes::AnalysisType GetAnalysisMethodForReactions() const = 0;
   virtual void SetAnalysisMethodForReactions(pgsTypes::AnalysisType analysisType) = 0;

   // Controls the method used to rating factors for emergency vehicle loading cases
   virtual xbrTypes::EmergencyRatingMethod GetEmergencyRatingMethod() const = 0;
   virtual void SetEmergencyRatingMethod(xbrTypes::EmergencyRatingMethod emergencyRatingMethod) = 0;
   virtual bool IsWSDOTEmergencyRating(pgsTypes::LoadRatingType ratingType) const = 0;

   // Controls the method used to rating factors for permit loading cases
   virtual xbrTypes::PermitRatingMethod GetPermitRatingMethod() const = 0;
   virtual void SetPermitRatingMethod(xbrTypes::PermitRatingMethod permitRatingMethod) = 0;
   virtual bool IsWSDOTPermitRating(pgsTypes::LoadRatingType ratingType) const = 0;

   // Determine this from user settings and column size
   virtual bool DoCheckNegativeMomentBetweenFOCs(PierIDType pierID) const = 0;
};
