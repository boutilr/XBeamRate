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

// RatingOptionsPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "RatingOptionsPage.h"
#include "OptionsDlg.h"

#include <txnEditOptions.h>

#include <EAF\EAFDocument.h>

#include "..\Documentation\XBRate.hh"




// CRatingOptionsPage dialog

IMPLEMENT_DYNAMIC(CRatingOptionsPage, CPropertyPage)

CRatingOptionsPage::CRatingOptionsPage()
	: CPropertyPage(CRatingOptionsPage::IDD)
{

}

CRatingOptionsPage::~CRatingOptionsPage()
{
}

void CRatingOptionsPage::DoDataExchange(CDataExchange* pDX)
{
   CPropertyPage::DoDataExchange(pDX);

   COptionsDlg* pParent = (COptionsDlg*)GetParent();

   DDX_CBItemData(pDX,IDC_LRFD,pParent->m_Options.m_LRFDEdition);
   DDX_CBItemData(pDX,IDC_LRFR,pParent->m_Options.m_LRFREdition);


   DDX_Check_Bool(pDX,IDC_DESIGN_RATING,pParent->m_Options.m_bDesignRating);
   DDX_Check_Bool(pDX,IDC_DESIGN_SHEAR,pParent->m_Options.m_bDesignRateForShear);

   DDX_Check_Bool(pDX,IDC_LEGAL_RATING,pParent->m_Options.m_bLegalRating);
   DDX_Check_Bool(pDX,IDC_LEGAL_SHEAR,pParent->m_Options.m_bLegalRateForShear);
   DDX_CBEnum(pDX, IDC_EMERGENCY_FACTORS, pParent->m_Options.m_EmergencyRatingMethod);

   DDX_Check_Bool(pDX,IDC_PERMIT_RATING,pParent->m_Options.m_bPermitRating);
   DDX_Check_Bool(pDX,IDC_PERMIT_SHEAR,pParent->m_Options.m_bPermitRateForShear);
   DDX_Check_Bool(pDX,IDC_PERMIT_YIELD_STRESS,pParent->m_Options.m_bCheckYieldStress);
   DDX_Text(pDX,IDC_ALLOWABLE_STRESS,pParent->m_Options.m_YieldStressCoefficient);
   DDX_CBEnum(pDX,IDC_PERMIT_FACTORS,pParent->m_Options.m_PermitRatingMethod);

   DDX_Text(pDX,IDC_SYSTEM_FACTOR_FLEXURE,pParent->m_Options.m_SystemFactorFlexure);
   DDX_Text(pDX,IDC_SYSTEM_FACTOR_SHEAR,pParent->m_Options.m_SystemFactorShear);
}


BEGIN_MESSAGE_MAP(CRatingOptionsPage, CPropertyPage)
	ON_COMMAND(ID_HELP, OnHelp)
   ON_BN_CLICKED(IDC_DESIGN_RATING, &CRatingOptionsPage::OnBnClickedDesignRating)
   ON_BN_CLICKED(IDC_LEGAL_RATING, &CRatingOptionsPage::OnBnClickedLegalRating)
   ON_BN_CLICKED(IDC_PERMIT_RATING, &CRatingOptionsPage::OnBnClickedPermitRating)
END_MESSAGE_MAP()


// COptionsDlg message handlers


BOOL CRatingOptionsPage::OnInitDialog()
{
   FillEmergencyFactorList();
   FillPermitFactorList();

   GetDlgItem(IDC_LRFD_LABEL)->SetWindowText(WBFL::LRFD::BDSManager::GetSpecificationName());
   GetDlgItem(IDC_LRFR_LABEL)->SetWindowText(WBFL::LRFD::MBEManager::GetSpecificationName());

   FillLRFDList();
   FillLRFRList();

   CPropertyPage::OnInitDialog();

   // TODO:  Add extra initialization here

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}

