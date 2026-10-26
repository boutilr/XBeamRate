///////////////////////////////////////////////////////////////////////
// PGSuper - Prestressed Girder SUPERstructure Design and Analysis
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

// ScallopedPierLayoutDlg.cpp : implementation file
//

///////////////////////////////////////////////////////////////////////////
// NOTE: Duplicate code warning
//
// This dialog along with all its property pages are basically repeated in
// the PGSuperLibrary project. I could not get a single implementation to
// work because of issues with the module resources.
//
// If changes are made here, the same changes are likely needed in
// the other location.

#include "stdafx.h"
#include "PierLayoutPage.h"
#include "ScallopedPierLayoutDlg.h"
#include <EAF\EAFDisplayUnits.h>
#include <IFace\Project.h>
#include <IFace/Tools.h>
#include <PsgLib\GirderLabel.h>
#include <Math/Math.h>
#include "ScallopGeometry.h"

CScallopedPierLayoutDlg::CScallopedPierLayoutDlg(CWnd* pParent)
    :CDialog(IDD_PIER_LAYOUT_SCALLOPED, pParent)
{

    // only using the fixed option (no pinned at base of column,
    // it leads to unstable models before continuity is achieved)
    m_cbColumnFixity.SetFixityTypes(COLUMN_FIXITY_FIXED);
    m_ColumnLayoutGrid.SetScalloped(true);

}

// Add to message map
BEGIN_MESSAGE_MAP(CScallopedPierLayoutDlg, CDialog)
    ON_MESSAGE(WM_COLUMN_GRID_CELL_CHANGED, OnColumnGridCellChanged)
    ON_EN_KILLFOCUS(IDC_W, &CScallopedPierLayoutDlg::OnPierLayoutChanged)
    ON_EN_KILLFOCUS(IDC_H1L, &CScallopedPierLayoutDlg::OnPierLayoutChanged)
    ON_EN_KILLFOCUS(IDC_H1R, &CScallopedPierLayoutDlg::OnPierLayoutChanged)
    ON_EN_KILLFOCUS(IDC_X1L, &CScallopedPierLayoutDlg::OnPierLayoutChanged)
    ON_EN_KILLFOCUS(IDC_X1R, &CScallopedPierLayoutDlg::OnPierLayoutChanged)
    ON_EN_KILLFOCUS(IDC_OHL, &CScallopedPierLayoutDlg::OnPierLayoutChanged)
    ON_EN_KILLFOCUS(IDC_OHR, &CScallopedPierLayoutDlg::OnPierLayoutChanged)
    ON_EN_KILLFOCUS(IDC_REFCOLUMN_OFFSET, &CScallopedPierLayoutDlg::OnPierLayoutChanged)
    ON_EN_KILLFOCUS(IDC_RL, &CScallopedPierLayoutDlg::OnPierLayoutChanged)
    ON_EN_KILLFOCUS(IDC_RR, &CScallopedPierLayoutDlg::OnPierLayoutChanged)
    ON_CBN_SELCHANGE(IDC_REFCOLUMN, &CScallopedPierLayoutDlg::OnPierLayoutChanged)
    ON_CBN_SELCHANGE(IDC_REFCOLUMN_MEASUREMENT, &CScallopedPierLayoutDlg::OnPierLayoutChanged)
    ON_CBN_SELCHANGE(IDC_FIXITY, &CScallopedPierLayoutDlg::OnPierLayoutChanged)

    ON_CBN_SELCHANGE(IDC_HEIGHT_MEASURE, OnHeightMeasureChanged)
    ON_BN_CLICKED(IDC_ADD_COLUMN, &CScallopedPierLayoutDlg::OnAddColumn)
    ON_BN_CLICKED(IDC_REMOVE_COLUMN, &CScallopedPierLayoutDlg::OnRemoveColumns)


END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScallopedPierLayoutDlg message handlers

