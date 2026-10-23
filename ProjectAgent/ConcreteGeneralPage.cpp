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

// ConcreteGeneralPage.cpp : implementation file
//

#include "stdafx.h"
#include "ProjectAgent.h"
#include "resource.h"
#include "PGSuperUnits.h"
#include "ConcreteDetailsDlg.h"
#include "ConcreteGeneralPage.h"

#include <System\Tokenizer.h>
#include <LRFD\Lrfd.h>
#include <EAF\EAFDisplayUnits.h>
#include <IFace\Bridge.h>

#include <PGSuperColors.h>

#include <EAF\EAFDocument.h>


/////////////////////////////////////////////////////////////////////////////
// CConcreteGeneralPage dialog


CConcreteGeneralPage::CConcreteGeneralPage() : CPropertyPage(CConcreteGeneralPage::IDD)
{
	//{{AFX_DATA_INIT(CConcreteGeneralPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
   m_MinNWCDensity = WBFL::LRFD::ConcreteUtil::GetNWCDensityLimit();
   m_MaxLWCDensity = WBFL::LRFD::ConcreteUtil::GetLWCDensityLimit();

   WBFL::LRFD::ConcreteUtil::GetPCIUHPCStrengthRange(&m_MinFcUHPC, &m_MaxFcUHPC);
}


void CConcreteGeneralPage::DoDataExchange(CDataExchange* pDX)
{
   m_bErrorInDDX = false;
   try
   {
	   CPropertyPage::DoDataExchange(pDX);
	   //{{AFX_DATA_MAP(CConcreteDetailsDlg)
	   DDX_Control(pDX, IDC_DS_UNIT, m_ctrlStrengthDensityUnit);
	   DDX_Control(pDX, IDC_DS_TITLE, m_ctrlStrengthDensityTitle);
	   //}}AFX_DATA_MAP

      CConcreteDetailsDlg* pParent = (CConcreteDetailsDlg*)GetParent();

	   DDX_Control(pDX, IDC_EC,      m_ctrlEc);
	   DDX_Control(pDX, IDC_MOD_E,   m_ctrlEcCheck);
	   DDX_Control(pDX, IDC_FC,      m_ctrlFc);
	   DDX_Control(pDX, IDC_DS,      m_ctrlStrengthDensity);

       auto pBroker = EAFGetBroker();

      GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

      DDX_CBItemData(pDX, IDC_CONCRETE_TYPE, pParent->Concrete.Type);

      DDX_UnitValueAndTag(pDX, IDC_FC, IDC_FC_UNIT, pParent->Concrete.Fc, pDisplayUnits->GetStressUnit() );
      DDV_UnitValueGreaterThanZero(pDX,IDC_FC, pParent->Concrete.Fc, pDisplayUnits->GetStressUnit() );

      DDX_Check_Bool(pDX, IDC_MOD_E, pParent->Concrete.bUserEc);
      if (pParent->Concrete.bUserEc || !pDX->m_bSaveAndValidate)
      {
         DDX_UnitValueAndTag(pDX, IDC_EC, IDC_EC_UNIT, pParent->Concrete.Ec, pDisplayUnits->GetModEUnit() );
         DDV_UnitValueGreaterThanZero(pDX, IDC_EC, pParent->Concrete.Ec, pDisplayUnits->GetModEUnit() );
      }

      DDX_UnitValueAndTag(pDX, IDC_DS, IDC_DS_UNIT, pParent->Concrete.StrengthDensity, pDisplayUnits->GetDensityUnit() );
      DDV_UnitValueGreaterThanZero(pDX, IDC_DS, pParent->Concrete.StrengthDensity, pDisplayUnits->GetDensityUnit() );

      DDX_UnitValueAndTag(pDX, IDC_DW, IDC_DW_UNIT, pParent->Concrete.WeightDensity, pDisplayUnits->GetDensityUnit() );
      DDV_UnitValueGreaterThanZero(pDX, IDC_DW, pParent->Concrete.WeightDensity, pDisplayUnits->GetDensityUnit() );

      // Ds <= Dw
      DDV_UnitValueLimitOrMore(pDX, IDC_DW, pParent->Concrete.WeightDensity, pParent->Concrete.StrengthDensity, pDisplayUnits->GetDensityUnit() );

      DDX_UnitValueAndTag(pDX, IDC_AGG_SIZE, IDC_AGG_SIZE_UNIT, pParent->Concrete.MaxAggregateSize, pDisplayUnits->GetComponentDimUnit() );
      DDV_UnitValueGreaterThanZero(pDX, IDC_AGG_SIZE, pParent->Concrete.MaxAggregateSize, pDisplayUnits->GetComponentDimUnit() );


      if ( pDX->m_bSaveAndValidate && m_ctrlEcCheck.GetCheck() == 1 )
      {
         m_ctrlEc.GetWindowText(m_strUserEc);
      }

      if (!pDX->m_bSaveAndValidate)
      {
         ShowStrengthDensity(!pParent->Concrete.bUserEc);
      }
   }
   catch(...)
   {
      m_bErrorInDDX = true;
      throw;
   }
}


