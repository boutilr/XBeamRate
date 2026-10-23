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

// LongitudinalRebarGrid.cpp : implementation file
//

#pragma Reminder("UPDATE: Update all grids of PGSuper with the techniques for getting rebar information used here")
// rebar information is "discovered" rather than hard coded

#include "stdafx.h"
#include <AgentTools.h>
#include "resource.h"
#include "LongitudinalRebarGrid.h"
#include <EAF\EAFDisplayUnits.h>
#include <LRFD\RebarPool.h>
#include <XBeamRateExt\ReinforcementPage.h>


GRID_IMPLEMENT_REGISTER(CLongitudinalRebarGrid, CS_DBLCLKS, 0, 0, 0);

/////////////////////////////////////////////////////////////////////////////
// CLongitudinalRebarGrid

CLongitudinalRebarGrid::CLongitudinalRebarGrid()
{
   RegisterClass();
}

CLongitudinalRebarGrid::~CLongitudinalRebarGrid()
{
}

BEGIN_MESSAGE_MAP(CLongitudinalRebarGrid, CGXGridWnd)
	//{{AFX_MSG_MAP(CLongitudinalRebarGrid)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLongitudinalRebarGrid message handlers

void CLongitudinalRebarGrid::CustomInit()
{
// Initialize the grid. For CWnd based grids this call is // 
// essential. For view based grids this initialization is done 
// in OnInitialUpdate.
	Initialize( );

	GetParam( )->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   SetMergeCellsMode(gxnMergeDelayEval);

   const int num_rows = 0;
   const int num_cols = 10;

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
         .SetValue(_T("Row"))
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetEnabled(FALSE)          // disables usage as current cell
		);

   SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
         .SetValue(_T("XBeam Face"))
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetEnabled(FALSE)          // disables usage as current cell
		);

   SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
         .SetValue(_T("Datum"))
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetEnabled(FALSE)          // disables usage as current cell
		);

   CString cv;
   cv.Format(_T("Start\n(%s)"),pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   cv.Format(_T("Length\n(%s)"),pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   cv.Format(_T("Cover\n(%s)"),pDisplayUnits->GetComponentDimUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(_T("Bar\nSize"))
		);

	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
         .SetValue(_T("# of\nBars"))
		);

   cv.Format(_T("Spacing\n(%s)"),pDisplayUnits->GetComponentDimUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
         .SetValue(_T("Left\nHook"))
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetEnabled(FALSE)          // disables usage as current cell
		);

   SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
         .SetValue(_T("Right\nHook"))
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetEnabled(FALSE)          // disables usage as current cell
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
}

void CLongitudinalRebarGrid::AddBar()
{
   xbrLongitudinalRebarData::RebarRow rebarData;

	GetParam( )->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   AddRebarRow(rebarData);

   GetParam()->SetLockReadOnly(TRUE);
	GetParam( )->EnableUndo(TRUE);
}

void CLongitudinalRebarGrid::RemoveSelectedBars()
{
	GetParam( )->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   CDWordArray selRows;
   ROWCOL nSelRows = GetSelectedRows(selRows);
   for ( int r = nSelRows-1; r >= 0; r-- )
   {
      ROWCOL selRow = selRows[r];
      RemoveRows(selRow,selRow);
   }

   GetParam()->SetLockReadOnly(TRUE);
	GetParam( )->EnableUndo(TRUE);
}

void CLongitudinalRebarGrid::GetRebarData(xbrLongitudinalRebarData& rebars)
{
   rebars.RebarRows.clear();
   ROWCOL nRows = GetRowCount();
   for ( ROWCOL row = 1; row <= nRows; row++ )
   {
      xbrLongitudinalRebarData::RebarRow rebarData;
      GetRebarData(row,rebarData);
      rebars.RebarRows.push_back(rebarData);
   }
}

void CLongitudinalRebarGrid::SetRebarData(const xbrLongitudinalRebarData& rebars)
{
   GetParam()->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   ROWCOL nRows = GetRowCount();
   if ( 0 < nRows )
   {
      RemoveRows(1,nRows);
   }

   for (const auto& rebarData : rebars.RebarRows)
   {
      AddRebarRow(rebarData);
   }

   GetParam()->SetLockReadOnly(TRUE);
   GetParam()->EnableUndo(TRUE);
}

CString CLongitudinalRebarGrid::GetDatumOptions()
{
   IReinforcementPageParent* pParent = ((CReinforcementPage*)GetParent())->GetPageParent();

   CString strBeamFaceChoiceList;
   if ( pParent->GetPierType() == xbrTypes::pctIntegral )
   {
      strBeamFaceChoiceList = _T("Top\nTop Lower XBeam\nBottom\n");
   }
   else
   {
      strBeamFaceChoiceList = _T("Top\nBottom\n");
   }
   return strBeamFaceChoiceList;
}

