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

// SpanGirderReportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PierReportDlg.h"

#include <IFace\Tools.h>
#include <IFace\Bridge.h>
#include <IFace\DocumentType.h>

#include <PsgLib\GirderLabel.h>
#include <MFCTools\CustomDDX.h>

#include <XBeamRateExt\XBeamRateUtilities.h>

#include <..\..\PGSuper\Include\IFace\Project.h>
#include <PsgLib\BridgeDescription2.h>

#include <EAF\EAFDocument.h>

#include "XBeamRateReportSpecification.h"


// PierReportDlg dialog

IMPLEMENT_DYNAMIC(CPierReportDlg, CDialog)

CPierReportDlg::CPierReportDlg(const WBFL::ReportMgr::ReportDescription& rptDesc,std::shared_ptr<WBFL::ReportMgr::ReportSpecification>& pRptSpec,CWnd* pParent)
: CDialog(CPierReportDlg::IDD, pParent), m_RptDesc(rptDesc), m_pInitRptSpec(pRptSpec)
{
   m_PierID = INVALID_ID;
   m_bReportEvenIncrements = true;
}

CPierReportDlg::~CPierReportDlg()
{
}

void CPierReportDlg::DoDataExchange(CDataExchange* pDX)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_ChList);

   DDX_CBItemData(pDX, IDC_PIER, m_PierID);
   DDX_CBIndex(pDX, IDC_EVEN_INCREMENTS, m_ReportEvenIncrementsIval);

   if ( pDX->m_bSaveAndValidate )
   {
      m_ChapterList.clear();

      int cSelChapters = 0;
      int cChapters = m_ChList.GetCount();
      for ( int ch = 0; ch < cChapters; ch++ )
      {
         if ( m_ChList.GetCheck( ch ) == 1 )
         {
            cSelChapters++;

            CString strChapter;
            m_ChList.GetText(ch,strChapter);

            m_ChapterList.push_back(std::_tstring(strChapter));
         }
      }

      if ( cSelChapters == 0 )
      {
         pDX->PrepareCtrl(IDC_LIST);
         AfxMessageBox(IDS_E_NOCHAPTERS);
         pDX->Fail();
      }

      m_bReportEvenIncrements = m_ReportEvenIncrementsIval == 0;
   }
}


BEGIN_MESSAGE_MAP(CPierReportDlg, CDialog)
	ON_COMMAND(IDHELP, OnHelp)
END_MESSAGE_MAP()


// CPierReportDlg message handlers
void CPierReportDlg::UpdateChapterList()
{
   // Clear out the list box
   m_ChList.ResetContent();

   // Get the chapters in the report
   std::vector<WBFL::ReportMgr::ChapterInfo> chInfos = m_RptDesc.GetChapterInfo();

   // Populate the list box with the names of the chapters
   std::vector<WBFL::ReportMgr::ChapterInfo>::iterator iter;
   for ( iter = chInfos.begin(); iter != chInfos.end(); iter++ )
   {
      WBFL::ReportMgr::ChapterInfo chInfo = *iter;

      int idx = m_ChList.AddString( chInfo.Name.c_str() );
      if ( idx != LB_ERR ) // no error
      {
         m_ChList.SetCheck( idx, chInfo.Select ? 1 : 0 ); // turn the check on
         m_ChList.SetItemData( idx, chInfo.MaxLevel );
      }
   }

   // don't select any items in the chapter list
   m_ChList.SetCurSel(-1);
}

