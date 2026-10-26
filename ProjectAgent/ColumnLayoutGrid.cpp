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


#include "stdafx.h"
#include "ProjectAgent.h"
#include "ColumnLayoutGrid.h"
#include <EAF\EAFDisplayUnits.h>
#include <PGSuperUIUtil.h>
#include <cmath>


GRID_IMPLEMENT_REGISTER(CColumnLayoutGrid, CS_DBLCLKS, 0, 0, 0);

/////////////////////////////////////////////////////////////////////////////
// CColumnLayoutGrid

CColumnLayoutGrid::CColumnLayoutGrid()
{
//   RegisterClass();
}

CColumnLayoutGrid::~CColumnLayoutGrid()
{
}

BEGIN_MESSAGE_MAP(CColumnLayoutGrid, CGXGridWnd)
	//{{AFX_MSG_MAP(CColumnLayoutGrid)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColumnLayoutGrid message handlers

void CColumnLayoutGrid::CustomInit()
{
// Initialize the grid. For CWnd based grids this call is // 
// essential. For view based grids this initialization is done 
// in OnInitialUpdate.
	Initialize( );

	GetParam( )->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   SetMergeCellsMode(gxnMergeDelayEval);

   const int num_rows = 0;
   const int num_cols = m_Scalloped ? 8 : 6;

	SetRowCount(num_rows);
	SetColCount(num_cols);

		// Turn off selecting whole columns when clicking on a column header
	GetParam()->EnableSelection((WORD) (GX_SELFULL & ~GX_SELCOL & ~GX_SELTABLE & ~GX_SELMULTIPLE & ~GX_SELSHIFT & ~GX_SELKEYBOARD));

   // no row moving
	GetParam()->EnableMoveRows(FALSE);

   // disable left side
	SetStyleRange(CGXRange(0,0,num_rows,0), CGXStyle()
			.SetControl(GX_IDS_CTRL_HEADER)
			.SetEnabled(FALSE)          // disables usage as current cell
		);


    auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

// set text along top row
   int col = 0;

	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(_T("Col"))
		);

   CString cv;
   cv.Format(_T("Height\n(%s)"),pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetValue(cv)
		);

	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(_T("Fixity"))
		);

	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(_T("Shape"))
		);

   cv.Format(_T("Diameter\nWidth\n(%s)"),pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   cv.Format(_T(" \nDepth\n(%s)"),pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   cv.Format(_T("S\nSpacing\n(%s)"),pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);


   if (m_Scalloped)
   {
      cv.Format(_T("R\nBay Radius\n(%s)"), pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure.UnitTag().c_str());
      SetStyleRange(CGXRange(0, 7), CGXStyle().SetWrapText(TRUE).SetEnabled(FALSE)
         .SetHorizontalAlignment(DT_CENTER).SetVerticalAlignment(DT_VCENTER).SetValue(cv));
      cv.Format(_T("D\nScallop Depth\n(%s)"), pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure.UnitTag().c_str());
      SetStyleRange(CGXRange(0, 8), CGXStyle().SetWrapText(TRUE).SetEnabled(FALSE)
         .SetHorizontalAlignment(DT_CENTER).SetVerticalAlignment(DT_VCENTER).SetValue(cv));
   }

   // make it so that text fits correctly in header row
	ResizeRowHeightsToFit(CGXRange(0,0,0,num_cols));

   // don't allow users to resize grids
   GetParam( )->EnableTrackColWidth(0); 
   GetParam( )->EnableTrackRowHeight(0); 

	EnableIntelliMouse();
	SetFocus();

   GetParam()->SetLockReadOnly(TRUE);
	GetParam( )->EnableUndo(TRUE);
}

void CColumnLayoutGrid::SetHeightMeasurementType(CColumnData::ColumnHeightMeasurementType measure)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   CString cv;
   cv.Format(_T("%s\n(%s)"),(measure == CColumnData::chtHeight ? _T("Height") : _T("Bottom\nElev")),pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,1), CGXStyle()
         .SetWrapText(TRUE)
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetValue(cv)
		);
}