void CRatingOptionsPage::FillEmergencyFactorList()
{
   CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_EMERGENCY_FACTORS);
   pCB->ResetContent();

   int idx = pCB->AddString(_T("AASHTO MBE Equation 6A.4.2.1-1"));
   pCB->SetItemData(idx, (DWORD_PTR)xbrTypes::ermAASHTO);

   idx = pCB->AddString(_T("WSDOT BDM Equation 13.1.1A-2"));
   pCB->SetItemData(idx, (DWORD_PTR)xbrTypes::ermWSDOT);
}

void CRatingOptionsPage::FillPermitFactorList()
{
   CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_PERMIT_FACTORS);
   pCB->ResetContent();

   int idx = pCB->AddString(_T("AASHTO MBE Equation 6A.4.2.1-1"));
   pCB->SetItemData(idx, (DWORD_PTR)xbrTypes::prmAASHTO);

   idx = pCB->AddString(_T("WSDOT BDM Equation 13.1.1A-2"));
   pCB->SetItemData(idx, (DWORD_PTR)xbrTypes::prmWSDOT);
}

void CRatingOptionsPage::FillLRFDList()
{
   CComboBox* pSpec = (CComboBox*)GetDlgItem(IDC_LRFD);
   for (auto e : pgsTypes::enum_range<WBFL::LRFD::BDSManager::Edition>(WBFL::LRFD::BDSManager::Edition::FirstEdition1994, WBFL::LRFD::BDSManager::GetLatestEdition()))
   {
      int idx = pSpec->AddString(WBFL::LRFD::BDSManager::GetEditionAsString(e));
      pSpec->SetItemData(idx, (DWORD)(e));
   }
}

void CRatingOptionsPage::FillLRFRList()
{
   CComboBox* pSpec = (CComboBox*)GetDlgItem(IDC_LRFR);
   for (auto e : pgsTypes::enum_range<WBFL::LRFD::MBEManager::Edition>(WBFL::LRFD::MBEManager::Edition::FirstEdition2008, WBFL::LRFD::MBEManager::GetLatestEdition()))
   {
      int idx = pSpec->AddString(WBFL::LRFD::MBEManager::GetEditionAsString(e));
      pSpec->SetItemData(idx, (DWORD)(e));
   }
}

void CRatingOptionsPage::OnHelp() 
{
   EAFHelp(EAFGetDocument()->GetDocumentationSetName(), IDH_OPTIONS_GENERAL);
}

void CRatingOptionsPage::OnBnClickedDesignRating()
{
   // TODO: Add your control notification handler code here
   BOOL bEnable = (IsDlgButtonChecked(IDC_DESIGN_RATING) == BST_CHECKED) ? TRUE : FALSE;
   GetDlgItem(IDC_DESIGN_SHEAR)->EnableWindow(bEnable);
}

void CRatingOptionsPage::OnBnClickedLegalRating()
{
   // TODO: Add your control notification handler code here
   BOOL bEnable = (IsDlgButtonChecked(IDC_LEGAL_RATING) == BST_CHECKED) ? TRUE : FALSE;
   GetDlgItem(IDC_LEGAL_SHEAR)->EnableWindow(bEnable);
   GetDlgItem(IDC_EMERGENCY_FACTORS)->EnableWindow(bEnable);
}

void CRatingOptionsPage::OnBnClickedPermitRating()
{
   // TODO: Add your control notification handler code here
   BOOL bEnable = (IsDlgButtonChecked(IDC_PERMIT_RATING) == BST_CHECKED) ? TRUE : FALSE;
   GetDlgItem(IDC_PERMIT_SHEAR)->EnableWindow(bEnable);
   GetDlgItem(IDC_PERMIT_YIELD_STRESS)->EnableWindow(bEnable);
   GetDlgItem(IDC_ALLOWABLE_STRESS_LABEL)->EnableWindow(bEnable);
   GetDlgItem(IDC_ALLOWABLE_STRESS)->EnableWindow(bEnable);
   GetDlgItem(IDC_ALLOWABLE_STRESS_TAG)->EnableWindow(bEnable);
   GetDlgItem(IDC_PERMIT_FACTORS)->EnableWindow(bEnable);
}
