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

#include <XBeamRateExt\PointOfInterest.h>

/////////////////////////////////////////////////////////////////////////////
// CSectionCutDlg dialog

class CSectionCutDlg : public CDialog
{
// Construction
public:
   CSectionCutDlg(PierIDType pierID,const xbrPointOfInterest& initialPoi,CWnd* pParent=nullptr);

   xbrPointOfInterest GetPOI();

// Dialog Data
	//{{AFX_DATA(CSectionCutDlg)
	enum { IDD = IDD_SECTION_CUT };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSectionCutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSectionCutDlg)
	virtual BOOL OnInitDialog() override;
	afx_msg void OnHelp();
   afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
   PierIDType m_PierID;
   xbrPointOfInterest m_InitialPOI;

   std::vector<xbrPointOfInterest> m_vPOI;

   CSliderCtrl m_Slider;
   CStatic m_Label;
   int m_SliderPos;

   void UpdateSliderLabel();
   void UpdatePOI();
};