void CColumnLayoutGrid::AddColumn()
{
   ROWCOL nRows = GetRowCount();

   // Capture displayed values before inserting a row. The final radius cell
   // is unused, so inherit the radius from the last existing bay.
   Float64 scallopDepth = 0.0;
   Float64 scallopRadius = 0.0;
   if (m_Scalloped)
   {
      auto pBroker = EAFGetBroker();
      GET_IFACE2(pBroker, IEAFDisplayUnits, pDisplayUnits);
      const auto unit = pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure;
      scallopDepth = nRows > 0 ? _tstof(GetCellValue(nRows, 8))
         : WBFL::Units::ConvertFromSysUnits(
            WBFL::Units::ConvertToSysUnits(5.0, WBFL::Units::Measure::Feet), unit);
      scallopRadius = nRows > 1 ? _tstof(GetCellValue(nRows - 1, 7))
         : WBFL::Units::ConvertFromSysUnits(
            WBFL::Units::ConvertToSysUnits(10.0, WBFL::Units::Measure::Feet), unit);
   }

   CColumnData columnData;
   Float64 S;

   if ( 0 < nRows )
   {
      GetColumnData(nRows,&columnData,&S); // get column data from the last row
   }

   S = WBFL::Units::ConvertToSysUnits(10.0,WBFL::Units::Measure::Feet); // default spacing
   if ( 1 < nRows )
   {
      // get the spacing between the last two columns
      CColumnData columnData1;
      Float64 S1;
      GetColumnData(nRows-1,&columnData1,&S1);
      S = S1;
   }

	GetParam( )->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   if ( nRows == 0 )
   {
      AddColumn(columnData,0);
   }
   else
   {
      // set the default spacing in the last row
      SetColumnData(nRows,columnData,S);
      // add the new column (adds row to the grid)
      AddColumn(columnData,0);
   }

   if (m_Scalloped)
   {
      if (nRows > 0)
         SetGeometryCell(nRows, 7, scallopRadius);
      SetGeometryCell(nRows + 1, 8, scallopDepth);
      UpdateLastRowStyles();
      ResizeColWidthsToFit(CGXRange(0, 0, GetRowCount(), GetColCount()));
   }

   GetParam()->SetLockReadOnly(TRUE);
   GetParam( )->EnableUndo(TRUE);
}

void CColumnLayoutGrid::RemoveSelectedColumns()
{
	GetParam( )->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   CDWordArray selRows;
   ROWCOL nSelRows = GetSelectedRows(selRows);
   for ( int r = nSelRows-1; r >= 0; r-- )
   {
      ROWCOL selRow = selRows[r];
      if (GetRowCount() > 1)
         RemoveRows(selRow,selRow);
   }

   ROWCOL nRows = GetRowCount();
   if ( 0 < nRows )
   {
      SetStyleRange(CGXRange(nRows,6), CGXStyle()
         .SetEnabled(FALSE)
         .SetReadOnly(TRUE)
         .SetInterior(::GetSysColor(COLOR_BTNFACE))
         .SetTextColor(::GetSysColor(COLOR_BTNFACE))
         .SetHorizontalAlignment(DT_RIGHT)
            );
   }

   UpdateLastRowStyles();

   GetParam()->SetLockReadOnly(TRUE);
	GetParam( )->EnableUndo(TRUE);
}

void CColumnLayoutGrid::GetColumnData(xbrPierData& pier)
{
   ROWCOL nRows = GetRowCount();
   if (nRows == 0)
      return;
   pier.SetColumnCount((ColumnIndexType)nRows);
   for ( ROWCOL row = 1; row <= nRows; row++ )
   {
      ColumnIndexType colIdx = (ColumnIndexType)(row-1);
      CColumnData column;
      Float64 S;
      GetColumnData(row,&column,&S);
      pier.SetColumnData(colIdx,column);
      if (m_Scalloped)
      {
         auto pBroker = EAFGetBroker();
         GET_IFACE2(pBroker, IEAFDisplayUnits, pDisplayUnits);
         const auto unit = pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure;
         pier.SetColumnScallopDepth(colIdx, WBFL::Units::ConvertToSysUnits(
            _tstof(GetCellValue(row, 8)), unit));
         if (row < nRows)
            pier.SetColumnScallopRadius(colIdx, WBFL::Units::ConvertToSysUnits(
               _tstof(GetCellValue(row, 7)), unit));
      }
      if ( row < nRows )
      {
         pier.SetColumnSpacing(colIdx,S);
      }
   }
}

