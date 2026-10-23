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

// CrackedSectionStressStrainModel.cpp : Implementation of CCrackedSectionStressStrainModel
#include "stdafx.h"
#include "EngAgent.h"
#include "CrackedSectionStressStrainModel.h"


/////////////////////////////////////////////////////////////////////////////
// CrackedSectionStressStrainModel

HRESULT CCrackedSectionStressStrainModel::FinalConstruct()
{
   m_Ec = 99999;
   return S_OK;
}

void CCrackedSectionStressStrainModel::FinalRelease()
{
}

void CCrackedSectionStressStrainModel::SetXBeamModE(Float64 Ec)
{
   m_Ec = Ec;
}

/////////////////////////////////////////////////////////////////////////////
// IStressStrain
STDMETHODIMP CCrackedSectionStressStrainModel::put_Name(BSTR name)
{
   ATLASSERT(false);
   return E_NOTIMPL;
}

STDMETHODIMP CCrackedSectionStressStrainModel::get_Name(BSTR *name)
{
   ATLASSERT(false);
   return E_NOTIMPL;
}

STDMETHODIMP CCrackedSectionStressStrainModel::ComputeStress(Float64 strain,Float64 *pVal)
{
   ATLASSERT(false);
   return E_NOTIMPL;
}

STDMETHODIMP CCrackedSectionStressStrainModel::StrainLimits(Float64* minStrain,Float64* maxStrain)
{
   ATLASSERT(false);
   return E_NOTIMPL;
}

STDMETHODIMP CCrackedSectionStressStrainModel::get_StrainAtPeakStress(Float64* strain)
{
   ATLASSERT(false);
   return E_NOTIMPL;
}

STDMETHODIMP CCrackedSectionStressStrainModel::get_YieldStrain(Float64* pey)
{
   ATLASSERT(false);
   return E_NOTIMPL;
}

STDMETHODIMP CCrackedSectionStressStrainModel::get_ModulusOfElasticity(Float64* pE)
{
   CHECK_RETVAL(pE);
   *pE = m_Ec;
   return S_OK;
}