BOOL CScallopedPierLayoutDlg::OnInitDialog()
{

    m_ColumnLayoutGrid.SubclassDlgItem(IDC_COLUMN_GRID, this);
    m_ColumnLayoutGrid.CustomInit();

    m_Pier.GetRefColumnLocation(&m_TransverseOffsetMeasurement, &m_RefColumnIdx, &m_TransverseOffset);

    m_XBeamOverhang[pgsTypes::stLeft] = m_Pier.GetOHL();
    m_XBeamOverhang[pgsTypes::stRight] = m_Pier.GetOHR();

    m_XBeamOverhangRadius[pgsTypes::stLeft] = m_Pier.GetXBeamOverhangRadius(pgsTypes::stLeft);
    m_XBeamOverhangRadius[pgsTypes::stRight] = m_Pier.GetXBeamOverhangRadius(pgsTypes::stRight);
    std::vector<CPierPointData> pvpp;
    m_Pier.GetLowerXBeamDimensions(&m_XBeamHeight[pgsTypes::stLeft], &m_XBeamHeight[pgsTypes::stRight], &m_XBeamTaperHeight[pgsTypes::stLeft],
        &m_XBeamTaperHeight[pgsTypes::stRight], &m_XBeamEndSlopeOffset[pgsTypes::stLeft], &m_XBeamEndSlopeOffset[pgsTypes::stRight],
        &m_XBeamTaperLength[pgsTypes::stLeft], &m_XBeamTaperLength[pgsTypes::stRight], &m_XBeamWidth, &pvpp);

    m_ColumnFixity = m_Pier.GetColumnFixity();

    FillTransverseLocationComboBox();
    FillRefColumnComboBox(m_Pier.GetColumnCount());
    FillHeightMeasureComboBox();

    CDialog::OnInitDialog();

    OnHeightMeasureChanged();
    m_Initialized = true;


    return TRUE;
}

void CScallopedPierLayoutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_FIXITY, m_cbColumnFixity);

    auto pBroker = EAFGetBroker();
    GET_IFACE2(pBroker, IEAFDisplayUnits, pDisplayUnits);

    DDX_MetaFileStatic(pDX, IDC_PIER_LAYOUT_GUIDE, m_LayoutPicture,_T("SCALLOPEDPIERLAYOUT"), _T("Metafile") );

    DDX_UnitValueAndTag(pDX, IDC_H1L, IDC_H1L_UNIT, m_XBeamHeight[pgsTypes::stLeft], pDisplayUnits->GetSpanLengthUnit());
    DDX_UnitValueAndTag(pDX, IDC_X1L, IDC_X1L_UNIT, m_XBeamEndSlopeOffset[pgsTypes::stLeft], pDisplayUnits->GetSpanLengthUnit());

    DDX_UnitValueAndTag(pDX, IDC_H1R, IDC_H1R_UNIT, m_XBeamHeight[pgsTypes::stRight], pDisplayUnits->GetSpanLengthUnit());
    DDX_UnitValueAndTag(pDX, IDC_X1R, IDC_X1R_UNIT, m_XBeamEndSlopeOffset[pgsTypes::stRight], pDisplayUnits->GetSpanLengthUnit());

    DDX_UnitValueAndTag(pDX, IDC_OHL, IDC_OHL_UNIT, m_XBeamOverhang[pgsTypes::stLeft], pDisplayUnits->GetSpanLengthUnit());
    DDX_UnitValueAndTag(pDX, IDC_OHR, IDC_OHR_UNIT, m_XBeamOverhang[pgsTypes::stRight], pDisplayUnits->GetSpanLengthUnit());

    DDX_UnitValueAndTag(pDX, IDC_W, IDC_W_UNIT, m_XBeamWidth, pDisplayUnits->GetSpanLengthUnit());
    DDX_UnitValueAndTag(pDX, IDC_RL, IDC_RL_UNIT, m_XBeamOverhangRadius[pgsTypes::stLeft], pDisplayUnits->GetSpanLengthUnit());
    DDX_UnitValueAndTag(pDX, IDC_RR, IDC_RR_UNIT, m_XBeamOverhangRadius[pgsTypes::stRight], pDisplayUnits->GetSpanLengthUnit());

    DDX_CBIndex(pDX, IDC_REFCOLUMN, m_RefColumnIdx);
    DDX_OffsetAndTag(pDX, IDC_REFCOLUMN_OFFSET, IDC_REFCOLUMN_OFFSET_UNIT, m_TransverseOffset, pDisplayUnits->GetSpanLengthUnit());
    DDX_CBItemData(pDX, IDC_REFCOLUMN_MEASUREMENT, m_TransverseOffsetMeasurement);

    DDX_CBItemData(pDX, IDC_FIXITY, m_ColumnFixity);

    CColumnLayoutGrid::DDV_ColumnGrid(pDX, m_ColumnLayoutGrid);
    CColumnLayoutGrid::DDX_ColumnGrid(pDX, m_ColumnLayoutGrid, &m_Pier);

    m_ColumnHeightMeasurementType = m_Pier.GetColumnData(0).GetColumnHeightMeasurementType();
    DDX_CBItemData(pDX, IDC_HEIGHT_MEASURE, m_ColumnHeightMeasurementType);

    if (pDX->m_bSaveAndValidate)
    {
        // XBeam width, W, must be greater than zero
        DDV_UnitValueGreaterThanZero(pDX, IDC_W, m_XBeamWidth, pDisplayUnits->GetSpanLengthUnit());
        DDV_UnitValueGreaterThanZero(pDX, IDC_RL, m_XBeamOverhangRadius[pgsTypes::stLeft], pDisplayUnits->GetSpanLengthUnit());
        DDV_UnitValueGreaterThanZero(pDX, IDC_RR, m_XBeamOverhangRadius[pgsTypes::stRight], pDisplayUnits->GetSpanLengthUnit());

        // H1 and H3 must be > 0
        DDV_UnitValueGreaterThanZero(pDX, IDC_H1L, m_XBeamHeight[pgsTypes::stLeft], pDisplayUnits->GetSpanLengthUnit());
        DDV_UnitValueGreaterThanZero(pDX, IDC_H1R, m_XBeamHeight[pgsTypes::stRight], pDisplayUnits->GetSpanLengthUnit());

        // X1..X4 must be >= 0
        DDV_UnitValueZeroOrMore(pDX, IDC_X1L, m_XBeamEndSlopeOffset[pgsTypes::stLeft], pDisplayUnits->GetSpanLengthUnit());
        DDV_UnitValueZeroOrMore(pDX, IDC_X1R, m_XBeamEndSlopeOffset[pgsTypes::stRight], pDisplayUnits->GetSpanLengthUnit());

        DDV_UnitValueZeroOrMore(pDX, IDC_OHL, m_XBeamOverhang[pgsTypes::stLeft], pDisplayUnits->GetSpanLengthUnit());
        DDV_UnitValueZeroOrMore(pDX, IDC_OHR, m_XBeamOverhang[pgsTypes::stRight], pDisplayUnits->GetSpanLengthUnit());

        ValidateScallopGeometry(pDX);

    }
}

