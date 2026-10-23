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

// AASHTOConcretePage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ConcreteDetailsDlg.h"
#include "AASHTOConcretePage.h"
#include <EAF\EAFApp.h>
#include <EAF\EAFDocument.h>


/////////////////////////////////////////////////////////////////////////////
// CAASHTOConcretePage dialog


CAASHTOConcretePage::CAASHTOConcretePage() : CPropertyPage(CAASHTOConcretePage::IDD)
{
	//{{AFX_DATA_INIT(CAASHTOConcretePage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAASHTOConcretePage::DoDataExchange(CDataExchange* pDX)
{
   CEAFApp* pApp = EAFGetApp();
   const WBFL::Units::IndirectMeasure* pDisplayUnits = pApp->GetDisplayUnits();

   CConcreteDetailsDlg* pParent = (CConcreteDetailsDlg*)GetParent();

   CPropertyPage::DoDataExchange(pDX);

   //{{AFX_DATA_MAP(CAASHTOConcretePage)
   //}}AFX_DATA_MAP
   DDX_Check_Bool(pDX, IDC_HAS_AGG_STRENGTH, pParent->Concrete.bHasFct);
   DDX_UnitValueAndTag(pDX, IDC_AGG_STRENGTH, IDC_AGG_STRENGTH_T, pParent->Concrete.Fct, pDisplayUnits->Stress );
   if ( pParent->Concrete.bHasFct || !pDX->m_bSaveAndValidate )
   {
      if ( !pDX->m_bSaveAndValidate )
      {
         CWnd* pWnd = GetDlgItem(IDC_AGG_STRENGTH);
         pWnd->GetWindowText(m_strFct);
      }
   }

   DDX_Text(pDX, IDC_EC_K1, pParent->Concrete.EcK1 );
   DDV_NonNegativeDouble(pDX,IDC_EC_K1,pParent->Concrete.EcK1);

   DDX_Text(pDX, IDC_EC_K2, pParent->Concrete.EcK2 );
   DDV_NonNegativeDouble(pDX,IDC_EC_K2,pParent->Concrete.EcK2);

   DDX_Text(pDX, IDC_CREEP_K1, pParent->Concrete.CreepK1);
   DDV_NonNegativeDouble(pDX,IDC_CREEP_K1,pParent->Concrete.CreepK1);

   DDX_Text(pDX, IDC_CREEP_K2, pParent->Concrete.CreepK2 );
   DDV_NonNegativeDouble(pDX,IDC_CREEP_K2,pParent->Concrete.CreepK2);

   DDX_Text(pDX, IDC_SHRINKAGE_K1, pParent->Concrete.ShrinkageK1 );
   DDV_NonNegativeDouble(pDX,IDC_SHRINKAGE_K1,pParent->Concrete.ShrinkageK1);

   DDX_Text(pDX, IDC_SHRINKAGE_K2, pParent->Concrete.ShrinkageK2 );
   DDV_NonNegativeDouble(pDX,IDC_SHRINKAGE_K2,pParent->Concrete.ShrinkageK2);
}


BEGIN_MESSAGE_MAP(CAASHTOConcretePage, CPropertyPage)
	//{{AFX_MSG_MAP(CAASHTOConcretePage)
	ON_BN_CLICKED(ID_HELP, OnHelp)
   ON_BN_CLICKED(IDC_HAS_AGG_STRENGTH,OnAggSplittingStrengthClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAASHTOConcretePage message handlers
void CAASHTOConcretePage::OnHelp()
{
   EAFHelp( EAFGetDocument()->GetDocumentationSetName(), IDH_CONCRETE_AASHTO );
}


BOOL CAASHTOConcretePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CAASHTOConcretePage::OnSetActive()
{
   CPropertyPage::OnSetActive();

   // the aggregate strength parameters are only applicable to lightweigth concrete
   CConcreteDetailsDlg* pParent = (CConcreteDetailsDlg*)GetParent();
   int nShowCmd = (pParent->Concrete.Type == pgsTypes::AllLightweight || pParent->Concrete.Type == pgsTypes::SandLightweight ? SW_SHOW : SW_HIDE);
   GetDlgItem(IDC_HAS_AGG_STRENGTH)->ShowWindow(nShowCmd);
   GetDlgItem(IDC_AGG_STRENGTH)->ShowWindow(nShowCmd);
   GetDlgItem(IDC_AGG_STRENGTH_T)->ShowWindow(nShowCmd);

   if (nShowCmd == SW_SHOW)
   {
      OnAggSplittingStrengthClicked();
   }

   return TRUE;
}

void CAASHTOConcretePage::OnAggSplittingStrengthClicked()
{
   CButton* pButton = (CButton*)GetDlgItem(IDC_HAS_AGG_STRENGTH);
   ASSERT(pButton);
   BOOL bIsChecked = pButton->GetCheck();
   GetDlgItem(IDC_AGG_STRENGTH)->EnableWindow(bIsChecked);
   GetDlgItem(IDC_AGG_STRENGTH_T)->EnableWindow(bIsChecked);
}