BOOL CPierReportDlg::OnInitDialog()
{
   CWnd* pwndTitle = GetDlgItem(IDC_REPORT_TITLE);
   pwndTitle->SetWindowText(m_RptDesc.GetReportName().c_str());

   m_ReportEvenIncrementsIval = m_bReportEvenIncrements ? 0 : 1;

   if ( IsStandAlone())
   {
      CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_PIER);
      int idx = pCB->AddString(_T("INVALID_ID"));
      pCB->SetItemData(idx,(DWORD_PTR)INVALID_ID);
      m_PierID = INVALID_ID;

      // Hide the Pier combo box
      pCB->ShowWindow(SW_HIDE);
      
      // Hide the group box around the pier selector
      CWnd* pGroupBox = GetDlgItem(IDC_GROUP);
      pGroupBox->ShowWindow(SW_HIDE);

      // Move the list lable to where the group box was
      CRect rGroupBox;
      pGroupBox->GetWindowRect(&rGroupBox);
      LONG grpTop = rGroupBox.top;
      ScreenToClient(rGroupBox);

      CWnd* pChListLabel = GetDlgItem(IDC_LABEL);
      CRect rLabel;
      pChListLabel->GetWindowRect(&rLabel);
      pChListLabel->SetWindowPos(nullptr,rGroupBox.left,rGroupBox.top,0,0,SWP_NOSIZE | SWP_NOZORDER);

      // Make the chapter list taller
      CWnd* pChList = GetDlgItem(IDC_LIST);
      CRect rList;
      pChList->GetWindowRect(&rList);
      LONG clearance = rList.top - rLabel.bottom; // clearance between ch list label and the ch list
      rList.top = grpTop + rLabel.Height() + clearance;

      ScreenToClient(rList);
      pChList->SetWindowPos(nullptr,rList.left,rList.top,rList.Width(),rList.Height(),SWP_NOZORDER);

   }
   else
   {
      CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_PIER);

      m_PierID = INVALID_ID;

      auto broker = EAFGetBroker();
      GET_IFACE2(broker,IBridgeDescription,pIBridgeDesc);
      const CBridgeDescription2* pBridgeDesc = pIBridgeDesc->GetBridgeDescription();
      PierIndexType nPiers = pBridgeDesc->GetPierCount();
      for ( PierIndexType pierIdx = 0; pierIdx < nPiers; pierIdx++ )
      {
         CString strPierLabel;
         strPierLabel.Format(_T("Pier %s"),LABEL_PIER(pierIdx));
         int idx = pCB->AddString(strPierLabel);

         const CPierData2* pPier = pBridgeDesc->GetPier(pierIdx);
         PierIDType pierID = pPier->GetID();
         pCB->SetItemData(idx,(DWORD_PTR)pierID);

         if (pPier->GetPierModelType() == pgsTypes::pmtPhysical && m_PierID == INVALID_ID)
         {
            // this is the first physical pier model.... capture its ID so we can
            // use it to set the default
            m_PierID = pierID;
         }
      }

      if (m_PierID == INVALID_ID)
      {
         // no physical pier models found, use pier 0 as the default
         m_PierID = pBridgeDesc->GetPier(0)->GetID();
      }
   }

   CDialog::OnInitDialog();

   UpdateChapterList();

   if ( m_pInitRptSpec )
   {
      InitFromRptSpec();
   }

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}

void CPierReportDlg::OnHelp() 
{
   EAFHelp( EAFGetDocument()->GetDocumentationSetName(), IDH_DIALOG_REPORT );
}

void CPierReportDlg::ClearChapterCheckMarks()
{
   int cChapters = m_ChList.GetCount();
   for ( int ch = 0; ch < cChapters; ch++ )
   {
      m_ChList.SetCheck(ch,0);
   }
}

void CPierReportDlg::InitFromRptSpec()
{
   std::shared_ptr<CXBeamRateReportSpecification> pRptSpec = std::dynamic_pointer_cast<CXBeamRateReportSpecification>(m_pInitRptSpec);
   m_PierID = pRptSpec->GetPierID();
   InitChapterListFromSpec();

   UpdateData(FALSE);
}

void CPierReportDlg::InitChapterListFromSpec()
{
   ClearChapterCheckMarks();
   std::vector<WBFL::ReportMgr::ChapterInfo> chInfo = m_pInitRptSpec->GetChapterInfo();
   std::vector<WBFL::ReportMgr::ChapterInfo>::iterator iter;
   for ( iter = chInfo.begin(); iter != chInfo.end(); iter++ )
   {
      WBFL::ReportMgr::ChapterInfo& ch = *iter;
      int cChapters = m_ChList.GetCount();
      for ( int idx = 0; idx < cChapters; idx++ )
      {
         CString strChapter;
         m_ChList.GetText(idx,strChapter);
         if ( strChapter == ch.Name.c_str() )
         {
            m_ChList.SetCheck(idx,1);
            break;
         }
      }
   }
}
