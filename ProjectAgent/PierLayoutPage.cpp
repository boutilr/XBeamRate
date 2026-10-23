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



void DDX_ColumnGrid(CDataExchange* pDX,CColumnLayoutGrid& grid,xbrPierData& pier)
{
   if ( pDX->m_bSaveAndValidate )
   {
      grid.GetColumnData(pier);
   }
   else
   {
      grid.SetColumnData(pier);
   }
}

void DDV_ColumnGrid(CDataExchange* pDX,CColumnLayoutGrid& grid)
{
   if ( pDX->m_bSaveAndValidate )
   {
      if ( grid.GetRowCount() == 0 )
      {
         AfxMessageBox(_T("The pier must have at least one column"),MB_OK | MB_ICONEXCLAMATION);
         pDX->Fail();
      }

      if (grid.GetRowCount() == 1)
      {
         // single column piers must have a fixed connection
         xbrPierData pier;
         grid.GetColumnData(pier);
         ATLASSERT(pier.GetColumnCount() == 1);
         auto column = pier.GetColumnData(0);
         pgsTypes::ColumnTransverseFixityType fixity = column.GetTransverseFixity();
         if (fixity != pgsTypes::ctftTopFixedBottomFixed)
         {
            AfxMessageBox(_T("Single column piers must be fixed. Change the column fixity."), MB_OK | MB_ICONEXCLAMATION);
            pDX->Fail();
         }
      }
   }
}

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

   auto pBroker = EAFGetBroker();
   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   CPierDlg* pParent = (CPierDlg*)GetParent();

   DDX_MetaFileStatic(pDX, IDC_PIER_LAYOUT, m_LayoutPicture,_T("PIERLAYOUT"), _T("Metafile") );

   // Transverse location of the pier
   DDX_CBIndex(pDX,IDC_REFCOLUMN,pParent->m_PierData.m_PierData.GetRefColumnIndex());
   DDX_OffsetAndTag(pDX,IDC_REFCOLUMN_OFFSET,IDC_REFCOLUMN_OFFSET_UNIT,pParent->m_PierData.m_PierData.GetRefColumnOffset(), pDisplayUnits->GetSpanLengthUnit() );
   DDX_CBItemData(pDX,IDC_REFCOLUMN_MEASUREMENT,pParent->m_PierData.m_PierData.GetColumnLayoutDatum());

   DDX_UnitValueAndTag(pDX,IDC_H1,IDC_H1_UNIT,pParent->m_PierData.m_PierData.GetH1(),pDisplayUnits->GetSpanLengthUnit() );
   DDX_UnitValueAndTag(pDX,IDC_H2,IDC_H2_UNIT,pParent->m_PierData.m_PierData.GetH2(),pDisplayUnits->GetSpanLengthUnit() );
   DDX_UnitValueAndTag(pDX,IDC_X1,IDC_X1_UNIT,pParent->m_PierData.m_PierData.GetX1(),pDisplayUnits->GetSpanLengthUnit() );
   DDX_UnitValueAndTag(pDX,IDC_X2,IDC_X2_UNIT,pParent->m_PierData.m_PierData.GetX2(),pDisplayUnits->GetSpanLengthUnit() );

   DDX_UnitValueAndTag(pDX,IDC_H3,IDC_H3_UNIT,pParent->m_PierData.m_PierData.GetH3(),pDisplayUnits->GetSpanLengthUnit() );
   DDX_UnitValueAndTag(pDX,IDC_H4,IDC_H4_UNIT,pParent->m_PierData.m_PierData.GetH4(),pDisplayUnits->GetSpanLengthUnit() );
   DDX_UnitValueAndTag(pDX,IDC_X3,IDC_X3_UNIT,pParent->m_PierData.m_PierData.GetX3(),pDisplayUnits->GetSpanLengthUnit() );
   DDX_UnitValueAndTag(pDX,IDC_X4,IDC_X4_UNIT,pParent->m_PierData.m_PierData.GetX4(),pDisplayUnits->GetSpanLengthUnit() );

   DDX_UnitValueAndTag(pDX,IDC_W,IDC_W_UNIT,pParent->m_PierData.m_PierData.GetW(),pDisplayUnits->GetSpanLengthUnit() );

   DDX_UnitValueAndTag(pDX,IDC_X5,IDC_X5_UNIT,pParent->m_PierData.m_PierData.GetX5(),pDisplayUnits->GetSpanLengthUnit() );
   DDX_UnitValueAndTag(pDX,IDC_X6,IDC_X6_UNIT,pParent->m_PierData.m_PierData.GetX6(),pDisplayUnits->GetSpanLengthUnit() );

   DDV_ColumnGrid(pDX,m_ColumnLayoutGrid);
   DDX_ColumnGrid(pDX,m_ColumnLayoutGrid,pParent->m_PierData.m_PierData);

   if ( pDX->m_bSaveAndValidate )
   {
      CColumnData::ColumnHeightMeasurementType measure;
      DDX_CBItemData(pDX,IDC_HEIGHT_MEASURE,measure);
      ColumnIndexType nColumns = pParent->m_PierData.m_PierData.GetColumnCount();
      for ( ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++ )
      {
         pParent->m_PierData.m_PierData.GetColumnData(colIdx).SetColumnHeightMeasurementType(measure);
      }
   }
   else
   {
      CColumnData::ColumnHeightMeasurementType measure = pParent->m_PierData.m_PierData.GetColumnData(0).GetColumnHeightMeasurementType();
      DDX_CBItemData(pDX,IDC_HEIGHT_MEASURE,measure);
   }

   DDX_CBEnum(pDX, IDC_CONDITION_FACTOR_TYPE, pParent->m_PierData.m_PierData.GetConditionFactorType());
   DDX_Text(pDX,   IDC_CONDITION_FACTOR,      pParent->m_PierData.m_PierData.GetConditionFactor());

   if (pDX->m_bSaveAndValidate)
   {
      // XBeam width, W, must be greater than zeo
      DDV_UnitValueGreaterThanZero(pDX, IDC_W, pParent->m_PierData.m_PierData.GetW(), pDisplayUnits->GetSpanLengthUnit());

      // H1 and H3 must be > 0
      DDV_UnitValueGreaterThanZero(pDX, IDC_H1, pParent->m_PierData.m_PierData.GetH1(), pDisplayUnits->GetSpanLengthUnit());
      DDV_UnitValueGreaterThanZero(pDX, IDC_H3, pParent->m_PierData.m_PierData.GetH3(), pDisplayUnits->GetSpanLengthUnit());

      // X1, X2, X3 and X4 must be >= 0
      DDV_UnitValueZeroOrMore(pDX, IDC_X1, pParent->m_PierData.m_PierData.GetX1(), pDisplayUnits->GetSpanLengthUnit());
      DDV_UnitValueZeroOrMore(pDX, IDC_X2, pParent->m_PierData.m_PierData.GetX2(), pDisplayUnits->GetSpanLengthUnit());
      DDV_UnitValueZeroOrMore(pDX, IDC_X3, pParent->m_PierData.m_PierData.GetX3(), pDisplayUnits->GetSpanLengthUnit());
      DDV_UnitValueZeroOrMore(pDX, IDC_X4, pParent->m_PierData.m_PierData.GetX4(), pDisplayUnits->GetSpanLengthUnit());

      // Left end
      if (0 < pParent->m_PierData.m_PierData.GetX1())
      {
         // if H2 > 0, then X1 must be > 0
         if ( IsZero(pParent->m_PierData.m_PierData.GetH2()) )
         {
            pDX->PrepareCtrl(IDC_H2);
            AfxMessageBox(_T("H2 must be greater than zero when X1 is greater than zero."));
            pDX->Fail();
         }
         else if ( pParent->m_PierData.m_PierData.GetX1() < pParent->m_PierData.m_PierData.GetX2() )
         {
            pDX->PrepareCtrl(IDC_X1);
            AfxMessageBox(_T("X1 must be greater than X2 when X1 is greater than zero."));
            pDX->Fail();
         }
      }
      else if ( !IsZero(pParent->m_PierData.m_PierData.GetH2()) )
      {
         // if X1 is zero, then H2 must also be zero
         pDX->PrepareCtrl(IDC_H2);
         AfxMessageBox(_T("H2 must be zero when X1 is zero"));
         pDX->Fail();
      }

      // Right end
      if (0 < pParent->m_PierData.m_PierData.GetX3())
      {
         // if H4 > 0, then X3 must be > 0
         if ( IsZero(pParent->m_PierData.m_PierData.GetH4()) )
         {
            pDX->PrepareCtrl(IDC_H4);
            AfxMessageBox(_T("H4 must be greater than zero when X3 is greater than zero."));
            pDX->Fail();
         }
         else if ( pParent->m_PierData.m_PierData.GetX3() < pParent->m_PierData.m_PierData.GetX4() )
         {
            pDX->PrepareCtrl(IDC_X3);
            AfxMessageBox(_T("X3 must be greater than X4 when X3 is greater than zero."));
            pDX->Fail();
         }
      }
      else if ( !IsZero(pParent->m_PierData.m_PierData.GetH4()) )
      {
         // if X3 is zero, then H4 must also be zero
         pDX->PrepareCtrl(IDC_H4);
         AfxMessageBox(_T("H4 must be zero when X3 is zero"));
         pDX->Fail();
      }

      Float64 D1, D2;
      // X5 must be >= diameter of first column
      pParent->m_PierData.m_PierData.GetColumnData(0).GetColumnDimensions(&D1, &D2);
      DDV_UnitValueLimitOrMore(pDX, IDC_X5, pParent->m_PierData.m_PierData.GetX5(), D1/2,pDisplayUnits->GetSpanLengthUnit());

      // X6 must be >= diameter of first column
      ColumnIndexType nColumns = pParent->m_PierData.m_PierData.GetColumnCount();
      pParent->m_PierData.m_PierData.GetColumnData(nColumns - 1).GetColumnDimensions(&D1, &D2);
      DDV_UnitValueLimitOrMore(pDX, IDC_X6, pParent->m_PierData.m_PierData.GetX6(), D1/2, pDisplayUnits->GetSpanLengthUnit());

      // X1 + X3 must be less than X5 + X6 + Sum(S)
      ATLASSERT(1 <= nColumns);
      Float64 S = 0;
      for (SpacingIndexType spaIdx = 0; spaIdx < nColumns - 1; spaIdx++)
      {
         S += pParent->m_PierData.m_PierData.GetColumnSpacing(spaIdx);
      }
      Float64 pierWidth = pParent->m_PierData.m_PierData.GetX5() + pParent->m_PierData.m_PierData.GetX6() + S;
      Float64 sumOverhangs = pParent->m_PierData.m_PierData.GetX1() + pParent->m_PierData.m_PierData.GetX3();
      if (pierWidth < sumOverhangs)
      {
         pDX->PrepareCtrl(IDC_X5);
         AfxMessageBox(_T("X1 + X3 cannot exceed the overall pier width (X5 + X6 + summation of S)"));
         pDX->Fail();
      }
   }
}

