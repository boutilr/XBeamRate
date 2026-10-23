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

#include "stdafx.h"
#include <XBeamRateExt\RatingArtifact.h>

#if defined _USE_MULTITHREADING
#include <future>
#endif




struct PostingLoad
{
   Float64 Load;
   Float64 RF;
   Float64 VehicleWeight;
   std::_tstring VehicleName;
   PostingLoad() : Load(DBL_MAX), RF(1.0), VehicleWeight(0.0), VehicleName(_T("Unknown"))
   {
   }
};

template<class T>
PostingLoad ComputeSafePostingLoad(const T& ratingArtifacts)
{
   PostingLoad postingLoad;

   for (const auto& item : ratingArtifacts)
   {
      const auto& artifact = item.second;
      const auto& strVehicle = artifact.GetVehicleName();
      auto W = artifact.GetVehicleWeight();
      auto RF = artifact.GetRatingFactor();
      Float64 spl = W*(RF - 0.3) / 0.7; // safe posting load, MBE Equation 6A.8.3-1
      if (spl < 0)
      {
         spl = 0;
      }

      if (spl < postingLoad.Load)
      {
         postingLoad.Load = spl;
         postingLoad.RF = RF;
         postingLoad.VehicleWeight = W;
         postingLoad.VehicleName = strVehicle;
      }
   }

   return postingLoad;
}

/****************************************************************************
CLASS
   xbrRatingArtifact
****************************************************************************/
xbrRatingArtifact::xbrRatingArtifact(pgsTypes::LoadRatingType ratingType) :
   m_RatingType(ratingType)
{
}

xbrRatingArtifact::xbrRatingArtifact(const xbrRatingArtifact& rOther)
{
   MakeCopy(rOther);
}

xbrRatingArtifact::~xbrRatingArtifact()
{
}

xbrRatingArtifact& xbrRatingArtifact::operator=(const xbrRatingArtifact& rOther)
{
   if ( this != &rOther )
   {
      MakeAssignment(rOther);
   }

   return *this;
}

void xbrRatingArtifact::AddArtifact(const xbrPointOfInterest& poi,const xbrMomentRatingArtifact& artifact,bool bPositiveMoment)
{
   if ( bPositiveMoment )
   {
      m_PositiveMomentRatings.insert( std::make_pair(poi,artifact) );
   }
   else
   {
      m_NegativeMomentRatings.insert( std::make_pair(poi,artifact) );
   }
}

void xbrRatingArtifact::AddArtifact(const xbrPointOfInterest& poi,const xbrShearRatingArtifact& artifact)
{
   m_ShearRatings.insert( std::make_pair(poi,artifact) );
}

void xbrRatingArtifact::AddArtifact(const xbrPointOfInterest& poi,const xbrYieldStressRatioArtifact& artifact,bool bPositiveMoment)
{
   if ( bPositiveMoment )
   {
      m_PositiveMomentYieldStressRatios.insert( std::make_pair(poi,artifact) );
   }
   else
   {
      m_NegativeMomentYieldStressRatios.insert( std::make_pair(poi,artifact) );
   }
}

const xbrRatingArtifact::MomentRatings& xbrRatingArtifact::GetMomentRatings(bool bPositiveMoment) const
{
   return (bPositiveMoment ? m_PositiveMomentRatings : m_NegativeMomentRatings);
}

const xbrRatingArtifact::ShearRatings& xbrRatingArtifact::GetShearRatings() const
{
   return m_ShearRatings;
}

