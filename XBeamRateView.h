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

// XBeamRateView.h : interface of the CXBeamRateView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_XBeamRateView_H__536DF63B_D04E_4BD4_A347_53ADDDDA2863__INCLUDED_)
#define AFX_XBeamRateView_H__536DF63B_D04E_4BD4_A347_53ADDDDA2863__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <DManip/DManip.h>
#include <XBeamRateExt\PointOfInterest.h>

class CXBeamRateDoc;
class CXBeamRateChildFrame;
interface IShape;

#define HINT_SECTION_CUT_MOVED 1

class CXBeamRateView : public CDisplayView
{
   friend CXBeamRateChildFrame;

protected: // create from serialization only
	CXBeamRateView();
	DECLARE_DYNCREATE(CXBeamRateView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXBeamRateView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	protected:
	virtual void OnDraw(CDC* pDC) override;      // overridden to draw this view
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo) override;
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo) override;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_VIRTUAL

   virtual void OnInitialUpdate() override;

   virtual DROPEFFECT CanDrop(COleDataObject* pDataObject,DWORD dwKeyState,const WBFL::Geometry::Point2d& point) override;
   virtual void OnDropped(COleDataObject* pDataObject,DROPEFFECT dropEffect,const WBFL::Geometry::Point2d& point) override;

   PierIDType GetPierID();
   PierIndexType GetPierIndex();

   xbrPointOfInterest GetCutLocation();

// Implementation
public:
	virtual ~CXBeamRateView();
#ifdef _DEBUG
	virtual void AssertValid() const override;
	virtual void Dump(CDumpContext& dc) const override;
#endif

protected:
   virtual void OnUpdate(CView* pSender,LPARAM lHint,CObject* pHint) override;
   void UpdateDisplayObjects();
   void UpdateRoadwayDisplayObjects();
   void UpdateXBeamDisplayObjects();
   void UpdateColumnDisplayObjects();
   void UpdateBearingDisplayObjects();
   void UpdateGirderDisplayObjects();
   void UpdateRebarDisplayObjects();
   void UpdateStirrupDisplayObjects();
   void UpdateDimensionsDisplayObjects();
   void UpdateSectionCutDisplayObjects();

   std::shared_ptr<WBFL::DManip::iLineDisplayObject> CreateLineDisplayObject(const WBFL::Geometry::Point2d& pntStart,const WBFL::Geometry::Point2d& pntEnd);
   void BuildDimensionLine(std::shared_ptr<WBFL::DManip::iDisplayList> pDL, const WBFL::Geometry::Point2d& fromPoint, const WBFL::Geometry::Point2d& toPoint,bool bOmitForZeroDistance = true);
   void BuildDimensionLine(std::shared_ptr<WBFL::DManip::iDisplayList> pDL, const WBFL::Geometry::Point2d& fromPoint, const WBFL::Geometry::Point2d& toPoint,Float64 dimension);
   void SkewGirderShape(Float64 skew,Float64 shear,IShape* pShape,IShape** ppSkewedShape);

	virtual void HandleLButtonDblClk(UINT nFlags, CPoint logPoint) override;

   IDType m_DisplayObjectID; // used to generate display object IDs

   bool m_bIsIdealized;

   CXBeamRateChildFrame* m_pFrame;

// Generated message map functions
protected:
	//{{AFX_MSG(CXBeamRateView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnExportPier();
   afx_msg void OnEditPier();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   virtual void HandleContextMenu(CWnd* pWnd,CPoint logPoint) override;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XBeamRateView_H__536DF63B_D04E_4BD4_A347_53ADDDDA2863__INCLUDED_)
