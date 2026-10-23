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

// PermitLiveLoadReactionsPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PermitLiveLoadReactionsPage.h"
#include "PierDlg.h"
#include <XBeamRateExt\XBeamRateUtilities.h>
#include <EAF\EAFDocument.h>
#include "..\Documentation\XBRate.hh"



// CPermitLiveLoadReactionsPage dialog

IMPLEMENT_DYNAMIC(CPermitLiveLoadReactionsPage, CPropertyPage)

CPermitLiveLoadReactionsPage::CPermitLiveLoadReactionsPage()
	: CPropertyPage(CPermitLiveLoadReactionsPage::IDD),
   m_PermitRoutineGrid(pgsTypes::lrPermit_Routine),
   m_PermitSpecialGrid(pgsTypes::lrPermit_Special)
{

}

CPermitLiveLoadReactionsPage::~CPermitLiveLoadReactionsPage()
{
}

void CPermitLiveLoadReactionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

   CPierDlg* pParent = (CPierDlg*)GetParent();

   DDX_Text(pDX,IDC_LF_PERMIT_ROUTINE,pParent->m_PierData.m_gLL[GET_INDEX(pgsTypes::StrengthII_PermitRoutine)]);
   DDX_Text(pDX,IDC_LF_PERMIT_SPECIAL,pParent->m_PierData.m_gLL[GET_INDEX(pgsTypes::StrengthII_PermitSpecial)]);
   DDX_Text(pDX,IDC_LF_PERMIT_ROUTINE_SERVICEI,pParent->m_PierData.m_gLL[GET_INDEX(pgsTypes::ServiceI_PermitRoutine)]);
   DDX_Text(pDX,IDC_LF_PERMIT_SPECIAL_SERVICEI,pParent->m_PierData.m_gLL[GET_INDEX(pgsTypes::ServiceI_PermitSpecial)]);

   DDX_LiveLoadReactionsGrid(pDX,m_PermitRoutineGrid, pParent->m_PierData.m_PermitRoutineLiveLoad);
   DDX_LiveLoadReactionsGrid(pDX,m_PermitSpecialGrid, pParent->m_PierData.m_PermitSpecialLiveLoad);
}


BEGIN_MESSAGE_MAP(CPermitLiveLoadReactionsPage, CPropertyPage)
   ON_BN_CLICKED(IDC_PERMIT_ROUTINE_ADD,OnPermitRoutineAdd)
   ON_BN_CLICKED(IDC_PERMIT_ROUTINE_REMOVE,OnPermitRoutineRemove)
   ON_BN_CLICKED(IDC_PERMIT_SPECIAL_ADD,OnPermitSpecialAdd)
   ON_BN_CLICKED(IDC_PERMIT_SPECIAL_REMOVE,OnPermitSpecialRemove)
	ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()


// CPermitLiveLoadReactionsPage message handlers

BOOL CPermitLiveLoadReactionsPage::OnInitDialog()
{
   m_PermitRoutineGrid.SubclassDlgItem(IDC_PERMIT_ROUTINE_GRID, this);
   m_PermitRoutineGrid.CustomInit();

   m_PermitSpecialGrid.SubclassDlgItem(IDC_PERMIT_SPECIAL_GRID, this);
   m_PermitSpecialGrid.CustomInit();

   CPropertyPage::OnInitDialog();

   // TODO:  Add extra initialization here

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}

void CPermitLiveLoadReactionsPage::OnPermitRoutineAdd()
{
   m_PermitRoutineGrid.AddVehicle();
}

void CPermitLiveLoadReactionsPage::OnPermitRoutineRemove()
{
   m_PermitRoutineGrid.RemoveSelectedVehicles();
}

void CPermitLiveLoadReactionsPage::OnPermitSpecialAdd()
{
   m_PermitSpecialGrid.AddVehicle();
}

void CPermitLiveLoadReactionsPage::OnPermitSpecialRemove()
{
   m_PermitSpecialGrid.RemoveSelectedVehicles();
}

void CPermitLiveLoadReactionsPage::OnHelp()
{
   EAFHelp(EAFGetDocument()->GetDocumentationSetName(), IDH_LIVE_LOAD);
}
