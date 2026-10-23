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

// StirrupGrid.cpp : implementation file
//
#include "stdafx.h"
#include <AgentTools.h>
#include "resource.h"
#include "StirrupGrid.h"
#include <XBeamRateExt\ReinforcementPage.h>
#include <EAF\EAFDisplayUnits.h>
#include <EAF\EAFUtilities.h>
#include <EAF\EAFApp.h>
#include <LRFD\RebarPool.h>


 GRID_IMPLEMENT_REGISTER(CStirrupGrid, CS_DBLCLKS, 0, 0, 0);

/////////////////////////////////////////////////////////////////////////////
// CStirrupGrid

CStirrupGrid::CStirrupGrid()
{
   RegisterClass();
}

CStirrupGrid::~CStirrupGrid()
{
}

BEGIN_MESSAGE_MAP(CStirrupGrid, CGXGridWnd)
	//{{AFX_MSG_MAP(CStirrupGrid)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CStirrupGrid message handlers

void CStirrupGrid::CustomInit(LPCTSTR lpszGridName)
{
   m_strGridName = lpszGridName;

// Initialize the grid. For CWnd based grids this call is 
// essential. For view based grids this initialization is done 
// in OnInitialUpdate.
	Initialize( );

	GetParam( )->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   const int num_rows = 0;
   const int num_cols = 4;

	SetRowCount(num_rows);
	SetColCount(num_cols);

		// Turn off selecting whole columns when clicking on a column header
	GetParam()->EnableSelection((WORD) (GX_SELFULL & ~GX_SELCOL & ~GX_SELTABLE & ~GX_SELMULTIPLE & ~GX_SELSHIFT & ~GX_SELKEYBOARD));

   // no row moving
	GetParam()->EnableMoveRows(FALSE);

    auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   // disable left side
	SetStyleRange(CGXRange(0,0,num_rows,0), CGXStyle()
			.SetControl(GX_IDS_CTRL_HEADER)
			.SetEnabled(FALSE)          // disables usage as current cell
		);

// set text along top row
	SetStyleRange(CGXRange(0,0), CGXStyle()
         .SetWrapText(TRUE)
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetEnabled(FALSE)          // disables usage as current cell
			.SetValue(_T("Zone\n#"))
		);

   CString cv;
   cv.Format(_T("Zone Length\n(%s)"),pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,1), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

	SetStyleRange(CGXRange(0,2), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(_T("Bar\nSize"))
		);

   cv.Format(_T("Spacing\n(%s)"),pDisplayUnits->GetComponentDimUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,3), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

	SetStyleRange(CGXRange(0,4), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
         .SetValue(_T("# of\nLegs"))
		);

   // make it so that text fits correctly in header row
	ResizeRowHeightsToFit(CGXRange(0,0,0,num_cols));

   // don't allow users to resize grids
   GetParam( )->EnableTrackColWidth(0); 
   GetParam( )->EnableTrackRowHeight(0); 

	EnableIntelliMouse();
	SetFocus();

   GetParam()->SetLockReadOnly(TRUE);
	GetParam( )->EnableUndo(TRUE);


   CReinforcementPage* pParent = (CReinforcementPage*)GetParent();
   WBFL::Materials::Rebar::Type type;
   WBFL::Materials::Rebar::Grade grade;
   pParent->GetRebarMaterial(&type,&grade);
   WBFL::LRFD::RebarIter rebarIter(type,grade,true/*stirrup only*/);
   for ( rebarIter.Begin(); rebarIter; rebarIter.Next() )
   {
      const auto* pRebar = rebarIter.GetCurrentRebar();
      m_strBarSizeChoiceList += pRebar->GetName().c_str();
      m_strBarSizeChoiceList += _T("\n");
   }
}

void CStirrupGrid::AddZone()
{
   xbrStirrupData::StirrupZone zoneData;

	GetParam()->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   AddZoneData(zoneData);

   ROWCOL nRows = GetRowCount();
   if ( 1 < nRows )
   {
      // there are 2 rows... 
      // before adding the length of the one and only row was "to center" or "to end"
      // now the second (last) row is "to center" or "to end" and the zone length of
      // the previous last row needs to be updated
	   SetStyleRange(CGXRange(nRows-1,1),CGXStyle()
         .SetEnabled(TRUE)
         .SetReadOnly(FALSE)
         .SetInterior(::GetSysColor(COLOR_WINDOW))
         .SetValue(0.0)
         );
   }

   UpdateLastZoneLength();
   ResizeColWidthsToFit(CGXRange(0,0,GetRowCount(),GetColCount()));

   GetParam()->SetLockReadOnly(TRUE);
	GetParam()->EnableUndo(TRUE);
}