BEGIN_MESSAGE_MAP(CPierLayoutPage, CPropertyPage)
	//{{AFX_MSG_MAP(CPierLayoutPage)
	ON_COMMAND(ID_HELP, OnHelp)
   ON_CBN_SELCHANGE(IDC_CONDITION_FACTOR_TYPE, OnConditionFactorTypeChanged)
   ON_CBN_SELCHANGE(IDC_HEIGHT_MEASURE, OnHeightMeasureChanged)
	//}}AFX_MSG_MAP
   ON_BN_CLICKED(IDC_ADD_COLUMN, &CPierLayoutPage::OnAddColumn)
   ON_BN_CLICKED(IDC_REMOVE_COLUMN, &CPierLayoutPage::OnRemoveColumns)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPierLayoutPage message handlers

BOOL CPierLayoutPage::OnInitDialog() 
{
   m_ColumnLayoutGrid.SubclassDlgItem(IDC_COLUMN_GRID, this);
   m_ColumnLayoutGrid.CustomInit();

   CPierDlg* pParent = (CPierDlg*)GetParent();
   FillRefColumnComboBox(pParent->m_PierData.m_PierData.GetColumnCount());
   FillTransverseLocationComboBox();
   FillHeightMeasureComboBox();

   CComboBox* pcbConditionFactor = (CComboBox*)GetDlgItem(IDC_CONDITION_FACTOR_TYPE);
   pcbConditionFactor->AddString(_T("Good or Satisfactory (Structure condition rating 6 or higher)"));
   pcbConditionFactor->AddString(_T("Fair (Structure condition rating of 5)"));
   pcbConditionFactor->AddString(_T("Poor (Structure condition rating 4 or lower)"));
   pcbConditionFactor->AddString(_T("Other"));
   pcbConditionFactor->SetCurSel(0);

   CPropertyPage::OnInitDialog();

   OnConditionFactorTypeChanged();
   OnHeightMeasureChanged();

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

void CPierLayoutPage::FillTransverseLocationComboBox()
{
   CComboBox* pcbMeasure = (CComboBox*)GetDlgItem(IDC_REFCOLUMN_MEASUREMENT);
   pcbMeasure->ResetContent();
   int idx = pcbMeasure->AddString(_T("from the Alignment"));
   pcbMeasure->SetItemData(idx,(DWORD_PTR)pgsTypes::omtAlignment);
   idx = pcbMeasure->AddString(_T("from the Bridgeline"));
   pcbMeasure->SetItemData(idx,(DWORD_PTR)pgsTypes::omtBridge);
}

void CPierLayoutPage::FillRefColumnComboBox(ColumnIndexType nColumns)
{
   CComboBox* pcbRefColumn = (CComboBox*)GetDlgItem(IDC_REFCOLUMN);
   int curSel = pcbRefColumn->GetCurSel();
   pcbRefColumn->ResetContent();
   if ( nColumns == INVALID_INDEX )
   {
      nColumns = (ColumnIndexType)m_ColumnLayoutGrid.GetRowCount();
   }

   for ( ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++ )
   {
      CString strLabel;
      strLabel.Format(_T("Column %d"),LABEL_COLUMN(colIdx));
      pcbRefColumn->AddString(strLabel);
   }

   if ( pcbRefColumn->SetCurSel(curSel) == CB_ERR )
   {
      pcbRefColumn->SetCurSel(0);
   }
}

void CPierLayoutPage::FillHeightMeasureComboBox()
{
   CComboBox* pcbHeightMeasure = (CComboBox*)GetDlgItem(IDC_HEIGHT_MEASURE);
   pcbHeightMeasure->ResetContent();
   int idx = pcbHeightMeasure->AddString(_T("Height (H)"));
   pcbHeightMeasure->SetItemData(idx,(DWORD_PTR)CColumnData::chtHeight);
   idx = pcbHeightMeasure->AddString(_T("Bottom Elevation"));
   pcbHeightMeasure->SetItemData(idx,(DWORD_PTR)CColumnData::chtBottomElevation);
}

void CPierLayoutPage::OnHeightMeasureChanged()
{
   CComboBox* pcbHeightMeasure = (CComboBox*)GetDlgItem(IDC_HEIGHT_MEASURE);
   int curSel = pcbHeightMeasure->GetCurSel();
   CColumnData::ColumnHeightMeasurementType measure = (CColumnData::ColumnHeightMeasurementType)(pcbHeightMeasure->GetItemData(curSel));
   m_ColumnLayoutGrid.SetHeightMeasurementType(measure);
}

void CPierLayoutPage::OnHelp() 
{
   EAFHelp(EAFGetDocument()->GetDocumentationSetName(), IDH_SUBSTRUCTURE);
}

void CPierLayoutPage::OnAddColumn()
{
   m_ColumnLayoutGrid.AddColumn();
   FillRefColumnComboBox();
}

void CPierLayoutPage::OnRemoveColumns()
{
   m_ColumnLayoutGrid.RemoveSelectedColumns();
   FillRefColumnComboBox();
}
