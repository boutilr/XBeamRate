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

// PierLayoutPat.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ProjectAgent.h"
#include "PierLayoutPage.h"
#include "PierDlg.h"

#include <EAF\EAFDisplayUnits.h>
#include <EAF\EAFDocument.h>
#include <MFCTools\CustomDDX.h>

#include <PsgLib\GirderLabel.h>
#include "..\Documentation\XBRate.hh"

// CPierLayoutPage dialog


/////////////////////////////////////////////////////////////////////////////
// CPierLayoutPage property page

IMPLEMENT_DYNCREATE(CPierLayoutPage, CPropertyPage)

CPierLayoutPage::CPierLayoutPage() : CPropertyPage(CPierLayoutPage::IDD)
{
	//{{AFX_DATA_INIT(CPierLayoutPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPierLayoutPage::~CPierLayoutPage()
{
}

void CPierLayoutPage::DoDataExchange(CDataExchange* pDX)
{
  
   CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPierConnectionsPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

   CPierDlg* pParent = (CPierDlg*)GetParent();

   DDX_CBEnum(pDX, IDC_PIER_LAYOUT_TYPE, m_PierLayoutType);

   DDX_CBEnum(pDX, IDC_CONDITION_FACTOR_TYPE, pParent->m_PierData.m_PierData.GetConditionFactorType());
   DDX_Text(pDX,   IDC_CONDITION_FACTOR,      pParent->m_PierData.m_PierData.GetConditionFactor());

   if (pDX->m_bSaveAndValidate)
   {
       // Only validate and save data owned by this page here.
       // Embedded child dialogs are validated and committed from OnApply/OnKillActive
       // so we do not recurse into UpdateData(TRUE) from inside DoDataExchange.

       m_pPier->SetPierLayoutType(m_PierLayoutType);
   }

}

BEGIN_MESSAGE_MAP(CPierLayoutPage, CPropertyPage)
	//{{AFX_MSG_MAP(CPierLayoutPage)
	ON_COMMAND(ID_HELP, OnHelp)
   ON_CBN_SELCHANGE(IDC_CONDITION_FACTOR_TYPE, OnConditionFactorTypeChanged)
    ON_CBN_SELCHANGE(IDC_PIER_LAYOUT_TYPE, OnPierLayoutTypeChanged)
    //ON_MESSAGE(WM_PIER_LAYOUT_CHANGED, OnPierLayoutChanged)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPierLayoutPage message handlers

BOOL CPierLayoutPage::OnInitDialog() 
{

	const auto& parent = (CPierDlg*)GetParent();
	if (parent)
	{
		m_pPier = parent->GetXBRPierData();
	}

    m_PierLayoutType = m_pPier->GetPierLayoutType();

    FillPierLayoutTypeComboBox();

    CWnd* pBox = GetDlgItem(IDC_STATIC_BOUNDS);
    pBox->ShowWindow(SW_HIDE);

    CRect boxRect;
    pBox->GetWindowRect(&boxRect);
    ScreenToClient(boxRect);

    m_CommonPierLayoutDlg.SetPierData(*m_pPier);
    VERIFY(m_CommonPierLayoutDlg.Create(IDD_PIER_LAYOUT_COMMON, this));
    VERIFY(m_CommonPierLayoutDlg.SetWindowPos(GetDlgItem(IDC_STATIC_BOUNDS), boxRect.left, boxRect.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE));//|SWP_NOMOVE));

    m_ScallopedPierLayoutDlg.SetPierData(*m_pPier);
    VERIFY(m_ScallopedPierLayoutDlg.Create(IDD_PIER_LAYOUT_SCALLOPED, this));
    VERIFY(m_ScallopedPierLayoutDlg.SetWindowPos(GetDlgItem(IDC_STATIC_BOUNDS), boxRect.left, boxRect.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE));//|SWP_NOMOVE));

    m_UserDefinedPierLayoutDlg.SetPierData(*m_pPier);
    VERIFY(m_UserDefinedPierLayoutDlg.Create(IDD_PIER_LAYOUT_USERDEFINED, this));
    VERIFY(m_UserDefinedPierLayoutDlg.SetWindowPos(GetDlgItem(IDC_STATIC_BOUNDS), boxRect.left, boxRect.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE));//|SWP_NOMOVE));

   CComboBox* pcbConditionFactor = (CComboBox*)GetDlgItem(IDC_CONDITION_FACTOR_TYPE);
   pcbConditionFactor->AddString(_T("Good or Satisfactory (Structure condition rating 6 or higher)"));
   pcbConditionFactor->AddString(_T("Fair (Structure condition rating of 5)"));
   pcbConditionFactor->AddString(_T("Poor (Structure condition rating 4 or lower)"));
   pcbConditionFactor->AddString(_T("Other"));
   pcbConditionFactor->SetCurSel(0);

   CPropertyPage::OnInitDialog();

   OnConditionFactorTypeChanged();

   SwapDialogs();

   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPierLayoutPage::OnConditionFactorTypeChanged()
{
   CEdit* pEdit = (CEdit*)GetDlgItem(IDC_CONDITION_FACTOR);
   CComboBox* pcbConditionFactor = (CComboBox*)GetDlgItem(IDC_CONDITION_FACTOR_TYPE);

   int idx = pcbConditionFactor->GetCurSel();
   switch(idx)
   {
   case 0:
      pEdit->EnableWindow(FALSE);
      pEdit->SetWindowText(_T("1.00"));
      break;
   case 1:
      pEdit->EnableWindow(FALSE);
      pEdit->SetWindowText(_T("0.95"));
      break;
   case 2:
      pEdit->EnableWindow(FALSE);
      pEdit->SetWindowText(_T("0.85"));
      break;
   case 3:
      pEdit->EnableWindow(TRUE);
      break;
   }
}

void CPierLayoutPage::OnPierLayoutTypeChanged()
{
    CComboBox* pcbPierModel = (CComboBox*)GetDlgItem(IDC_PIER_LAYOUT_TYPE);
    int curSel = pcbPierModel->GetCurSel();
    m_PierLayoutType = (pgsTypes::PierLayoutType)pcbPierModel->GetItemData(curSel);

    SwapDialogs();
}

void CPierLayoutPage::SwapDialogs() // call UpdateData(TRUE) on these?
{

    if (m_PierLayoutType == pgsTypes::pltCommon)
    {
        m_CommonPierLayoutDlg.ShowWindow(SW_SHOW);
        m_ScallopedPierLayoutDlg.ShowWindow(SW_HIDE);
        m_UserDefinedPierLayoutDlg.ShowWindow(SW_HIDE);
    }
    else if (m_PierLayoutType == pgsTypes::pltScalloped)
    {
        m_CommonPierLayoutDlg.ShowWindow(SW_HIDE);
        m_ScallopedPierLayoutDlg.ShowWindow(SW_SHOW);
        m_UserDefinedPierLayoutDlg.ShowWindow(SW_HIDE);
    }
    else if (m_PierLayoutType == pgsTypes::pltUserDefined)
    {
        m_CommonPierLayoutDlg.ShowWindow(SW_HIDE);
        m_ScallopedPierLayoutDlg.ShowWindow(SW_HIDE);
        m_UserDefinedPierLayoutDlg.ShowWindow(SW_SHOW);
    }
    else
    {
        m_CommonPierLayoutDlg.ShowWindow(SW_HIDE);
        m_ScallopedPierLayoutDlg.ShowWindow(SW_HIDE);
        m_UserDefinedPierLayoutDlg.ShowWindow(SW_HIDE);
    }

}

void CPierLayoutPage::FillPierLayoutTypeComboBox()
{
    CComboBox* pcbPierModel = (CComboBox*)GetDlgItem(IDC_PIER_LAYOUT_TYPE);
    pcbPierModel->ResetContent();

    int idx = pcbPierModel->AddString(_T("Common"));
    pcbPierModel->SetItemData(idx, (DWORD_PTR)pgsTypes::pltCommon);

    idx = pcbPierModel->AddString(_T("Scalloped"));
    pcbPierModel->SetItemData(idx, (DWORD_PTR)pgsTypes::pltScalloped);

    idx = pcbPierModel->AddString(_T("User-Defined"));
    pcbPierModel->SetItemData(idx, (DWORD_PTR)pgsTypes::pltUserDefined);

}

void CPierLayoutPage::OnHelp() 
{
   EAFHelp(EAFGetDocument()->GetDocumentationSetName(), IDH_SUBSTRUCTURE);
}

bool CPierLayoutPage::CommitCommonPierLayout()
{

    if (!m_CommonPierLayoutDlg.UpdateData(TRUE))
    {
        return false;
    }

    m_pPier->SetRefColumnLocation(m_CommonPierLayoutDlg.m_TransverseOffsetMeasurement, m_CommonPierLayoutDlg.m_RefColumnIdx, m_CommonPierLayoutDlg.m_TransverseOffset);

    std::vector<CPierPointData> pvpp;
	m_pPier->SetLowerXBeamDimensions(m_CommonPierLayoutDlg.m_XBeamHeight[pgsTypes::stLeft], m_CommonPierLayoutDlg.m_XBeamHeight[pgsTypes::stRight], m_CommonPierLayoutDlg.m_XBeamTaperHeight[pgsTypes::stLeft],
        m_CommonPierLayoutDlg.m_XBeamTaperHeight[pgsTypes::stRight], m_CommonPierLayoutDlg.m_XBeamTaperLength[pgsTypes::stLeft], m_CommonPierLayoutDlg.m_XBeamTaperLength[pgsTypes::stRight], 
        m_CommonPierLayoutDlg.m_XBeamEndSlopeOffset[pgsTypes::stLeft], m_CommonPierLayoutDlg.m_XBeamEndSlopeOffset[pgsTypes::stRight], m_CommonPierLayoutDlg.m_XBeamWidth, 0, 0, pvpp);

    m_pPier->SetColumnFixity(m_CommonPierLayoutDlg.m_ColumnFixity);
    m_CommonPierLayoutDlg.m_ColumnLayoutGrid.GetColumnData(*m_pPier);

    ColumnIndexType nColumns = m_pPier->GetColumnCount();
    for (ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++)
    {
        CColumnData column = m_pPier->GetColumnData(colIdx);
        column.SetColumnHeightMeasurementType(m_CommonPierLayoutDlg.m_ColumnHeightMeasurementType);
        m_pPier->SetColumnData(colIdx, column);
    }

    return true;
}

bool CPierLayoutPage::CommitScallopedPierLayout()
{

    if (!m_ScallopedPierLayoutDlg.UpdateData(TRUE))
    {
        return false;
    }

    std::vector<CPierPointData> pvpp;
    m_pPier->SetLowerXBeamDimensions(m_ScallopedPierLayoutDlg.m_XBeamHeight[pgsTypes::stLeft], m_ScallopedPierLayoutDlg.m_XBeamHeight[pgsTypes::stRight], m_ScallopedPierLayoutDlg.m_XBeamTaperHeight[pgsTypes::stLeft],
        m_ScallopedPierLayoutDlg.m_XBeamTaperHeight[pgsTypes::stRight], m_ScallopedPierLayoutDlg.m_XBeamTaperLength[pgsTypes::stLeft], m_ScallopedPierLayoutDlg.m_XBeamTaperLength[pgsTypes::stRight],
        m_ScallopedPierLayoutDlg.m_XBeamEndSlopeOffset[pgsTypes::stLeft], m_ScallopedPierLayoutDlg.m_XBeamEndSlopeOffset[pgsTypes::stRight], m_ScallopedPierLayoutDlg.m_XBeamWidth,
        m_ScallopedPierLayoutDlg.m_XBeamRadius, m_ScallopedPierLayoutDlg.m_XBeamDepth, pvpp);


    m_pPier->SetColumnFixity(m_ScallopedPierLayoutDlg.m_ColumnFixity);
    m_ScallopedPierLayoutDlg.m_ColumnLayoutGrid.GetColumnData(*m_pPier);

    ColumnIndexType nColumns = m_pPier->GetColumnCount();
    for (ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++)
    {
        CColumnData column = m_pPier->GetColumnData(colIdx);
        column.SetColumnHeightMeasurementType(m_ScallopedPierLayoutDlg.m_ColumnHeightMeasurementType);
        m_pPier->SetColumnData(colIdx, column);
    }

    return true;
}

bool CPierLayoutPage::CommitUserDefinedPierLayout()
{

    if (!m_UserDefinedPierLayoutDlg.UpdateData(TRUE))
    {
        return false;
    }

	std::vector<CPierPointData> vPoints;
    m_pPier->SetLowerXBeamDimensions(m_UserDefinedPierLayoutDlg.m_XBeamHeight[pgsTypes::stLeft], m_UserDefinedPierLayoutDlg.m_XBeamHeight[pgsTypes::stRight], m_UserDefinedPierLayoutDlg.m_XBeamTaperHeight[pgsTypes::stLeft],
        m_UserDefinedPierLayoutDlg.m_XBeamTaperHeight[pgsTypes::stRight], m_UserDefinedPierLayoutDlg.m_XBeamTaperLength[pgsTypes::stLeft], m_UserDefinedPierLayoutDlg.m_XBeamTaperLength[pgsTypes::stRight],
        m_UserDefinedPierLayoutDlg.m_XBeamEndSlopeOffset[pgsTypes::stLeft], m_UserDefinedPierLayoutDlg.m_XBeamEndSlopeOffset[pgsTypes::stRight], m_UserDefinedPierLayoutDlg.m_XBeamWidth, 0, 0, vPoints);

    m_pPier->SetColumnFixity(m_UserDefinedPierLayoutDlg.m_ColumnFixity);
    m_UserDefinedPierLayoutDlg.m_ColumnLayoutGrid.GetColumnData(*m_pPier);

    ColumnIndexType nColumns = m_pPier->GetColumnCount();
    for (ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++)
    {
        CColumnData column = m_pPier->GetColumnData(colIdx);
        column.SetColumnHeightMeasurementType(m_UserDefinedPierLayoutDlg.m_ColumnHeightMeasurementType);
        m_pPier->SetColumnData(colIdx, column);
    }

    m_UserDefinedPierLayoutDlg.m_PierPointGrid.GetPierPointData(*m_pPier);

	PierPointIndexType ppIdx = 0;
	for (const auto& pierPoint : m_pPier->GetPierPointData())
    {
        m_pPier->SetPierPointData(ppIdx++, pierPoint);
    }

    return true;
}

BOOL CPierLayoutPage::OnKillActive()
{
    if (!UpdateData(TRUE))
    {
        return FALSE;
    }

    if (m_PierLayoutType == pgsTypes::pltCommon)
    {
        if (!CommitCommonPierLayout())
        {
            return FALSE;
        }
    }
    else if (m_PierLayoutType == pgsTypes::pltScalloped)
    {
        if (!CommitScallopedPierLayout())
        {
            return FALSE;
        }
    }
    else if (m_PierLayoutType == pgsTypes::pltUserDefined)
    {
        if (!CommitUserDefinedPierLayout())
        {
            return FALSE;
        }
    }

    return CPropertyPage::OnKillActive();
}

BOOL CPierLayoutPage::OnApply()
{
    if (!UpdateData(TRUE))
    {
        return FALSE;
    }

    if (m_PierLayoutType == pgsTypes::pltCommon)
    {
        if (!CommitCommonPierLayout())
        {
            return FALSE;
        }
    }
    else if (m_PierLayoutType == pgsTypes::pltScalloped)
    {
        if (!CommitScallopedPierLayout())
        {
            return FALSE;
        }
    }
    else if (m_PierLayoutType == pgsTypes::pltUserDefined)
    {
        if (!CommitUserDefinedPierLayout())
        {
            return FALSE;
        }
    }

    SetModified(FALSE);
    return CPropertyPage::OnApply();
}