Float64 xbrRatingArtifact::GetMomentRatingFactorEx(bool bPositiveMoment,const xbrMomentRatingArtifact** ppArtifact) const
{
   Float64 RF = DBL_MAX;
   (*ppArtifact) = nullptr;

   MomentRatings::const_iterator iter;
   const MomentRatings* pRatings = (bPositiveMoment ? &m_PositiveMomentRatings : &m_NegativeMomentRatings);

   for ( iter = pRatings->begin(); iter != pRatings->end(); iter++ )
   {
      const xbrMomentRatingArtifact& artifact = iter->second;
      Float64 rating_factor = artifact.GetRatingFactor();
      if ( rating_factor < RF )
      {
         RF = rating_factor;
         (*ppArtifact) = &artifact;
      }
   }

   if ( *ppArtifact == nullptr && 0 < pRatings->size() )
   {
      ATLASSERT(RF == DBL_MAX);
      const xbrMomentRatingArtifact& artifact = pRatings->begin()->second;
      (*ppArtifact) = &artifact;
   }

   return RF;
}

Float64 xbrRatingArtifact::GetMomentRatingFactor(bool bPositiveMoment) const
{
   const xbrMomentRatingArtifact* pArtifact;
   return GetMomentRatingFactorEx(bPositiveMoment,&pArtifact);
}

const xbrMomentRatingArtifact* xbrRatingArtifact::GetMomentRatingArtifact(const xbrPointOfInterest& poi,bool bPositiveMoment) const
{
   const MomentRatings* pRatings = (bPositiveMoment ? &m_PositiveMomentRatings : &m_NegativeMomentRatings);
   MomentRatings::const_iterator found = pRatings->find(poi);
   if ( found == pRatings->end() )
   {
      return nullptr;
   }
   else
   {
      const xbrMomentRatingArtifact& artifact = found->second;
      return &artifact;
   }
}

Float64 xbrRatingArtifact::GetShearRatingFactorEx(const xbrShearRatingArtifact** ppArtifact) const
{
   Float64 RF = DBL_MAX;
   (*ppArtifact) = nullptr;

   ShearRatings::const_iterator iter;
   for ( iter = m_ShearRatings.begin(); iter != m_ShearRatings.end(); iter++ )
   {
      const xbrShearRatingArtifact& artifact = iter->second;
      Float64 rating_factor = artifact.GetRatingFactor();
      if ( rating_factor < RF )
      {
         RF = rating_factor;
         *ppArtifact = &artifact;
      }
   }

   if ( *ppArtifact == nullptr && 0 < m_ShearRatings.size() )
   {
      ATLASSERT(RF == DBL_MAX);
      const xbrShearRatingArtifact& artifact = m_ShearRatings.begin()->second;
      (*ppArtifact) = &artifact;
   }

   return RF;
}

Float64 xbrRatingArtifact::GetShearRatingFactor() const
{
   const xbrShearRatingArtifact* pArtifact;
   return GetShearRatingFactorEx(&pArtifact);
}

const xbrRatingArtifact::YieldStressRatios& xbrRatingArtifact::GetYieldStressRatios(bool bPositiveMoment) const
{
   return (bPositiveMoment ? m_PositiveMomentYieldStressRatios : m_NegativeMomentYieldStressRatios);
}

Float64 xbrRatingArtifact::GetYieldStressRatioEx(bool bPositiveMoment,const xbrYieldStressRatioArtifact** ppArtifact) const
{
   Float64 RF = DBL_MAX;
   (*ppArtifact) = nullptr;

   const YieldStressRatios* pRatios = (bPositiveMoment ? &m_PositiveMomentYieldStressRatios : &m_NegativeMomentYieldStressRatios);
   YieldStressRatios::const_iterator iter;
   for ( iter = pRatios->begin(); iter != pRatios->end(); iter++ )
   {
      const xbrYieldStressRatioArtifact& artifact = iter->second;
      Float64 ratio = artifact.GetStressRatio();
      if ( ratio < RF )
      {
         RF = ratio;
         *ppArtifact = &artifact;
      }
   }

   if ( *ppArtifact == nullptr && 0 < pRatios->size() )
   {
      ATLASSERT(RF == DBL_MAX);
      const xbrYieldStressRatioArtifact& artifact = pRatios->begin()->second;
      (*ppArtifact) = &artifact;
   }

   return RF;
}

Float64 xbrRatingArtifact::GetYieldStressRatio(bool bPositiveMoment) const
{
   const xbrYieldStressRatioArtifact* pArtifact;
   return GetYieldStressRatioEx(bPositiveMoment,&pArtifact);
}

