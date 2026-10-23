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

// TestAgentImp.cpp : Implementation of CTestAgentImp
#include "stdafx.h"
#include "TestAgent.h"
#include "TestAgentImp.h"

#include <EAF/AutoProgress.h>
#include <IFace\VersionInfo.h>
#include <IFace\Project.h>
#include <IFace\PointOfInterest.h>
#include <IFace\AnalysisResults.h>
#include <IFace\LoadRating.h>


bool CTestAgentImp::RegisterInterfaces()
{
   EAF_AGENT_REGISTER_INTERFACES;

   REGISTER_INTERFACE(IXBRTest);

   return true;
};

bool CTestAgentImp::Init()
{
   EAF_AGENT_INIT;

   return true;
}

bool CTestAgentImp::Reset()
{
   EAF_AGENT_RESET;
   return true;
}

CLSID CTestAgentImp::GetCLSID() const
{
   return CLSID_XBeamRateTestAgent;
}

bool CTestAgentImp::ShutDown()
{
   EAF_AGENT_SHUTDOWN;
   return true;
}

//////////////////////////////////////////
// IXBRTest
HRESULT CTestAgentImp::RunTest(PierIDType pierID,LPCTSTR lpszResultsFile)
{
   std::_tofstream ofResults;

   ofResults.open(lpszResultsFile);
   if ( !ofResults )
   {
      return E_FAIL;
   }

   // create progress window
   GET_IFACE(IEAFProgress,pProgress);
   WBFL::EAF::AutoProgress ap(pProgress);

   GET_IFACE(IXBRPointOfInterest,pPoi);
   std::vector<xbrPointOfInterest> vPoi( pPoi->GetXBeamPointsOfInterest(pierID,POI_GRID) );

   CString strProcessID = GetProcessID();

   RunProductLoadActionsTest(ofResults,strProcessID,pierID,vPoi);
   RunCombinedLoadActionsTest(ofResults,strProcessID,pierID,vPoi);
   RunLiveLoadActionsTest(ofResults,strProcessID,pierID,vPoi);
   RunLimitStateLoadActionsTest(ofResults,strProcessID,pierID,vPoi);
   RunMomentCapacityTest(ofResults,strProcessID,pierID,vPoi);
   RunShearCapacityTest(ofResults,strProcessID,pierID,vPoi);
   RunCrackedSectionTest(ofResults,strProcessID,pierID,vPoi);
   RunLoadRatingTest(ofResults,strProcessID,pierID);

   return S_OK;
}

CString CTestAgentImp::GetProcessID()
{
   //// the process ID is going to be the version number
   //GET_IFACE(IXBRVersionInfo,pVI);
   //return pVI->GetVersion(true);

   // we stopped using the version number as the process ID
   // because we never compared results from one process to another
   // as described in the 12-50 process. Since the move to 
   // GIT VCS, merging files is difficult and it is especially difficult
   // with reg test results that have the version number encoded in it
   //
   // now using a "generic" number for the process id
   return CString(_T("0.0.0.0"));
}

void CTestAgentImp::RunProductLoadActionsTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi)
{
   GET_IFACE(IXBRAnalysisResults,pResults);

   for ( int i = 0; i < 8; i++ )
   {
      xbrTypes::ProductForceType pfType = (xbrTypes::ProductForceType)i;

      CString strMomentTestID;
      strMomentTestID.Format(_T("7770%d01"),i);

      CString strShearTestID;
      strShearTestID.Format(_T("7770%d02"),i);

      for (const auto& poi : vPoi)
      {
         Float64 M = pResults->GetMoment(pierID,pfType,poi);
         os << lpszProcessID << _T(", ") << (LPCTSTR)strMomentTestID << _T(", ") << poi.GetDistFromStart() << _T(", ") << M << std::endl;

         WBFL::System::SectionValue V = pResults->GetShear(pierID,pfType,poi);
         os << lpszProcessID << _T(", ") << (LPCTSTR)strShearTestID << _T(", ") << poi.GetDistFromStart() << _T(", ") << V.Right() << std::endl;
      }
   }
}

