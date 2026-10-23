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
   IXBeamRateAgent

DESCRIPTION
   This is a dummy interface used to determining if XBeam Rate is acting
   as a stand alone application or an extension to PGSuper/PGSplice.
   This interface is implemented on XBeamRateAgent. If a request for this
   interface fails, the XBeam Rate is acting as a stand alone application.
*****************************************************************************/
// {D07B5589-24FA-48b3-8C15-1371C80353AF}
DEFINE_GUID(IID_IXBeamRateAgent, 
0xd07b5589, 0x24fa, 0x48b3, 0x8c, 0x15, 0x13, 0x71, 0xc8, 0x3, 0x53, 0xaf);
class IXBeamRateAgent
{
public:
   // always returns true. If this interface is implemented
   // XBeam Rate is an extension to PGSuper/PGSplice
   virtual bool IsExtendingPGSuper() = 0;
};

// IXBRViews
// {49B3DFCF-5611-42d7-8B19-A1FC52778846}
DEFINE_GUID(IID_IXBRViews, 
0x49b3dfcf, 0x5611, 0x42d7, 0x8b, 0x19, 0xa1, 0xfc, 0x52, 0x77, 0x88, 0x46);
class IXBRViews
{
public:
   virtual void CreateReportView(IndexType rptIdx,BOOL bPromptForSpec=TRUE) = 0;
   virtual void CreateGraphView(IndexType graphIdx) = 0;
   virtual void CreatePierView() = 0;
};

// IXBREditByUI
// {C0FA29DA-CCA6-4a49-A21C-16F97DA3DED4}
DEFINE_GUID(IID_IXBREditByUI, 
0xc0fa29da, 0xcca6, 0x4a49, 0xa2, 0x1c, 0x16, 0xf9, 0x7d, 0xa3, 0xde, 0xd4);
class IXBREditByUI
{
public:
   virtual UINT GetStdToolBarID() = 0;
};
