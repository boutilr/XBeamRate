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
#include "LiveLoadGraphDefinition.h"


CXBRLiveLoadGraphDefinition::CXBRLiveLoadGraphDefinition()
{
}

CXBRLiveLoadGraphDefinition::CXBRLiveLoadGraphDefinition(
IDType id,
LPCTSTR name,
pgsTypes::LimitState ls
): m_ID(id),m_Name(name)
{
   m_GraphType = graphLimitState;
   m_LoadType.LimitStateType = ls;
}

// constructor for combinations
CXBRLiveLoadGraphDefinition::CXBRLiveLoadGraphDefinition(IDType id,LPCTSTR name,xbrTypes::CombinedForceType comb) :
m_ID(id), m_Name(name)
{
   m_GraphType = graphCombined;
   m_LoadType.CombinedLoadType = comb;
}

// constructor for product loads
CXBRLiveLoadGraphDefinition::CXBRLiveLoadGraphDefinition(
IDType id,
LPCTSTR name,
xbrTypes::ProductForceType type
): m_ID(id),m_Name(name)
{
   m_GraphType = graphProduct;
   m_LoadType.ProductLoadType = type;
}

// constructor for vehicular live loads
CXBRLiveLoadGraphDefinition::CXBRLiveLoadGraphDefinition(IDType id,LPCTSTR name,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) :
m_ID(id),
m_Name(name)
{
   m_GraphType = graphVehicularLiveLoad;
   m_LoadType.LiveLoadType = ratingType;
   m_VehicleIndex = vehicleIdx;
}

// constructor for live loads
CXBRLiveLoadGraphDefinition::CXBRLiveLoadGraphDefinition(IDType id,LPCTSTR name,pgsTypes::LoadRatingType ratingType) :
m_ID(id),
m_Name(name)
{
   m_GraphType = graphLiveLoad;
   m_LoadType.LiveLoadType = ratingType;
}

// constructor for ultimate forces
CXBRLiveLoadGraphDefinition::CXBRLiveLoadGraphDefinition(
IDType id,
LPCTSTR name
): m_ID(id),m_Name(name)
{
   m_GraphType = graphCapacity;
}

//void CAnalysisResultsGraphDefinition::AddIntervals(const std::vector<IntervalIndexType>& intervals)
//{
//   // add new intervals to vector
//   m_IntervalApplicability.insert(intervals.begin(),intervals.end());
//}

//////////////////////////////////////////////////////////////////////
CXBRLiveLoadGraphDefinitions::CXBRLiveLoadGraphDefinitions()
{
}

void CXBRLiveLoadGraphDefinitions::AddGraphDefinition(const CXBRLiveLoadGraphDefinition& def)
{
   m_Definitions.push_back(def);
}

IndexType CXBRLiveLoadGraphDefinitions::GetGraphDefinitionCount() const
{
   return m_Definitions.size();
}

CXBRLiveLoadGraphDefinition& CXBRLiveLoadGraphDefinitions::GetGraphDefinition(IndexType graphIdx)
{
   return m_Definitions[graphIdx];
}

const CXBRLiveLoadGraphDefinition& CXBRLiveLoadGraphDefinitions::GetGraphDefinition(IndexType graphIdx) const
{
   return m_Definitions[graphIdx];
}

CXBRLiveLoadGraphDefinition& CXBRLiveLoadGraphDefinitions::FindGraphDefinition(IDType graphID)
{
   GraphDefinitionIterator iter(m_Definitions.begin());
   GraphDefinitionIterator end(m_Definitions.end());
   for ( ; iter != end; iter++ )
   {
      CXBRLiveLoadGraphDefinition& def(*iter);
      if ( def.m_ID == graphID )
      {
         return def;
      }
   }

   ATLASSERT(false); // should never get here
   return m_Definitions.back();
}

const CXBRLiveLoadGraphDefinition& CXBRLiveLoadGraphDefinitions::FindGraphDefinition(IDType graphID) const
{
   ConstGraphDefinitionIterator iter(m_Definitions.begin());
   ConstGraphDefinitionIterator end(m_Definitions.end());
   for ( ; iter != end; iter++ )
   {
      const CXBRLiveLoadGraphDefinition& def(*iter);
      if ( def.m_ID == graphID )
      {
         return def;
      }
   }

   ATLASSERT(false); // should never get here
   return m_Definitions.back();
}

IndexType CXBRLiveLoadGraphDefinitions::GetGraphIndex(IDType graphID) const
{
   IndexType index = 0;
   ConstGraphDefinitionIterator iter(m_Definitions.begin());
   ConstGraphDefinitionIterator end(m_Definitions.end());
   for ( ; iter != end; iter++, index++ )
   {
      const CXBRLiveLoadGraphDefinition& def = *iter;
      if ( def.m_ID == graphID )
      {
         return index;
      }
   }

   ATLASSERT(false); // graphID not found
   return INVALID_INDEX;
}

void CXBRLiveLoadGraphDefinitions::RemoveGraphDefinition(IndexType graphIdx)
{
   m_Definitions.erase(m_Definitions.begin()+graphIdx);
}

void CXBRLiveLoadGraphDefinitions::RemoveGraphDefinitionByID(IDType graphID)
{
   RemoveGraphDefinition(GetGraphIndex(graphID));
}

void CXBRLiveLoadGraphDefinitions::Clear()
{
   m_Definitions.clear();
}