void CScallopedPierLayoutDlg::ValidateScallopGeometry(CDataExchange* pDX)
{
    using xbrScallop::Point;
    for (Float64 value : {m_XBeamWidth, m_XBeamHeight[0], m_XBeamHeight[1],
        m_XBeamEndSlopeOffset[0], m_XBeamEndSlopeOffset[1], m_XBeamOverhang[0],
        m_XBeamOverhang[1], m_XBeamOverhangRadius[0], m_XBeamOverhangRadius[1], m_TransverseOffset})
    {
        if (!std::isfinite(value))
        {
            pDX->PrepareCtrl(IDC_W);
            AfxMessageBox(_T("Cross beam dimensions and reference offset must be finite numbers."));
            pDX->Fail();
        }
    }
    const ColumnIndexType count = m_Pier.GetColumnCount();
    if (count == 0 || m_RefColumnIdx >= count)
    {
        pDX->PrepareCtrl(IDC_REFCOLUMN);
        AfxMessageBox(_T("Select a valid reference column."));
        pDX->Fail();
    }
    std::vector<Float64> stations(count, m_XBeamOverhang[pgsTypes::stLeft]);
    for (ColumnIndexType i = 1; i < count; ++i)
        stations[i] = stations[i - 1] + m_Pier.GetColumnSpacing(i - 1);
    const Float64 length = stations.back() + m_XBeamOverhang[pgsTypes::stRight];
    const Float64 xLeft = m_XBeamEndSlopeOffset[pgsTypes::stLeft];
    const Float64 xRight = length - m_XBeamEndSlopeOffset[pgsTypes::stRight];
    if (xLeft > stations.front() || xRight < stations.back())
    {
        pDX->PrepareCtrl(xLeft > stations.front() ? IDC_X1L : IDC_X1R);
        AfxMessageBox(_T("The end slope offset cannot exceed its overhang length."));
        pDX->Fail();
    }

    // Locate the cap relative to the alignment using the edited reference
    // column. Deck-profile coordinates are measured in the plane of the pier.
    Float64 refOffset = m_TransverseOffset;
    if (m_TransverseOffsetMeasurement == pgsTypes::omtBridge)
        refOffset += m_Pier.GetBridgeLineOffset();
    Float64 pierLeft = refOffset - stations[m_RefColumnIdx];

    WBFL::Math::PiecewiseFunction deck;
    std::vector<Point> deckPoints;
    if (m_Pier.GetDeckSurfaceType() == xbrPierData::Profile)
    {
        CComPtr<IPoint2dCollection> profile;
        m_Pier.GetDeckProfile(&profile);
        IndexType nPoints;
        profile->get_Count(&nPoints);
        for (IndexType i = 0; i < nPoints; ++i)
        {
            CComPtr<IPoint2d> point;
            profile->get_Item(i, &point);
            Float64 x, y;
            point->Location(&x, &y);
            deck.AddPoint(x, y);
            deckPoints.push_back({x, y});
        }
    }
    else
    {
        CComPtr<IAngle> angle;
        angle.CoCreateInstance(CLSID_Angle);
        angle->FromString(CComBSTR(m_Pier.GetSkew()));
        Float64 skew;
        angle->get_Value(&skew);
        const Float64 c = std::cos(skew);
        Float64 sl, sr;
        m_Pier.GetCrownSlope(&sl, &sr);
        const Float64 cpo = m_Pier.GetCrownPointOffset();
        const Float64 elevation = m_Pier.GetDeckElevation() + (cpo > 0 ? cpo * sr : -sl * cpo);
        // Extend both sides beyond the cap so a crown outside the cap
        // still selects the correct deck slope.
        const Float64 crown = cpo / c;
        const Float64 xl = Min(pierLeft, crown) - length - 1.0;
        const Float64 xr = Max(pierLeft + length, crown) + length + 1.0;
        deckPoints = {{xl, elevation - sl * (xl * c - cpo)},
                      {crown, elevation},
                      {xr, elevation + sr * (xr * c - cpo)}};
        for (const auto& point : deckPoints)
            deck.AddPoint(point.x, point.y);
    }

    // A constant deck/diaphragm offset cancels in the clearance check.
    std::vector<Point> top = {{0.0, deck.Evaluate(pierLeft)}};
    for (const auto& point : deckPoints)
    {
        const Float64 x = point.x - pierLeft;
        if (x > 0 && x < length)
            top.push_back({x, point.y});
    }
    top.push_back({length, deck.Evaluate(pierLeft + length)});

    std::vector<Point> ends = {{xLeft, top.front().y - m_XBeamHeight[pgsTypes::stLeft]}};
    for (ColumnIndexType i = 0; i < count; ++i)
        ends.push_back({stations[i], deck.Evaluate(pierLeft + stations[i]) - m_Pier.GetColumnScallopDepth(i)});
    ends.push_back({xRight, top.back().y - m_XBeamHeight[pgsTypes::stRight]});

    for (ColumnIndexType arcIdx = 0; arcIdx <= count; ++arcIdx)
    {
        const Float64 radius = arcIdx == 0 ? m_XBeamOverhangRadius[pgsTypes::stLeft]
            : arcIdx == count ? m_XBeamOverhangRadius[pgsTypes::stRight]
            : m_Pier.GetColumnScallopRadius(arcIdx - 1);
        const auto error = xbrScallop::ValidateArc(ends[arcIdx], ends[arcIdx + 1], radius, top);
        if (error == xbrScallop::ArcError::None)
            continue;
        if (error == xbrScallop::ArcError::AboveTop)
        {
            const ColumnIndexType column = arcIdx == 0 ? 0 : arcIdx == count ? count - 1
                : m_Pier.GetColumnScallopDepth(arcIdx - 1) <= m_Pier.GetColumnScallopDepth(arcIdx) ? arcIdx - 1 : arcIdx;
            pDX->PrepareCtrl(IDC_COLUMN_GRID);
            m_ColumnLayoutGrid.SetCurrentCell(column + 1, 8);
        }
        else
        {
            pDX->PrepareCtrl(arcIdx == 0 ? IDC_RL : arcIdx == count ? IDC_RR : IDC_COLUMN_GRID);
            if (arcIdx > 0 && arcIdx < count)
                m_ColumnLayoutGrid.SetCurrentCell(arcIdx, 7);
        }
        CString location, message;
        if (arcIdx == 0) location = _T("Left overhang");
        else if (arcIdx == count) location = _T("Right overhang");
        else location.Format(_T("Bay between columns %u and %u"), arcIdx, arcIdx + 1);
        message.Format(_T("%s: %s"), location.GetString(),
            error == xbrScallop::ArcError::InvalidChord
                ? _T("Radius must be at least half the distance between arc endpoints, including their elevation difference.")
                : error == xbrScallop::ArcError::AboveTop
                    ? _T("Arc extends above the lower cross beam top. Increase the column depth or radius.")
                    : _T("Arc turns back past an endpoint. Increase the radius or adjust the endpoint depths."));
        AfxMessageBox(message);
        pDX->Fail();
    }
}

