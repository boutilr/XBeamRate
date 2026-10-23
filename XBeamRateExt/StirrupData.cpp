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

#include "StdAfx.h"
#include <XBeamRateExt\StirrupData.h>
#include <LRFD\RebarPool.h>


xbrStirrupData::xbrStirrupData()
{
   Symmetric = true;
   //Zones.push_back(StirrupZone());
}

xbrStirrupData::xbrStirrupData(const xbrStirrupData& rOther)
{
   MakeCopy(rOther);
}

xbrStirrupData::~xbrStirrupData()
{
}

xbrStirrupData& xbrStirrupData::operator= (const xbrStirrupData& rOther)
{
   if( this != &rOther )
   {
      MakeAssignment(rOther);
   }

   return *this;
}

bool xbrStirrupData::operator == (const xbrStirrupData& rOther) const
{
   if ( Symmetric != rOther.Symmetric )
      return false;

   if ( Zones != rOther.Zones)
      return false;

   return true;
}

bool xbrStirrupData::operator != (const xbrStirrupData& rOther) const
{
   return !operator==( rOther );
}

HRESULT xbrStirrupData::Save(IStructuredSave* pStrSave)
{
   HRESULT hr = S_OK;

   pStrSave->BeginUnit(_T("StirrupData"),1.0);
   pStrSave->put_Property(_T("Symmetric"),CComVariant(Symmetric));
   for (const auto& zone : Zones)
   {
      pStrSave->BeginUnit(_T("Zone"),1.0);
      pStrSave->put_Property(_T("Length"), CComVariant(zone.Length) );
      pStrSave->put_Property(_T("BarSpacing"), CComVariant(zone.BarSpacing) );
      pStrSave->put_Property(_T("BarSize"),CComVariant((long)zone.BarSize) );
      pStrSave->put_Property(_T("nBars"),   CComVariant(zone.nBars) );
      pStrSave->EndUnit(); // Zone
   }
   pStrSave->EndUnit();

   return hr;
}

HRESULT xbrStirrupData::Load(IStructuredLoad* pStrLoad)
{
   CComVariant var;
   CHRException hr;

   try
   {
      hr = pStrLoad->BeginUnit(_T("StirrupData"));

      var.vt = VT_BOOL;
      hr = pStrLoad->get_Property(_T("Symmetric"),&var);
      Symmetric = (var.boolVal == VARIANT_TRUE ? true : false);

      Zones.clear();
      while ( SUCCEEDED(pStrLoad->BeginUnit(_T("Zone"))) )
      {
         StirrupZone zone;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("Length"),&var);
         zone.Length = var.dblVal;

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("BarSpacing"),&var);
         zone.BarSpacing = var.dblVal;

         var.vt = VT_I4;
         hr = pStrLoad->get_Property(_T("BarSize"),&var);
         BarSizeType key = var.lVal;
         zone.BarSize = WBFL::Materials::Rebar::Size(key);

         var.vt = VT_R8;
         hr = pStrLoad->get_Property(_T("nBars"),&var);
         zone.nBars = var.dblVal;

         Zones.push_back(zone);

         hr = pStrLoad->EndUnit(); // Zone
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

void xbrStirrupData::MakeCopy(const xbrStirrupData& rOther)
{
   Symmetric = rOther.Symmetric;
   Zones = rOther.Zones;
}

void xbrStirrupData::MakeAssignment(const xbrStirrupData& rOther)
{
   MakeCopy( rOther );
}
