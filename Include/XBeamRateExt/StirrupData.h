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
#include <Materials/Rebar.h>
#include <Units\Units.h>


/*****************************************************************************
CLASS 
   xbrStirrupData

   Utility class for shear zone description data.
*****************************************************************************/

class XBREXTCLASS xbrStirrupData
{
public:
   class XBREXTCLASS StirrupZone
   {
   public:
      Float64 Length;
      Float64 BarSpacing;
      WBFL::Materials::Rebar::Size BarSize;
      Float64 nBars;

      StirrupZone():
      Length(0), BarSpacing(WBFL::Units::ConvertToSysUnits(6.0,WBFL::Units::Measure::Inch)), BarSize(WBFL::Materials::Rebar::Size::bs6), nBars(4)
      {;}

      bool operator==(const StirrupZone& other) const
      {
         if (BarSize != other.BarSize) return false;
         if ( !IsEqual(Length,  other.Length) ) return false;
         if ( !IsEqual(BarSpacing,  other.BarSpacing) ) return false;
         if ( !IsEqual(nBars,other.nBars) ) return false;

         return true;
      };
   };

   bool Symmetric;
   std::vector<StirrupZone> Zones;

   // GROUP: LIFECYCLE

   //------------------------------------------------------------------------
   // Default constructor
   xbrStirrupData();

   //------------------------------------------------------------------------
   // Copy constructor
   xbrStirrupData(const xbrStirrupData& rOther);

   //------------------------------------------------------------------------
   // Destructor
   ~xbrStirrupData();

   // GROUP: OPERATORS
   //------------------------------------------------------------------------
   // Assignment operator
   xbrStirrupData& operator = (const xbrStirrupData& rOther);
   bool operator == (const xbrStirrupData& rOther) const;
   bool operator != (const xbrStirrupData& rOther) const;

   // GROUP: OPERATIONS

	HRESULT Load(IStructuredLoad* pStrLoad);
	HRESULT Save(IStructuredSave* pStrSave);

   // GROUP: ACCESS
   // GROUP: INQUIRY

protected:
   // GROUP: DATA MEMBERS
   // GROUP: LIFECYCLE
   // GROUP: OPERATORS
   // GROUP: OPERATIONS
   //------------------------------------------------------------------------
   void MakeCopy(const xbrStirrupData& rOther);

   //------------------------------------------------------------------------
   void MakeAssignment(const xbrStirrupData& rOther);

   // GROUP: ACCESS
   // GROUP: INQUIRY

private:
   // GROUP: DATA MEMBERS
   // GROUP: LIFECYCLE
   // GROUP: OPERATORS
   // GROUP: OPERATIONS
   // GROUP: ACCESS
   // GROUP: INQUIRY
};

// INLINE METHODS
//

// EXTERNAL REFERENCES
//
//class PSGLIBCLASS HorizontalInterfaceZoneDataLess
//{
//public:
//   bool operator()(const CHorizontalInterfaceZoneData& a, const CHorizontalInterfaceZoneData& b)
//   {
//      return a.ZoneNum < b.ZoneNum;
//   }
//};

