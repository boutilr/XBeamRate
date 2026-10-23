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
#include <XBeamRateExt\PointOfInterest.h>
#include <XBeamRateExt\MomentRatingArtifact.h>
#include <XBeamRateExt\ShearRatingArtifact.h>
#include <XBeamRateExt\YieldStressRatioArtifact.h>
#include <map>

/*****************************************************************************
CLASS 
   xbrRatingArtifact

   Artifact for load rating analysis


DESCRIPTION
   Artifact for load rating analysis. Holds rating artificts for
   design, legal, and permit load ratings.

LOG
   rab : 12.07.2009 : Created file
*****************************************************************************/

class XBREXTCLASS xbrRatingArtifact
{
public:
   typedef std::map<xbrPointOfInterest,xbrMomentRatingArtifact> MomentRatings;
   typedef std::map<xbrPointOfInterest,xbrShearRatingArtifact>  ShearRatings;
   typedef std::map<xbrPointOfInterest,xbrYieldStressRatioArtifact> YieldStressRatios;

   xbrRatingArtifact(pgsTypes::LoadRatingType ratingType);
   xbrRatingArtifact(const xbrRatingArtifact& rOther);
   virtual ~xbrRatingArtifact();

   xbrRatingArtifact& operator = (const xbrRatingArtifact& rOther);

   pgsTypes::LoadRatingType GetLoadRatingType() const;

   void AddArtifact(const xbrPointOfInterest& poi,const xbrMomentRatingArtifact& artifact,bool bPositiveMoment);
   void AddArtifact(const xbrPointOfInterest& poi,const xbrShearRatingArtifact& artifact);
   void AddArtifact(const xbrPointOfInterest& poi,const xbrYieldStressRatioArtifact& artifact,bool bPositiveMoment);

   const MomentRatings& GetMomentRatings(bool bPositiveMoment) const;
   const ShearRatings& GetShearRatings() const;
   const YieldStressRatios& GetYieldStressRatios(bool bPositiveMoment) const;

   Float64 GetMomentRatingFactor(bool bPositiveMoment) const;
   Float64 GetMomentRatingFactorEx(bool bPositiveMoment,const xbrMomentRatingArtifact** ppArtifact) const;
   const xbrMomentRatingArtifact* GetMomentRatingArtifact(const xbrPointOfInterest& poi,bool bPositiveMoment) const;

   Float64 GetShearRatingFactor() const;
   Float64 GetShearRatingFactorEx(const xbrShearRatingArtifact** ppArtifact) const;

   Float64 GetYieldStressRatio(bool bPositiveMoment) const;
   Float64 GetYieldStressRatioEx(bool bPositiveMoment,const xbrYieldStressRatioArtifact** ppArtifact) const;

   Float64 GetRatingFactor() const;
   Float64 GetRatingFactorEx(const xbrMomentRatingArtifact** ppPositiveMoment,const xbrMomentRatingArtifact** ppNegativeMoment,
                             const xbrShearRatingArtifact** ppShear,
                             const xbrYieldStressRatioArtifact** ppYieldStressPositiveMoment,const xbrYieldStressRatioArtifact** ppYieldStressNegativeMoment) const;

   bool IsLoadPostingRequired() const;
   void GetSafePostingLoad(Float64* pPostingLoad,Float64* pWeight,Float64* pRF,std::_tstring* pVehicle) const;

protected:
   void MakeCopy(const xbrRatingArtifact& rOther);
   void MakeAssignment(const xbrRatingArtifact& rOther);

   pgsTypes::LoadRatingType m_RatingType;

   MomentRatings m_PositiveMomentRatings;
   MomentRatings m_NegativeMomentRatings;
   ShearRatings  m_ShearRatings;
   YieldStressRatios m_PositiveMomentYieldStressRatios;
   YieldStressRatios m_NegativeMomentYieldStressRatios;
};