void CTestAgentImp::RunCombinedLoadActionsTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi)
{
   GET_IFACE(IXBRAnalysisResults,pResults);

   for ( int i = 0; i < 6; i++ )
   {
      xbrTypes::CombinedForceType cfType = (xbrTypes::CombinedForceType)i;

      CString strMomentTestID;
      strMomentTestID.Format(_T("7771%d01"),i);

      CString strShearTestID;
      strShearTestID.Format(_T("7771%d02"),i);

      for (const auto& poi : vPoi)
      {
         Float64 M = pResults->GetMoment(pierID,cfType,poi);
         os << lpszProcessID << _T(", ") << (LPCTSTR)strMomentTestID << _T(", ") << poi.GetDistFromStart() << _T(", ") << M << std::endl;

         WBFL::System::SectionValue V = pResults->GetShear(pierID,cfType,poi);
         os << lpszProcessID << _T(", ") << (LPCTSTR)strShearTestID << _T(", ") << poi.GetDistFromStart() << _T(", ") << V.Right() << std::endl;
      }
   }
}

void CTestAgentImp::RunLiveLoadActionsTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi)
{
   GET_IFACE(IXBRAnalysisResults,pResults);

   int n = (int)pgsTypes::lrLoadRatingTypeCount;
   for ( int i = 0; i < n; i++ )
   {
      pgsTypes::LoadRatingType ratingType = (pgsTypes::LoadRatingType)i;

      CString strMomentTestID;
      strMomentTestID.Format(_T("7772%d01"),i);

      CString strShearTestID;
      strShearTestID.Format(_T("7772%d02"),i);

      for (const auto& poi : vPoi)
      {
         Float64 Mmin,Mmax;
         VehicleIndexType minVehicleIdx, maxVehicleIdx;
         pResults->GetMoment(pierID,ratingType,poi,&Mmin,&Mmax,&minVehicleIdx,&maxVehicleIdx);
         os << lpszProcessID << _T(", ") << (LPCTSTR)strMomentTestID << _T("a, ") << poi.GetDistFromStart() << _T(", ") << Mmin << _T(", ") << minVehicleIdx << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)strMomentTestID << _T("b, ") << poi.GetDistFromStart() << _T(", ") << Mmax << _T(", ") << maxVehicleIdx << std::endl;

         WBFL::System::SectionValue Vmin, Vmax;
         pResults->GetShear(pierID,ratingType,poi,&Vmin,&Vmax,nullptr,&minVehicleIdx,nullptr,&maxVehicleIdx);
         os << lpszProcessID << _T(", ") << (LPCTSTR)strShearTestID << _T("a, ") << poi.GetDistFromStart() << _T(", ") << Vmin.Right() << _T(", ") << minVehicleIdx << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)strShearTestID << _T("b, ") << poi.GetDistFromStart() << _T(", ") << Vmax.Right() << _T(", ") << maxVehicleIdx << std::endl;
      }
   }
}