BOOL CStirrupGrid::OnLButtonHitRowCol(ROWCOL nHitRow,ROWCOL nHitCol,ROWCOL nDragRow,ROWCOL nDragCol,CPoint point,UINT flags,WORD nHitState)
{
   if ( WBFL::System::Flags<WORD>::IsSet(nHitState,GX_HITEND) )
   {
      CReinforcementPage* pParent = (CReinforcementPage*)GetParent();

      ROWCOL nRows = GetRowCount();

      CDWordArray selRows;
      ROWCOL nSelRows = GetSelectedRows(selRows);
      if (nSelRows == 0)
      {
         return FALSE;
      }

      ROWCOL lastSelectedRow = selRows.GetAt(nSelRows-1);

      if (nDragRow != 0 && nRows != 0)
      {
         pParent->OnEnableDelete(GetDlgCtrlID(),true);
      }
      else
      {
         pParent->OnEnableDelete(GetDlgCtrlID(),false);
      }
   }

   return CGXGridWnd::OnLButtonHitRowCol(nHitRow,nHitCol,nDragRow,nDragCol,point,flags,nHitState);
}

BOOL CStirrupGrid::OnLButtonClickedRowCol(ROWCOL nRow, ROWCOL nCol, UINT nFlags, CPoint pt)
{
   CReinforcementPage* pParent = (CReinforcementPage*)GetParent();

   if (nCol == 0 && nRow != 0)
   {
      pParent->OnEnableDelete(GetDlgCtrlID(),true);
   }
   else
   {
      pParent->OnEnableDelete(GetDlgCtrlID(),false);
   }

   return TRUE;
}

void CStirrupGrid::RemoveSelectedZones()
{
	GetParam( )->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   ROWCOL lastRow = GetRowCount();
   bool bLastRowRemoved = false;

   CDWordArray selRows;
   ROWCOL nSelRows = GetSelectedRows(selRows);
   for ( int r = nSelRows-1; r >= 0; r-- )
   {
      ROWCOL selRow = selRows[r];
      RemoveRows(selRow,selRow);
      if ( selRow == lastRow )
      {
         bLastRowRemoved = true;
      }
   }

   if ( bLastRowRemoved )
   {
      UpdateLastZoneLength();
   }

   GetParam()->SetLockReadOnly(TRUE);
	GetParam( )->EnableUndo(TRUE);
}

bool CStirrupGrid::GetStirrupData(xbrStirrupData& stirrups)
{
   stirrups.Zones.clear();
   ROWCOL nRows = GetRowCount();
   for ( ROWCOL row = 1; row <= nRows; row++ )
   {
      xbrStirrupData::StirrupZone zoneData;
      if ( !GetZoneData(row,zoneData) )
      {
         return false;
      }
      stirrups.Zones.push_back(zoneData);
   }

   return true;
}

void CStirrupGrid::SetStirrupData(const xbrStirrupData& stirrups)
{
   GetParam()->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   ROWCOL nRows = GetRowCount();
   if ( 0 < nRows )
   {
      RemoveRows(1,nRows);
   }

   ROWCOL row = 1;
   for (const auto& zoneData : stirrups.Zones)
   {
      InsertRows(GetRowCount()+1,1);
      SetZoneData(row++,zoneData);
   }

   SetSymmetry(stirrups.Symmetric);

   GetParam()->SetLockReadOnly(TRUE);
   GetParam()->EnableUndo(TRUE);
}

