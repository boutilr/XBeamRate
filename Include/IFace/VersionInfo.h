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
   IXBRVersionInfo

   Interface for getting version information

DESCRIPTION
   Interface for getting version information
*****************************************************************************/
// {765B1C25-B6AA-4a0d-B831-E35E4895569A}
DEFINE_GUID(IID_IXBRVersionInfo, 
0x765b1c25, 0xb6aa, 0x4a0d, 0xb8, 0x31, 0xe3, 0x5e, 0x48, 0x95, 0x56, 0x9a);
class IXBRVersionInfo
{
public:
   // Returns the version string that should be displayed in reports, etc
   // This string has the format
   // Version a.b.c.d - Built on MM/YYYY
   // The build number (.d) is omitted if bIncludeBuildNumber is false
   virtual CString GetVersionString(bool bIncludeBuildNumber = false) = 0;

   // Returns the version number as a string in the format a.b.c.d
   // The build number (.d) is omitted if bIncludeBuildNumber is false
   virtual CString GetVersion(bool bIncludeBuildNumber = false) = 0;
};