void CTestAgentImp::RunLimitStateLoadActionsTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi)
{
   GET_IFACE(IXBRAnalysisResults,pResults);

   std::vector<pgsTypes::LimitState> vLimitStates;
   vLimitStates.push_back(pgsTypes::StrengthI_Inventory);
   vLimitStates.push_back(pgsTypes::StrengthI_Operating);
   vLimitStates.push_back(pgsTypes::StrengthI_LegalRoutine);
   vLimitStates.push_back(pgsTypes::StrengthI_LegalSpecial);
   vLimitStates.push_back(pgsTypes::StrengthII_PermitRoutine);
   vLimitStates.push_back(pgsTypes::ServiceI_PermitRoutine);
   vLimitStates.push_back(pgsTypes::StrengthII_PermitSpecial);
   vLimitStates.push_back(pgsTypes::ServiceI_PermitSpecial);

   int i = 0;
   for (const auto& limitState : vLimitStates)
   {
      CString strMomentTestID;
      strMomentTestID.Format(_T("7773%d01"),i);

      CString strShearTestID;
      strShearTestID.Format(_T("7773%d02"),i);

      for (const auto& poi : vPoi)
      {
         Float64 Mmin,Mmax;
         pResults->GetMoment(pierID,limitState,poi,&Mmin,&Mmax);
         os << lpszProcessID << _T(", ") << (LPCTSTR)strMomentTestID << _T("a, ") << poi.GetDistFromStart() << _T(", ") << Mmin << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)strMomentTestID << _T("b, ") << poi.GetDistFromStart() << _T(", ") << Mmax << std::endl;

         WBFL::System::SectionValue Vmin, Vmax;
         pResults->GetShear(pierID,limitState,poi,&Vmin,&Vmax);
         os << lpszProcessID << _T(", ") << (LPCTSTR)strShearTestID << _T("a, ") << poi.GetDistFromStart() << _T(", ") << Vmin.Right() << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)strShearTestID << _T("b, ") << poi.GetDistFromStart() << _T(", ") << Vmax.Right() << std::endl;
      }

      i++;
   }
}

void CTestAgentImp::RunMomentCapacityTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi)
{
   GET_IFACE(IXBRMomentCapacity,pMomentCapacity);

   std::vector<pgsTypes::LimitState> vLimitStates;
   vLimitStates.push_back(pgsTypes::StrengthI_Inventory);
   vLimitStates.push_back(pgsTypes::StrengthI_Operating);
   vLimitStates.push_back(pgsTypes::StrengthI_LegalRoutine);
   vLimitStates.push_back(pgsTypes::StrengthI_LegalSpecial);
   vLimitStates.push_back(pgsTypes::StrengthII_PermitRoutine);
   vLimitStates.push_back(pgsTypes::StrengthII_PermitSpecial);

   for ( int i = 0; i < 2; i++ )
   {
      bool bPositiveMoment = (i == 0 ? true : false);

      CString strMomentCapacityBaseTestID;
      strMomentCapacityBaseTestID.Format(_T("7774%d"),i);

      for (const auto& poi : vPoi)
      {
         const MomentCapacityDetails& mcd = pMomentCapacity->GetMomentCapacityDetails(pierID,xbrTypes::Stage2,poi,bPositiveMoment);
         const CrackingMomentDetails& cmd = pMomentCapacity->GetCrackingMomentDetails(pierID,xbrTypes::Stage2,poi,bPositiveMoment);

         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("00, ")) << poi.GetDistFromStart() << _T(", ") << mcd.dt << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("01, ")) << poi.GetDistFromStart() << _T(", ") << mcd.de << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("02, ")) << poi.GetDistFromStart() << _T(", ") << mcd.dc << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("03, ")) << poi.GetDistFromStart() << _T(", ") << mcd.Mr << std::endl;

         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("04, ")) << poi.GetDistFromStart() << _T(", ") << cmd.Mcr << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("05, ")) << poi.GetDistFromStart() << _T(", ") << cmd.Mdnc << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("06, ")) << poi.GetDistFromStart() << _T(", ") << cmd.fr << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("07, ")) << poi.GetDistFromStart() << _T(", ") << cmd.fcpe << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("08, ")) << poi.GetDistFromStart() << _T(", ") << cmd.Snc << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("09, ")) << poi.GetDistFromStart() << _T(", ") << cmd.Sc << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("10, ")) << poi.GetDistFromStart() << _T(", ") << cmd.McrLimit << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("11, ")) << poi.GetDistFromStart() << _T(", ") << cmd.g1 << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("12, ")) << poi.GetDistFromStart() << _T(", ") << cmd.g2 << std::endl;
         os << lpszProcessID << _T(", ") << (LPCTSTR)(strMomentCapacityBaseTestID+_T("13, ")) << poi.GetDistFromStart() << _T(", ") << cmd.g3 << std::endl;

         for (const auto& limitState : vLimitStates)
         {
            CString strBaseTestID;
            strBaseTestID.Format(_T("%s%d"),strMomentCapacityBaseTestID,(int)limitState);
            const MinMomentCapacityDetails& mmcd = pMomentCapacity->GetMinMomentCapacityDetails(pierID,limitState,xbrTypes::Stage2,poi,bPositiveMoment);
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("0, ")) << poi.GetDistFromStart() << _T(", ") << mmcd.Mr << std::endl;
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("1, ")) << poi.GetDistFromStart() << _T(", ") << mmcd.Mcr << std::endl;
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("2, ")) << poi.GetDistFromStart() << _T(", ") << mmcd.MrMin1 << std::endl;
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("3, ")) << poi.GetDistFromStart() << _T(", ") << mmcd.MrMin2 << std::endl;
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("4, ")) << poi.GetDistFromStart() << _T(", ") << mmcd.MrMin << std::endl;
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("5, ")) << poi.GetDistFromStart() << _T(", ") << mmcd.Mu << std::endl;
         }
      }
   }
}

