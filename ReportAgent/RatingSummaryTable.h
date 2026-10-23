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

#include <Reporting\ReportingExp.h>

class IEAFDisplayUnits;

/*****************************************************************************
CLASS 
   CRatingSummaryTable

   Encapsulates the construction of the rating summary table
*****************************************************************************/

class CRatingSummaryTable
{
public:
   enum RatingTableType
   { Design, Legal, Permit };

   //------------------------------------------------------------------------
   // Default constructor
   CRatingSummaryTable();

   //------------------------------------------------------------------------
   // Destructor
   virtual ~CRatingSummaryTable();

   //------------------------------------------------------------------------
   // Builds the rating summary table with results listed by limit state and structural action
   virtual rptRcTable* BuildByLimitState(std::shared_ptr<WBFL::EAF::Broker> pBroker,PierIDType pierID,RatingTableType ratingTableType) const;

   //------------------------------------------------------------------------
   // Builds the rating summary table with results listed by rating vehicle
   virtual rptRcTable* BuildByVehicle(std::shared_ptr<WBFL::EAF::Broker> pBroker,PierIDType pierID,pgsTypes::LoadRatingType ratingType) const;

   //------------------------------------------------------------------------
   // Builds the load posting table for a a legal load rating (but not emergency vehicles)
   virtual rptRcTable* BuildLoadPosting(std::shared_ptr<WBFL::EAF::Broker> pBroker,PierIDType pierID,pgsTypes::LoadRatingType ratingType, bool* pbMustCloseBridge) const;

   //------------------------------------------------------------------------
   // Builds the load posting table for emergency vehicles
   virtual rptRcTable* BuildEmergencyVehicleLoadPosting(std::shared_ptr<WBFL::EAF::Broker> pBroker, PierIDType pierID) const;
};

