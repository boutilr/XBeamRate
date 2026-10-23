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

#include <EAF\EAFChildFrame.h>
#include "SectionCut.h"
#include <XBeamRateExt\PointOfInterest.h>

class CXBeamRateChildFrame : public CEAFChildFrame, public iCutLocation
{
	DECLARE_DYNCREATE(CXBeamRateChildFrame)
protected:
	CXBeamRateChildFrame();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:
   PierIDType GetPierID();
   PierIndexType GetPierIndex();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXBeamRateChildFrame)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW, const RECT& rect = rectDefault, CMDIFrameWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr) override;
   virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr) override;
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	//}}AFX_VIRTUAL

public:
   // iCutLocation
   virtual Float64 GetCurrentCutLocation() override;
   virtual void CutAt(Float64 Xp) override;
   virtual void CutAtNext() override;
   virtual void CutAtPrev() override;
   virtual void ShowCutDlg() override;
   virtual Float64 GetMinCutLocation() override;
   virtual Float64 GetMaxCutLocation() override;

   void UpdatePierList();
   void UpdateSectionCutExtents();
   xbrPointOfInterest GetCutLocation();


// Implementation
protected:
	virtual ~CXBeamRateChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const override;
	virtual void Dump(CDumpContext& dc) const override;
#endif

   CDialogBar m_ControlBar;

   Float64 m_CutLocation;
   Float64 m_Xmin;
   Float64 m_Xmax;

   void UpdateCutLocation(const xbrPointOfInterest& poi);

   // Generated message map functions
	//{{AFX_MSG(CXBeamRateChildFrame)
   afx_msg void OnPierChanged();
   afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   virtual void OnUpdateFrameTitle(BOOL bAddToTitle) override;
};