void CColumnLayoutGrid::SetColumnData(const xbrPierData& pier)
{
   GetParam()->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   ROWCOL nRows = GetRowCount();
   if ( 0 < nRows )
   {
      RemoveRows(1,nRows);
   }

   if ( pier.GetColumnCount() == 0 )
   {
      // Always have one column
      CColumnData column;
      AddColumn(column,0);
   }
   else
   {
      IndexType nColumns = pier.GetColumnCount();
      for ( IndexType colIdx = 0; colIdx < nColumns; colIdx++ )
      {
         const CColumnData& column = pier.GetColumnData(colIdx);
         Float64 S = (colIdx != nColumns-1 ? pier.GetColumnSpacing(colIdx) : 0);
         AddColumn(column,S);
      }
   }

   if (m_Scalloped)
   {
      auto pBroker = EAFGetBroker();
      GET_IFACE2(pBroker, IEAFDisplayUnits, pDisplayUnits);
      const auto unit = pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure;
      for (ROWCOL row = 1; row <= GetRowCount(); ++row)
      {
         SetGeometryCell(row, 8, WBFL::Units::ConvertFromSysUnits(
            pier.GetColumnScallopDepth(row - 1), unit));
         if (row < GetRowCount())
            SetGeometryCell(row, 7, WBFL::Units::ConvertFromSysUnits(
               pier.GetColumnScallopRadius(row - 1), unit));
      }
      UpdateLastRowStyles();
   }

   ResizeColWidthsToFit(CGXRange(0,0,GetRowCount(),GetColCount()));

   GetParam()->SetLockReadOnly(TRUE);
   GetParam()->EnableUndo(TRUE);
}

void CColumnLayoutGrid::SetGeometryCell(ROWCOL row, ROWCOL col, Float64 value)
{
   SetStyleRange(CGXRange(row, col), CGXStyle().SetEnabled(TRUE).SetReadOnly(FALSE)
      .SetInterior(::GetSysColor(COLOR_WINDOW)).SetTextColor(::GetSysColor(COLOR_WINDOWTEXT))
      .SetHorizontalAlignment(DT_RIGHT).SetValue(value));
}

void CColumnLayoutGrid::UpdateLastRowStyles()
{
   const ROWCOL nRows = GetRowCount();
   if (nRows == 0)
      return;
   for (ROWCOL row = 1; row <= nRows; ++row)
   {
      for (ROWCOL col : {ROWCOL(6), ROWCOL(7)})
      {
         if (col == 7 && !m_Scalloped)
            continue;
         const BOOL enabled = row < nRows;
         SetStyleRange(CGXRange(row, col), CGXStyle().SetEnabled(enabled).SetReadOnly(!enabled)
            .SetInterior(::GetSysColor(enabled ? COLOR_WINDOW : COLOR_BTNFACE))
            .SetTextColor(::GetSysColor(enabled ? COLOR_WINDOWTEXT : COLOR_BTNFACE)));
      }
   }
}