CString CLongitudinalRebarGrid::GetDatum(xbrTypes::LongitudinalRebarDatumType datum)
{
   if ( datum == xbrTypes::Bottom )
   {
      return _T("Bottom");
   }

   IReinforcementPageParent* pParent = ((CReinforcementPage*)GetParent())->GetPageParent();
   if ( pParent->GetPierType() != xbrTypes::pctIntegral && datum == xbrTypes::TopLowerXBeam )
   {
      // For non-integral superstructure connections, map TopLowerXBeam into Top.
      return _T("Top");
   }

   if ( datum == xbrTypes::Top )
   {
      if ( pParent->GetPierType() == xbrTypes::pctIntegral )
      {
         return _T("Top");
      }
      else
      {
         // xbrTypes::Top bars are not valid for non-integral cross beams... return the keyword "Invalid"
         // so we can omit this one from the grid
         return _T("Invalid");
      }
   }

   return _T("Top Lower XBeam");
}

LPCTSTR CLongitudinalRebarGrid::GetRebarLayoutType(xbrTypes::LongitudinalRebarLayoutType layoutType)
{
   switch(layoutType)
   {
   case xbrTypes::blLeftEnd:
      return _T("Left End");

   case xbrTypes::blRightEnd:
      return _T("Right End");

   case xbrTypes::blFullLength:
      return _T("Full Length");
   }
   ATLASSERT(false);
   return _T("Full Length");
}

xbrTypes::LongitudinalRebarLayoutType CLongitudinalRebarGrid::GetRebarLayoutType(LPCTSTR lpszLayoutType)
{
   CString str(lpszLayoutType);
   if ( str == _T("Left End") )
   {
      return xbrTypes::blLeftEnd;
   }
   else if ( str == _T("Right End") )
   {
      return xbrTypes::blRightEnd;
   }
   else if ( str == _T("Full Length") )
   {
      return xbrTypes::blFullLength;
   }

   ATLASSERT(false);
   return xbrTypes::blFullLength;
}

bool CLongitudinalRebarGrid::SetRebarData(ROWCOL row,const xbrLongitudinalRebarData::RebarRow& rebarData)
{
   ROWCOL col = 1;

   IReinforcementPageParent* pParent = ((CReinforcementPage*)GetParent())->GetPageParent();

   CString strBeamFaceChoiceList = GetDatumOptions();
   CString strDatum = GetDatum(rebarData.Datum);

   if ( strDatum == _T("Invalid") )
   {
      // bar datum is not valid for this type of cross beam... just skip it
      return false;
   }

   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   // Datum
   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetControl(GX_IDS_CTRL_CBS_DROPDOWNLIST)
      .SetChoiceList(strBeamFaceChoiceList)
      .SetValue(strDatum)
      );

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetControl(GX_IDS_CTRL_CBS_DROPDOWNLIST)
      .SetChoiceList(_T("Left End\nRight End\nFull Length"))
      .SetValue(GetRebarLayoutType(rebarData.LayoutType))
      );

   Float64 value = WBFL::Units::ConvertFromSysUnits(rebarData.Start,pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);
   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );

   value = WBFL::Units::ConvertFromSysUnits(rebarData.Length,pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);
   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );

   // grey out the Start and Length fields if rebar layout is full length
   if ( rebarData.LayoutType == xbrTypes::blFullLength )
   {
      SetStyleRange(CGXRange(row,col-2,row,col-1),CGXStyle()
         .SetEnabled(FALSE)
         .SetReadOnly(TRUE)
         .SetInterior(::GetSysColor(COLOR_BTNFACE))
         .SetTextColor(::GetSysColor(COLOR_BTNFACE))
         .SetHorizontalAlignment(DT_RIGHT)
         );   
   }

   // Cover
   value = WBFL::Units::ConvertFromSysUnits(rebarData.Cover,pDisplayUnits->GetComponentDimUnit().UnitOfMeasure);
   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );

   // Bar Size
   WBFL::Materials::Rebar::Type type = pParent->GetRebarType();
   WBFL::Materials::Rebar::Grade grade = pParent->GetRebarGrade();
   CString strBarSizeChoiceList;
   WBFL::LRFD::RebarIter rebarIter(type,grade);
   for ( rebarIter.Begin(); rebarIter; rebarIter.Next() )
   {
      const auto* pRebar = rebarIter.GetCurrentRebar();
      strBarSizeChoiceList += pRebar->GetName().c_str();
      strBarSizeChoiceList += _T("\n");
   }

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetControl(GX_IDS_CTRL_CBS_DROPDOWNLIST)
      .SetChoiceList(strBarSizeChoiceList)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(WBFL::LRFD::RebarPool::GetBarSize(rebarData.BarSize).c_str())
      );

   // Bar Count
   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue((UINT)rebarData.NumberOfBars)
      );

   // Spacing
   value = WBFL::Units::ConvertFromSysUnits(rebarData.BarSpacing,pDisplayUnits->GetComponentDimUnit().UnitOfMeasure);
   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_CENTER)
      .SetControl(GX_IDS_CTRL_CHECKBOX3D)
      .SetValue((UINT)rebarData.bHookStart)
      );

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_CENTER)
      .SetControl(GX_IDS_CTRL_CHECKBOX3D)
      .SetValue((UINT)rebarData.bHookEnd)
      );

   ResizeColWidthsToFit(CGXRange(0,0,GetRowCount(),GetColCount()));

   return true;
}

