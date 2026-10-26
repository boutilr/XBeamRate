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
#include <IFace/Tools.h>
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
    ON_MESSAGE(WM_PIER_LAYOUT_CHANGED, OnPierLayoutChanged)
    ON_REGISTERED_MESSAGE(WM_PIER_LAYOUT_POPOUT_CLOSED, OnPierLayoutPopoutClosed)
    ON_BN_CLICKED(IDC_LAYOUT_GRAPHIC, OnShowLiveClicked)
    ON_WM_DESTROY()
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
    if (curSel == CB_ERR)
        return;

    pgsTypes::PierLayoutType layoutType = (pgsTypes::PierLayoutType)pcbPierModel->GetItemData(curSel);
    if (layoutType == m_PierLayoutType)
        return;

    bool saved = false;
    switch (m_PierLayoutType)
    {
    case pgsTypes::pltCommon:
        saved = SaveColumnLayout(m_CommonPierLayoutDlg, m_CommonPierLayoutDlg.m_ColumnLayoutGrid);
        break;
    case pgsTypes::pltScalloped:
        saved = SaveColumnLayout(m_ScallopedPierLayoutDlg, m_ScallopedPierLayoutDlg.m_ColumnLayoutGrid);
        break;
    case pgsTypes::pltUserDefined:
        saved = SaveColumnLayout(m_UserDefinedPierLayoutDlg, m_UserDefinedPierLayoutDlg.m_ColumnLayoutGrid);
        break;
    }

    if (!saved)
    {
        CDataExchange dx(this, FALSE);
        DDX_CBEnum(&dx, IDC_PIER_LAYOUT_TYPE, m_PierLayoutType);
        return;
    }

    m_PierLayoutType = layoutType;
    switch (m_PierLayoutType)
    {
    case pgsTypes::pltCommon:
        LoadColumnLayout(m_CommonPierLayoutDlg, m_CommonPierLayoutDlg.m_ColumnLayoutGrid, m_CommonPierLayoutDlg.m_Pier);
        break;
    case pgsTypes::pltScalloped:
        LoadColumnLayout(m_ScallopedPierLayoutDlg, m_ScallopedPierLayoutDlg.m_ColumnLayoutGrid, m_ScallopedPierLayoutDlg.m_Pier);
        break;
    case pgsTypes::pltUserDefined:
        LoadColumnLayout(m_UserDefinedPierLayoutDlg, m_UserDefinedPierLayoutDlg.m_ColumnLayoutGrid, m_UserDefinedPierLayoutDlg.m_Pier);
        break;
    }

    SwapDialogs();
    switch (m_PierLayoutType)
    {
    case pgsTypes::pltCommon: m_CommonPierLayoutDlg.OnPierLayoutChanged(); break;
    case pgsTypes::pltScalloped: m_ScallopedPierLayoutDlg.OnPierLayoutChanged(); break;
    case pgsTypes::pltUserDefined: m_UserDefinedPierLayoutDlg.OnPierLayoutChanged(); break;
    }
    RefreshPierLayoutPopout(true);
}

bool CPierLayoutPage::SaveColumnLayout(CDialog& dialog, CColumnLayoutGrid& grid)
{
    auto pBroker = EAFGetBroker();
    GET_IFACE2(pBroker, IEAFDisplayUnits, pDisplayUnits);

    ColumnIndexType refColumnIdx;
    Float64 offset;
    pgsTypes::OffsetMeasurementType offsetType;
    m_pPier->GetRefColumnLocation(&offsetType, &refColumnIdx, &offset);
    pgsTypes::ColumnLongitudinalBaseFixityType fixity = m_pPier->GetColumnFixity();
    CColumnData::ColumnHeightMeasurementType heightMeasure = m_pPier->GetColumnData(0).GetColumnHeightMeasurementType();

    try
    {
        CDataExchange dx(&dialog, TRUE);
        CColumnLayoutGrid::DDV_ColumnGrid(&dx, grid);
        DDX_CBIndex(&dx, IDC_REFCOLUMN, refColumnIdx);
        DDX_OffsetAndTag(&dx, IDC_REFCOLUMN_OFFSET, IDC_REFCOLUMN_OFFSET_UNIT, offset, pDisplayUnits->GetSpanLengthUnit());
        DDX_CBItemData(&dx, IDC_REFCOLUMN_MEASUREMENT, offsetType);
        DDX_CBItemData(&dx, IDC_FIXITY, fixity);
        DDX_CBItemData(&dx, IDC_HEIGHT_MEASURE, heightMeasure);
    }
    catch (CUserException* pException)
    {
        pException->Delete();
        return false;
    }

    // Save only the shared column layout; cap inputs belong to each layout dialog.
    grid.GetColumnData(*m_pPier);
    m_pPier->SetRefColumnLocation(offsetType, refColumnIdx, offset);
    m_pPier->SetColumnFixity(fixity);
    for (ColumnIndexType colIdx = 0; colIdx < m_pPier->GetColumnCount(); colIdx++)
    {
        CColumnData column = m_pPier->GetColumnData(colIdx);
        column.SetColumnHeightMeasurementType(heightMeasure);
        m_pPier->SetColumnData(colIdx, column);
    }
    return true;
}