BEGIN_MESSAGE_MAP(CConcreteGeneralPage, CPropertyPage)
	//{{AFX_MSG_MAP(CConcreteGeneralPage)
	ON_BN_CLICKED(ID_HELP, OnHelp)
	ON_BN_CLICKED(IDC_MOD_E, OnUserEc)
	ON_EN_CHANGE(IDC_FC, OnChangeFc)
	ON_EN_CHANGE(IDC_DS, OnChangeDs)
   ON_CBN_SELCHANGE(IDC_CONCRETE_TYPE,OnConcreteType)
	//}}AFX_MSG_MAP
   ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConcreteDetailsDlg message handlers

BOOL CConcreteGeneralPage::OnInitDialog() 
{
   CConcreteDetailsDlg* pParent = (CConcreteDetailsDlg*)GetParent();
   GetDlgItem(IDC_FC_LABEL)->SetWindowText(_T("Strength - f'c"));
   GetDlgItem(IDC_MOD_E)->SetWindowText(_T("Mod. Elasticity, Ec"));

   CComboBox* pcbConcreteType = (CComboBox*)GetDlgItem(IDC_CONCRETE_TYPE);
   if ( WBFL::LRFD::BDSManager::GetEdition() < WBFL::LRFD::BDSManager::Edition::SeventhEditionWith2016Interims )
   {  
      int idx = pcbConcreteType->AddString(_T("Normal weight"));
      pcbConcreteType->SetItemData(idx,(DWORD_PTR)pgsTypes::Normal);

      idx = pcbConcreteType->AddString(_T("All lightweight"));
      pcbConcreteType->SetItemData(idx,(DWORD_PTR)pgsTypes::AllLightweight);

      idx = pcbConcreteType->AddString(_T("Sand lightweight"));
      pcbConcreteType->SetItemData(idx,(DWORD_PTR)pgsTypes::SandLightweight);
   }
   else
   {
      int idx = pcbConcreteType->AddString(_T("Normal weight"));
      pcbConcreteType->SetItemData(idx,(DWORD_PTR)pgsTypes::Normal);

      idx = pcbConcreteType->AddString(_T("Lightweight"));
      pcbConcreteType->SetItemData(idx,(DWORD_PTR)pgsTypes::SandLightweight);
   }

	CPropertyPage::OnInitDialog();
	
   BOOL bEnable = m_ctrlEcCheck.GetCheck();
   GetDlgItem(IDC_EC)->EnableWindow(bEnable);
   GetDlgItem(IDC_EC_UNIT)->EnableWindow(bEnable);

   OnConcreteType();

   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConcreteGeneralPage::OnHelp() 
{
   EAFHelp(EAFGetDocument()->GetDocumentationSetName(), IDH_CONCRETE_GENERAL );
}

void CConcreteGeneralPage::OnUserEc()
{
   // Ec check box was clicked
   BOOL bIsChecked = m_ctrlEcCheck.GetCheck();

   if (bIsChecked == FALSE)
   {
      // unchecked... compute Ec based on parameters
      m_ctrlEc.GetWindowText(m_strUserEc);
      UpdateEc();
      ShowStrengthDensity(true);
   }
   else
   {
      // check, restore user input value
      m_ctrlEc.SetWindowText(m_strUserEc);
      ShowStrengthDensity(false);
   }

   // enable/disable the Ec input and unit controls
   GetDlgItem(IDC_EC)->EnableWindow(bIsChecked);
   GetDlgItem(IDC_EC_UNIT)->EnableWindow(bIsChecked);
}

void CConcreteGeneralPage::ShowStrengthDensity(bool enable)
{
   int code = enable ? SW_SHOW : SW_HIDE;
   m_ctrlStrengthDensity.ShowWindow(code);
   m_ctrlStrengthDensityUnit.ShowWindow(code);
   m_ctrlStrengthDensityTitle.ShowWindow(code);
}

void CConcreteGeneralPage::OnChangeFc() 
{
   UpdateEc();
}

void CConcreteGeneralPage::OnChangeDs() 
{
   UpdateEc();
}

void CConcreteGeneralPage::UpdateEc()
{
   // update modulus
   if (m_ctrlEcCheck.GetCheck() == 0)
   {
      // need to manually parse strength and density values
      CString strFc, strDensity, strK1, strK2;
      m_ctrlFc.GetWindowText(strFc);
      m_ctrlStrengthDensity.GetWindowText(strDensity);

      CConcreteDetailsDlg* pParent = (CConcreteDetailsDlg*)GetParent();

      strK1.Format(_T("%f"),pParent->Concrete.EcK1);
      strK2.Format(_T("%f"),pParent->Concrete.EcK1);

      pgsTypes::ConcreteType type = GetConcreteType();
      CString strEc = CConcreteDetailsDlg::UpdateEc(type,strFc,strDensity,strK1,strK2);
      m_ctrlEc.SetWindowText(strEc);
   }
}

pgsTypes::ConcreteType CConcreteGeneralPage::GetConcreteType()
{
   CComboBox* pcbConcreteType = (CComboBox*)GetDlgItem(IDC_CONCRETE_TYPE);
   pgsTypes::ConcreteType type = (pgsTypes::ConcreteType)pcbConcreteType->GetItemData(pcbConcreteType->GetCurSel());
   return type;
}

void CConcreteGeneralPage::OnConcreteType()
{
   GetDlgItem(IDC_DS)->Invalidate();
   GetDlgItem(IDC_DW)->Invalidate();
   UpdateEc();
}

HBRUSH CConcreteGeneralPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
   HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

   if ( pWnd->GetDlgCtrlID() == IDC_DS && 0 < pWnd->GetWindowTextLength())
   {
      try
      {
         // if the user enters a number like .125, the first keystroke is ".". DDX_UnitValue calls
         // into MFC DDX_Text which calls AfxTextFloatFormat. This "." does not evaluate to a number
         // so an error message is displayed... this gets the user caught in an infinte loop of
         // pressing the OK button. The only way out is to crash the program.
         //
         // To work around this issue, we need to determine if the value in the field evaluates to
         // a number. If not, assume the density is not consistent with NWC and color the text red
         // otherwise, go on to normal processing.
	      const int TEXT_BUFFER_SIZE = 400;
	      TCHAR szBuffer[TEXT_BUFFER_SIZE];
         ::GetWindowText(GetDlgItem(IDC_DS)->GetSafeHwnd(), szBuffer, _countof(szBuffer));
		   Float64 d;
   		if (_sntscanf_s(szBuffer, _countof(szBuffer), _T("%lf"), &d) != 1)
         {
            pDC->SetTextColor( RED );
         }
         else
         {
            CDataExchange dx(this,TRUE);

            auto pBroker = EAFGetBroker();

            GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);
            Float64 value;
            DDX_UnitValue(&dx, IDC_DS, value, pDisplayUnits->GetDensityUnit() );

            if ( !IsDensityInRange(value,GetConcreteType()) )
            {
               pDC->SetTextColor( RED );
            }
         }
      }
      catch(...)
      {
      }
   }

   return hbr;
}

void CConcreteGeneralPage::OnOK()
{
   CPropertyPage::OnOK();

   CConcreteDetailsDlg* pParent = (CConcreteDetailsDlg*)GetParent();
   if ( !m_bErrorInDDX && !IsDensityInRange(pParent->Concrete.StrengthDensity,pParent->Concrete.Type) )
   {
      AFX_MANAGE_STATE(AfxGetStaticModuleState());
      AfxMessageBox(pParent->Concrete.Type == pgsTypes::Normal ? IDS_NWC_MESSAGE : IDS_LWC_MESSAGE,MB_OK | MB_ICONINFORMATION);
   }
}

bool CConcreteGeneralPage::IsDensityInRange(Float64 density,pgsTypes::ConcreteType type)
{
   ATLASSERT(!IsUHPC(type)); // piers should not be UHPC
   if (type == pgsTypes::Normal)
   {
      return ( IsLE(m_MinNWCDensity,density) );
   }
   else
   {
      return ( IsLE(density,m_MaxLWCDensity) );
   }
}