Float64 xbrRatingArtifact::GetRatingFactor() const
{
   const xbrMomentRatingArtifact* pPositiveMoment;
   const xbrMomentRatingArtifact* pNegativeMoment;
   const xbrShearRatingArtifact* pShear;
   const xbrYieldStressRatioArtifact* pYieldStressPositiveMoment;
   const xbrYieldStressRatioArtifact* pYieldStressNegativeMoment;

   return GetRatingFactorEx(&pPositiveMoment, &pNegativeMoment, &pShear, &pYieldStressPositiveMoment, &pYieldStressNegativeMoment);
}

Float64 xbrRatingArtifact::GetRatingFactorEx(const xbrMomentRatingArtifact** ppPositiveMoment,const xbrMomentRatingArtifact** ppNegativeMoment,
                                             const xbrShearRatingArtifact** ppShear,
                                             const xbrYieldStressRatioArtifact** ppYieldStressPositiveMoment,const xbrYieldStressRatioArtifact** ppYieldStressNegativeMoment) const
{
   Float64 RF_pM = GetMomentRatingFactorEx(true,ppPositiveMoment);
   Float64 RF_nM = GetMomentRatingFactorEx(false,ppNegativeMoment);
   Float64 RF_V  = GetShearRatingFactorEx(ppShear);
   Float64 RF_ys_pM = GetYieldStressRatioEx(true,ppYieldStressPositiveMoment);
   Float64 RF_ys_nM = GetYieldStressRatioEx(false,ppYieldStressNegativeMoment);

   Float64 RF = DBL_MAX;
   int i = -1; // initialize to an invalid value so that we know if a rating factor wasn't found
   if ( RF_pM < RF )
   {
      RF = RF_pM;
      i = 0;
   }

   if ( RF_nM < RF )
   {
      RF = RF_nM;
      i = 1;
   }

   if ( RF_V < RF )
   {
      RF = RF_V;
      i = 2;
   }

   if ( RF_ys_pM < RF )
   {
      RF = RF_ys_pM;
      i = 3;
   }

   if ( RF_ys_nM < RF )
   {
      RF = RF_ys_nM;
      i = 4;
   }

   // nullptr out all but the controlling rating
   if ( i == 0 )
   {
      //(*ppPositiveMoment)            = nullptr;
      (*ppNegativeMoment)            = nullptr;
      (*ppShear)                     = nullptr;
      (*ppYieldStressPositiveMoment) = nullptr;
      (*ppYieldStressNegativeMoment) = nullptr;
   }
   else if ( i == 1 )
   {
      (*ppPositiveMoment)            = nullptr;
      //(*ppNegativeMoment)            = nullptr;
      (*ppShear)                     = nullptr;
      (*ppYieldStressPositiveMoment) = nullptr;
      (*ppYieldStressNegativeMoment) = nullptr;
   }
   else if ( i == 2 )
   {
      (*ppPositiveMoment)            = nullptr;
      (*ppNegativeMoment)            = nullptr;
      //(*ppShear)                     = nullptr;
      (*ppYieldStressPositiveMoment) = nullptr;
      (*ppYieldStressNegativeMoment) = nullptr;
   }
   else if ( i == 3 )
   {
      (*ppPositiveMoment)            = nullptr;
      (*ppNegativeMoment)            = nullptr;
      (*ppShear)                     = nullptr;
      //(*ppYieldStressPositiveMoment) = nullptr;
      (*ppYieldStressNegativeMoment) = nullptr;
   }
   else if ( i == 4 )
   {
      (*ppPositiveMoment)            = nullptr;
      (*ppNegativeMoment)            = nullptr;
      (*ppShear)                     = nullptr;
      (*ppYieldStressPositiveMoment) = nullptr;
      //(*ppYieldStressNegativeMoment) = nullptr;
   }
   else
   {
      // ??? rating wasn't done?
      // this can happen if we are rating with a negative moment only truck
      // but the bridge is simple span... we run the truck because we want
      // reactions, but the reating factors are DBL_MAX...
      // since all types of ratings control equally, use positive moment as controlling factor
      ATLASSERT(i == -1);
      //(*ppPositiveMoment)            = nullptr;
      (*ppNegativeMoment)            = nullptr;
      (*ppShear)                     = nullptr;
      (*ppYieldStressPositiveMoment) = nullptr;
      (*ppYieldStressNegativeMoment) = nullptr;
   }
   return RF;
}