void CPierLayoutPage::LoadColumnLayout(CDialog& dialog, CColumnLayoutGrid& grid, xbrPierData& pier)
{
    ColumnIndexType nColumns = m_pPier->GetColumnCount();
    pier.SetColumnCount(nColumns);
    for (ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++)
    {
        pier.SetColumnData(colIdx, m_pPier->GetColumnData(colIdx));
        pier.SetColumnScallopDepth(colIdx, m_pPier->GetColumnScallopDepth(colIdx));
        if (colIdx + 1 < nColumns)
        {
            pier.SetColumnSpacing(colIdx, m_pPier->GetColumnSpacing(colIdx));
            pier.SetColumnScallopRadius(colIdx, m_pPier->GetColumnScallopRadius(colIdx));
        }
    }

    ColumnIndexType refColumnIdx;
    Float64 offset;
    pgsTypes::OffsetMeasurementType offsetType;
    m_pPier->GetRefColumnLocation(&offsetType, &refColumnIdx, &offset);
    pier.SetRefColumnLocation(offsetType, refColumnIdx, offset);
    pgsTypes::ColumnLongitudinalBaseFixityType fixity = m_pPier->GetColumnFixity();
    pier.SetColumnFixity(fixity);
    CColumnData::ColumnHeightMeasurementType heightMeasure = pier.GetColumnData(0).GetColumnHeightMeasurementType();
    grid.SetColumnData(pier);
    grid.SetHeightMeasurementType(heightMeasure);

    CComboBox* pcbRefColumn = (CComboBox*)dialog.GetDlgItem(IDC_REFCOLUMN);
    pcbRefColumn->ResetContent();
    for (ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++)
    {
        CString label;
        label.Format(_T("Column %d"), LABEL_COLUMN(colIdx));
        pcbRefColumn->AddString(label);
    }

    auto pBroker = EAFGetBroker();
    GET_IFACE2(pBroker, IEAFDisplayUnits, pDisplayUnits);
    CDataExchange dx(&dialog, FALSE);
    DDX_CBIndex(&dx, IDC_REFCOLUMN, refColumnIdx);
    DDX_OffsetAndTag(&dx, IDC_REFCOLUMN_OFFSET, IDC_REFCOLUMN_OFFSET_UNIT, offset, pDisplayUnits->GetSpanLengthUnit());
    DDX_CBItemData(&dx, IDC_REFCOLUMN_MEASUREMENT, offsetType);
    DDX_CBItemData(&dx, IDC_FIXITY, fixity);
    DDX_CBItemData(&dx, IDC_HEIGHT_MEASURE, heightMeasure);
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

	m_pPier->SetLowerXBeamDimensions(m_CommonPierLayoutDlg.m_XBeamHeight[pgsTypes::stLeft], m_CommonPierLayoutDlg.m_XBeamHeight[pgsTypes::stRight], m_CommonPierLayoutDlg.m_XBeamTaperHeight[pgsTypes::stLeft], 
    m_CommonPierLayoutDlg.m_XBeamTaperHeight[pgsTypes::stRight], m_CommonPierLayoutDlg.m_XBeamEndSlopeOffset[pgsTypes::stLeft], m_CommonPierLayoutDlg.m_XBeamEndSlopeOffset[pgsTypes::stRight], 
    m_CommonPierLayoutDlg.m_XBeamTaperLength[pgsTypes::stLeft], m_CommonPierLayoutDlg.m_XBeamTaperLength[pgsTypes::stRight],
    m_CommonPierLayoutDlg.m_XBeamWidth, m_pPier->GetPierPointData());

    m_pPier->SetColumnFixity(m_CommonPierLayoutDlg.m_ColumnFixity);
    m_CommonPierLayoutDlg.m_ColumnLayoutGrid.GetColumnData(*m_pPier);

    m_pPier->SetXBeamOverhangs(m_CommonPierLayoutDlg.m_XBeamOverhang[pgsTypes::stLeft], m_CommonPierLayoutDlg.m_XBeamOverhang[pgsTypes::stRight]);

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

    m_pPier->SetRefColumnLocation(m_ScallopedPierLayoutDlg.m_TransverseOffsetMeasurement, m_ScallopedPierLayoutDlg.m_RefColumnIdx, m_ScallopedPierLayoutDlg.m_TransverseOffset);

    m_pPier->SetLowerXBeamDimensions(m_ScallopedPierLayoutDlg.m_XBeamHeight[pgsTypes::stLeft], m_ScallopedPierLayoutDlg.m_XBeamHeight[pgsTypes::stRight], m_ScallopedPierLayoutDlg.m_XBeamTaperHeight[pgsTypes::stLeft],
        m_ScallopedPierLayoutDlg.m_XBeamTaperHeight[pgsTypes::stRight], m_ScallopedPierLayoutDlg.m_XBeamEndSlopeOffset[pgsTypes::stLeft], m_ScallopedPierLayoutDlg.m_XBeamEndSlopeOffset[pgsTypes::stRight],
        m_ScallopedPierLayoutDlg.m_XBeamTaperLength[pgsTypes::stLeft], m_ScallopedPierLayoutDlg.m_XBeamTaperLength[pgsTypes::stRight], m_ScallopedPierLayoutDlg.m_XBeamWidth, m_pPier->GetPierPointData());

    m_pPier->SetColumnFixity(m_ScallopedPierLayoutDlg.m_ColumnFixity);
    m_ScallopedPierLayoutDlg.m_ColumnLayoutGrid.GetColumnData(*m_pPier);
    m_pPier->SetXBeamOverhangRadius(pgsTypes::stLeft,
        m_ScallopedPierLayoutDlg.m_XBeamOverhangRadius[pgsTypes::stLeft]);
    m_pPier->SetXBeamOverhangRadius(pgsTypes::stRight,
        m_ScallopedPierLayoutDlg.m_XBeamOverhangRadius[pgsTypes::stRight]);

    m_pPier->SetXBeamOverhangs(m_ScallopedPierLayoutDlg.m_XBeamOverhang[pgsTypes::stLeft], m_ScallopedPierLayoutDlg.m_XBeamOverhang[pgsTypes::stRight]);

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

    m_pPier->SetRefColumnLocation(m_UserDefinedPierLayoutDlg.m_TransverseOffsetMeasurement, m_UserDefinedPierLayoutDlg.m_RefColumnIdx, m_UserDefinedPierLayoutDlg.m_TransverseOffset);

    m_pPier->SetLowerXBeamDimensions(m_UserDefinedPierLayoutDlg.m_XBeamHeight[pgsTypes::stLeft], m_UserDefinedPierLayoutDlg.m_XBeamHeight[pgsTypes::stRight], m_UserDefinedPierLayoutDlg.m_XBeamTaperHeight[pgsTypes::stLeft],
        m_UserDefinedPierLayoutDlg.m_XBeamTaperHeight[pgsTypes::stRight], m_UserDefinedPierLayoutDlg.m_XBeamEndSlopeOffset[pgsTypes::stLeft], m_UserDefinedPierLayoutDlg.m_XBeamEndSlopeOffset[pgsTypes::stRight],
        m_UserDefinedPierLayoutDlg.m_XBeamTaperLength[pgsTypes::stLeft], m_UserDefinedPierLayoutDlg.m_XBeamTaperLength[pgsTypes::stRight],
        m_UserDefinedPierLayoutDlg.m_XBeamWidth, m_pPier->GetPierPointData());


    m_pPier->SetColumnFixity(m_UserDefinedPierLayoutDlg.m_ColumnFixity);
    m_UserDefinedPierLayoutDlg.m_ColumnLayoutGrid.GetColumnData(*m_pPier);

	m_pPier->SetXBeamOverhangs(m_UserDefinedPierLayoutDlg.m_XBeamOverhang[pgsTypes::stLeft], m_UserDefinedPierLayoutDlg.m_XBeamOverhang[pgsTypes::stRight]);

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

    if (::IsWindow(m_PierLayoutPopout.GetSafeHwnd()))
        m_PierLayoutPopout.DestroyWindow();
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

IPierLayoutDataSource* CPierLayoutPage::GetLayoutDataSource()
{
    switch (m_PierLayoutType)
    {
    case pgsTypes::pltCommon: return &m_CommonPierLayoutDlg;
    case pgsTypes::pltScalloped: return &m_ScallopedPierLayoutDlg;
    case pgsTypes::pltUserDefined: return &m_UserDefinedPierLayoutDlg;
    default: return nullptr;
    }
}

void CPierLayoutPage::RefreshPierLayoutPopout(bool fit)
{
    if (!::IsWindow(m_PierLayoutPopout.GetSafeHwnd()))
        return;
    IPierLayoutDataSource* source = GetLayoutDataSource();
    if (!source)
        return;

    // Superstructure edits belong to the property sheet's transaction. Overlay
    // only this page's pending layout, leaving project data and caches untouched.
    xbrPierData preview = *m_pPier;
    const xbrPierData& layout = *source->GetPierData();
    preview.SetPierLayoutType(m_PierLayoutType);
    Float64 h1l, h1r, h2l, h2r, x1l, x1r, x2l, x2r, width;
    std::vector<CPierPointData> points;
    layout.GetLowerXBeamDimensions(&h1l, &h1r, &h2l, &h2r, &x1l, &x1r, &x2l, &x2r, &width, &points);
    preview.SetLowerXBeamDimensions(h1l, h1r, h2l, h2r, x1l, x1r, x2l, x2r, width, points);
    preview.SetColumnCount(layout.GetColumnCount());
    for (ColumnIndexType idx = 0; idx < layout.GetColumnCount(); ++idx)
    {
        preview.SetColumnData(idx, layout.GetColumnData(idx));
        preview.SetColumnScallopDepth(idx, layout.GetColumnScallopDepth(idx));
        if (idx + 1 < layout.GetColumnCount())
        {
            preview.SetColumnSpacing(idx, layout.GetColumnSpacing(idx));
            preview.SetColumnScallopRadius(idx, layout.GetColumnScallopRadius(idx));
        }
    }
    pgsTypes::OffsetMeasurementType datum;
    IndexType column;
    Float64 offset, left, right;
    layout.GetRefColumnLocation(&datum, &column, &offset);
    preview.SetRefColumnLocation(datum, column, offset);
    layout.GetXBeamOverhangs(&left, &right);
    preview.SetXBeamOverhangs(left, right);
    preview.SetXBeamOverhangRadius(pgsTypes::stLeft, layout.GetXBeamOverhangRadius(pgsTypes::stLeft));
    preview.SetXBeamOverhangRadius(pgsTypes::stRight, layout.GetXBeamOverhangRadius(pgsTypes::stRight));
    m_PierLayoutPopout.Refresh(preview, fit);
}

LRESULT CPierLayoutPage::OnPierLayoutChanged(WPARAM, LPARAM)
{
    RefreshPierLayoutPopout();
    return 0;
}

LRESULT CPierLayoutPage::OnPierLayoutPopoutClosed(WPARAM, LPARAM)
{
    if (CWnd* button = GetDlgItem(IDC_LAYOUT_GRAPHIC))
        button->SetWindowText(_T("Show Popout Window"));
    return 0;
}

void CPierLayoutPage::OnShowLiveClicked()
{
    if (::IsWindow(m_PierLayoutPopout.GetSafeHwnd()))
    {
        m_PierLayoutPopout.DestroyWindow();
        return;
    }
    IPierLayoutDataSource* source = GetLayoutDataSource();
    if (!source)
        return;
    switch (m_PierLayoutType)
    {
    case pgsTypes::pltCommon: m_CommonPierLayoutDlg.OnPierLayoutChanged(); break;
    case pgsTypes::pltScalloped: m_ScallopedPierLayoutDlg.OnPierLayoutChanged(); break;
    case pgsTypes::pltUserDefined: m_UserDefinedPierLayoutDlg.OnPierLayoutChanged(); break;
    }
    if (m_PierLayoutPopout.CreatePopout(*source->GetPierData(), this))
    {
        RefreshPierLayoutPopout(true);
        GetDlgItem(IDC_LAYOUT_GRAPHIC)->SetWindowText(_T("Hide Popout Window"));
    }
}

void CPierLayoutPage::OnDestroy()
{
    if (::IsWindow(m_PierLayoutPopout.GetSafeHwnd()))
        m_PierLayoutPopout.DestroyWindow();
    CPropertyPage::OnDestroy();
}
