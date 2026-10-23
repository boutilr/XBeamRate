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
#include <XBeamRateExt\ReportPointOfInterest.h>


rptXBRPointOfInterest::rptXBRPointOfInterest(const WBFL::Units::Length* pUnitOfMeasure,
                                       Float64 zeroTolerance,
                                       bool bShowUnitTag) :
rptLengthUnitValue(pUnitOfMeasure,zeroTolerance,bShowUnitTag),m_bPrefixAttributes(true)
{
}

rptXBRPointOfInterest::rptXBRPointOfInterest(const rptXBRPointOfInterest& rOther) :
rptLengthUnitValue( rOther )
{
   MakeCopy( rOther );
}

rptXBRPointOfInterest& rptXBRPointOfInterest::operator = (const rptXBRPointOfInterest& rOther)
{
   if ( this != &rOther )
   {
      MakeAssignment( rOther );
   }

   return *this;
}

rptReportContent* rptXBRPointOfInterest::CreateClone() const
{
   return new rptXBRPointOfInterest( *this );
}

rptReportContent& rptXBRPointOfInterest::SetValue(const xbrPointOfInterest& poi)
{
   m_POI = poi;
   return rptLengthUnitValue::SetValue( poi.GetDistFromStart() );
}

std::_tstring rptXBRPointOfInterest::AsString() const
{
   std::_tstring strAttrib = m_POI.GetAttributes(true);
   std::_tstring strValue = rptLengthUnitValue::AsString();

   std::_tstring str;

   if ( strAttrib.size() == 0 )
   {
      str += strValue;
   }
   else
   {
      if ( m_bPrefixAttributes )
      {
         str += _T("(") + strAttrib + _T(") ") + strValue;
      }
      else
      {
         str += strValue + _T(" (") + strAttrib + _T(")");
      }
   }

   return str;
}

void rptXBRPointOfInterest::MakeCopy(const rptXBRPointOfInterest& rOther)
{
   m_POI                   = rOther.m_POI;
   m_bPrefixAttributes     = rOther.m_bPrefixAttributes;
}

void rptXBRPointOfInterest::MakeAssignment(const rptXBRPointOfInterest& rOther)
{
   rptLengthUnitValue::MakeAssignment( rOther );
   MakeCopy( rOther );
}

void rptXBRPointOfInterest::PrefixAttributes(bool bPrefixAttributes)
{
   m_bPrefixAttributes = bPrefixAttributes;
}

bool rptXBRPointOfInterest::PrefixAttributes() const
{
   return m_bPrefixAttributes;
}