void CLongitudinalRebarGrid::AddRebarRow(const xbrLongitudinalRebarData::RebarRow& rebarData)
{
   InsertRows(GetRowCount()+1,1);
   ROWCOL row = GetRowCount();

   if ( !SetRebarData(row,rebarData) )
   {
      RemoveRows(row,row);
   }
}

void CLongitudinalRebarGrid::GetRebarData(ROWCOL row,xbrLongitudinalRebarData::RebarRow& rebarData)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   ROWCOL col = 1;

   // Beam Face
   rebarData.Datum = GetDatum(row,col++);

   CString strLayout = GetCellValue(row,col++);
   rebarData.LayoutType = GetRebarLayoutType(strLayout);

   Float64 start = _tstof(GetCellValue(row,col++));
   rebarData.Start = WBFL::Units::ConvertToSysUnits(start,pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);

   Float64 length = _tstof(GetCellValue(row,col++));
   rebarData.Length = WBFL::Units::ConvertToSysUnits(length,pDisplayUnits->GetXSectionDimUnit().UnitOfMeasure);

   // Cover
   Float64 cover = _tstof(GetCellValue(row,col++));
   rebarData.Cover = WBFL::Units::ConvertToSysUnits(cover,pDisplayUnits->GetComponentDimUnit().UnitOfMeasure);

   // Bar Size
   rebarData.BarSize = GetBarSize(row,col++);

   // # Bars
   Int16 nBars = _tstoi(GetCellValue(row,col++));
   rebarData.NumberOfBars = nBars;

   // Spacing;
   Float64 spacing = _tstof(GetCellValue(row,col++));
   rebarData.BarSpacing = WBFL::Units::ConvertToSysUnits(spacing,pDisplayUnits->GetComponentDimUnit().UnitOfMeasure);

   CString strCheck = GetCellValue(row,col++);
   rebarData.bHookStart = (strCheck == _T("1") ? true : false);

   strCheck = GetCellValue(row,col++);
   rebarData.bHookEnd = (strCheck == _T("1") ? true : false);
}

CString CLongitudinalRebarGrid::GetCellValue(ROWCOL nRow, ROWCOL nCol)
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

xbrTypes::LongitudinalRebarDatumType CLongitudinalRebarGrid::GetDatum(ROWCOL row,ROWCOL col)
{
   CString strDatum = GetCellValue(row,col);
   if ( strDatum == _T("Top") )
   {
      IReinforcementPageParent* pParent = ((CReinforcementPage*)GetParent())->GetPageParent();
      if ( pParent->GetPierType() == xbrTypes::pctIntegral )
      {
         return xbrTypes::Top;
      }
      else
      {
         return xbrTypes::TopLowerXBeam;
      }
   }
   else if ( strDatum == _T("Bottom") )
   {
      return xbrTypes::Bottom;
   }
   else
   {
      return xbrTypes::TopLowerXBeam;
   }
}

WBFL::Materials::Rebar::Size CLongitudinalRebarGrid::GetBarSize(ROWCOL row,ROWCOL col)
{
   std::_tstring strBarSize = (LPCTSTR)GetCellValue(row,col);
   CReinforcementPage* pParent = (CReinforcementPage*)GetParent();
   WBFL::Materials::Rebar::Type type;
   WBFL::Materials::Rebar::Grade grade;
   pParent->GetRebarMaterial(&type,&grade);
   WBFL::LRFD::RebarIter rebarIter(type,grade);
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

void CLongitudinalRebarGrid::OnModifyCell(ROWCOL nRow,ROWCOL nCol)
{
   if ( nCol == 2 )
   {
      GetParam()->EnableUndo(FALSE);
      GetParam()->SetLockReadOnly(FALSE);

      CString str = GetCellValue(nRow,nCol);
      xbrTypes::LongitudinalRebarLayoutType layoutType = GetRebarLayoutType(str);
      if ( layoutType == xbrTypes::blFullLength )
      {
         SetStyleRange(CGXRange(nRow,nCol+1,nRow,nCol+2),CGXStyle()
            .SetEnabled(FALSE)
            .SetReadOnly(TRUE)
            .SetInterior(::GetSysColor(COLOR_BTNFACE))
            .SetTextColor(::GetSysColor(COLOR_BTNFACE))
            .SetHorizontalAlignment(DT_RIGHT)
            );
      }
      else
      {
         SetStyleRange(CGXRange(nRow,nCol+1,nRow,nCol+2),CGXStyle()
            .SetEnabled(TRUE)
            .SetReadOnly(FALSE)
            .SetInterior(::GetSysColor(COLOR_WINDOW))
            .SetTextColor(::GetSysColor(COLOR_WINDOWTEXT))
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
