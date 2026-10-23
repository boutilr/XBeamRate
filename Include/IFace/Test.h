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

/*****************************************************************************
INTERFACE
   IXBRTest

DESCRIPTION
   Interface for performing regression tests
*****************************************************************************/
// {CE8A4F81-A616-4ed7-8A88-9982C0096B14}
DEFINE_GUID(IID_IXBRTest, 
0xce8a4f81, 0xa616, 0x4ed7, 0x8a, 0x88, 0x99, 0x82, 0xc0, 0x9, 0x6b, 0x14);
class IXBRTest
{
public:
   virtual HRESULT RunTest(PierIDType pierID,LPCTSTR lpszResultsFile) = 0;
};
