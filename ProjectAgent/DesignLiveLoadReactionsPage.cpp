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

// DesignLiveLoadReactionsPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DesignLiveLoadReactionsPage.h"
#include "PierDlg.h"
#include <XBeamRateExt\XBeamRateUtilities.h>
#include <EAF\EAFDocument.h>
#include "..\Documentation\XBRate.hh"




// CDesignLiveLoadReactionsPage dialog

IMPLEMENT_DYNAMIC(CDesignLiveLoadReactionsPage, CPropertyPage)

CDesignLiveLoadReactionsPage::CDesignLiveLoadReactionsPage()
	: CPropertyPage(CDesignLiveLoadReactionsPage::IDD),
   m_DesignGrid(pgsTypes::lrDesign_Inventory)
{

}

CDesignLiveLoadReactionsPage::~CDesignLiveLoadReactionsPage()
{
}

void CDesignLiveLoadReactionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

   CPierDlg* pParent = (CPierDlg*)GetParent();

   DDX_Text(pDX,IDC_LF_INVENTORY,pParent->m_PierData.m_gLL[GET_INDEX(pgsTypes::StrengthI_Inventory)]);
   DDX_Text(pDX,IDC_LF_OPERATING,pParent->m_PierData.m_gLL[GET_INDEX(pgsTypes::StrengthI_Operating)]);

   DDX_LiveLoadReactionsGrid(pDX,m_DesignGrid,        pParent->m_PierData.m_DesignLiveLoad);
}


BEGIN_MESSAGE_MAP(CDesignLiveLoadReactionsPage, CPropertyPage)
   ON_BN_CLICKED(IDC_DESIGN_ADD,OnDesignAdd)
   ON_BN_CLICKED(IDC_DESIGN_REMOVE,OnDesignRemove)
	ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()


// CDesignLiveLoadReactionsPage message handlers

BOOL CDesignLiveLoadReactionsPage::OnInitDialog()
{
   m_DesignGrid.SubclassDlgItem(IDC_DESIGN_GRID, this);
   m_DesignGrid.CustomInit();

   CPropertyPage::OnInitDialog();

   // TODO:  Add extra initialization here

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}

void CDesignLiveLoadReactionsPage::OnDesignAdd()
{
   m_DesignGrid.AddVehicle();
}

void CDesignLiveLoadReactionsPage::OnDesignRemove()
{
   m_DesignGrid.RemoveSelectedVehicles();
}

void CDesignLiveLoadReactionsPage::OnHelp()
{
   EAFHelp(EAFGetDocument()->GetDocumentationSetName(), IDH_LIVE_LOAD);
}
