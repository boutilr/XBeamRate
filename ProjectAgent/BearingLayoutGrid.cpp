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
#include "BearingLayoutGrid.h"
#include <EAF\EAFDisplayUnits.h>


GRID_IMPLEMENT_REGISTER(CBearingLayoutGrid, CS_DBLCLKS, 0, 0, 0);

/////////////////////////////////////////////////////////////////////////////
// CBearingLayoutGrid

CBearingLayoutGrid::CBearingLayoutGrid()
{
//   RegisterClass();
   m_ReactionLoadType = xbrTypes::rltConcentrated;
}

CBearingLayoutGrid::~CBearingLayoutGrid()
{
}

BEGIN_MESSAGE_MAP(CBearingLayoutGrid, CGXGridWnd)
	//{{AFX_MSG_MAP(CBearingLayoutGrid)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBearingLayoutGrid message handlers

void CBearingLayoutGrid::CustomInit(xbrTypes::ReactionLoadType reactionLoadType)
{
// Initialize the grid. For CWnd based grids this call is // 
// essential. For view based grids this initialization is done 
// in OnInitialUpdate.
	Initialize( );

	GetParam( )->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   SetMergeCellsMode(gxnMergeDelayEval);

   const int num_rows = 0;
   const int num_cols = 8;

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
         .SetValue(_T("Brg"))
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetEnabled(FALSE)          // disables usage as current cell
		);

   CString cv;
   cv.Format(_T("DC\n(%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   cv.Format(_T("DW\n(%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   cv.Format(_T("CR\n(%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   cv.Format(_T("SH\n%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   cv.Format(_T("PS\n(%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   cv.Format(_T("RE\n(%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   cv.Format(_T("W\n(%s)"),pDisplayUnits->GetSpanLengthUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);

   cv.Format(_T("S\n(%s)"),pDisplayUnits->GetSpanLengthUnit().UnitOfMeasure.UnitTag().c_str());
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(cv)
		);


   // make it so that text fits correctly in header row
	ResizeRowHeightsToFit(CGXRange(0,0,0,num_cols));
   ResizeColWidthsToFit(CGXRange(0,0,0,num_cols));

   // don't allow users to resize grids
   GetParam( )->EnableTrackColWidth(0); 
   GetParam( )->EnableTrackRowHeight(0); 

	EnableIntelliMouse();
	SetFocus();

   SetReactionLoadType(reactionLoadType);

   GetParam()->SetLockReadOnly(TRUE);
	GetParam( )->EnableUndo(TRUE);
}

BOOL CBearingLayoutGrid::OnEndEditing(ROWCOL nRow,ROWCOL nCol)
{
   ResizeColWidthsToFit(CGXRange(0,0,GetRowCount(),GetColCount()));
   return CGXGridWnd::OnEndEditing(nRow,nCol);
}

void CBearingLayoutGrid::SetReactionLoadType(xbrTypes::ReactionLoadType reactionLoadType)
{
   m_ReactionLoadType = reactionLoadType;
   OnReactionLoadTypeChanged();
}

void CBearingLayoutGrid::AddBearing()
{
   ROWCOL nRows = GetRowCount();

   Float64 DC, DW, CR, SH, PS, RE, W, S;
   GetBearingData(nRows,&DC,&DW,&CR,&SH,&PS,&RE,&W,&S); // bearing data in the last row (we want DC and DW)

   Float64 Sdefault = WBFL::Units::ConvertToSysUnits(6.0,WBFL::Units::Measure::Feet); // default spacing
   Float64 Wdefault = Sdefault;
   if ( 1 < nRows )
   {
      // get the spacing between the last two bearings
      Float64 DC1, DW1, CR1, SH1, PS1, RE1, W1, S1;
      GetBearingData(nRows-1,&DC1,&DW1,&CR1,&SH1,&PS1,&RE1,&W1,&S1);
      Wdefault = W1;
      Sdefault = S1;
   }

	GetParam( )->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   // set the default spacing in the last row
   if (nRows > 0)
   {
       SetBearingData(nRows, DC, DW, CR, SH, PS, RE, Wdefault, Sdefault);
   }

   // add the new bearing (adds row to the grid)
   AddBearingRow(DC,DW,CR,SH,PS,RE,Wdefault,0);

   GetParam()->SetLockReadOnly(TRUE);
   GetParam( )->EnableUndo(TRUE);
}

void CBearingLayoutGrid::RemoveSelectedBearings()
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

   ROWCOL nRows = GetRowCount();
   SetStyleRange(CGXRange(nRows,8), CGXStyle()
      .SetEnabled(FALSE)
      .SetReadOnly(TRUE)
      .SetInterior(::GetSysColor(COLOR_BTNFACE))
      .SetTextColor(::GetSysColor(COLOR_BTNFACE))
      .SetHorizontalAlignment(DT_RIGHT)
         );

   GetParam()->SetLockReadOnly(TRUE);
	GetParam( )->EnableUndo(TRUE);
}

void CBearingLayoutGrid::GetBearingData(xbrBearingLineData& brgLineData,std::vector<txnDeadLoadReaction>& deadLoadReactions)
{
   deadLoadReactions.clear();
   ROWCOL nRows = GetRowCount();
   brgLineData.SetBearingCount(nRows);
   for ( ROWCOL row = 1; row <= nRows; row++ )
   {
      Float64 DC, DW, CR, SH, PS, RE, W, S;
      GetBearingData(row,&DC,&DW,&CR,&SH,&PS,&RE,&W,&S);
      if ( row < nRows )
      {
         brgLineData.SetSpacing(row-1,S);
      }
      txnDeadLoadReaction reaction;
      reaction.m_DC = DC;
      reaction.m_DW = DW;
      reaction.m_CR = CR;
      reaction.m_SH = SH;
      reaction.m_PS = PS;
      reaction.m_RE = RE;
      reaction.m_W  = W;
      deadLoadReactions.push_back(reaction);
   }
}

void CBearingLayoutGrid::SetBearingData(xbrBearingLineData& brgLineData,std::vector<txnDeadLoadReaction>& deadLoadReactions)
{
   GetParam()->EnableUndo(FALSE);
   GetParam()->SetLockReadOnly(FALSE);

   ROWCOL nRows = GetRowCount();
   if ( 0 < nRows )
   {
      RemoveRows(1,nRows);
   }

   if ( brgLineData.GetBearingCount() == 0 )
   {
      // Always have one bearing
      AddBearingRow(0,0,0,0,0,0,0,0);
   }
   else
   {
      IndexType nBearings = brgLineData.GetBearingCount();
      for ( IndexType brgIdx = 0; brgIdx < nBearings; brgIdx++ )
      {
         Float64 S = (brgIdx != nBearings-1 ? brgLineData.GetSpacing(brgIdx) : 0);
         // A newly initialized bearing line may not have reactions yet.
         const txnDeadLoadReaction zeroReaction;
         const txnDeadLoadReaction& reaction = (brgIdx < deadLoadReactions.size() ? deadLoadReactions[brgIdx] : zeroReaction);
         Float64 DC = reaction.m_DC;
         Float64 DW = reaction.m_DW;
         Float64 CR = reaction.m_CR;
         Float64 SH = reaction.m_SH;
         Float64 PS = reaction.m_PS;
         Float64 RE = reaction.m_RE;
         Float64 W  = reaction.m_W;
         AddBearingRow(DC,DW,CR,SH,PS,RE,W,S);
      }
   }

   GetParam()->SetLockReadOnly(TRUE);
   GetParam()->EnableUndo(TRUE);
}

IndexType CBearingLayoutGrid::GetBearingCount()
{
   return (IndexType)GetRowCount();
}

void CBearingLayoutGrid::SetBearingData(ROWCOL row,Float64 DC,Float64 DW,Float64 CR,Float64 SH,Float64 PS,Float64 RE,Float64 W,Float64 S)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   ROWCOL col = 1;

   // DC
   CString value;
   if ( m_ReactionLoadType == xbrTypes::rltConcentrated )
   {
      value = ::FormatDimension(DC,pDisplayUnits->GetGeneralForceUnit(),false);
   }
   else
   {
      value = ::FormatDimension(DC,pDisplayUnits->GetForcePerLengthUnit(),false);
   }

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );

   // DW
   if ( m_ReactionLoadType == xbrTypes::rltConcentrated )
   {
      value = ::FormatDimension(DW,pDisplayUnits->GetGeneralForceUnit(),false);
   }
   else
   {
      value = ::FormatDimension(DW,pDisplayUnits->GetForcePerLengthUnit(),false);
   }

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );

   // CR
   if ( m_ReactionLoadType == xbrTypes::rltConcentrated )
   {
      value = ::FormatDimension(CR,pDisplayUnits->GetGeneralForceUnit(),false);
   }
   else
   {
      value = ::FormatDimension(CR,pDisplayUnits->GetForcePerLengthUnit(),false);
   }

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );

   // SH
   if ( m_ReactionLoadType == xbrTypes::rltConcentrated )
   {
      value = ::FormatDimension(SH,pDisplayUnits->GetGeneralForceUnit(),false);
   }
   else
   {
      value = ::FormatDimension(SH,pDisplayUnits->GetForcePerLengthUnit(),false);
   }

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );

   // PS
   if ( m_ReactionLoadType == xbrTypes::rltConcentrated )
   {
      value = ::FormatDimension(PS,pDisplayUnits->GetGeneralForceUnit(),false);
   }
   else
   {
      value = ::FormatDimension(PS,pDisplayUnits->GetForcePerLengthUnit(),false);
   }

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );

   // RE
   if ( m_ReactionLoadType == xbrTypes::rltConcentrated )
   {
      value = ::FormatDimension(RE,pDisplayUnits->GetGeneralForceUnit(),false);
   }
   else
   {
      value = ::FormatDimension(RE,pDisplayUnits->GetForcePerLengthUnit(),false);
   }

   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );

   // W
   value = ::FormatDimension(W,pDisplayUnits->GetSpanLengthUnit(),false);
   SetStyleRange(CGXRange(row,col++), CGXStyle()
      .SetEnabled(TRUE)
      .SetReadOnly(FALSE)
      .SetHorizontalAlignment(DT_RIGHT)
      .SetValue(value)
      );

   // Spacing
   // Set the value for the spacing to the next bearing and disable this cell
   // This assumes this bearing is in the last row and spacing to next isn't applicable
   value = ::FormatDimension(S,pDisplayUnits->GetSpanLengthUnit(),false);
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

   ResizeColWidthsToFit(CGXRange(GetRowCount(),GetColCount()));

   col++;
}

