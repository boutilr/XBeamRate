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


// LiveLoadReactionGrid.cpp : implementation file
//

#include "stdafx.h"
#include "ProjectAgent.h"
#include "LiveLoadReactionGrid.h"
#include <EAF\EAFDisplayUnits.h>



void DDX_LiveLoadReactionsGrid(CDataExchange* pDX, CLiveLoadReactionGrid& grid, txnLiveLoadReactions& llData)
{
   if (pDX->m_bSaveAndValidate)
   {
      grid.GetLiveLoadData(llData);
   }
   else
   {
      grid.SetLiveLoadData(llData);
   }
}


GRID_IMPLEMENT_REGISTER(CLiveLoadReactionGrid, CS_DBLCLKS, 0, 0, 0);

/////////////////////////////////////////////////////////////////////////////
// CLiveLoadReactionGrid

CLiveLoadReactionGrid::CLiveLoadReactionGrid()
{
   //   RegisterClass();
   m_LoadRatingType = pgsTypes::lrDesign_Inventory;
}

CLiveLoadReactionGrid::CLiveLoadReactionGrid(pgsTypes::LoadRatingType ratingType)
{
   //   RegisterClass();
   m_LoadRatingType = ratingType;
}

CLiveLoadReactionGrid::~CLiveLoadReactionGrid()
{
}

BEGIN_MESSAGE_MAP(CLiveLoadReactionGrid, CGXGridWnd)
	//{{AFX_MSG_MAP(CLiveLoadReactionGrid)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLiveLoadReactionGrid message handlers

pgsTypes::LoadRatingType CLiveLoadReactionGrid::GetLoadRatingType()
{
   return m_LoadRatingType;
}

void CLiveLoadReactionGrid::CustomInit()
{
// Initialize the grid. For CWnd based grids this call is // 
// essential. For view based grids this initialization is done 
// in OnInitialUpdate.
	Initialize( );

	GetParam( )->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   const int num_rows = 0;
   const int num_cols = 3;

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
   ROWCOL col = 1;
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
         .SetValue(_T("Name"))
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetEnabled(FALSE)          // disables usage as current cell
		);

   CString cv;
   cv.Format(_T("LL+IM (%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   cv.Format(_T("Weight (%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   if ( !::IsLegalRatingType(m_LoadRatingType) )
   {
      // only show weight column for legal rating types
      HideCols(3,3);
   }

   // make it so that text fits correctly in header row
	ResizeRowHeightsToFit(CGXRange(0,0,0,num_cols));
   ResizeColWidthsToFit(CGXRange(0,0,0,num_cols));

   // don't allow users to resize grids
   GetParam( )->EnableTrackColWidth(0); 
   GetParam( )->EnableTrackRowHeight(0); 

	EnableIntelliMouse();
	SetFocus();

   GetParam()->SetLockReadOnly(TRUE);
	GetParam( )->EnableUndo(TRUE);
}

void CLiveLoadReactionGrid::AddVehicle()
{
   ROWCOL nRows = GetRowCount();

	GetParam( )->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   InsertRows(nRows+1,1);
   SetRowStyle(GetRowCount());

   GetParam()->SetLockReadOnly(TRUE);
	GetParam( )->EnableUndo(TRUE);
}

void CLiveLoadReactionGrid::RemoveSelectedVehicles()
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

void CLiveLoadReactionGrid::GetLiveLoadData(txnLiveLoadReactions& llimData)
{
   llimData.m_LLIM.clear();
   ROWCOL nRows = GetRowCount();
   for ( ROWCOL row = 1; row <= nRows; row++ )
   {
      xbrLiveLoadReactionData reactionData;
      GetLiveLoadData(row,reactionData);
      
      llimData.m_LLIM.push_back(reactionData);
   }
}

void CLiveLoadReactionGrid::SetLiveLoadData(const txnLiveLoadReactions& llimData)
{
   GetParam()->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   ROWCOL nRows = GetRowCount();
   if ( 0 < nRows )
   {
      RemoveRows(1,nRows);
   }

   for (const auto& reactionData : llimData.m_LLIM)
   {
      AddLiveLoadData(reactionData);
   }

   ResizeColWidthsToFit(CGXRange(0,0,GetRowCount(),GetColCount()));

   GetParam()->SetLockReadOnly(TRUE);
   GetParam()->EnableUndo(TRUE);
}

void CLiveLoadReactionGrid::SetRowStyle(ROWCOL row)
{
   ROWCOL col = 1;

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      );

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      );

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      );
}

void CLiveLoadReactionGrid::SetLiveLoadData(ROWCOL row,const xbrLiveLoadReactionData& reactionData)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   ROWCOL col = 1;

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetValue(reactionData.Name.c_str())
      );

   CString value = ::FormatDimension(reactionData.LLIM,pDisplayUnits->GetGeneralForceUnit(),false);
   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetValue(value)
      );

   value = ::FormatDimension(reactionData.W,pDisplayUnits->GetGeneralForceUnit(),false);
   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetValue(value)
      );

   if (::IsEmergencyRatingType(m_LoadRatingType))
   {
      col = 1;
      // Disable Name column... can't change name of emergency rating vehicles
      SetStyleRange(CGXRange(row, col++), CGXStyle()
         .SetEnabled(FALSE)
         .SetReadOnly(TRUE)
         .SetInterior(::GetSysColor(COLOR_BTNFACE))
         .SetTextColor(::GetSysColor(COLOR_WINDOWTEXT))
      );

      col++; // skipe the reaction column

      // Disable Weight column... can't change emergency rating vehicles
      SetStyleRange(CGXRange(row, col++), CGXStyle()
         .SetEnabled(FALSE)
         .SetReadOnly(TRUE)
         .SetInterior(::GetSysColor(COLOR_BTNFACE))
         .SetTextColor(::GetSysColor(COLOR_WINDOWTEXT))
      );
   }
}

void CLiveLoadReactionGrid::AddLiveLoadData(const xbrLiveLoadReactionData& reactionData)
{
   InsertRows(GetRowCount()+1,1);
   ROWCOL row = GetRowCount();
   SetRowStyle(row);
   SetLiveLoadData(row,reactionData);
}

void CLiveLoadReactionGrid::GetLiveLoadData(ROWCOL row,xbrLiveLoadReactionData& reactionData)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);
   
   reactionData.Name = GetCellValue(row,1);
   
   Float64 llim = _tstof(GetCellValue(row,2));
   reactionData.LLIM = WBFL::Units::ConvertToSysUnits(llim,pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure);
   
   Float64 w = _tstof(GetCellValue(row,3));
   reactionData.W = WBFL::Units::ConvertToSysUnits(w,pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure);
}

CString CLiveLoadReactionGrid::GetCellValue(ROWCOL nRow, ROWCOL nCol)
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

BOOL CLiveLoadReactionGrid::OnEndEditing(ROWCOL nRow,ROWCOL nCol)
{
   ResizeColWidthsToFit(CGXRange(0,0,GetRowCount(),GetColCount()));
   return CGXGridWnd::OnEndEditing(nRow,nCol);
}
