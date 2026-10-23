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
#include <Reporter\Reporter.h>

/*****************************************************************************
CLASS 
   rptXBRPointOfInterest

   Report content for points of interest.
*****************************************************************************/

class XBREXTCLASS rptXBRPointOfInterest : public rptLengthUnitValue
{
public:
   //------------------------------------------------------------------------
   rptXBRPointOfInterest(const WBFL::Units::Length* pUnitOfMeasure = 0,
                      Float64 zeroTolerance = 0.,
                      bool bShowUnitTag = true);

   //------------------------------------------------------------------------
   rptXBRPointOfInterest(const rptXBRPointOfInterest& rOther);

   //------------------------------------------------------------------------
   rptXBRPointOfInterest& operator = (const rptXBRPointOfInterest& rOther);

   //------------------------------------------------------------------------
   virtual rptReportContent* CreateClone() const override;

   //------------------------------------------------------------------------
   virtual rptReportContent& SetValue(const xbrPointOfInterest& poi);

   //------------------------------------------------------------------------
   std::_tstring AsString() const;

   //------------------------------------------------------------------------
   // If set to true, the poi attribute prefixes the distance value
   // eg  (HP) 2.34ft, otherwise it post-fixes the distance value 2.34ft (HP)
   void PrefixAttributes(bool bPrefixAttributes=true);

   //------------------------------------------------------------------------
   // Returns true if the poi is to be prefixed
   bool PrefixAttributes() const;

protected:
   void MakeCopy(const rptXBRPointOfInterest& rOther);
   void MakeAssignment(const rptXBRPointOfInterest& rOther);

private:
   xbrPointOfInterest m_POI;
   bool m_bPrefixAttributes;
};