void CTestAgentImp::RunShearCapacityTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi)
{
   GET_IFACE(IXBRShearCapacity,pShearCapacity);

   CString strShearCapacityBaseTestID(_T("77750"));

   for (const auto& poi : vPoi)
   {
      const ShearCapacityDetails& scd = pShearCapacity->GetShearCapacityDetails(pierID,xbrTypes::Stage2,poi);
      const AvOverSDetails& AvSDetails = pShearCapacity->GetAverageAvOverSDetails(pierID,xbrTypes::Stage2,poi);
      const DvDetails& dvDetails = pShearCapacity->GetDvDetails(pierID,xbrTypes::Stage2,poi);

      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("00, ")) << poi.GetDistFromStart() << _T(", ") << scd.beta << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("01, ")) << poi.GetDistFromStart() << _T(", ") << scd.theta << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("02, ")) << poi.GetDistFromStart() << _T(", ") << scd.bv << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("03, ")) << poi.GetDistFromStart() << _T(", ") << scd.dv1 << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("04, ")) << poi.GetDistFromStart() << _T(", ") << scd.dv2 << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("05, ")) << poi.GetDistFromStart() << _T(", ") << scd.dv << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("06, ")) << poi.GetDistFromStart() << _T(", ") << scd.Av_over_S1 << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("07, ")) << poi.GetDistFromStart() << _T(", ") << scd.Av_over_S2 << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("08, ")) << poi.GetDistFromStart() << _T(", ") << scd.Vc << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("09, ")) << poi.GetDistFromStart() << _T(", ") << scd.Vs << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("10, ")) << poi.GetDistFromStart() << _T(", ") << scd.Vn1 << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("11, ")) << poi.GetDistFromStart() << _T(", ") << scd.Vn2 << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("12, ")) << poi.GetDistFromStart() << _T(", ") << scd.Vn << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("13, ")) << poi.GetDistFromStart() << _T(", ") << scd.Vr << std::endl;

      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("14, ")) << poi.GetDistFromStart() << _T(", ") << AvSDetails.ShearFailurePlaneLength << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("15, ")) << poi.GetDistFromStart() << _T(", ") << AvSDetails.AvgAvOverS << std::endl;

      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("15, ")) << poi.GetDistFromStart() << _T(", ") << dvDetails.h << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("16, ")) << poi.GetDistFromStart() << _T(", ") << dvDetails.de[0] << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("17, ")) << poi.GetDistFromStart() << _T(", ") << dvDetails.de[1] << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("18, ")) << poi.GetDistFromStart() << _T(", ") << dvDetails.MomentArm[0] << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("19, ")) << poi.GetDistFromStart() << _T(", ") << dvDetails.MomentArm[1] << std::endl;
      os << lpszProcessID << _T(", ") << (LPCTSTR)(strShearCapacityBaseTestID+_T("20, ")) << poi.GetDistFromStart() << _T(", ") << dvDetails.dv << std::endl;
   }
}

