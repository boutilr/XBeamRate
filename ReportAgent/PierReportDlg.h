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

#include "resource.h"

class CPierReportDlg : public CDialog
{
	DECLARE_DYNAMIC(CPierReportDlg)

public:
	CPierReportDlg(const WBFL::ReportMgr::ReportDescription& rptDesc,std::shared_ptr<WBFL::ReportMgr::ReportSpecification>& pRptSpec,CWnd* pParent = nullptr);
	virtual ~CPierReportDlg();

// Dialog Data
	enum { IDD = IDD_PIERREPORT };

   PierIDType m_PierID;
   std::vector<std::_tstring> m_ChapterList;

protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

   void UpdateChapterList();
   void ClearChapterCheckMarks();
   void InitChapterListFromSpec();
   void InitFromRptSpec();

   CCheckListBox	m_ChList;

   const WBFL::ReportMgr::ReportDescription& m_RptDesc;

   std::shared_ptr<WBFL::ReportMgr::ReportSpecification> m_pInitRptSpec; // report spec for initializing the dialog

   int m_ReportEvenIncrementsIval;

public:
   bool m_bReportEvenIncrements;

	// Generated message map functions
	//{{AFX_MSG(CPierReportDlg)
	virtual BOOL OnInitDialog() override;
	afx_msg void OnHelp();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
