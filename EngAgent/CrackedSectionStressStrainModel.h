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
#include <WBFLRCCapacity.h>

/////////////////////////////////////////////////////////////////////////////
// CCrackedSectionStressStrainModel
class ATL_NO_VTABLE CCrackedSectionStressStrainModel : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCrackedSectionStressStrainModel,&CLSID_NULL>,
	public IStressStrain
{
public:
   CCrackedSectionStressStrainModel()
	{
	}

   HRESULT FinalConstruct();
   void FinalRelease();

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCrackedSectionStressStrainModel)
	COM_INTERFACE_ENTRY(IStressStrain)
END_COM_MAP()

protected:
   Float64 m_Ec;

public:
   void SetXBeamModE(Float64 Ec);

// IStressStrain
public:
   STDMETHOD(put_Name)(/*[in]*/BSTR name) override;
	STDMETHOD(get_Name)(/*[out,retval]*/BSTR *name) override;
   STDMETHOD(ComputeStress)(/*[in]*/ Float64 strain,/*[out,retval]*/Float64* pVal) override;
   STDMETHOD(StrainLimits)(/*[out]*/Float64* minStrain,/*[out]*/Float64* maxStrain) override;
   STDMETHOD(get_YieldStrain)(/*[out,retval]*/Float64* pey) override;
   STDMETHOD(get_ModulusOfElasticity)(/*[out,retval]*/Float64* pE) override;
   STDMETHOD(get_StrainAtPeakStress)(/*[out,retval]*/Float64* strain) override;
};