void CBearingLayoutGrid::OnReactionLoadTypeChanged()
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   CString strDC,strDW,strCR,strSH,strPS,strRE;
   if ( m_ReactionLoadType == xbrTypes::rltConcentrated )
   {
      strDC.Format(_T("DC\n(%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
      strDW.Format(_T("DW\n(%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
      strCR.Format(_T("CR\n(%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
      strSH.Format(_T("SH\n(%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
      strPS.Format(_T("PS\n(%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
      strRE.Format(_T("RE\n(%s)"),pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure.UnitTag().c_str());
   }
   else
   {
      strDC.Format(_T("DC\n(%s)"),pDisplayUnits->GetForcePerLengthUnit().UnitOfMeasure.UnitTag().c_str());
      strDW.Format(_T("DW\n(%s)"),pDisplayUnits->GetForcePerLengthUnit().UnitOfMeasure.UnitTag().c_str());
      strCR.Format(_T("CR\n(%s)"),pDisplayUnits->GetForcePerLengthUnit().UnitOfMeasure.UnitTag().c_str());
      strSH.Format(_T("SH\n(%s)"),pDisplayUnits->GetForcePerLengthUnit().UnitOfMeasure.UnitTag().c_str());
      strPS.Format(_T("PS\n(%s)"),pDisplayUnits->GetForcePerLengthUnit().UnitOfMeasure.UnitTag().c_str());
      strRE.Format(_T("RE\n(%s)"),pDisplayUnits->GetForcePerLengthUnit().UnitOfMeasure.UnitTag().c_str());
   }

   ROWCOL col = 1;
   //DC
   SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(strDC)
		);
   //DW
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(strDW)
		);
    //CR
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(strCR)
		);
    //SH
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(strSH)
		);
    //PS
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(strPS)
		);
    //RE
	SetStyleRange(CGXRange(0,col++), CGXStyle()
         .SetWrapText(TRUE)
			.SetEnabled(FALSE)          // disables usage as current cell
         .SetHorizontalAlignment(DT_CENTER)
         .SetVerticalAlignment(DT_VCENTER)
			.SetValue(strRE)
		);

   // Hide/Show the "W" column
   BOOL bHide = (m_ReactionLoadType == xbrTypes::rltConcentrated ? TRUE : FALSE);
   HideCols(col,col,bHide);

   ResizeColWidthsToFit(CGXRange(0,0,GetRowCount(),GetColCount()));
}

void CBearingLayoutGrid::AddBearingRow(Float64 DC,Float64 DW,Float64 CR,Float64 SH,Float64 PS,Float64 RE,Float64 W,Float64 S)
{
   InsertRows(GetRowCount()+1,1);
   ROWCOL row = GetRowCount();
   SetBearingData(row,DC,DW,CR,SH,PS,RE,W,S);

   ResizeColWidthsToFit(CGXRange(0,0,GetRowCount(),GetColCount()));
}

void CBearingLayoutGrid::GetBearingData(ROWCOL row,Float64* pDC,Float64* pDW,Float64* pCR,Float64* pSH,Float64* pPS,Float64* pRE,Float64* pW,Float64* pS)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   ROWCOL col = 1;

   Float64 DC = _tstof(GetCellValue(row,col++));
   if ( m_ReactionLoadType == xbrTypes::rltConcentrated )
   {
      *pDC = WBFL::Units::ConvertToSysUnits(DC,pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure);
   }
   else
   {
      *pDC = WBFL::Units::ConvertToSysUnits(DC,pDisplayUnits->GetForcePerLengthUnit().UnitOfMeasure);
   }

   Float64 DW = _tstof(GetCellValue(row,col++));
   if ( m_ReactionLoadType == xbrTypes::rltConcentrated )
   {
      *pDW = WBFL::Units::ConvertToSysUnits(DW,pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure);
   }
   else
   {
      *pDW = WBFL::Units::ConvertToSysUnits(DW,pDisplayUnits->GetForcePerLengthUnit().UnitOfMeasure);
   }

   Float64 CR = _tstof(GetCellValue(row,col++));
   if ( m_ReactionLoadType == xbrTypes::rltConcentrated )
   {
      *pCR = WBFL::Units::ConvertToSysUnits(CR,pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure);
   }
   else
   {
      *pCR = WBFL::Units::ConvertToSysUnits(CR,pDisplayUnits->GetForcePerLengthUnit().UnitOfMeasure);
   }

   Float64 SH = _tstof(GetCellValue(row,col++));
   if ( m_ReactionLoadType == xbrTypes::rltConcentrated )
   {
      *pSH = WBFL::Units::ConvertToSysUnits(SH,pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure);
   }
   else
   {
      *pSH = WBFL::Units::ConvertToSysUnits(SH,pDisplayUnits->GetForcePerLengthUnit().UnitOfMeasure);
   }

   Float64 PS = _tstof(GetCellValue(row,col++));
   if ( m_ReactionLoadType == xbrTypes::rltConcentrated )
   {
      *pPS = WBFL::Units::ConvertToSysUnits(PS,pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure);
   }
   else
   {
      *pPS = WBFL::Units::ConvertToSysUnits(PS,pDisplayUnits->GetForcePerLengthUnit().UnitOfMeasure);
   }

   Float64 RE = _tstof(GetCellValue(row,col++));
   if ( m_ReactionLoadType == xbrTypes::rltConcentrated )
   {
      *pRE = WBFL::Units::ConvertToSysUnits(RE,pDisplayUnits->GetGeneralForceUnit().UnitOfMeasure);
   }
   else
   {
      *pRE = WBFL::Units::ConvertToSysUnits(RE,pDisplayUnits->GetForcePerLengthUnit().UnitOfMeasure);
   }

   Float64 W = _tstof(GetCellValue(row,col++));
   if ( m_ReactionLoadType == xbrTypes::rltUniform )
   {
      *pW = WBFL::Units::ConvertToSysUnits(W,pDisplayUnits->GetSpanLengthUnit().UnitOfMeasure);
   }
   else
   {
      *pW = 0; // set W to zero for concentrated loads
   }

   Float64 S = _tstof(GetCellValue(row,col++));
   *pS = WBFL::Units::ConvertToSysUnits(S,pDisplayUnits->GetSpanLengthUnit().UnitOfMeasure);
}

CString CBearingLayoutGrid::GetCellValue(ROWCOL nRow, ROWCOL nCol)
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