void CColumnLayoutGrid::SetColumnData(ROWCOL row,const CColumnData& column,Float64 S)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   ROWCOL col = 1;

   // Height/Bottom Elevation
   Float64 value = WBFL::Units::ConvertFromSysUnits(column.GetColumnHeight(),pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);
   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );

   // Fixity
   CString strChoiceList = CString(CFCB_TBFIX_STR) + CString(_T("\n")) + CString(CFCB_TFIX_BPIN_STR) + CString(_T("\n")) + CString(CFCB_TPIN_BFIX_STR);

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
		.SetControl(GX_IDS_CTRL_CBS_DROPDOWNLIST)
		.SetChoiceList( strChoiceList )
      .SetValue( GetTransverseFixityString(column.GetTransverseFixity()))
      .SetHorizontalAlignment(DT_RIGHT)
      );

   // Section
	SetStyleRange(CGXRange(row,col++), CGXStyle()
		.SetControl(GX_IDS_CTRL_CBS_DROPDOWNLIST)
		.SetChoiceList(_T("Circle\nRectangle"))
      .SetValue(column.GetColumnShape() == CColumnData::cstCircle ? _T("Circle") : _T("Rectangle"))
      .SetHorizontalAlignment(DT_RIGHT)
		);

   Float64 D1, D2;
   column.GetColumnDimensions(&D1,&D2);
   value = WBFL::Units::ConvertFromSysUnits(D1,pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);
   SetStyleRange(CGXRange(row,col++),CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );

   value = WBFL::Units::ConvertFromSysUnits(D2,pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);
   SetStyleRange(CGXRange(row,col++),CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );
   if ( column.GetColumnShape() == CColumnData::cstCircle )
   {
      SetStyleRange(CGXRange(row,col-1),CGXStyle()
         .SetEnabled(FALSE)
         .SetReadOnly(TRUE)
         .SetInterior(::GetSysColor(COLOR_BTNFACE))
         .SetTextColor(::GetSysColor(COLOR_BTNFACE))
         .SetHorizontalAlignment(DT_RIGHT)
         );
   }


   // Spacing
   // Set the value for the spacing to the next column and disable this cell
   // This assumes this column is in the last row and spacing to next isn't applicable
   value = WBFL::Units::ConvertFromSysUnits(S,pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);
   SetStyleRange(CGXRange(row,col), CGXStyle()
      .SetEnabled(FALSE)
      .SetReadOnly(TRUE)
      .SetInterior(::GetSysColor(COLOR_BTNFACE))
      .SetTextColor(::GetSysColor(COLOR_BTNFACE))
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
         );

   if ( 1 < row )
   {
      // if this isn't the first row, enable the spacing in the previous row
      // because it obviously isn't the last row
      SetStyleRange(CGXRange(row-1,col),CGXStyle()
         .SetEnabled(TRUE)
         .SetReadOnly(FALSE)
         .SetInterior(::GetSysColor(COLOR_WINDOW))
        .SetTextColor(::GetSysColor(COLOR_WINDOWTEXT))
         );
   }

   col++;
}

void CColumnLayoutGrid::AddColumn(const CColumnData& column,Float64 S)
{
   InsertRows(GetRowCount()+1,1);
   ROWCOL row = GetRowCount();
   SetColumnData(row,column,S);
}

void CColumnLayoutGrid::GetColumnData(ROWCOL row,CColumnData* pColumn,Float64* pS)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   ROWCOL col = 1;

   Float64 H = _tstof(GetCellValue(row,col++));
   H = WBFL::Units::ConvertToSysUnits(H,pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);
   pColumn->SetColumnHeight(H);

   // Fixity
   CString strFixity = GetCellValue(row,col++);
   pgsTypes::ColumnTransverseFixityType fixity = GetTransverseFixityTypeFromString(strFixity);
   pColumn->SetTransverseFixity(fixity);

   // Shape Details
   CString strShape = GetCellValue(row,col++);
   if ( strShape == _T("Circle") )
   {
      pColumn->SetColumnShape(CColumnData::cstCircle);
   }
   else
   {
      pColumn->SetColumnShape(CColumnData::cstRectangle);
   }

   Float64 D1 = _tstof(GetCellValue(row,col++));
   D1 = WBFL::Units::ConvertToSysUnits(D1,pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);
   Float64 D2 = _tstof(GetCellValue(row,col++));
   D2 = WBFL::Units::ConvertToSysUnits(D2,pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);
   pColumn->SetColumnDimensions(D1,D2);
   
   // Spacing
   Float64 S = _tstof(GetCellValue(row,col++));
   *pS = WBFL::Units::ConvertToSysUnits(S,pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);
}

