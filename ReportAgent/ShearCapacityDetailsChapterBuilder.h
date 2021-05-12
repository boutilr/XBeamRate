///////////////////////////////////////////////////////////////////////
// XBeamRate - Cross Beam Load Rating
// Copyright � 1999-2021  Washington State Department of Transportation
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

#include <ReportManager\ChapterBuilder.h>
#include "XBeamRateChapterBuilder.h"

/*****************************************************************************
CLASS 
   CTestChapterBuilder

*****************************************************************************/

class CShearCapacityDetailsChapterBuilder : public CXBeamRateChapterBuilder
{
public:
   CShearCapacityDetailsChapterBuilder();
   virtual LPCTSTR GetName() const override;
   rptChapter* Build(CReportSpecification* pRptSpec,Uint16 level) const;
   virtual CChapterBuilder* Clone() const override;

private:
   // Prevent accidental copying and assignment
   CShearCapacityDetailsChapterBuilder(const CShearCapacityDetailsChapterBuilder&) = delete;
   CShearCapacityDetailsChapterBuilder& operator=(const CShearCapacityDetailsChapterBuilder&) = delete;
};
