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

/****************************************************************************
CLASS
   CXBRateCommandLineInfo
****************************************************************************/
#include "stdafx.h"
#include "XBeamRateCommandLineInfo.h"

#include <system\tokenizer.h>


////////////////////////// PUBLIC     ///////////////////////////////////////

//======================== LIFECYCLE  =======================================
CXBRateCommandLineInfo::CXBRateCommandLineInfo() :
CEAFCommandLineInfo(),
m_Count(0),
m_bRegTests(false)
{
}

CXBRateCommandLineInfo::~CXBRateCommandLineInfo()
{
}

//======================== OPERATORS  =======================================
//======================== OPERATIONS =======================================
void CXBRateCommandLineInfo::ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
{
   m_Count++;

   bool bMyParameter = false;

   CString strParam(lpszParam);
   if ( bFlag )
   {
      // Parameter is a flag (-flag or /flag)
      if ( strParam.Left(3).CompareNoCase(_T("App")) == 0 )
      {
#if defined _DEBUG
         CString strAppArg = strParam.Right(strParam.GetLength() - 4);
         ATLASSERT(strAppArg.CompareNoCase(_T("XBRate")) == 0 || 
                   strAppArg.CompareNoCase(_T("PGSuper")) == 0 || 
                   strAppArg.CompareNoCase(_T("PGSplice")) == 0); 
#endif

         // the /App=<appname> option was used on the command line directing command line
         // options to us.
         // We don't have to do anything, just acknowledge it is our parameter and we are in command line mode
         m_bCommandLineMode = true;
         bMyParameter = true;
         m_Count--; // pretend the option was never used
      }
      else if ( strParam.CompareNoCase(_T("XBRTest")) == 0 )
      {
         m_bRegTests = true;
         m_bCommandLineMode = true;
         bMyParameter = true;
      }
   }

   if ( !bMyParameter )
   {
     CEAFCommandLineInfo::ParseParam(lpszParam, bFlag, bLast);
   }
}

CString CXBRateCommandLineInfo::GetUsageMessage()
{
   CString strOptions(_T("/XBRTest - Generates NCHRP 12-50 test results"));
   return strOptions;
}

CString CXBRateCommandLineInfo::GetErrorMessage()
{
   CString strMsg(_T("XBRate was started with invalid command line options."));
   return strMsg;
}
