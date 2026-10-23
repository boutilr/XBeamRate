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
#include <XBeamRateExt\BearingLineData.h>



xbrBearingLineData::xbrBearingLineData()
{
   m_RefBearingDatum = pgsTypes::omtAlignment;
   m_RefBearingIndex = 0;
   m_RefBearingOffset = 0;

   m_BrgLineOffset = 0;
}

xbrBearingLineData::xbrBearingLineData(const xbrBearingLineData& rOther)
{
   MakeCopy(rOther);
}

xbrBearingLineData::~xbrBearingLineData()
{
}

xbrBearingLineData& xbrBearingLineData::operator=(const xbrBearingLineData& rOther)
{
   MakeAssignment(rOther);
   return *this;
}

void xbrBearingLineData::SetBearingLineOffset(Float64 offset)
{
   m_BrgLineOffset = offset;
}

Float64 xbrBearingLineData::GetBearingLineOffset() const
{
   return m_BrgLineOffset;
}

Float64& xbrBearingLineData::GetBearingLineOffset()
{
   return m_BrgLineOffset;
}

void xbrBearingLineData::SetReferenceBearing(pgsTypes::OffsetMeasurementType datum,IndexType refBrgIdx,Float64 refBrgOffset)
{
   m_RefBearingDatum  = datum;
   m_RefBearingIndex  = refBrgIdx;
   m_RefBearingOffset = refBrgOffset;
}

void xbrBearingLineData::GetReferenceBearing(pgsTypes::OffsetMeasurementType* pdatum,IndexType* prefBrgIdx,Float64* prefBrgOffset) const
{
   *pdatum        = m_RefBearingDatum;
   *prefBrgIdx    = m_RefBearingIndex;
   *prefBrgOffset = m_RefBearingOffset;
}

pgsTypes::OffsetMeasurementType& xbrBearingLineData::GetRefBearingDatum()
{
   return m_RefBearingDatum;
}

IndexType& xbrBearingLineData::GetRefBearingIndex()
{
   return m_RefBearingIndex;
}

Float64& xbrBearingLineData::GetRefBearingOffset()
{
   return m_RefBearingOffset;
}

void xbrBearingLineData::SetSpacing(const std::vector<Float64>& vSpacing)
{
   m_vSpacing = vSpacing;
}

const std::vector<Float64>& xbrBearingLineData::GetSpacing() const
{
   return m_vSpacing;
}

void xbrBearingLineData::SetSpacing(IndexType spaceIdx,Float64 s)
{
   m_vSpacing[spaceIdx] = s;
}

Float64 xbrBearingLineData::GetSpacing(IndexType spaceIdx) const
{
   return m_vSpacing[spaceIdx];
}

void xbrBearingLineData::SetBearingCount(IndexType nBearings)
{
   ATLASSERT(1 <= nBearings); // must always be at least one bearing
   IndexType nCurrentBearings = GetBearingCount();
   if ( nBearings != nCurrentBearings )
   {
      if ( nBearings < nCurrentBearings )
      {
         // remove spacing
         std::vector<Float64>::iterator spaBegin = m_vSpacing.begin();
         std::vector<Float64>::iterator spaEnd   = m_vSpacing.end();
         spaBegin += (nBearings-1);
         m_vSpacing.erase(spaBegin,spaEnd);
      }
      else
      {
         // adding spacing
         IndexType nToAdd = nBearings - nCurrentBearings;
         if ( m_vSpacing.size() == 0 )
         {
            m_vSpacing.insert(m_vSpacing.end(),nToAdd,0);
         }
         else
         {
            m_vSpacing.insert(m_vSpacing.end(),nToAdd,m_vSpacing.back());
         }
      }
   }

   ATLASSERT(nBearings == GetBearingCount());
}

IndexType xbrBearingLineData::GetBearingCount() const
{
   return m_vSpacing.size() + 1;
}

HRESULT xbrBearingLineData::Save(IStructuredSave* pStrSave) const
{
   pStrSave->BeginUnit(_T("BearingLine"),1.0);
      pStrSave->put_Property(_T("BearingLineOffset"),CComVariant(m_BrgLineOffset));
      pStrSave->put_Property(_T("RefBrgDatum"),CComVariant(m_RefBearingDatum));
      pStrSave->put_Property(_T("RefBrgIndex"),CComVariant(m_RefBearingIndex));
      pStrSave->put_Property(_T("RefBrgOffset"),CComVariant(m_RefBearingOffset));
      for (const auto& s : m_vSpacing)
      {
         pStrSave->put_Property(_T("S"),CComVariant(s));
      }
   pStrSave->EndUnit(); // BearingLine

   return S_OK;
}

HRESULT xbrBearingLineData::Load(IStructuredLoad* pStrLoad)
{
   USES_CONVERSION;
   CHRException hr;
   try
   {
      CComVariant var;
      HRESULT hRes = pStrLoad->BeginUnit(_T("BearingLine"));
      if ( FAILED(hRes) )
      {
         return hRes;
      }

      var.vt = VT_R8;
      hr = pStrLoad->get_Property(_T("BearingLineOffset"),&var);
      m_BrgLineOffset = var.dblVal;

      var.vt = VT_I4;
      hr = pStrLoad->get_Property(_T("RefBrgDatum"),&var);
      m_RefBearingDatum = (pgsTypes::OffsetMeasurementType)var.lVal;

      var.vt = VT_INDEX;
      hr = pStrLoad->get_Property(_T("RefBrgIndex"),&var);
      m_RefBearingIndex = VARIANT2INDEX(var);

      var.vt = VT_R8;
      hr = pStrLoad->get_Property(_T("RefBrgOffset"),&var);
      m_RefBearingOffset = var.dblVal;

      var.vt = VT_R8;
      m_vSpacing.clear();
      while ( SUCCEEDED(pStrLoad->get_Property(_T("S"),&var)) )
      {
         Float64 s = var.dblVal;
         m_vSpacing.push_back(s);
      }

      hr = pStrLoad->EndUnit(); // BearingLine
   }
   catch (HRESULT)
   {
      ATLASSERT(false);
      THROW_LOAD(InvalidFileFormat,pStrLoad);
   }
   return S_OK;
}

void xbrBearingLineData::MakeCopy(const xbrBearingLineData& rOther)
{
   m_RefBearingDatum  = rOther.m_RefBearingDatum;
   m_RefBearingIndex  = rOther.m_RefBearingIndex;
   m_RefBearingOffset = rOther.m_RefBearingOffset;
   m_vSpacing         = rOther.m_vSpacing;
   m_BrgLineOffset    = rOther.m_BrgLineOffset;
}

void xbrBearingLineData::MakeAssignment(const xbrBearingLineData& rOther)
{
   MakeCopy(rOther);
}
