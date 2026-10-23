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

// XBeamRateReportView.cpp : implementation file
//

#include "stdafx.h"
#include "XBeamRateReportView.h"
#include "XBeamRateDoc.h"


/////////////////////////////////////////////////////////////////////////////
// CXBeamRateReportView


IMPLEMENT_DYNCREATE(CXBeamRateReportView, CEAFAutoCalcReportView)

CXBeamRateReportView::CXBeamRateReportView()
{
}

CXBeamRateReportView::~CXBeamRateReportView()
{
}

BEGIN_MESSAGE_MAP(CXBeamRateReportView, CEAFAutoCalcReportView)
	//{{AFX_MSG_MAP(CXBeamRateReportView)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateReportView drawing

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateReportView diagnostics

#ifdef _DEBUG
void CXBeamRateReportView::AssertValid() const
{
	CEAFAutoCalcReportView::AssertValid();
}

void CXBeamRateReportView::Dump(CDumpContext& dc) const
{
	CEAFAutoCalcReportView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPGSuperReportView message handlers
void CXBeamRateReportView::OnInitialUpdate()
{
   CEAFAutoCalcReportView::OnInitialUpdate();
}

void CXBeamRateReportView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
   if ( m_bIsNewReport )
   {
      return; // this the OnUpdate that comes from OnInitialUpdate() ... nothing to do here
   }

   CEAFAutoCalcReportView::OnUpdate(pSender,lHint,pHint);
}

BOOL CXBeamRateReportView::PreTranslateMessage(MSG* pMsg) 
{
   //if (pMsg->message == WM_KEYDOWN) 
   //{
   //   if (pMsg->wParam == VK_F1)
   //   {
   //      EAFHelp( EAFGetDocument()->GetDocumentationSetName(), IDH_REPORT_VIEW );
   //      return TRUE;
   //   }
   //}

	return CEAFAutoCalcReportView::PreTranslateMessage(pMsg);
}

HRESULT CXBeamRateReportView::UpdateReportBrowser(const std::shared_ptr<const WBFL::ReportMgr::ReportHint>& pHint)
{
   //auto pBroker = EAFGetBroker();

   //GET_IFACE2(pBroker,IEAFProgress,pProgress);
   //WBFL::EAF::AutoProgress ap(pProgress);

   //pProgress->UpdateMessage(_T("Working..."));

   //GET_IFACE2(pBroker,IProjectProperties,pProjectProperties);

   //// Set Header and footer information for printed reports
   //std::_tstring  foot1(_T("Bridge: "));
   //foot1 += pProjectProperties->GetBridgeName();
   //m_pReportSpec->SetLeftFooter(foot1.c_str());
   //std::_tstring  foot2(_T("Job: "));
   //foot2 += pProjectProperties->GetJobNumber();
   //m_pReportSpec->SetCenterFooter(foot2.c_str());

   //AFX_MANAGE_STATE(AfxGetAppModuleState()); /////////

   return CEAFAutoCalcReportView::UpdateReportBrowser(pHint);
}

void CXBeamRateReportView::RefreshReport()
{
   //auto pBroker = EAFGetBroker();
   //GET_IFACE2(pBroker,IEAFProgress,pProgress);
   //WBFL::EAF::AutoProgress progress(pProgress);
   //pProgress->UpdateMessage(_T("Updating report..."));

   CEAFAutoCalcReportView::RefreshReport();
}

WBFL::ReportMgr::ReportHint* CXBeamRateReportView::TranslateHint(CView* pSender, LPARAM lHint, CObject* pHint)
{
   //if ( lHint == HINT_GIRDERCHANGED )
   //{
   //   CGirderHint* pGirderHint = (CGirderHint*)pHint;
   //   CGirderReportHint* pSGHint = new CGirderReportHint(pGirderHint->girderKey,pGirderHint->lHint);
   //   return pSGHint;
   //}
   return nullptr;
}

int CXBeamRateReportView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEAFAutoCalcReportView::OnCreate(lpCreateStruct) == -1)
   {
		return -1;
   }

   return 0;
}

bool CXBeamRateReportView::CreateReport(IndexType rptIdx,bool bPromptForSpec)
{
   // Everything in this version of CreateReport is done in support of multi-view report
   // creation because the underlying framework doesn't support it directly.

   CEAFDocument* pEAFDoc = (CEAFDocument*)GetDocument();
   CEAFAutoCalcDocMixin* pAutoCalcDoc = dynamic_cast<CEAFAutoCalcDocMixin*>(pEAFDoc);
   ATLASSERT(pAutoCalcDoc); // your document must use the autocalc mix in

   if ( pAutoCalcDoc->IsAutoCalcEnabled() )
   {
      return CEAFAutoCalcReportView::CreateReport(rptIdx,bPromptForSpec);
   }
   else
   {
      // AutoCalc is off so we have to mimic CreateReport with the exception that we don't actually
      // create the report... Create the default report specification and then initialize the report view
      CreateReportSpecification(rptIdx,bPromptForSpec);
      return CEAFAutoCalcReportView::InitReport(m_pReportSpec,m_pRptSpecBuilder);
   }
}