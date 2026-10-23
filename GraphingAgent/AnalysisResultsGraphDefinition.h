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

#include <GraphingTypes.h>
#include <IFace\AnalysisResults.h>

class CXBRAnalysisResultsGraphDefinition
{
public:
   IDType m_ID;
   std::_tstring m_Name;
   GraphType m_GraphType;

   union LoadType 
   {
      pgsTypes::LimitState         LimitStateType;
      xbrTypes::ProductForceType   ProductLoadType;
      xbrTypes::CombinedForceType  CombinedLoadType;
      pgsTypes::LoadRatingType     LiveLoadType;
   } m_LoadType;

   VehicleIndexType m_VehicleIndex;

   CXBRAnalysisResultsGraphDefinition();
   
   // constructor for limit states
   CXBRAnalysisResultsGraphDefinition(IDType id,LPCTSTR name,pgsTypes::LimitState ls);
   
   // constructor for combinations
   CXBRAnalysisResultsGraphDefinition(IDType id,LPCTSTR name,xbrTypes::CombinedForceType comb);
   
   // constructor for product loads
   CXBRAnalysisResultsGraphDefinition(IDType id,LPCTSTR name,xbrTypes::ProductForceType type);
   
   // constructor for vehicular live loads
   CXBRAnalysisResultsGraphDefinition(IDType id,LPCTSTR name,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx);

   // constructor for live loads
   CXBRAnalysisResultsGraphDefinition(IDType id,LPCTSTR name,pgsTypes::LoadRatingType ratingType);


   // constructor for capacity
   CXBRAnalysisResultsGraphDefinition(IDType id,LPCTSTR name);

   bool operator< (const CXBRAnalysisResultsGraphDefinition& rOther) const
   {
      return m_ID < rOther.m_ID;
   }
};

class CXBRAnalysisResultsGraphDefinitions
{
public:
   CXBRAnalysisResultsGraphDefinitions();

   void AddGraphDefinition(const CXBRAnalysisResultsGraphDefinition& def);
   IndexType GetGraphDefinitionCount() const;
   CXBRAnalysisResultsGraphDefinition& GetGraphDefinition(IndexType graphIdx);
   const CXBRAnalysisResultsGraphDefinition& GetGraphDefinition(IndexType graphIdx) const;
   CXBRAnalysisResultsGraphDefinition& FindGraphDefinition(IDType graphID);
   const CXBRAnalysisResultsGraphDefinition& FindGraphDefinition(IDType graphID) const;
   IndexType GetGraphIndex(IDType graphID) const;
   void RemoveGraphDefinition(IndexType graphIdx);
   void RemoveGraphDefinitionByID(IDType graphID);

   void Clear();

private:
   typedef std::vector<CXBRAnalysisResultsGraphDefinition> GraphDefinitionContainer;
   typedef GraphDefinitionContainer::iterator GraphDefinitionIterator;
   typedef GraphDefinitionContainer::const_iterator ConstGraphDefinitionIterator;
   GraphDefinitionContainer m_Definitions;
};
