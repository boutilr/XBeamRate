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

#include "LiveLoadReactionGrid.h"

// CLegalLiveLoadReactionsPage dialog

class CLegalLiveLoadReactionsPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CLegalLiveLoadReactionsPage)

public:
   CLegalLiveLoadReactionsPage();
	virtual ~CLegalLiveLoadReactionsPage();

// Dialog Data
	enum { IDD = IDD_LEGAL_LIVE_LOAD_REACTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

   afx_msg void OnLegalRoutineAdd();
   afx_msg void OnLegalRoutineRemove();

   afx_msg void OnLegalSpecialAdd();
   afx_msg void OnLegalSpecialRemove();

   afx_msg void OnLegalEmergencyAdd();
   afx_msg void OnLegalEmergencyRemove();

   afx_msg void OnHelp();

	DECLARE_MESSAGE_MAP()

   CLiveLoadReactionGrid m_LegalRoutineGrid;
   CLiveLoadReactionGrid m_LegalSpecialGrid;
   CLiveLoadReactionGrid m_LegalEmergencyGrid;

public:
   virtual BOOL OnInitDialog() override;
};