void CScallopedPierLayoutDlg::FillTransverseLocationComboBox()
{
    CComboBox* pcbMeasure = (CComboBox*)GetDlgItem(IDC_REFCOLUMN_MEASUREMENT);
    pcbMeasure->ResetContent();
    int idx = pcbMeasure->AddString(_T("from the Alignment"));
    pcbMeasure->SetItemData(idx, (DWORD_PTR)pgsTypes::omtAlignment);
    idx = pcbMeasure->AddString(_T("from the Bridgeline"));
    pcbMeasure->SetItemData(idx, (DWORD_PTR)pgsTypes::omtBridge);
}

void CScallopedPierLayoutDlg::FillRefColumnComboBox(ColumnIndexType nColumns)
{
    CComboBox* pcbRefColumn = (CComboBox*)GetDlgItem(IDC_REFCOLUMN);
    int curSel = pcbRefColumn->GetCurSel();
    pcbRefColumn->ResetContent();
    if (nColumns == INVALID_INDEX)
    {
        nColumns = (ColumnIndexType)m_ColumnLayoutGrid.GetRowCount();
    }

    for (ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++)
    {
        CString strLabel;
        strLabel.Format(_T("Column %d"), LABEL_COLUMN(colIdx));
        pcbRefColumn->AddString(strLabel);
    }

    if (pcbRefColumn->SetCurSel(curSel) == CB_ERR)
    {
        if (m_RefColumnIdx < nColumns)
        {
            pcbRefColumn->SetCurSel(m_RefColumnIdx);
        }
        else
        {
            pcbRefColumn->SetCurSel(0);
        }
    }
}

