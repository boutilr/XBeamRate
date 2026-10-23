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

// PierLayoutPage.h : header file
//

#include "resource.h"
#include <PsgLib\ColumnData.h>
#include "ColumnLayoutGrid.h"

/////////////////////////////////////////////////////////////////////////////
// CPierLayoutPage dialog
class CPierLayoutPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CPierLayoutPage)

// Construction
public:
	CPierLayoutPage();
	~CPierLayoutPage();

// Dialog Data
	//{{AFX_DATA(CPierLayoutPage)
	enum { IDD = IDD_PIER_LAYOUT_PAGE };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

   // Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPierLayoutPage)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPierLayoutPage)
	virtual BOOL OnInitDialog() override;
   afx_msg void OnHelp();
   afx_msg void OnConditionFactorTypeChanged();
   afx_msg void OnHeightMeasureChanged();
   afx_msg void OnAddColumn();
   afx_msg void OnRemoveColumns();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   CMetaFileStatic m_LayoutPicture;
   CColumnLayoutGrid m_ColumnLayoutGrid;

   void FillRefColumnComboBox(ColumnIndexType nColumns = INVALID_INDEX);
   void FillHeightMeasureComboBox();
   void FillTransverseLocationComboBox();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