void CStirrupGrid::SetZoneData(ROWCOL row,const xbrStirrupData::StirrupZone& zoneData)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   Float64 value = WBFL::Units::ConvertFromSysUnits(zoneData.Length,pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);
	SetStyleRange(CGXRange(row,1), CGXStyle()
			.SetUserAttribute(GX_IDS_UA_VALID_MIN, _T("0.0e01"))
			.SetUserAttribute(GX_IDS_UA_VALID_MAX, _T("1.0e99"))
			.SetUserAttribute(GX_IDS_UA_VALID_MSG, _T("Please enter a positive value"))
         .SetHorizontalAlignment(DT_RIGHT)
         .SetValue(value)
		);

   // Bar Size
	SetStyleRange(CGXRange(row,2), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
		.SetControl(GX_IDS_CTRL_CBS_DROPDOWNLIST)
		.SetChoiceList(m_strBarSizeChoiceList)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(WBFL::LRFD::RebarPool::GetBarSize(zoneData.BarSize).c_str())
      );

   value = WBFL::Units::ConvertFromSysUnits(zoneData.BarSpacing,pDisplayUnits->GetComponentDimUnit().UnitOfMeasure);
	SetStyleRange(CGXRange(row,3), CGXStyle()
			.SetUserAttribute(GX_IDS_UA_VALID_MIN, _T("0.0e01"))
			.SetUserAttribute(GX_IDS_UA_VALID_MAX, _T("1.0e99"))
			.SetUserAttribute(GX_IDS_UA_VALID_MSG, _T("Please enter a positive value"))
         .SetHorizontalAlignment(DT_RIGHT)
         .SetValue(value)
		);

	SetStyleRange(CGXRange(row,4), CGXStyle()
			.SetUserAttribute(GX_IDS_UA_VALID_MIN, _T("0.0e01"))
			.SetUserAttribute(GX_IDS_UA_VALID_MAX, _T("1.0e3"))
			.SetUserAttribute(GX_IDS_UA_VALID_MSG, _T("Please enter a positive value between 0.0-1000.0"))
         .SetHorizontalAlignment(DT_RIGHT)
         .SetValue(zoneData.nBars)
         );
}

void CStirrupGrid::AddZoneData(const xbrStirrupData::StirrupZone& zoneData)
{
   InsertRows(GetRowCount()+1,1);
   ROWCOL row = GetRowCount();
   SetZoneData(row,zoneData);
}

bool CStirrupGrid::GetZoneData(ROWCOL row,xbrStirrupData::StirrupZone& zoneData)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);
   
   if ( row == GetRowCount() )
   {
      // this is the last zone...
      // zone length is either "to center" or "to end"
      zoneData.Length = -1;
   }
   else
   {
      Float64 length = _tstof(GetCellValue(row,1));
      zoneData.Length = WBFL::Units::ConvertToSysUnits(length,pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);

      if ( zoneData.Length <= 0 )
      {
         CString strMsg;
         strMsg.Format(_T("%s, Row %d: Stirrup zone length must be greater than zero."),m_strGridName,row);
         AfxMessageBox(strMsg,MB_OK | MB_ICONEXCLAMATION);
         return false;
      }
   }

   zoneData.BarSize = GetBarSize(row,2);

   Float64 spacing = _tstof(GetCellValue(row,3));
   zoneData.BarSpacing = WBFL::Units::ConvertToSysUnits(spacing,pDisplayUnits->GetComponentDimUnit().UnitOfMeasure);

   Float64 nBars = _tstof(GetCellValue(row,4));
   zoneData.nBars = nBars;

   return true;
}

CString CStirrupGrid::GetCellValue(ROWCOL nRow, ROWCOL nCol)
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

WBFL::Materials::Rebar::Size CStirrupGrid::GetBarSize(ROWCOL row,ROWCOL col)
{
   std::_tstring strBarSize = (LPCTSTR)GetCellValue(row,col);
   CReinforcementPage* pParent = (CReinforcementPage*)GetParent();
   WBFL::Materials::Rebar::Type type;
   WBFL::Materials::Rebar::Grade grade;
   pParent->GetRebarMaterial(&type,&grade);
   WBFL::LRFD::RebarIter rebarIter(type,grade,true/*stirrups only*/);
   for ( rebarIter.Begin(); rebarIter; rebarIter.Next() )
   {
      if ( rebarIter.GetCurrentRebar()->GetName() == strBarSize )
      {
         return rebarIter.GetCurrentRebar()->GetSize();
      }
   }

   ATLASSERT(false); // should never get here
   return WBFL::Materials::Rebar::Size::bs3;
}

void CStirrupGrid::SetSymmetry(bool isSymmetrical)
{
   m_IsSymmetrical = isSymmetrical;
   ROWCOL nRows = GetRowCount();
   if ( 0 < nRows )
   {
      UpdateLastZoneLength();
   }
   ResizeColWidthsToFit(CGXRange(0,0,GetRowCount(),GetColCount()));
}

void CStirrupGrid::UpdateLastZoneLength()
{
   ROWCOL lastRow = GetRowCount();
   GetParam()->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   // Set text in last row
   CString lastzlen = (m_IsSymmetrical) ? _T("to center") : _T("to end");

   SetStyleRange(CGXRange(lastRow,1), CGXStyle()
      .SetEnabled(FALSE)
      .SetReadOnly(TRUE)
      .SetInterior(::GetSysColor(COLOR_BTNFACE))
      .SetValue(lastzlen)
      );

   GetParam( )->EnableUndo(TRUE);
   GetParam()->SetLockReadOnly(TRUE);
}
