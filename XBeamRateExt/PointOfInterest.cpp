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
#include <XBeamRateExt\PointOfInterest.h>

#include <System\Flags.h>



#if defined _DEBUG
#define UPDATE_ATTRIBUTES UpdateAttributeString()
#else
#define UPDATE_ATTRIBUTES
#endif


xbrPointOfInterest::xbrPointOfInterest(PoiIDType id,ColumnIndexType colIdx,Float64 Xpoi,PoiAttributeType attribute) :
m_ID(id),m_ColumnIndex(colIdx),m_Xpoi(Xpoi),m_Attributes(attribute)
{
   UPDATE_ATTRIBUTES;
}

xbrPointOfInterest::xbrPointOfInterest(PoiIDType id,Float64 Xpoi,PoiAttributeType attribute) :
m_ID(id),m_ColumnIndex(INVALID_INDEX),m_Xpoi(Xpoi),m_Attributes(attribute)
{
   UPDATE_ATTRIBUTES;
}

bool xbrPointOfInterest::operator==(const xbrPointOfInterest& other) const
{
   if ( m_ColumnIndex == other.m_ColumnIndex && IsEqual(m_Xpoi,other.m_Xpoi) && m_Attributes == other.GetAttributes() )
   {
      return true;
   }
   else
   {
      return false;
   }
}

bool xbrPointOfInterest::operator<(const xbrPointOfInterest& other) const
{
   if ( m_ColumnIndex == other.m_ColumnIndex )
   {
      // We want to put right of column poi at end of =='s list
      bool bpcr = HasAttribute(POI_COLUMN_RIGHT);
      bool botherpcr = other.HasAttribute(POI_COLUMN_RIGHT);
      if (m_Xpoi == other.m_Xpoi && botherpcr && !bpcr )
      {
         return true; 
      }
      else
      {
         return m_Xpoi < other.m_Xpoi;
      }
   }

   return m_ColumnIndex < other.m_ColumnIndex;
}

PoiIDType xbrPointOfInterest::GetID() const
{
   return m_ID;
}

bool xbrPointOfInterest::IsColumnPOI() const
{
   return m_ColumnIndex == INVALID_INDEX ? false : true;
}

ColumnIndexType xbrPointOfInterest::GetColumnIndex() const
{
   return m_ColumnIndex;
}

void xbrPointOfInterest::SetColumnIndex(ColumnIndexType colIdx)
{
   m_ColumnIndex = colIdx;
}

void xbrPointOfInterest::SetDistFromStart(Float64 Xpoi)
{
   m_Xpoi = Xpoi;
}

Float64 xbrPointOfInterest::GetDistFromStart() const
{
   return m_Xpoi;
}

void xbrPointOfInterest::SetAttributes(PoiAttributeType attribute)
{
   m_Attributes = attribute;
   UPDATE_ATTRIBUTES;
}

PoiAttributeType xbrPointOfInterest::GetAttributes() const
{
   return m_Attributes;
}

bool xbrPointOfInterest::HasAttributes() const
{
   return (m_Attributes != 0 ? true : false);
}

bool xbrPointOfInterest::HasAttribute(PoiAttributeType attribute) const
{
   return WBFL::System::Flags<PoiAttributeType>::IsSet(m_Attributes,attribute) ? true : false;
}

void xbrPointOfInterest::ClearAttributes()
{
   m_Attributes = 0;
   UPDATE_ATTRIBUTES;
}