CString CColumnLayoutGrid::GetCellValue(ROWCOL nRow, ROWCOL nCol)
{
    if (IsCurrentCell(nRow, nCol) && IsActiveCurrentCell())
    {
        CString s;
        CGXControl* pControl = GetControl(nRow, nCol);
        pControl->GetValue(s);
        return s;
    }
    else
    {
        return GetValueRowCol(nRow, nCol);
    }
}

void CColumnLayoutGrid::OnClickedButtonRowCol(ROWCOL nRow,ROWCOL nCol)
{
   if ( nCol != 3 )
   {
      return;
   }
}

void CColumnLayoutGrid::OnModifyCell(ROWCOL nRow,ROWCOL nCol)
{
   if ( nCol == 3 )
   {
      GetParam()->EnableUndo(FALSE);
      GetParam()->SetLockReadOnly(FALSE);

      CString str = GetCellValue(nRow,nCol);
      if ( str == _T("Rectangle") )
      {
         SetStyleRange(CGXRange(nRow,nCol+2),CGXStyle()
            .SetEnabled(TRUE)
            .SetReadOnly(FALSE)
            .SetInterior(::GetSysColor(COLOR_WINDOW))
            .SetTextColor(::GetSysColor(COLOR_WINDOWTEXT))
            .SetHorizontalAlignment(DT_RIGHT)
            );
      }
      else
      {
         SetStyleRange(CGXRange(nRow,nCol+2),CGXStyle()
            .SetEnabled(FALSE)
            .SetReadOnly(TRUE)
            .SetInterior(::GetSysColor(COLOR_BTNFACE))
            .SetTextColor(::GetSysColor(COLOR_BTNFACE))
            .SetHorizontalAlignment(DT_RIGHT)
            );
      }

      GetParam()->SetLockReadOnly(TRUE);
      GetParam()->EnableUndo(TRUE);
   }
   else
   {
      __super::OnModifyCell(nRow, nCol);
   }
}

void CColumnLayoutGrid::DDX_ColumnGrid(CDataExchange* pDX, CColumnLayoutGrid& grid, xbrPierData* pier)
{
    if (pDX->m_bSaveAndValidate)
    {
        grid.GetColumnData(*pier);
    }
    else
    {
        grid.SetColumnData(*pier);
    }
}

void CColumnLayoutGrid::DDV_ColumnGrid(CDataExchange* pDX, CColumnLayoutGrid& grid)
{
    if (pDX->m_bSaveAndValidate)
    {
        if (grid.GetRowCount() == 0)
        {
            AfxMessageBox(_T("The pier must have at least one column"), MB_OK | MB_ICONEXCLAMATION);
            pDX->Fail();
        }

        if (grid.m_Scalloped)
        {
            for (ROWCOL row = 1; row <= grid.GetRowCount(); ++row)
            {
                for (ROWCOL col : {ROWCOL(6), ROWCOL(7), ROWCOL(8)})
                {
                    if (row == grid.GetRowCount() && col != 8)
                        continue;
                    CString text = grid.GetCellValue(row, col);
                    text.Trim();
                    TCHAR* end = nullptr;
                    const Float64 value = _tcstod(text.GetString(), &end);
                    if (text.IsEmpty() || end == text.GetString() || *end != 0 ||
                        !std::isfinite(value) || value <= 0.0)
                    {
                        grid.SetCurrentCell(row, col);
                        pDX->PrepareCtrl(grid.GetDlgCtrlID());
                        CString message;
                        message.Format(_T("Column %u: %s must be a positive, finite number."), row,
                            col == 6 ? _T("spacing") : col == 7 ? _T("bay radius") : _T("scallop depth"));
                        AfxMessageBox(message);
                        pDX->Fail();
                    }
                }
            }
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

BOOL CColumnLayoutGrid::OnEndEditing(ROWCOL nRow, ROWCOL nCol)
{
    CGXControl* control = GetControl(nRow, nCol);
    const BOOL modified = control && control->GetModify();
    const BOOL accepted = CGXGridWnd::OnEndEditing(nRow, nCol);
    if (accepted && modified && GetParent())
        GetParent()->PostMessage(WM_COLUMN_GRID_CELL_CHANGED, (WPARAM)nRow, (LPARAM)nCol);
    return accepted;
}
