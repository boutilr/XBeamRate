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

// LegalLiveLoadReactionsPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "LegalLiveLoadReactionsPage.h"
#include "PierDlg.h"
#include <XBeamRateExt\XBeamRateUtilities.h>
#include <EAF\EAFDocument.h>
#include "..\Documentation\XBRate.hh"




// CLegalLiveLoadReactionsPage dialog

IMPLEMENT_DYNAMIC(CLegalLiveLoadReactionsPage, CPropertyPage)

CLegalLiveLoadReactionsPage::CLegalLiveLoadReactionsPage()
	: CPropertyPage(CLegalLiveLoadReactionsPage::IDD),
   m_LegalRoutineGrid(pgsTypes::lrLegal_Routine),
   m_LegalSpecialGrid(pgsTypes::lrLegal_Special),
   m_LegalEmergencyGrid(pgsTypes::lrLegal_Emergency)
{

}

CLegalLiveLoadReactionsPage::~CLegalLiveLoadReactionsPage()
{
}

void CLegalLiveLoadReactionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

   CPierDlg* pParent = (CPierDlg*)GetParent();

   DDX_Text(pDX,IDC_LF_LEGAL_ROUTINE,pParent->m_PierData.m_gLL[GET_INDEX(pgsTypes::StrengthI_LegalRoutine)]);
   DDX_Text(pDX, IDC_LF_LEGAL_SPECIAL, pParent->m_PierData.m_gLL[GET_INDEX(pgsTypes::StrengthI_LegalSpecial)]);
   DDX_Text(pDX, IDC_LF_LEGAL_EMERGENCY, pParent->m_PierData.m_gLL[GET_INDEX(pgsTypes::StrengthI_LegalEmergency)]);

   DDX_LiveLoadReactionsGrid(pDX,m_LegalRoutineGrid,  pParent->m_PierData.m_LegalRoutineLiveLoad);
   DDX_LiveLoadReactionsGrid(pDX, m_LegalSpecialGrid, pParent->m_PierData.m_LegalSpecialLiveLoad);
   DDX_LiveLoadReactionsGrid(pDX, m_LegalEmergencyGrid, pParent->m_PierData.m_LegalEmergencyLiveLoad);
}


BEGIN_MESSAGE_MAP(CLegalLiveLoadReactionsPage, CPropertyPage)
   ON_BN_CLICKED(IDC_LEGAL_ROUTINE_ADD,OnLegalRoutineAdd)
   ON_BN_CLICKED(IDC_LEGAL_ROUTINE_REMOVE,OnLegalRoutineRemove)
   ON_BN_CLICKED(IDC_LEGAL_SPECIAL_ADD, OnLegalSpecialAdd)
   ON_BN_CLICKED(IDC_LEGAL_SPECIAL_REMOVE, OnLegalSpecialRemove)
   ON_BN_CLICKED(IDC_LEGAL_EMERGENCY_ADD, OnLegalEmergencyAdd)
   ON_BN_CLICKED(IDC_LEGAL_EMERGENCY_REMOVE, OnLegalEmergencyRemove)
	ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()


// CLegalLiveLoadReactionsPage message handlers

BOOL CLegalLiveLoadReactionsPage::OnInitDialog()
{
   m_LegalRoutineGrid.SubclassDlgItem(IDC_LEGAL_ROUTINE_GRID, this);
   m_LegalRoutineGrid.CustomInit();

   m_LegalSpecialGrid.SubclassDlgItem(IDC_LEGAL_SPECIAL_GRID, this);
   m_LegalSpecialGrid.CustomInit();

   m_LegalEmergencyGrid.SubclassDlgItem(IDC_LEGAL_EMERGENCY_GRID, this);
   m_LegalEmergencyGrid.CustomInit();

   // Can't add/remove for emergency vehicles
   GetDlgItem(IDC_LEGAL_EMERGENCY_ADD)->ShowWindow(SW_HIDE);
   GetDlgItem(IDC_LEGAL_EMERGENCY_REMOVE)->ShowWindow(SW_HIDE);

   CPropertyPage::OnInitDialog();

   // TODO:  Add extra initialization here

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}

void CLegalLiveLoadReactionsPage::OnLegalRoutineAdd()
{
   m_LegalRoutineGrid.AddVehicle();
}

void CLegalLiveLoadReactionsPage::OnLegalRoutineRemove()
{
   m_LegalRoutineGrid.RemoveSelectedVehicles();
}

void CLegalLiveLoadReactionsPage::OnLegalSpecialAdd()
{
   m_LegalSpecialGrid.AddVehicle();
}

void CLegalLiveLoadReactionsPage::OnLegalSpecialRemove()
{
   m_LegalSpecialGrid.RemoveSelectedVehicles();
}

void CLegalLiveLoadReactionsPage::OnLegalEmergencyAdd()
{
   m_LegalEmergencyGrid.AddVehicle();
}

void CLegalLiveLoadReactionsPage::OnLegalEmergencyRemove()
{
   m_LegalEmergencyGrid.RemoveSelectedVehicles();
}

void CLegalLiveLoadReactionsPage::OnHelp()
{
   EAFHelp(EAFGetDocument()->GetDocumentationSetName(), IDH_LIVE_LOAD);
}