std::_tstring xbrPointOfInterest::GetAttributes(bool bIncludeMarkup) const
{
   std::_tstring strAttrib;
   if ( !HasAttributes() )
   {
      return strAttrib;
   }

   int nAttributes = 0;

   if ( HasAttribute(POI_SECTIONCHANGE) )
   {
      if ( 0 < nAttributes )
      {
         strAttrib += _T(", ");
      }

      strAttrib += _T("ST");
      nAttributes++;
   }

   if ( HasAttribute(POI_COLUMN_LEFT) )
   {
      if ( 0 < nAttributes )
      {
         strAttrib += _T(", ");
      }

      strAttrib += _T("Col_left ");
      nAttributes++;
   }

   if ( HasAttribute(POI_COLUMN_RIGHT) )
   {
      if ( 0 < nAttributes )
      {
         strAttrib += _T(", ");
      }

      strAttrib += _T("Col_right ");
      nAttributes++;
   }

   if ( HasAttribute(POI_BRG) )
   {
      if ( 0 < nAttributes )
      {
         strAttrib += _T(", ");
      }

      strAttrib += _T("Brg");
      nAttributes++;
   }

   //if ( HasAttribute(POI_WHEELLINE) )
   //{
   //   if ( 0 < nAttributes )
   //   {
   //      strAttrib += _T(", ");
   //   }

   //   strAttrib += _T("WL");
   //   nAttributes++;
   //}

   //if ( HasAttribute(POI_GRID) )
   //{
   //   if ( 0 < nAttributes )
   //   {
   //      strAttrib += _T(", ");
   //   }

   //   strAttrib += _T("Grid");
   //   nAttributes++;
   //}

   if ( HasAttribute(POI_FACEOFCOLUMN) )
   {
      if ( 0 < nAttributes )
      {
         strAttrib += _T(", ");
      }

      strAttrib += _T("FoC");
      nAttributes++;
   }

   if ( HasAttribute(POI_FOC_DV2) )
   {
      if ( 0 < nAttributes )
      {
         strAttrib += _T(", ");
      }

      strAttrib += _T("FoC_dv2 ");
      nAttributes++;
   }

   if ( HasAttribute(POI_FOC_DV) )
   {
      if ( 0 < nAttributes )
      {
         strAttrib += _T(", ");
      }

      strAttrib += _T("FoC_dv ");
      nAttributes++;
   }

   if ( HasAttribute(POI_MIDPOINT) )
   {
      if ( 0 < nAttributes )
      {
         strAttrib += _T(", ");
      }

      strAttrib += _T("MP");
      nAttributes++;
   }

   return strAttrib;
}

#if defined _DEBUG
void xbrPointOfInterest::UpdateAttributeString()
{
   m_strAttributes.clear();
   std::_tostringstream os;

   if ( WBFL::System::Flags<PoiAttributeType>::IsSet(m_Attributes,POI_SECTIONCHANGE) )
   {
      os << _T("POI_SECTIONCHANGE | ");
   }

   if ( WBFL::System::Flags<PoiAttributeType>::IsSet(m_Attributes,POI_COLUMN_LEFT) )
   {
      os << _T("POI_COLUMN_LEFT | ");
   }

   if ( WBFL::System::Flags<PoiAttributeType>::IsSet(m_Attributes,POI_COLUMN_RIGHT) )
   {
      os << _T("POI_COLUMN_RIGHT | ");
   }

   if ( WBFL::System::Flags<PoiAttributeType>::IsSet(m_Attributes,POI_BRG) )
   {
      os << _T("POI_BRG | ");
   }

   if ( WBFL::System::Flags<PoiAttributeType>::IsSet(m_Attributes,POI_WHEELLINE) )
   {
      os << _T("POI_WHEELLINE | ");
   }

   if ( WBFL::System::Flags<PoiAttributeType>::IsSet(m_Attributes,POI_GRID) )
   {
      os << _T("POI_GRID | ");
   }

   if ( WBFL::System::Flags<PoiAttributeType>::IsSet(m_Attributes,POI_FACEOFCOLUMN) )
   {
      os << _T("POI_FACEOFCOLUMN | ");
   }

   if ( WBFL::System::Flags<PoiAttributeType>::IsSet(m_Attributes,POI_MIDPOINT) )
   {
      os << _T("POI_MIDPOINT | ");
   }

   if ( WBFL::System::Flags<PoiAttributeType>::IsSet(m_Attributes,POI_FOC_DV2) )
   {
      os << _T("POI_FOC_dv2 | ");
   }

   if ( WBFL::System::Flags<PoiAttributeType>::IsSet(m_Attributes,POI_FOC_DV) )
   {
      os << _T("POI_FOC_dv | ");
   }

   os << std::endl;

   m_strAttributes = os.str();
}
#endif // _DEBUG