void CTestAgentImp::RunCrackedSectionTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi)
{
   GET_IFACE(IXBRCrackedSection,pCrackedSection);

   for ( int i = 0; i < 2; i++ )
   {
      bool bPositiveMoment = (i == 0 ? true : false);

      for ( int j = 0; j < 2; j++ )
      {
         xbrTypes::LoadType loadType = (xbrTypes::LoadType)j;

         CString strBaseTestID;
         strBaseTestID.Format(_T("7776%d%d"),i,j);

         for (const auto& poi : vPoi)
         {
            const CrackedSectionDetails& csd = pCrackedSection->GetCrackedSectionDetails(pierID,xbrTypes::Stage2,poi,bPositiveMoment,loadType);

            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("0, ")) << poi.GetDistFromStart() << _T(", ") << csd.n << std::endl;
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("1, ")) << poi.GetDistFromStart() << _T(", ") << csd.c << std::endl;
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("2, ")) << poi.GetDistFromStart() << _T(", ") << csd.Ycr << std::endl;
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("3, ")) << poi.GetDistFromStart() << _T(", ") << csd.Icr << std::endl;
         }
      }
   }
}

void CTestAgentImp::RunLoadRatingTest(std::_tostream& os,LPCTSTR lpszProcessID,PierIDType pierID)
{
   GET_IFACE(IXBRProject,pProject);
   GET_IFACE(IXBRArtifact,pArtifact);

   int n = (int)pgsTypes::lrLoadRatingTypeCount;
   for ( int i = 0; i < n; i++ )
   {
      pgsTypes::LoadRatingType ratingType = (pgsTypes::LoadRatingType)i;

     std::_tstring strName = pProject->GetLiveLoadName(pierID,ratingType,0);

      if ( strName == NO_LIVE_LOAD_DEFINED )
      {
         continue;
      }

      VehicleIndexType nVehicles = pProject->GetLiveLoadReactionCount(pierID,ratingType);
      for ( VehicleIndexType vehicleIdx = 0; vehicleIdx < nVehicles; vehicleIdx++ )
      {
         const xbrRatingArtifact* pRatingArtifact = pArtifact->GetXBeamRatingArtifact(pierID,ratingType,vehicleIdx);

         CString strBaseTestID;
         strBaseTestID.Format(_T("7777%d"),i);

         const xbrMomentRatingArtifact *pm, *nm;
         const xbrShearRatingArtifact *v;
         const xbrYieldStressRatioArtifact *pys, *nys;
         Float64 RF = pRatingArtifact->GetRatingFactorEx(&pm,&nm,&v,&pys,&nys);

         os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("00, ")) << RF << std::endl;
         if ( pm )
         {
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("01, ")) << pm->GetPointOfInterest().GetDistFromStart() << std::endl;
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("02, ")) << 0 << std::endl;
         }

         if ( nm )
         {
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("01, ")) << nm->GetPointOfInterest().GetDistFromStart() << std::endl;
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("02, ")) << 1 << std::endl;
         }

         if ( v )
         {
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("01, ")) << v->GetPointOfInterest().GetDistFromStart() << std::endl;
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("02, ")) << 2 << std::endl;
         }

         if ( pys )
         {
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("01, ")) << pys->GetPointOfInterest().GetDistFromStart() << std::endl;
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("02, ")) << 3 << std::endl;
         }

         if ( nys )
         {
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("01, ")) << nys->GetPointOfInterest().GetDistFromStart() << std::endl;
            os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("02, ")) << 4 << std::endl;
         }

         os << lpszProcessID << _T(", ") << (LPCTSTR)(strBaseTestID+_T("03, ")) << vehicleIdx << std::endl;
      }
   }
}
