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

// LoadRatingOptionsPage.cpp : implementation file
//

#include "stdafx.h"
#include <AgentTools.h>
#include "resource.h"
#include "LoadRatingOptionsPage.h"

#include <MFCTools\CustomDDX.h>
#include <EAF\EAFDisplayUnits.h>
#include <EAF\EAFDocument.h>



// CLoadRatingOptionsPage dialog

IMPLEMENT_DYNAMIC(CLoadRatingOptionsPage, CPropertyPage)

CLoadRatingOptionsPage::CLoadRatingOptionsPage()
	: CPropertyPage(CLoadRatingOptionsPage::IDD)
{
   m_psp.dwFlags |= PSP_HASHELP;
}

CLoadRatingOptionsPage::~CLoadRatingOptionsPage()
{
}

void CLoadRatingOptionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

   auto pBroker = EAFGetBroker();
   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   DDX_CBEnum(pDX,IDC_ANALYSIS_MODE,m_AnalysisType);

   DDX_CBEnum(pDX, IDC_PERMIT_FACTORS, m_PermitRatingMethod);
   DDX_CBEnum(pDX, IDC_EMERGENCY_FACTORS, m_EmergencyRatingMethod);

   DDX_UnitValueAndTag(pDX,IDC_LL_STEP,IDC_LL_STEP_UNIT,m_MaxLLStepSize, pDisplayUnits->GetSpanLengthUnit());
   DDV_UnitValueGreaterThanLimit(pDX,IDC_LL_STEP,m_MaxLLStepSize,0.0,pDisplayUnits->GetSpanLengthUnit());

   DDX_Keyword(pDX,IDC_MAX_LANES,_T("ALL"),m_MaxLoadedLanes);

   DDX_Text(pDX, IDC_SYSTEM_FACTOR_FLEXURE, m_SystemFactorFlexure);
   DDX_Text(pDX, IDC_SYSTEM_FACTOR_SHEAR, m_SystemFactorShear);

   DDX_Check_Bool(pDX, IDC_NEG_MOMENT_CHECK, m_bDoAnalyzeNegativeMomentBetweenFOC);
   DDX_UnitValueAndTag(pDX, IDC_NEG_MOMENT_WIDTH, IDC_NEG_MOMENT_WIDTH_UNIT, m_MinColumnWidthForNegMoment, pDisplayUnits->GetSpanLengthUnit());
   DDV_UnitValueGreaterThanLimit(pDX,IDC_NEG_MOMENT_WIDTH,m_MinColumnWidthForNegMoment, 0.0, pDisplayUnits->GetSpanLengthUnit());
}


BEGIN_MESSAGE_MAP(CLoadRatingOptionsPage, CPropertyPage)
   ON_BN_CLICKED(ID_HELP,OnHelp)
   ON_BN_CLICKED(IDC_NEG_MOMENT_CHECK, &CLoadRatingOptionsPage::OnBnClickedNegMomentCheck)
END_MESSAGE_MAP()


// CLoadRatingOptionsPage message handlers

BOOL CLoadRatingOptionsPage::OnInitDialog()
{
   FillAnalysisModeComboBox();
   FillEmergencyRatingMethodComboBox();
   FillPermitRatingMethodComboBox();

   CPropertyPage::OnInitDialog();

   OnNegMomCheckChanged();

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}

void CLoadRatingOptionsPage::OnHelp()
{
   EAFHelp(_T("XBRate"), IDH_PGS_RATING_OPTIONS);
}

void CLoadRatingOptionsPage::FillAnalysisModeComboBox()
{
   CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_ANALYSIS_MODE);
   pCB->ResetContent();
   int idx = pCB->AddString(_T("Simple"));
   pCB->SetItemData(idx,(DWORD_PTR)pgsTypes::Simple);

   idx = pCB->AddString(_T("Continuous"));
   pCB->SetItemData(idx,(DWORD_PTR)pgsTypes::Continuous);

   idx = pCB->AddString(_T("Envelope"));
   pCB->SetItemData(idx,(DWORD_PTR)pgsTypes::Envelope);
}

void CLoadRatingOptionsPage::FillEmergencyRatingMethodComboBox()
{
   CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_EMERGENCY_FACTORS);
   pCB->ResetContent();

   int idx = pCB->AddString(_T("AASHTO MBE Equation 6A.4.2.1-1"));
   pCB->SetItemData(idx, (DWORD_PTR)xbrTypes::ermAASHTO);

   idx = pCB->AddString(_T("WSDOT BDM Equation 13.1.1A-2"));
   pCB->SetItemData(idx, (DWORD_PTR)xbrTypes::ermWSDOT);
}

void CLoadRatingOptionsPage::FillPermitRatingMethodComboBox()
{
   CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_PERMIT_FACTORS);
   pCB->ResetContent();

   int idx = pCB->AddString(_T("AASHTO MBE Equation 6A.4.2.1-1"));
   pCB->SetItemData(idx, (DWORD_PTR)xbrTypes::prmAASHTO);

   idx = pCB->AddString(_T("WSDOT BDM Equation 13.1.1A-2"));
   pCB->SetItemData(idx, (DWORD_PTR)xbrTypes::prmWSDOT);
}

void CLoadRatingOptionsPage::OnBnClickedNegMomentCheck()
{
   OnNegMomCheckChanged();
}

void CLoadRatingOptionsPage::OnNegMomCheckChanged()
{
   BOOL bEnableUserInput= IsDlgButtonChecked(IDC_NEG_MOMENT_CHECK) == BST_CHECKED ? TRUE : FALSE;

   CWnd* pCtrl = GetDlgItem(IDC_NEG_MOMENT_WIDTH);
   pCtrl->EnableWindow(bEnableUserInput);
   pCtrl = GetDlgItem(IDC_NEG_MOMENT_WIDTH_UNIT);
   pCtrl->EnableWindow(bEnableUserInput);
   pCtrl = GetDlgItem(IDC_STATIC_MOM);
   pCtrl->EnableWindow(bEnableUserInput);
}