bool xbrRatingArtifact::IsLoadPostingRequired() const
{
   return (::IsLegalRatingType(m_RatingType) && !::IsEmergencyRatingType(m_RatingType) && GetRatingFactor() < 1.0 ? true : false);
}

void xbrRatingArtifact::GetSafePostingLoad(Float64* pPostingLoad,Float64* pWeight,Float64* pRF,std::_tstring* pVehicle) const
{
   PostingLoad postingLoad;
#if defined _USE_MULTITHREADING
   std::vector<std::future<PostingLoad>> vFutures;
   vFutures.emplace_back(std::async([&, this] {return ComputeSafePostingLoad(m_PositiveMomentRatings);}));
   vFutures.emplace_back(std::async([&, this] {return ComputeSafePostingLoad(m_NegativeMomentRatings);}));
   vFutures.emplace_back(std::async([&, this] {return ComputeSafePostingLoad(m_ShearRatings);}));
   vFutures.emplace_back(std::async([&, this] {return ComputeSafePostingLoad(m_PositiveMomentYieldStressRatios);}));
   vFutures.emplace_back(std::async([&, this] {return ComputeSafePostingLoad(m_NegativeMomentYieldStressRatios);}));

   for (auto& f : vFutures)
   {
      auto pl = f.get();
      if (pl.Load < postingLoad.Load)
      {
         postingLoad = pl;
      }
   }
#else
   PostingLoad pl_pM = ComputeSafePostingLoad(m_PositiveMomentRatings);
   PostingLoad pl_nM = ComputeSafePostingLoad(m_NegativeMomentRatings);
   PostingLoad pl_V = ComputeSafePostingLoad(m_ShearRatings);
   PostingLoad pl_YS_pM = ComputeSafePostingLoad(m_PositiveMomentYieldStressRatios);
   PostingLoad pl_YS_nM = ComputeSafePostingLoad(m_NegativeMomentYieldStressRatios);

   if (pl_pM.Load < postingLoad.Load)
   {
      postingLoad = pl_pM;
   }

   if (pl_nM.Load < postingLoad.Load)
   {
      postingLoad = pl_nM;
   }

   if (pl_V.Load < postingLoad.Load)
   {
      postingLoad = pl_V;
   }

   if (pl_pMpl_YS_pMLoad < postingLoad.Load)
   {
      postingLoad = pl_YS_pM;
   }

   if (pl_pMpl_YS_nMLoad < postingLoad.Load)
   {
      postingLoad = pl_YS_nM;
   }
#endif

   *pPostingLoad = postingLoad.Load;
   *pRF = postingLoad.RF;
   *pWeight = postingLoad.VehicleWeight;
   *pVehicle = postingLoad.VehicleName;
}

void xbrRatingArtifact::MakeCopy(const xbrRatingArtifact& rOther)
{
   m_RatingType                      = rOther.m_RatingType;
   m_PositiveMomentRatings           = rOther.m_PositiveMomentRatings;
   m_NegativeMomentRatings           = rOther.m_NegativeMomentRatings;
   m_ShearRatings                    = rOther.m_ShearRatings;
   m_PositiveMomentYieldStressRatios = rOther.m_PositiveMomentYieldStressRatios;
   m_NegativeMomentYieldStressRatios = rOther.m_NegativeMomentYieldStressRatios;

}

void xbrRatingArtifact::MakeAssignment(const xbrRatingArtifact& rOther)
{
   MakeCopy( rOther );
}
