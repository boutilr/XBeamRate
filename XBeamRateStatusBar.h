///////////////////////////////////////////////////////////////////////
// XBeamRate - Cross Beam Load Rating
// Copyright � 1999-2021  Washington State Department of Transportation
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

#include <EAF\EAFAutoCalcStatusBar.h>

class CXBeamRateStatusBar : public CEAFAutoCalcStatusBar
{
public:
	CXBeamRateStatusBar();
	virtual ~CXBeamRateStatusBar();

   virtual void GetStatusIndicators(const UINT** lppIDArray,int* pnIDCount) override;
   virtual BOOL SetStatusIndicators(const UINT* lpIDArray, int nIDCount) override;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXBeamRateStatusBar)
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()
};
