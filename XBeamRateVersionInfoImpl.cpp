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
#include "XBeamRateVersionInfoImpl.h"
#include <MFCTools\VersionInfo.h>



CXBeamRateVersionInfoImpl::CXBeamRateVersionInfoImpl()
{
}

CString CXBeamRateVersionInfoImpl::GetVersionString(bool bIncludeBuildNumber)
{
   CString str(_T("Version "));
   str += GetVersion(bIncludeBuildNumber);
#if defined _BETA_VERSION
   str += CString(_T(" BETA"));
#endif

   str += CString(_T(" - Built on "));
   str += CString(__DATE__);
   return str;
}

CString CXBeamRateVersionInfoImpl::GetVersion(bool bIncludeBuildNumber)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CWinApp* pApp = AfxGetApp();
   CString strExe( pApp->m_pszExeName );
   strExe += _T(".dll");

   CVersionInfo verInfo;
   verInfo.Load(strExe);
   
#if defined _DEBUG || defined _BETA_VERSION
   // always include the build number in debug and beta versions
   bIncludeBuildNumber = true;
#endif
   CString strVersion = verInfo.GetProductVersionAsString(bIncludeBuildNumber);

   return strVersion;
}
