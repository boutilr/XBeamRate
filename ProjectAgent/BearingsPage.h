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

#include "BearingLayoutGrid.h"

// CBearingsPage dialog

class CBearingsPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CBearingsPage)

public:
	CBearingsPage();
	virtual ~CBearingsPage();

// Dialog Data
	enum { IDD = IDD_BEARINGS_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

   CBearingLayoutGrid m_Grid[2];

	//{{AFX_MSG(CPierLayoutPage)
	virtual BOOL OnInitDialog() override;
   afx_msg void OnHelp();
   afx_msg void OnAddBack();
   afx_msg void OnRemoveBack();
   afx_msg void OnAddAhead();
   afx_msg void OnRemoveAhead();
   afx_msg void OnCopyAhead();
   afx_msg void OnCopyBack();
   afx_msg void OnBearingLineCountChanged();
   afx_msg void OnBackReactionTypeChanged();
   afx_msg void OnAheadReactionTypeChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   void FillRefBearingComboBox(IndexType brgLineIdx,bool bInitialFill=false);
   void FillRefBearingDatumComboBox(IndexType brgLineIdx);
   void FillReactionTypeComboBox(IndexType brgLineIdx);
};
