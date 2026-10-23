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

// GraphView.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "XBeamRateGraphView.h"
#include <EAF\EAFGraphBuilderBase.h>

#include "XBRateCalculationSheet.h"


/////////////////////////////////////////////////////////////////////////////
// CXBeamRateGraphView

IMPLEMENT_DYNCREATE(CXBeamRateGraphView, CEAFAutoCalcGraphView)

CXBeamRateGraphView::CXBeamRateGraphView()
{
}

CXBeamRateGraphView::~CXBeamRateGraphView()
{
}

BEGIN_MESSAGE_MAP(CXBeamRateGraphView, CEAFAutoCalcGraphView)
	//{{AFX_MSG_MAP(CXBeamRateGraphView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateGraphView drawing

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateGraphView diagnostics

#ifdef _DEBUG
void CXBeamRateGraphView::AssertValid() const
{
	CEAFAutoCalcGraphView::AssertValid();
}

void CXBeamRateGraphView::Dump(CDumpContext& dc) const
{
	CEAFAutoCalcGraphView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateGraphView message handlers
bool CXBeamRateGraphView::DoResultsExist()
{
   std::unique_ptr<WBFL::Graphing::GraphBuilder>& pGraphBuilder(GetGraphBuilder());
   CEAFGraphBuilderBase* pMyGraphBuilder = dynamic_cast<CEAFGraphBuilderBase*>(pGraphBuilder.get());
   return pMyGraphBuilder->IsValidGraph();
}

void CXBeamRateGraphView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
   auto pBroker = EAFGetBroker();

   
   XBRateCalculationSheet border;

   CRect rcPrint = border.Print(pDC, 1);

   if (rcPrint.IsRectEmpty())
   {
      CHECKX(0,_T("Can't print border - page too small?"));
      rcPrint = pInfo->m_rectDraw;
   }

   m_PrintRect = rcPrint;

	CEAFAutoCalcGraphView::OnPrint(pDC, pInfo);
}
