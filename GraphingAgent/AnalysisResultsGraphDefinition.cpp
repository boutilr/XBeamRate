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
#include "AnalysisResultsGraphDefinition.h"


CXBRAnalysisResultsGraphDefinition::CXBRAnalysisResultsGraphDefinition()
{
}

CXBRAnalysisResultsGraphDefinition::CXBRAnalysisResultsGraphDefinition(
IDType id,
LPCTSTR name,
pgsTypes::LimitState ls
): m_ID(id),m_Name(name)
{
   m_GraphType = graphLimitState;
   m_LoadType.LimitStateType = ls;
}

// constructor for combinations
CXBRAnalysisResultsGraphDefinition::CXBRAnalysisResultsGraphDefinition(IDType id,LPCTSTR name,xbrTypes::CombinedForceType comb) :
m_ID(id), m_Name(name)
{
   m_GraphType = graphCombined;
   m_LoadType.CombinedLoadType = comb;
}

// constructor for product loads
CXBRAnalysisResultsGraphDefinition::CXBRAnalysisResultsGraphDefinition(
IDType id,
LPCTSTR name,
xbrTypes::ProductForceType type
): m_ID(id),m_Name(name)
{
   m_GraphType = graphProduct;
   m_LoadType.ProductLoadType = type;
}

// constructor for vehicular live loads
CXBRAnalysisResultsGraphDefinition::CXBRAnalysisResultsGraphDefinition(IDType id,LPCTSTR name,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx) :
m_ID(id),
m_Name(name)
{
   m_GraphType = graphVehicularLiveLoad;
   m_LoadType.LiveLoadType = ratingType;
   m_VehicleIndex = vehicleIdx;
}

// constructor for live loads
CXBRAnalysisResultsGraphDefinition::CXBRAnalysisResultsGraphDefinition(IDType id,LPCTSTR name,pgsTypes::LoadRatingType ratingType) :
m_ID(id),
m_Name(name)
{
   m_GraphType = graphLiveLoad;
   m_LoadType.LiveLoadType = ratingType;
}

// constructor for ultimate forces
CXBRAnalysisResultsGraphDefinition::CXBRAnalysisResultsGraphDefinition(
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
CXBRAnalysisResultsGraphDefinitions::CXBRAnalysisResultsGraphDefinitions()
{
}

void CXBRAnalysisResultsGraphDefinitions::AddGraphDefinition(const CXBRAnalysisResultsGraphDefinition& def)
{
   m_Definitions.push_back(def);
}

IndexType CXBRAnalysisResultsGraphDefinitions::GetGraphDefinitionCount() const
{
   return m_Definitions.size();
}

CXBRAnalysisResultsGraphDefinition& CXBRAnalysisResultsGraphDefinitions::GetGraphDefinition(IndexType graphIdx)
{
   return m_Definitions[graphIdx];
}

const CXBRAnalysisResultsGraphDefinition& CXBRAnalysisResultsGraphDefinitions::GetGraphDefinition(IndexType graphIdx) const
{
   return m_Definitions[graphIdx];
}

CXBRAnalysisResultsGraphDefinition& CXBRAnalysisResultsGraphDefinitions::FindGraphDefinition(IDType graphID)
{
   GraphDefinitionIterator iter(m_Definitions.begin());
   GraphDefinitionIterator end(m_Definitions.end());
   for ( ; iter != end; iter++ )
   {
      CXBRAnalysisResultsGraphDefinition& def(*iter);
      if ( def.m_ID == graphID )
      {
         return def;
      }
   }

   ATLASSERT(false); // should never get here
   return m_Definitions.back();
}

const CXBRAnalysisResultsGraphDefinition& CXBRAnalysisResultsGraphDefinitions::FindGraphDefinition(IDType graphID) const
{
   ConstGraphDefinitionIterator iter(m_Definitions.begin());
   ConstGraphDefinitionIterator end(m_Definitions.end());
   for ( ; iter != end; iter++ )
   {
      const CXBRAnalysisResultsGraphDefinition& def(*iter);
      if ( def.m_ID == graphID )
      {
         return def;
      }
   }

   ATLASSERT(false); // should never get here
   return m_Definitions.back();
}

IndexType CXBRAnalysisResultsGraphDefinitions::GetGraphIndex(IDType graphID) const
{
   IndexType index = 0;
   ConstGraphDefinitionIterator iter(m_Definitions.begin());
   ConstGraphDefinitionIterator end(m_Definitions.end());
   for ( ; iter != end; iter++, index++ )
   {
      const CXBRAnalysisResultsGraphDefinition& def = *iter;
      if ( def.m_ID == graphID )
      {
         return index;
      }
   }

   ATLASSERT(false); // graphID not found
   return INVALID_INDEX;
}

void CXBRAnalysisResultsGraphDefinitions::RemoveGraphDefinition(IndexType graphIdx)
{
   m_Definitions.erase(m_Definitions.begin()+graphIdx);
}

void CXBRAnalysisResultsGraphDefinitions::RemoveGraphDefinitionByID(IDType graphID)
{
   RemoveGraphDefinition(GetGraphIndex(graphID));
}

void CXBRAnalysisResultsGraphDefinitions::Clear()
{
   m_Definitions.clear();
}
