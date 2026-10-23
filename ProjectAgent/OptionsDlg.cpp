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

// OptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "OptionsDlg.h"


// COptionsDlg dialog

IMPLEMENT_DYNAMIC(COptionsDlg, CPropertySheet)

COptionsDlg::COptionsDlg(CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(_T("Load Rating Options"),pParentWnd,iSelectPage)
{
   Init();
}

COptionsDlg::~COptionsDlg()
{
}

void COptionsDlg::Init()
{
   m_psh.dwFlags |= PSH_HASHELP | PSH_NOAPPLYNOW;

   m_RatingOptions.m_psp.dwFlags  |= PSP_HASHELP;
   m_AnalysisOptions.m_psp.dwFlags  |= PSP_HASHELP;
   m_CapacityOptions.m_psp.dwFlags  |= PSP_HASHELP;

   AddPage(&m_RatingOptions);
   AddPage(&m_AnalysisOptions);
   AddPage(&m_CapacityOptions);
}

void COptionsDlg::SetOptions(const txnEditOptionsData& options)
{
   m_Options = options;
}

const txnEditOptionsData& COptionsDlg::GetOptions() const
{
   return m_Options;
}

BEGIN_MESSAGE_MAP(COptionsDlg, CPropertySheet)
END_MESSAGE_MAP()

