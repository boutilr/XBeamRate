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
#include <XBeamRateExt\LongitudinalRebarData.h>


xbrLongitudinalRebarData::RebarRow::RebarRow():
   Datum(xbrTypes::Bottom), 
   LayoutType(xbrTypes::blFullLength), 
   Start(0), 
   Length(0), 
   BarSize(WBFL::Materials::Rebar::Size::bs10), 
   NumberOfBars(1), 
   Cover(WBFL::Units::ConvertToSysUnits(2.0,WBFL::Units::Measure::Inch)), 
   BarSpacing(WBFL::Units::ConvertToSysUnits(6.0,WBFL::Units::Measure::Inch)),
   bHookStart(false),
   bHookEnd(false)
{;}

bool xbrLongitudinalRebarData::RebarRow::operator==(const xbrLongitudinalRebarData::RebarRow& rOther) const
{
   if(Datum != rOther.Datum) return false;
   if(LayoutType != rOther.LayoutType) return false;
   if ( !IsEqual(Start,rOther.Start) ) return false;
   if ( !IsEqual(Length,rOther.Length) ) return false;
   if(BarSize != rOther.BarSize) return false;
   if ( !IsEqual(Cover,  rOther.Cover) ) return false;
   if ( !IsEqual(BarSpacing,  rOther.BarSpacing) ) return false;
   if ( NumberOfBars != rOther.NumberOfBars ) return false;
   if ( bHookStart != rOther.bHookStart) return false;
   if ( bHookEnd   != rOther.bHookEnd) return false;

   return true;
};

/****************************************************************************
CLASS
   xbrLongitudinalRebarData
****************************************************************************/
xbrLongitudinalRebarData::xbrLongitudinalRebarData()
{
}

xbrLongitudinalRebarData::xbrLongitudinalRebarData(const xbrLongitudinalRebarData& rOther)
{
   MakeCopy(rOther);
}

xbrLongitudinalRebarData::~xbrLongitudinalRebarData()
{
}

//======================== OPERATORS  =======================================
xbrLongitudinalRebarData& xbrLongitudinalRebarData::operator= (const xbrLongitudinalRebarData& rOther)
{
   if( this != &rOther )
   {
      MakeAssignment(rOther);
   }

   return *this;
}

bool xbrLongitudinalRebarData::operator == (const xbrLongitudinalRebarData& rOther) const
{
   return (RebarRows == rOther.RebarRows);
}

bool xbrLongitudinalRebarData::operator != (const xbrLongitudinalRebarData& rOther) const
{
   return !operator==( rOther );
}


HRESULT xbrLongitudinalRebarData::Save(IStructuredSave* pStrSave,std::shared_ptr<IEAFProgress> pProgress)
{
   HRESULT hr = S_OK;

   pStrSave->BeginUnit(_T("LongitudinalRebar"),1.0);

   for (const auto& rebar_row : RebarRows)
   {
      pStrSave->BeginUnit(_T("RebarRow"),1.0);
      pStrSave->put_Property(_T("Datum"),        CComVariant(rebar_row.Datum));
      pStrSave->put_Property(_T("LayoutType"),   CComVariant(rebar_row.LayoutType));
      pStrSave->put_Property(_T("Start"),        CComVariant(rebar_row.Start));
      pStrSave->put_Property(_T("Length"),       CComVariant(rebar_row.Length));
      pStrSave->put_Property(_T("Cover"),        CComVariant(rebar_row.Cover));
      pStrSave->put_Property(_T("NumberOfBars"), CComVariant(rebar_row.NumberOfBars));
      pStrSave->put_Property(_T("BarSize"),      CComVariant(+rebar_row.BarSize));
      pStrSave->put_Property(_T("BarSpacing"),   CComVariant(rebar_row.BarSpacing));
      pStrSave->put_Property(_T("StartHook"),    CComVariant(rebar_row.bHookStart));
      pStrSave->put_Property(_T("EndHook"),      CComVariant(rebar_row.bHookEnd));
      pStrSave->EndUnit();
   }

   pStrSave->EndUnit();

   return hr;
}

HRESULT xbrLongitudinalRebarData::Load(IStructuredLoad* pStrLoad,std::shared_ptr<IEAFProgress> pProgress)
{
   USES_CONVERSION;
   CComVariant var;
   CHRException hr;

   try
   {
      hr = pStrLoad->BeginUnit(_T("LongitudinalRebar")); 

      RebarRows.clear();
      while ( SUCCEEDED(pStrLoad->BeginUnit(_T("RebarRow"))) )
      {
         RebarRow rebar_row;

         var.vt = VT_I4;
         hr = pStrLoad->get_Property(_T("Datum"),         &var);
         rebar_row.Datum = (xbrTypes::LongitudinalRebarDatumType)(var.lVal);

         var.vt = VT_I4;
         hr = pStrLoad->get_Property(_T("LayoutType"),&var);
         rebar_row.LayoutType = (xbrTypes::LongitudinalRebarLayoutType)(var.lVal);

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("Start"), &var);
         rebar_row.Start = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("Length"), &var);
         rebar_row.Length = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("Cover"), &var);
         rebar_row.Cover = var.dblVal;

         var.vt = VT_I4;
         hr = pStrLoad->get_Property(_T("NumberOfBars"), &var);
         rebar_row.NumberOfBars = var.lVal;

         var.vt = VT_I4;
         hr = pStrLoad->get_Property(_T("BarSize"), &var );
         rebar_row.BarSize = WBFL::Materials::Rebar::Size(var.lVal);

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("BarSpacing"), &var);
         rebar_row.BarSpacing = var.dblVal;
      
         var.vt = VT_BOOL;
         hr = pStrLoad->get_Property(_T("StartHook"), &var);
         rebar_row.bHookStart = (var.boolVal == VARIANT_TRUE ? true : false);
      
         var.vt = VT_BOOL;
         hr = pStrLoad->get_Property(_T("EndHook"), &var);
         rebar_row.bHookEnd = (var.boolVal == VARIANT_TRUE ? true : false);

         RebarRows.push_back(rebar_row);

         hr = pStrLoad->EndUnit();
      }

      hr = pStrLoad->EndUnit();
   }
   catch (HRESULT)
   {
      ATLASSERT(false);
      THROW_LOAD(InvalidFileFormat,pStrLoad);
   }

   return hr;
}

void xbrLongitudinalRebarData::MakeCopy(const xbrLongitudinalRebarData& rOther)
{
   RebarRows = rOther.RebarRows;
}

void xbrLongitudinalRebarData::MakeAssignment(const xbrLongitudinalRebarData& rOther)
{
   MakeCopy( rOther );
}
