#include "stdafx.h"
#include <XBeamRateExt\BearingLineData.h>

xbrBearingLineData::xbrBearingLineData()
{
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
         spaBegin += nBearings;
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

HRESULT xbrBearingLineData::Save(IStructuredSave* pStrSave)
{
   pStrSave->BeginUnit(_T("BearingLine"),1.0);
      pStrSave->put_Property(_T("RefBrgDatum"),CComVariant(m_RefBearingDatum));
      pStrSave->put_Property(_T("RefBrgIndex"),CComVariant(m_RefBearingIndex));
      pStrSave->put_Property(_T("RefBrgOffset"),CComVariant(m_RefBearingOffset));
      BOOST_FOREACH(Float64 s,m_vSpacing)
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
      hr = pStrLoad->BeginUnit(_T("BearingLine"));

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
}

void xbrBearingLineData::MakeAssignment(const xbrBearingLineData& rOther)
{
   MakeCopy(rOther);
}
