///////////////////////////////////////////////////////////////////////
// XBeamRate - Cross Beam Load Rating
// Copyright � 1999-2023  Washington State Department of Transportation
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

#ifndef INCLUDED_APP_H_
#define INCLUDED_APP_H_

#include <EAF\EAFDocTemplate.h>
class CXBeamRatePluginApp : public CWinApp
{
public:
   CXBeamRatePluginApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXBeamRatePluginApp)
	public:
    virtual BOOL InitInstance() override;
    virtual int ExitInstance() override;

    CString GetVersion(bool bIncludeBuildNumber) const;

    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) override;

	//}}AFX_VIRTUAL

	//{{AFX_MSG(CXBeamRatePluginApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
   HMENU m_hSharedMenu;
};

#endif // INCLUDED_APP_H_