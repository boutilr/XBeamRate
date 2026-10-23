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

#pragma once

// BearingLayoutGrid.h : header file
//

#include "txnEditPier.h"


/////////////////////////////////////////////////////////////////////////////
// CBearingLayoutGrid window

class CBearingLayoutGrid : public CGXGridWnd
{
	GRID_DECLARE_REGISTER()
// Construction
public:
	CBearingLayoutGrid();
	virtual ~CBearingLayoutGrid();

// Attributes
public:

// Operations
public:
   void SetReactionLoadType(xbrTypes::ReactionLoadType reactionLoadType);

   void AddBearing();
   void RemoveSelectedBearings();

   void GetBearingData(xbrBearingLineData& brgLineData,std::vector<txnDeadLoadReaction>& deadLoadReactions);
   void SetBearingData(xbrBearingLineData& brgLineData,std::vector<txnDeadLoadReaction>& deadLoadReactions);

   IndexType GetBearingCount();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBearingLayoutGrid)
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
public:
	//{{AFX_MSG(CBearingLayoutGrid)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
   // custom stuff for grid
   void CustomInit(xbrTypes::ReactionLoadType reactionLoadType);

protected:
   virtual BOOL OnEndEditing(ROWCOL nRow,ROWCOL nCol) override;

private:
   xbrTypes::ReactionLoadType m_ReactionLoadType;
   void OnReactionLoadTypeChanged();

   void AddBearingRow(Float64 DC,Float64 DW,Float64 CR,Float64 SH,Float64 PS,Float64 RE,Float64 W,Float64 S);
   void SetBearingData(ROWCOL row,Float64 DC,Float64 DW,Float64 W,Float64 CR,Float64 SH,Float64 PS,Float64 RE,Float64 S);
   void GetBearingData(ROWCOL row,Float64* pDC,Float64* pDW,Float64* pCR,Float64* pSH,Float64* pPS,Float64* pRE,Float64* pW,Float64* pS);
   CString GetCellValue(ROWCOL nRow, ROWCOL nCol);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
