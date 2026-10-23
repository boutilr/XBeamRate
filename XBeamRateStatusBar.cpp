///////////////////////////////////////////////////////////////////////
// PGSuper - Prestressed Girder SUPERstructure Design and Analysis
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

// PGSuperStatusBar.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "XBeamRateStatusBar.h"



static UINT indicators[] =
{
   ID_SEPARATOR,           // status line indicator
   EAFID_INDICATOR_STATUS,
   EAFID_INDICATOR_AUTOSAVE_ON,
   EAFID_INDICATOR_MODIFIED,
   EAFID_INDICATOR_AUTOCALC_ON,
   ID_INDICATOR_CAPS,
   ID_INDICATOR_NUM,
   ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateStatusBar

CXBeamRateStatusBar::CXBeamRateStatusBar()
{
}

CXBeamRateStatusBar::~CXBeamRateStatusBar()
{
}

BEGIN_MESSAGE_MAP(CXBeamRateStatusBar, CEAFAutoCalcStatusBar)
	//{{AFX_MSG_MAP(CXBeamRateStatusBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CXBeamRateStatusBar::GetStatusIndicators(const UINT** lppIDArray,int* pnIDCount)
{
   *lppIDArray = indicators;
   *pnIDCount = sizeof(indicators)/sizeof(UINT);
}

BOOL CXBeamRateStatusBar::SetStatusIndicators(const UINT* lpIDArray, int nIDCount)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   return CEAFAutoCalcStatusBar::SetStatusIndicators(lpIDArray,nIDCount);
}

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateStatusBar message handlers
