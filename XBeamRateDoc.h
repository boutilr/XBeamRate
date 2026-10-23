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

// XBeamRateDoc.h : interface of the CXBeamRateDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_XBeamRateDoc_H__A5D5FE52_139D_46C7_84CF_35817D298084__INCLUDED_)
#define AFX_XBeamRateDoc_H__A5D5FE52_139D_46C7_84CF_35817D298084__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <EAF\EAFBrokerDocument.h>
#include <EAF\EAFAutoCalcDoc.h>
#include <WBFLUnitServer.h>

class CXBeamRateDocProxyAgent;

class CXBeamRateDoc : public CEAFBrokerDocument, public CEAFAutoCalcDocMixin
{
protected: // create from serialization only
	CXBeamRateDoc();
	DECLARE_DYNCREATE(CXBeamRateDoc)

// CEAFAutoCalcDocMixin over-rides
public:
   bool IsAutoCalcEnabled() const override;
   void EnableAutoCalc(bool bEnable) override;

// CEBrokerDocument over-rides
public:
   BOOL Init() override;
   std::pair<bool, WBFL::EAF::AgentErrors> LoadSpecialAgents() override;

   void OnChangedFavoriteReports(BOOL bIsFavorites, BOOL bFromMenu) override;
   void ShowCustomReportHelp(WBFL::EAF::CustomReportHelp helpType) override;
   void ShowCustomReportDefinitionHelp() override;

// Attributes
public:
   CString GetRootNodeName() override;
   Float64 GetRootNodeVersion() override;

// Operations
public:
   void GetDocUnitSystem(IDocUnitSystem** ppDocUnitSystem);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXBeamRateDoc)
	public:
	BOOL OnNewDocument() override;
    void OnCloseDocument() override;
	BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) override;
	//}}AFX_VIRTUAL	//}}AFX_VIRTUAL

   void LoadToolbarState() override;
   void SaveToolbarState() override;
   CString GetToolbarSectionName() override;
   BOOL GetStatusBarMessageString(UINT nID,CString& rMessage) const override;
   BOOL GetToolTipMessageString(UINT nID, CString& rMessage) const override;

// Implementation
public:
	virtual ~CXBeamRateDoc();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

protected:
   CATID GetAgentCategoryID() override;
   HINSTANCE GetResourceInstance() override;
   HRESULT LoadTheDocument(IStructuredLoad* pStrLoad) override;
   HRESULT WriteTheDocument(IStructuredSave* pStrSave) override;
   void CreateReportView(IndexType rptIdx,BOOL bPrompt) override;
   void CreateGraphView(IndexType graphIdx) override;

   void LoadDocumentSettings() override;
   void SaveDocumentSettings() override;

   CString GetDocumentationSetName() override;
   CString GetDocumentationRootLocation() override;

// Generated message map functions
protected:
	//{{AFX_MSG(CXBeamRateDoc)
   afx_msg void OnAbout();
	//}}AFX_MSG
   afx_msg void OnUpdateViewGraphs(CCmdUI* pCmdUI);
   afx_msg BOOL OnViewGraphs(NMHDR* pnmhdr,LRESULT* plr);
   afx_msg void OnUpdateViewReports(CCmdUI* pCmdUI);
   afx_msg BOOL OnViewReports(NMHDR* pnmhdr,LRESULT* plr);
   afx_msg void OnViewPier();
   afx_msg void OnAutoCalc();
   afx_msg void OnUpdateAutoCalc(CCmdUI* pCmdUI);
   afx_msg void OnUpdateNow();
	afx_msg void OnUpdateUpdateNow(CCmdUI* pCmdUI);
   afx_msg void OnHelpFinder();
   afx_msg void OnViewStatusCenter(UINT nID);
   DECLARE_MESSAGE_MAP()

   // callback IDs for any status callbacks we register
   StatusCallbackIDType m_scidInformationalError;
   StatusGroupIDType m_StatusGroupID;

   friend CXBeamRateDocProxyAgent;
   std::shared_ptr<CXBeamRateDocProxyAgent> m_pMyDocProxyAgent;

   CComPtr<IDocUnitSystem> m_DocUnitSystem;

   bool m_bAutoCalcEnabled;

   BOOL OpenTheDocument(LPCTSTR lpszPathName) override;
   void OnCreateFinalize() override;
   void BrokerShutDown() override;
   void OnStatusChanged() override;

   // called when the UI Hints have been reset
   void OnUIHintsReset() override;

   void PopulateReportMenu();
   void PopulateGraphMenu();

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XBeamRateDoc_H__A5D5FE52_139D_46C7_84CF_35817D298084__INCLUDED_)