void CScallopedPierLayoutDlg::FillHeightMeasureComboBox()
{
    CComboBox* pcbHeightMeasure = (CComboBox*)GetDlgItem(IDC_HEIGHT_MEASURE);
    pcbHeightMeasure->ResetContent();
    int idx = pcbHeightMeasure->AddString(_T("Column Height (H)"));
    pcbHeightMeasure->SetItemData(idx, (DWORD_PTR)CColumnData::chtHeight);
    idx = pcbHeightMeasure->AddString(_T("Bottom Elevation"));
    pcbHeightMeasure->SetItemData(idx, (DWORD_PTR)CColumnData::chtBottomElevation);
}

void CScallopedPierLayoutDlg::OnHeightMeasureChanged()
{
    CComboBox* pcbHeightMeasure = (CComboBox*)GetDlgItem(IDC_HEIGHT_MEASURE);
    int curSel = pcbHeightMeasure->GetCurSel();
    CColumnData::ColumnHeightMeasurementType measure = (CColumnData::ColumnHeightMeasurementType)(pcbHeightMeasure->GetItemData(curSel));
    m_ColumnLayoutGrid.SetHeightMeasurementType(measure);
    OnPierLayoutChanged();
}

LRESULT CScallopedPierLayoutDlg::OnColumnGridCellChanged(WPARAM, LPARAM)
{
    FillRefColumnComboBox();
    OnPierLayoutChanged();
    return 0;
}

void CScallopedPierLayoutDlg::OnAddColumn()
{
    m_ColumnLayoutGrid.AddColumn();
    FillRefColumnComboBox();

    // Update pier data with current column data
    m_ColumnLayoutGrid.GetColumnData(m_Pier);

    OnPierLayoutChanged();
}

void CScallopedPierLayoutDlg::OnRemoveColumns()
{
    m_ColumnLayoutGrid.RemoveSelectedColumns();

    FillRefColumnComboBox();

    // Update pier data with current column data
	const auto nCols = m_Pier.GetColumnCount();
	if (nCols > 1)
    {
        m_ColumnLayoutGrid.GetColumnData(m_Pier);
    }
    OnPierLayoutChanged();
}

void CScallopedPierLayoutDlg::SetPierModelType(const pgsTypes::PierModelType& pierModelType)
{
    m_PierModelType = pierModelType;
}

void CScallopedPierLayoutDlg::SetPierData(const xbrPierData& pierData)
{
    m_Pier = pierData;
    m_Pier.SetPierLayoutType(pgsTypes::pltScalloped);
}

const xbrPierData* CScallopedPierLayoutDlg::GetPierData() const
{
    return &m_Pier;
}

void CScallopedPierLayoutDlg::OnPierLayoutChanged()
{
    if (!m_Initialized || m_UpdatingLayout)
        return;
    m_UpdatingLayout = true;
    const xbrPierData previous = m_Pier;
    if (UpdateData(TRUE))
    {
        m_Pier.SetRefColumnLocation(m_TransverseOffsetMeasurement, m_RefColumnIdx, m_TransverseOffset);
        m_Pier.SetLowerXBeamDimensions(m_XBeamHeight[0], m_XBeamHeight[1],
            m_XBeamTaperHeight[0], m_XBeamTaperHeight[1],
            m_XBeamEndSlopeOffset[0], m_XBeamEndSlopeOffset[1],
            m_XBeamTaperLength[0], m_XBeamTaperLength[1], m_XBeamWidth, m_Pier.GetPierPointData());
        m_Pier.SetXBeamOverhangs(m_XBeamOverhang[0], m_XBeamOverhang[1]);
        m_Pier.SetColumnFixity(m_ColumnFixity);
        m_Pier.SetXBeamOverhangRadius(pgsTypes::stLeft, m_XBeamOverhangRadius[0]);
        m_Pier.SetXBeamOverhangRadius(pgsTypes::stRight, m_XBeamOverhangRadius[1]);
        for (ColumnIndexType idx = 0; idx < m_Pier.GetColumnCount(); ++idx)
            m_Pier.GetColumnData(idx).SetColumnHeightMeasurementType(m_ColumnHeightMeasurementType);
        GetParent()->PostMessage(WM_PIER_LAYOUT_CHANGED);
    }
    else
    {
        m_Pier = previous;
    }
    m_UpdatingLayout = false;
}
