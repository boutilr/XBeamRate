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

// AnalysisAgentImp.cpp : Implementation of CAnalysisAgentImp
#include "stdafx.h"
#include "AnalysisAgent.h"
#include "AnalysisAgentImp.h"

#include <IFace\Pier.h>
#include <IFace\PointOfInterest.h>
#include <IFace\RatingSpecification.h>
#include <EAF\EAFDisplayUnits.h>

#include <Units\System.h>
#include <MFCTools\Format.h>

#include <EAF/AutoProgress.h>
#include <XBeamRateExt\XBeamRateUtilities.h>
#include <XBeamRateExt\StatusItem.h>

#include <PsgLib\GirderLabel.h>
#include <psgLib/RatingLibraryEntry.h>

#include <numeric>
#include <algorithm>

#include <System\Flags.h>
#include <LRFD\Utility.h>

#include <System\FileStream.h>
#include <System\StructuredSaveXml.h>


#define MAX_CASES 10

/////////////////////////////////////////////////////////////////////////////
// NOTE: Any time you get live load results directly from the FEM model,
// apply the multiple presence factor.
/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// IAgent

bool CAnalysisAgentImp::Init()
{
   EAF_AGENT_INIT; // this macro defines pStatusCenter

   m_pModelData = std::make_unique<std::map<PierIDType, ModelData>>();
   m_pUnitLiveLoadResults = std::make_unique<std::map<PierIDType, std::set<UnitLiveLoadResult> > >();

   GET_IFACE(IEAFStatusCenter, pStatusCenter);
   m_StatusGroupID = pStatusCenter->CreateStatusGroupID();

   // Register status callbacks that we want to use
   m_scidBridgeWarning = pStatusCenter->RegisterCallback(std::make_shared<xbrBridgeStatusCallback>(WBFL::EAF::StatusSeverityType::Warning));
   m_scidBridgeError = pStatusCenter->RegisterCallback(std::make_shared<xbrBridgeStatusCallback>(WBFL::EAF::StatusSeverityType::Error));

   //
   // Attach to connection points
   //
   m_dwProjectCookie = REGISTER_EVENT_SINK(IXBRProjectEventSink);
   m_dwBridgeDescCookie = REGISTER_EVENT_SINK(IBridgeDescriptionEventSink);

   return true;
}

bool CAnalysisAgentImp::RegisterInterfaces()
{
   EAF_AGENT_REGISTER_INTERFACES;
   REGISTER_INTERFACE(IXBRProductForces);
   REGISTER_INTERFACE(IXBRAnalysisResults);

   return true;
};

bool CAnalysisAgentImp::Reset()
{
   EAF_AGENT_RESET;
   Invalidate(false);
   return true;
}

bool CAnalysisAgentImp::ShutDown()
{
   EAF_AGENT_SHUTDOWN;

   //
   // Detach to connection points
   //
   UNREGISTER_EVENT_SINK(IXBRProjectEventSink, m_dwProjectCookie);
   UNREGISTER_EVENT_SINK(IBridgeDescriptionEventSink, m_dwBridgeDescCookie);

   return S_OK;
}

CLSID CAnalysisAgentImp::GetCLSID() const
{
   return CLSID_XBeamRateAnalysisAgent;
}

IndexType CAnalysisAgentImp::GetPriority() const
{
   return 0;
}

CAnalysisAgentImp::ModelData* CAnalysisAgentImp::GetModelData(PierIDType pierID,int level) const
{
   BuildModel(pierID,level); // builds or updates the model if necessary
   std::map<PierIDType,ModelData>::iterator found = m_pModelData->find(pierID);
   ATLASSERT( found != m_pModelData->end() ); // should always find it!
   ModelData* pModelData = &(*found).second;
   return pModelData;
}

#define BEARING 0x0001
#define COLUMN  0x0002
#define CURB    0x0004
struct XBeamNode
{
   Float64 X;
   Int32 Type;
   JointIDType jntID;
   bool operator<(const XBeamNode& other)const { return IsLT(X,other.X,0.005); }
   bool operator==(const XBeamNode& other)const { return IsEqual(X,other.X,0.005); }
};

void CAnalysisAgentImp::BuildModel(PierIDType pierID,int level) const
{
   CanModelPier(pierID,m_StatusGroupID,m_scidBridgeError); // if this is not the kind of pier we can model, an Unwind exception will be thrown

   std::map<PierIDType,ModelData>::iterator found = m_pModelData->find(pierID);
   if ( found == m_pModelData->end() )
   {
      ModelData model_data;
      m_pModelData->insert( std::make_pair(pierID,std::move(model_data)) );
      found = m_pModelData->find(pierID);
   }

   ModelData* pModelData = &(found->second);

   if ( level <= pModelData->m_InitLevel )
   {
      return;
   }

   GET_IFACE(IEAFProgress,pProgress);
   WBFL::EAF::AutoProgress ap(pProgress);

   if ( MODEL_INIT_TOPOLOGY <= level && pModelData->m_InitLevel < MODEL_INIT_TOPOLOGY )
   {
      pModelData->m_Model = std::make_unique<WBFL::FEA2D::Model>();
      pModelData->m_Model->SetName(_T("XBeamRate"));

      // Build the frame model
      GET_IFACE(IXBRProject,pProject);
      GET_IFACE(IXBRPier,pPier);
      GET_IFACE(IXBRMaterial,pMaterial);
      GET_IFACE(IXBRSectionProperties,pSectProp);

      pProgress->UpdateMessage(_T("Building pier analysis model"));

      Float64 LCO, RCO;
      pProject->GetCurbLineOffset(pierID,&LCO,&RCO);

      IndexType nColumns = pProject->GetColumnCount(pierID);

      Float64 L = pPier->GetXBeamLength(xbrTypes::xblBottomXBeam, pierID);

      pModelData->m_Lmax = L;

      // Get the location of all the cross beam nodes

      // beginning/end of cross beam
      std::vector<XBeamNode> vXBeamNodes;
      XBeamNode n;
      n.X = 0;
      n.Type = 0;
      vXBeamNodes.push_back(n);

      n.X = L;
      n.Type = 0;
      vXBeamNodes.push_back(n);

      //// curb line
      //n.X = pPier->ConvertPierToCrossBeamCoordinate(pierID,LCO);
      //n.Type = CURB;
      //vXBeamNodes.push_back(n);

      //n.X = pPier->ConvertPierToCrossBeamCoordinate(pierID,RCO);
      //n.Type = CURB;
      //vXBeamNodes.push_back(n);

      // top of columns
      for ( ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++ )
      {
         n.X = pPier->GetColumnLocation(pierID,colIdx);
         n.Type = COLUMN;
         vXBeamNodes.push_back(n);
      }

      // bearings
      // if reactions are applied through bearings, we need a bearing transfer model
      // the first step for that model is FEM2D nodes at the bearing locations
      if ( pProject->GetReactionLoadApplicationType(pierID) == xbrTypes::rlaBearings )
      {
         IndexType nBearingLines = pPier->GetBearingLineCount(pierID);
         for ( IndexType brgLineIdx = 0; brgLineIdx < nBearingLines; brgLineIdx++ )
         {
            IndexType nBearings = pPier->GetBearingCount(pierID,brgLineIdx);
            for ( IndexType brgIdx = 0; brgIdx < nBearings; brgIdx++ )
            {
               n.X = pPier->GetBearingLocation(pierID,brgLineIdx,brgIdx);
               n.Type = BEARING;
               vXBeamNodes.push_back(n);
            }
         }
      }

      // sort in left-to-right order
      std::sort(vXBeamNodes.begin(),vXBeamNodes.end());

      // eliminate duplicates... if or more nodes are at the same location, merge the Type
      // attribute and eliminate the redundant node record
      std::vector<XBeamNode>::iterator result = std::adjacent_find(vXBeamNodes.begin(),vXBeamNodes.end());
      while ( result != vXBeamNodes.end() )
      {
         XBeamNode& n1 = *result;
         XBeamNode& n2 = *(result+1);
         n1.Type |= n2.Type;
         vXBeamNodes.erase(result+1);
         result = std::adjacent_find(vXBeamNodes.begin(),vXBeamNodes.end());
      }

      // Get properties
      Float64 Exb = pMaterial->GetXBeamEc(pierID);
      Float64 Axb = pSectProp->GetArea(pierID,xbrTypes::Stage2,xbrPointOfInterest(INVALID_ID,L/2));
      Float64 Ixb = pSectProp->GetIxx(pierID,xbrTypes::Stage2,xbrPointOfInterest(INVALID_ID,L/2));
      Float64 EAb = Exb*Axb;
      Float64 EIb = Exb*Ixb;

      // build the model
      WBFL::FEA2D::Model& femModel = *pModelData->m_Model;


      JointIDType jntID = 0;
      MemberIDType xbeamMbrID = 0;
      MemberIDType columnMbrID = -1;

      std::vector<XBeamNode>::iterator iter(vXBeamNodes.begin());
      std::vector<XBeamNode>::iterator end(vXBeamNodes.end());

      XBeamNode* pPrevNode = &(*iter);;
      JointIDType prevJointID = jntID++;
      pPrevNode->jntID = prevJointID;
      femModel.CreateJoint(prevJointID,pPrevNode->X,0);
      ColumnIndexType colIdx = 0;

      iter++;
      for ( ; iter != end; iter++ )
      {
         XBeamNode* pThisNode = &(*iter);
         JointIDType thisJointID = jntID++;
         pThisNode->jntID = thisJointID;

         femModel.CreateJoint(thisJointID,pThisNode->X,0);

         femModel.CreateMember(xbeamMbrID++,prevJointID,thisJointID,EAb,EIb);
         BeamMember capMbr;
         capMbr.Xs = pPrevNode->X;
         capMbr.Xe = pThisNode->X;
         capMbr.mbrID = xbeamMbrID-1;
         pModelData->m_XBeamMembers.push_back(capMbr);

         if ( WBFL::System::Flags<Int32>::IsSet(pThisNode->Type,COLUMN) )
         {
            Float64 columnHeight = pPier->GetColumnHeight(pierID,colIdx);

            // create joint at bottom of column
            WBFL::FEA2D::Joint& joint = femModel.CreateJoint(jntID++,pThisNode->X,-columnHeight);


            pgsTypes::ColumnTransverseFixityType columnFixity = pPier->GetColumnFixity(pierID,colIdx);
            joint.Support(); // fully fixed
            if ( columnFixity == pgsTypes::ctftTopFixedBottomPinned )
            {
               joint.ReleaseDof(WBFL::FEA2D::JointReleaseType::Mz);
            }

            // create column member
            Float64 Ecol = pMaterial->GetColumnEc(pierID,colIdx);
            Float64 Acol = pSectProp->GetArea(pierID,xbrTypes::Stage2,xbrPointOfInterest(INVALID_ID,colIdx,0.0));
            Float64 Icol = pSectProp->GetIyy(pierID,xbrTypes::Stage2,xbrPointOfInterest(INVALID_ID,colIdx,0.0));
            WBFL::FEA2D::Member& mbr = femModel.CreateMember(columnMbrID--,thisJointID,jntID-1,Ecol*Acol,Ecol*Icol);


            // Release top end of member, if specified
            if (columnFixity == pgsTypes::ctftTopPinnedBottomFixed)
            {
               mbr.ReleaseEnd(WBFL::FEA2D::MemberEndType::Start, WBFL::FEA2D::MemberReleaseType::Mz);
            }

            colIdx++;
         }

         pPrevNode = pThisNode;
         prevJointID = thisJointID;
      }

      // create the "superstructure" model upon which we will run the live load
      if ( pProject->GetReactionLoadApplicationType(pierID) == xbrTypes::rlaCrossBeam )
      {
         // live load is applied directly to the cross beam... the superstructure members
         // and the XBeam members are one in the same
         pModelData->m_SuperstructureMembers = pModelData->m_XBeamMembers;
      }
      else
      {
         // live load is applied to a load transfer model.

         // dummy properties of the transfer model
         //Float64 Y = WBFL::Units::ConvertToSysUnits(1.0,WBFL::Units::Measure::Inch); // offset the transfer model a small distance above the XBeam model

         // Live load is on the deck, so the height of the transfer model is the height of the superstructure diaphragm plus the deck thickness
         Float64 H,W;
         pProject->GetDiaphragmDimensions(pierID,&H,&W);
         Float64 tSlab = pProject->GetDeckThickness(pierID);
         Float64 Y = H + tSlab;

         if (IsZero(Y))
         {
            // monolithic piers are sometimes modeled with just the lower cross beam. The upper cross beam
            // height is set to 0.0. In this case, use the lower cross beam dimensions to get some sort of reasonable
            // offset for the live load transfer model
            Float64 H1, H2, H3, H4, X1, X2, X3, X4, W;
            pProject->GetLowerXBeamDimensions(pierID, &H1, &H2, &H3, &H4, &X1, &X2, &X3, &X4, &W);
            Y = Max(H1 + H2, H3 + H4);
         }

         Float64 EI = EIb/10000; // use members that are considerably less stiff than the XBeam members (we don't want to attract the dead load into this transfer model)
         Float64 EA = EAb/10000;

         std::vector<XBeamNode>::iterator iter(vXBeamNodes.begin());
         std::vector<XBeamNode>::iterator end(vXBeamNodes.end());

         XBeamNode* pPrevNode = &(*iter);
         JointIDType prevJointID = jntID++;
         femModel.CreateJoint(prevJointID,pPrevNode->X,Y);

         iter++;
         for ( ; iter != end; iter++ )
         {
            XBeamNode* pThisNode = &(*iter);
            JointIDType thisJointID = jntID++;

            femModel.CreateJoint(thisJointID,pThisNode->X,Y);

            femModel.CreateMember(xbeamMbrID++,prevJointID,thisJointID,EAb,EIb);
            BeamMember ssMbr;
            ssMbr.Xs = pPrevNode->X;
            ssMbr.Xe = pThisNode->X;
            ssMbr.mbrID = xbeamMbrID-1;
            pModelData->m_SuperstructureMembers.push_back(ssMbr);

            if ( WBFL::System::Flags<Int32>::IsSet(pThisNode->Type,BEARING) )
            {
               WBFL::FEA2D::Member& mbr = femModel.CreateMember(columnMbrID--,thisJointID,pThisNode->jntID,EA,EI);


               mbr.ReleaseEnd(WBFL::FEA2D::MemberEndType::End, WBFL::FEA2D::MemberReleaseType::Mz);
            }

            pPrevNode = pThisNode;
            prevJointID = thisJointID;
         }
      }

      ComputeLiveLoadLocations(pierID,pModelData);

      pModelData->m_InitLevel = MODEL_INIT_TOPOLOGY;
   }

   if ( MODEL_INIT_LOADS <= level && pModelData->m_InitLevel < MODEL_INIT_LOADS )
   {
      WBFL::FEA2D::Model& femModel = *pModelData->m_Model;


      ApplyWheelLineLoadsToFemModel(pModelData);
      ApplyDeadLoad(pierID,pModelData);

      // Assign POIs
      PoiIDType femPoiID = 0;

      GET_IFACE(IXBRPointOfInterest,pPoi);
      std::vector<xbrPointOfInterest> vPoi = pPoi->GetXBeamPointsOfInterest(pierID);
      for (const auto& poi : vPoi)
      {
         MemberIDType mbrID;
         Float64 mbrLocation;
         GetFemModelLocation(pModelData,poi,&mbrID,&mbrLocation);

         femModel.CreatePOI(femPoiID,mbrID,mbrLocation);
         pModelData->m_PoiMap.insert(std::make_pair(poi.GetID(),femPoiID));
         femPoiID++;
      }

      pModelData->m_InitLevel = MODEL_INIT_LOADS;

#if defined _DEBUG || defined _BETA_VERSION
      WBFL::System::FileStream file;
      file.open(_T("XBeamRate_Fem2d.xml"), /*read=*/false);
      WBFL::System::StructuredSaveXml save;
      save.BeginSave(&file);
      femModel.Save(&save);
      save.EndSave();
#endif // _DEBUG
   }
}

void CAnalysisAgentImp::ApplyDeadLoad(PierIDType pierID,ModelData* pModelData) const
{
   ApplyLowerXBeamDeadLoad(pierID,pModelData);
   ApplyUpperXBeamDeadLoad(pierID,pModelData);
   ApplySuperstructureDeadLoadReactions(pierID,pModelData);
}

void CAnalysisAgentImp::ApplyLowerXBeamDeadLoad(PierIDType pierID,ModelData* pModelData) const
{
   ValidateLowerXBeamDeadLoad(pierID,pModelData);

   WBFL::FEA2D::Model& femModel = *pModelData->m_Model;


   LoadCaseIDType loadCaseID = GetLoadCaseID(xbrTypes::pftLowerXBeam);
   WBFL::FEA2D::Loading& loading = femModel.CreateLoading(loadCaseID);


   LoadIDType loadID = 0;
   std::vector<LowerXBeamLoad>::iterator iter(pModelData->m_LowerXBeamLoads.begin());
   std::vector<LowerXBeamLoad>::iterator end(pModelData->m_LowerXBeamLoads.end());
   for ( ; iter != end; iter++ )
   {
      LowerXBeamLoad& load(*iter);

      MemberIDType startMbrID, endMbrID;
      Float64 startMbrLocation , endMbrLocation;
      GetFemModelLocation(pModelData,xbrPointOfInterest(INVALID_ID,load.Xs),&startMbrID,&startMbrLocation);
      GetFemModelLocation(pModelData,xbrPointOfInterest(INVALID_ID,load.Xe),&endMbrID,  &endMbrLocation);

      if ( startMbrID == endMbrID )
      {
         loading.CreateDistributedLoad(loadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startMbrLocation,endMbrLocation,-load.Ws,-load.We);
      }
      else
      {
         // load from start to end of first member
         WBFL::FEA2D::Member* mbr = femModel.FindMember(startMbrID);

         Float64 L = mbr->GetLength();

         JointIDType jntID = mbr->GetEndJoint();
         WBFL::FEA2D::Joint* joint = femModel.FindJoint(jntID);
         Float64 X = joint->GetX();

         Float64 Ws = load.Ws;
         Float64 We = ::LinInterp(X - load.Xs,load.Ws,load.We,load.Xe - load.Xs);

         if ( !IsEqual(startMbrLocation,L) )
         {
            loading.CreateDistributedLoad(loadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startMbrLocation,L,-Ws,-We);
         }

         Ws = We;

         // load all intermediate members
         for ( MemberIDType mbrID = startMbrID+1; mbrID < endMbrID; mbrID++ )
         {
            mbr = femModel.FindMember(mbrID);
            jntID = mbr->GetEndJoint();
            joint = femModel.FindJoint(jntID);
            X = joint->GetX();

            L = mbr->GetLength();

            We = ::LinInterp(X - load.Xs,load.Ws,load.We,load.Xe - load.Xs);

            loading.CreateDistributedLoad(loadID++,mbrID,WBFL::FEA2D::LoadDirection::Fy,0,L,-Ws,-We);

            Ws = We;
         }

         // load start of last member to the end of the loading
         if ( !IsZero(endMbrLocation) )
         {
            loading.CreateDistributedLoad(loadID++,endMbrID,WBFL::FEA2D::LoadDirection::Fy,0.0,endMbrLocation,-Ws,-load.We);
         }
      }
   }
}

void CAnalysisAgentImp::ApplyUpperXBeamDeadLoad(PierIDType pierID,ModelData* pModelData) const
{
   WBFL::FEA2D::Model& femModel = *pModelData->m_Model;


   LoadCaseIDType loadCaseID = GetLoadCaseID(xbrTypes::pftUpperXBeam);
   WBFL::FEA2D::Loading& loading = femModel.CreateLoading(loadCaseID);


   LoadIDType loadID = 0;

   Float64 w = GetUpperCrossBeamLoading(pierID);
   for (const auto& capMbr : pModelData->m_XBeamMembers)
   {
      loading.CreateDistributedLoad(loadID++,capMbr.mbrID,WBFL::FEA2D::LoadDirection::Fy,0,-1,-w,-w);
   }
}

void CAnalysisAgentImp::ApplySuperstructureDeadLoadReactions(PierIDType pierID,ModelData* pModelData) const
{
   GET_IFACE(IXBRPier,pPier);
   GET_IFACE(IXBRProject,pProject);

   LoadCaseIDType dcLoadCaseID = GetLoadCaseID(xbrTypes::pftDCReactions);
   LoadCaseIDType dwLoadCaseID = GetLoadCaseID(xbrTypes::pftDWReactions);
   LoadCaseIDType crLoadCaseID = GetLoadCaseID(xbrTypes::pftCRReactions);
   LoadCaseIDType shLoadCaseID = GetLoadCaseID(xbrTypes::pftSHReactions);
   LoadCaseIDType psLoadCaseID = GetLoadCaseID(xbrTypes::pftPSReactions);
   LoadCaseIDType reLoadCaseID = GetLoadCaseID(xbrTypes::pftREReactions);

   LoadIDType dcLoadID = 0;
   LoadIDType dwLoadID = 0;
   LoadIDType shLoadID = 0;
   LoadIDType crLoadID = 0;
   LoadIDType psLoadID = 0;
   LoadIDType reLoadID = 0;

   WBFL::FEA2D::Model& femModel = *pModelData->m_Model;


   WBFL::FEA2D::Loading& dcLoading = femModel.CreateLoading(dcLoadCaseID);

   WBFL::FEA2D::Loading& dwLoading = femModel.CreateLoading(dwLoadCaseID);

   WBFL::FEA2D::Loading& shLoading = femModel.CreateLoading(shLoadCaseID);

   WBFL::FEA2D::Loading& crLoading = femModel.CreateLoading(crLoadCaseID);

   WBFL::FEA2D::Loading& psLoading = femModel.CreateLoading(psLoadCaseID);

   WBFL::FEA2D::Loading& reLoading = femModel.CreateLoading(reLoadCaseID);


   Float64 Lxb = pPier->GetXBeamLength(xbrTypes::xblBottomXBeam, pierID);

   IndexType nBearingLines = pProject->GetBearingLineCount(pierID);
   for ( IndexType brgLineIdx = 0; brgLineIdx < nBearingLines; brgLineIdx++ )
   {
      xbrTypes::ReactionLoadType reactionType = pProject->GetBearingReactionType(pierID,brgLineIdx);

      IndexType nBearings = pProject->GetBearingCount(pierID,brgLineIdx);
      for ( IndexType brgIdx = 0; brgIdx < nBearings; brgIdx++ )
      {
         Float64 Xbrg = pPier->GetBearingLocation(pierID,brgLineIdx,brgIdx);

         if (Xbrg < 0 || Lxb < Xbrg)
         {
            // bearing is off the model... skip it
            CString strMsg;
            if (1 < nBearings)
            {
               strMsg.Format(_T("Bearing %d on Bearing Line %d is not on the cross beam."), LABEL_INDEX(brgIdx), LABEL_INDEX(brgLineIdx));
            }
            else
            {
               strMsg.Format(_T("Bearing %d is not on the cross beam."), LABEL_INDEX(brgIdx));
            }
            strMsg += _T(" Permanent load reactions at this bearing have been ignored.");

            GET_IFACE(IEAFStatusCenter, pStatusCenter);
            pStatusCenter->Add(std::make_shared<xbrBridgeStatusItem>(m_StatusGroupID, m_scidBridgeWarning, strMsg));

            continue;
         }

         Float64 DC, DW, CR, SH, PS, RE, W;
         pProject->GetBearingReactions(pierID,brgLineIdx,brgIdx,&DC,&DW,&CR,&SH,&PS,&RE,&W);

         if ( reactionType == xbrTypes::rltConcentrated || IsZero(W) )
         {
            // Concentrated load (or uniform load, but W is zero so treat as concentrated load)
            MemberIDType mbrID;
            Float64 mbrLocation;
            GetXBeamFemModelLocation(pModelData,Xbrg,&mbrID,&mbrLocation);

            if ( !IsZero(DC) )
            {
               dcLoading.CreatePointLoad(dcLoadID++,mbrID,mbrLocation,0.0,-DC,0.0,WBFL::FEA2D::LoadOrientation::Global);
            }

            if ( !IsZero(DW) )
            {
               dwLoading.CreatePointLoad(dwLoadID++,mbrID,mbrLocation,0.0,-DW,0.0,WBFL::FEA2D::LoadOrientation::Global);
            }

            if ( !IsZero(SH) )
            {
               shLoading.CreatePointLoad(shLoadID++,mbrID,mbrLocation,0.0,-SH,0.0,WBFL::FEA2D::LoadOrientation::Global);
            }

            if ( !IsZero(CR) )
            {
               crLoading.CreatePointLoad(crLoadID++,mbrID,mbrLocation,0.0,-CR,0.0,WBFL::FEA2D::LoadOrientation::Global);
            }

            if ( !IsZero(PS) )
            {
               psLoading.CreatePointLoad(psLoadID++,mbrID,mbrLocation,0.0,-PS,0.0,WBFL::FEA2D::LoadOrientation::Global);
            }

            if ( !IsZero(RE) )
            {
               reLoading.CreatePointLoad(reLoadID++,mbrID,mbrLocation,0.0,-RE,0.0,WBFL::FEA2D::LoadOrientation::Global);
            }
         }
         else
         {
            // Distributed load
            MemberIDType startMbrID, endMbrID;
            Float64 startLocation, endLocation;
            GetXBeamFemModelLocation(pModelData,Xbrg-W/2,&startMbrID,&startLocation);
            GetXBeamFemModelLocation(pModelData,Xbrg+W/2,&endMbrID,&endLocation);

            if ( startMbrID == endMbrID )
            {
               // distributed load is contained within a single FEM member
               if ( !IsZero(DC) )
               {
                  dcLoading.CreateDistributedLoad(dcLoadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startLocation,endLocation,-DC,-DC,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(DW) )
               {
                  dwLoading.CreateDistributedLoad(dwLoadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startLocation,endLocation,-DW,-DW,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(SH) )
               {
                  shLoading.CreateDistributedLoad(shLoadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startLocation,endLocation,-SH,-SH,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(CR) )
               {
                  crLoading.CreateDistributedLoad(crLoadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startLocation,endLocation,-CR,-CR,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(PS) )
               {
                  psLoading.CreateDistributedLoad(psLoadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startLocation,endLocation,-PS,-PS,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(RE) )
               {
                  reLoading.CreateDistributedLoad(reLoadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startLocation,endLocation,-RE,-RE,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }
            }
            else
            {
               // distributed loads span over multiple FEM members

               // apply loads to first member
               if ( !IsZero(DC) )
               {
                  dcLoading.CreateDistributedLoad(dcLoadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startLocation,-1.0,-DC,-DC,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(DW) )
               {
                  dwLoading.CreateDistributedLoad(dwLoadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startLocation,-1.0,-DW,-DW,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(SH) )
               {
                  shLoading.CreateDistributedLoad(shLoadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startLocation,-1.0,-SH,-SH,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(CR) )
               {
                  crLoading.CreateDistributedLoad(crLoadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startLocation,-1.0,-CR,-CR,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(PS) )
               {
                  psLoading.CreateDistributedLoad(psLoadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startLocation,-1.0,-PS,-PS,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(RE) )
               {
                  reLoading.CreateDistributedLoad(reLoadID++,startMbrID,WBFL::FEA2D::LoadDirection::Fy,startLocation,-1.0,-RE,-RE,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               // apply loads to intermediate members
               for ( MemberIDType mbrID = startMbrID+1; mbrID < endMbrID-1; mbrID++ )
               {
                  if ( !IsZero(DC) )
                  {
                     dcLoading.CreateDistributedLoad(dcLoadID++,mbrID,WBFL::FEA2D::LoadDirection::Fy,0.0,-1.0,-DC,-DC,WBFL::FEA2D::LoadOrientation::GlobalProjected);
                  }

                  if ( !IsZero(DW) )
                  {
                     dwLoading.CreateDistributedLoad(dwLoadID++,mbrID,WBFL::FEA2D::LoadDirection::Fy,0.0,-1.0,-DW,-DW,WBFL::FEA2D::LoadOrientation::GlobalProjected);
                  }

                  if ( !IsZero(SH) )
                  {
                     shLoading.CreateDistributedLoad(shLoadID++,mbrID,WBFL::FEA2D::LoadDirection::Fy,0.0,-1.0,-SH,-SH,WBFL::FEA2D::LoadOrientation::GlobalProjected);
                  }

                  if ( !IsZero(CR) )
                  {
                     crLoading.CreateDistributedLoad(crLoadID++,mbrID,WBFL::FEA2D::LoadDirection::Fy,0.0,-1.0,-CR,-CR,WBFL::FEA2D::LoadOrientation::GlobalProjected);
                  }

                  if ( !IsZero(PS) )
                  {
                     psLoading.CreateDistributedLoad(psLoadID++,mbrID,WBFL::FEA2D::LoadDirection::Fy,0.0,-1.0,-PS,-PS,WBFL::FEA2D::LoadOrientation::GlobalProjected);
                  }

                  if ( !IsZero(RE) )
                  {
                     reLoading.CreateDistributedLoad(reLoadID++,mbrID,WBFL::FEA2D::LoadDirection::Fy,0.0,-1.0,-RE,-RE,WBFL::FEA2D::LoadOrientation::GlobalProjected);
                  }
               }

               // apply loads to last member
               if ( !IsZero(DC) )
               {
                  dcLoading.CreateDistributedLoad(dcLoadID++,endMbrID,WBFL::FEA2D::LoadDirection::Fy,0.0,endLocation,-DC,-DC,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(DW) )
               {
                  dwLoading.CreateDistributedLoad(dwLoadID++,endMbrID,WBFL::FEA2D::LoadDirection::Fy,0.0,endLocation,-DW,-DW,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(SH) )
               {
                  shLoading.CreateDistributedLoad(shLoadID++,endMbrID,WBFL::FEA2D::LoadDirection::Fy,0.0,endLocation,-SH,-SH,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(CR) )
               {
                  crLoading.CreateDistributedLoad(crLoadID++,endMbrID,WBFL::FEA2D::LoadDirection::Fy,0.0,endLocation,-CR,-CR,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(PS) )
               {
                  psLoading.CreateDistributedLoad(psLoadID++,endMbrID,WBFL::FEA2D::LoadDirection::Fy,0.0,endLocation,-PS,-PS,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }

               if ( !IsZero(RE) )
               {
                  reLoading.CreateDistributedLoad(reLoadID++,endMbrID,WBFL::FEA2D::LoadDirection::Fy,0.0,endLocation,-RE,-RE,WBFL::FEA2D::LoadOrientation::GlobalProjected);
               }
            }
         }
      }
   }
}

void CAnalysisAgentImp::ValidateLowerXBeamDeadLoad(PierIDType pierID,ModelData* pModelData) const
{
   if ( 0 < pModelData->m_LowerXBeamLoads.size() )
   {
      return;
   }

   GET_IFACE(IXBRPointOfInterest,pPoi);
   std::vector<xbrPointOfInterest> vPoi = pPoi->GetXBeamPointsOfInterest(pierID,POI_SECTIONCHANGE);

   GET_IFACE(IXBRSectionProperties,pSectProp);
   GET_IFACE(IXBRMaterial,pMaterial);

   Float64 density = pMaterial->GetXBeamDensity(pierID);
   Float64 unitWeight = density*WBFL::Units::System::GetGravitationalAcceleration();

   std::vector<xbrPointOfInterest>::iterator iter(vPoi.begin());
   std::vector<xbrPointOfInterest>::iterator end(vPoi.end());
   Float64 Astart = pSectProp->GetArea(pierID,xbrTypes::Stage1,*iter);
   Float64 Wstart = unitWeight*Astart;
   Float64 Xstart = (*iter).GetDistFromStart();

   iter++;
   for ( ; iter != end; iter++ )
   {
      Float64 Aend = pSectProp->GetArea(pierID,xbrTypes::Stage1,*iter);
      Float64 Wend = unitWeight*Aend;
      Float64 Xend = (*iter).GetDistFromStart();

      LowerXBeamLoad load;
      load.Xs = Xstart;
      load.Xe = Xend;
      load.Ws = Wstart;
      load.We = Wend;
      pModelData->m_LowerXBeamLoads.push_back(load);

      Xstart = Xend;
      Astart = Aend;
      Wstart = Wend;
   }
}

void CAnalysisAgentImp::GetFemModelLocation(ModelData* pModelData,const xbrPointOfInterest& poi,MemberIDType* pMbrID,Float64* pMbrLocation) const
{
   ATLASSERT(!poi.IsColumnPOI()); // not supporting POIs on the columns yet

   Float64 Xpoi = poi.GetDistFromStart();
   GetXBeamFemModelLocation(pModelData,Xpoi,pMbrID,pMbrLocation);

   // We know that GetXBeamFemModelLocation will always return the member to the left of a joint location if at a joint.
   // For a POI_COLUMN_RIGHT, we want the member to the right
   if (poi.HasAttribute(POI_COLUMN_RIGHT))
   {
      (*pMbrID)++;
      *pMbrLocation = 0.0;
   }
}

void CAnalysisAgentImp::GetXBeamFemModelLocation(ModelData* pModelData,Float64 X,MemberIDType* pMbrID,Float64* pMbrLocation) const
{
   std::vector<BeamMember>::iterator iter(pModelData->m_XBeamMembers.begin());
   std::vector<BeamMember>::iterator end(pModelData->m_XBeamMembers.end());
   for ( ; iter != end; iter++ )
   {
      BeamMember& capMbr(*iter);
      if ( InRange(capMbr.Xs,X,capMbr.Xe) )
      {
         *pMbrID = capMbr.mbrID;
         *pMbrLocation = X - capMbr.Xs;
         return;
      }
   }
   ATLASSERT(false); // should never get here
}

void CAnalysisAgentImp::GetSuperstructureFemModelLocation(ModelData* pModelData,Float64 X,MemberIDType* pMbrID,Float64* pMbrLocation) const
{
   std::vector<BeamMember>::iterator iter(pModelData->m_SuperstructureMembers.begin());
   std::vector<BeamMember>::iterator end(pModelData->m_SuperstructureMembers.end());
   for ( ; iter != end; iter++ )
   {
      BeamMember& capMbr(*iter);
      if ( InRange(capMbr.Xs,X,capMbr.Xe) )
      {
         *pMbrID = capMbr.mbrID;
         *pMbrLocation = X - capMbr.Xs;
         return;
      }
   }
   ATLASSERT(false); // should never get here
}

LoadCaseIDType CAnalysisAgentImp::GetLoadCaseID(xbrTypes::ProductForceType pfType) const
{
   switch (pfType)
   {
   case xbrTypes::pftLowerXBeam:
      return 0;

   case xbrTypes::pftUpperXBeam:
      return 1;

   case xbrTypes::pftDCReactions:
      return 2;

   case xbrTypes::pftDWReactions:
      return 3;

   case xbrTypes::pftCRReactions:
      return 4;

   case xbrTypes::pftSHReactions:
      return 5;

   case xbrTypes::pftPSReactions:
      return 6;

   case xbrTypes::pftREReactions:
      return 7;

   default:
      ATLASSERT(false);
   }

   return INVALID_ID;
}

void CAnalysisAgentImp::Invalidate(bool bCreateNewDataStructures)
{
   if (m_pBroker)
   {
      GET_IFACE(IEAFStatusCenter, pStatusCenter);
      pStatusCenter->RemoveByStatusGroupID(m_StatusGroupID);
   }

   InvalidateModels(bCreateNewDataStructures);
   InvalidateResults(bCreateNewDataStructures);
}

//////////////////////////////////////////////////////////////////////
// IProjectEventSink
HRESULT CAnalysisAgentImp::OnProjectChanged()
{
   Invalidate();
   return S_OK;
}

//////////////////////////////////////////////////////////
// IBridgeDescriptionEventSink
HRESULT CAnalysisAgentImp::OnBridgeChanged(CBridgeChangedHint* pHint)
{
   Invalidate();
   return S_OK;
}

HRESULT CAnalysisAgentImp::OnGirderFamilyChanged()
{
   Invalidate();
   return S_OK;
}

HRESULT CAnalysisAgentImp::OnGirderChanged(const CGirderKey& girderKey,Uint32 lHint)
{
   Invalidate();
   return S_OK;
}

HRESULT CAnalysisAgentImp::OnLiveLoadChanged()
{
   Invalidate();
   return S_OK;
}

HRESULT CAnalysisAgentImp::OnLiveLoadNameChanged(LPCTSTR strOldName,LPCTSTR strNewName)
{
   Invalidate();
   return S_OK;
}

HRESULT CAnalysisAgentImp::OnConstructionLoadChanged()
{
   Invalidate();
   return S_OK;
}

//////////////////////////////////////////////////////////////////////
// IXBRProductForces
const std::vector<LowerXBeamLoad>& CAnalysisAgentImp::GetLowerCrossBeamLoading(PierIDType pierID) const
{
   ModelData* pModelData = GetModelData(pierID);
   return pModelData->m_LowerXBeamLoads;
}

Float64 CAnalysisAgentImp::GetUpperCrossBeamLoading(PierIDType pierID) const
{
   GET_IFACE(IXBRProject,pProject);
   Float64 H, W;
   pProject->GetDiaphragmDimensions(pierID,&H,&W);

   if ( pProject->GetPierType(pierID) == xbrTypes::pctExpansion )
   {
      W *= 2;
   }

   GET_IFACE(IXBRMaterial,pMaterial);
   Float64 density = pMaterial->GetXBeamDensity(pierID);
   Float64 unitWeight = density*WBFL::Units::System::GetGravitationalAcceleration();

   Float64 w = H*W*unitWeight;
   return w;
}

IndexType CAnalysisAgentImp::GetLiveLoadConfigurationCount(PierIDType pierID,pgsTypes::LoadRatingType ratingType) const
{
   ModelData* pModelData = GetModelData(pierID);
   GET_IFACE_NOCHECK(IXBRRatingSpecification,pRatingSpec);
   if ( (::IsPermitRatingType(ratingType) && pRatingSpec->GetPermitRatingMethod() == xbrTypes::prmAASHTO) 
      ||
      (ratingType == pgsTypes::lrLegal_Emergency && pRatingSpec->GetEmergencyRatingMethod() == xbrTypes::ermAASHTO))
   {
      return pModelData->m_LastSingleLaneLLConfigIdx + 1;
   }
   else
   {
      return pModelData->m_LiveLoadConfigurations.size();
   }
}

IndexType CAnalysisAgentImp::GetLoadedLaneCount(PierIDType pierID,IndexType liveLoadConfigIdx) const
{
   ModelData* pModelData = GetModelData(pierID);
   return pModelData->m_LiveLoadConfigurations[liveLoadConfigIdx].m_LoadCases.size();
}

std::vector<Float64> CAnalysisAgentImp::GetWheelLineLocations(PierIDType pierID) const
{
   ModelData* pModelData = GetModelData(pierID,MODEL_INIT_TOPOLOGY);
   return GetWheelLineLocations(pModelData);
}

WheelLineConfiguration CAnalysisAgentImp::GetLiveLoadConfiguration(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType liveLoadConfigIdx,IndexType permitLaneIdx) const
{
   ModelData* pModelData = GetModelData(pierID);
   LiveLoadConfiguration& llConfig = pModelData->m_LiveLoadConfigurations[liveLoadConfigIdx];

#if defined _DEBUG
   if ( liveLoadConfigIdx <= pModelData->m_LastSingleLaneLLConfigIdx )
   {
      ATLASSERT(llConfig.m_LoadCases.size() == 1);
   }
#endif

   GET_IFACE(IXBRProject,pProject);
   Float64 R = pProject->GetLiveLoadReaction(pierID,ratingType,vehicleIdx); // single lane reaction

   bool bIsPermit = ::IsPermitRatingType(ratingType);
   bool bIsEmergency = (ratingType == pgsTypes::lrLegal_Emergency ? true : false);

   GET_IFACE(IXBRRatingSpecification,pSpec);

   Float64 Rlgl = 0;
   if ( pSpec->IsWSDOTEmergencyRating(ratingType) || pSpec->IsWSDOTPermitRating(ratingType) )
   {
      Rlgl = GetMaxLegalReaction(pierID);
   }
#if defined _DEBUG
   else
   {
      ATLASSERT(permitLaneIdx == INVALID_INDEX); // permit lane idx isn't used for this case, expecting it to be INVALID_INDEX
   }
#endif

   WheelLineConfiguration wheelConfig;
   IndexType laneIdx = 0;
   for (const auto& lcid : llConfig.m_LoadCases)
   {
      std::map<LoadCaseIDType,LaneConfiguration>::iterator found = pModelData->m_LaneConfigurations.find(lcid);
      ATLASSERT(found != pModelData->m_LaneConfigurations.end());

      LaneConfiguration& laneConfig = found->second;

      if (pSpec->IsWSDOTEmergencyRating(ratingType) || pSpec->IsWSDOTPermitRating(ratingType))
      {
         if ( laneIdx == permitLaneIdx )
         {
            WheelLinePlacement leftWheelLinePlacement;
            leftWheelLinePlacement.P = -0.5*R;
            leftWheelLinePlacement.Xxb = laneConfig.Xleft;
            wheelConfig.push_back(leftWheelLinePlacement);

            WheelLinePlacement rightWheelLinePlacement;
            rightWheelLinePlacement.P = -0.5*R;
            rightWheelLinePlacement.Xxb = laneConfig.Xright;
            wheelConfig.push_back(rightWheelLinePlacement);
         }
         else
         {
            WheelLinePlacement leftWheelLinePlacement;
            leftWheelLinePlacement.P = -0.5*Rlgl;
            leftWheelLinePlacement.Xxb = laneConfig.Xleft;
            wheelConfig.push_back(leftWheelLinePlacement);

            WheelLinePlacement rightWheelLinePlacement;
            rightWheelLinePlacement.P = -0.5*Rlgl;
            rightWheelLinePlacement.Xxb = laneConfig.Xright;
            wheelConfig.push_back(rightWheelLinePlacement);
         }
      }
      else
      {
         WheelLinePlacement leftWheelLinePlacement;
         leftWheelLinePlacement.P = -0.5*R;
         leftWheelLinePlacement.Xxb = laneConfig.Xleft;
         wheelConfig.push_back(leftWheelLinePlacement);

         WheelLinePlacement rightWheelLinePlacement;
         rightWheelLinePlacement.P = -0.5*R;
         rightWheelLinePlacement.Xxb = laneConfig.Xright;
         wheelConfig.push_back(rightWheelLinePlacement);
      }

      laneIdx++;
   }

   return wheelConfig;
}

void CAnalysisAgentImp::GetGoverningMomentLiveLoadConfigurations(PierIDType pierID,const xbrPointOfInterest& poi,std::vector<IndexType>* pvMin,std::vector<IndexType>* pvMax) const
{
   pvMin->clear();
   pvMax->clear();

   UnitLiveLoadResult& unitLiveLoadResult = GetUnitLiveLoadResult(pierID,poi);

   *pvMin = unitLiveLoadResult.m_MzMinLiveLoadConfigs;
   *pvMax = unitLiveLoadResult.m_MzMaxLiveLoadConfigs;
}

void CAnalysisAgentImp::GetGoverningShearLiveLoadConfigurations(PierIDType pierID,const xbrPointOfInterest& poi,std::vector<IndexType>* pvLLConfig) const
{
   pvLLConfig->clear();

   UnitLiveLoadResult& unitLiveLoadResult = GetUnitLiveLoadResult(pierID,poi);
   *pvLLConfig = unitLiveLoadResult.m_FyLiveLoadConfigs;
}

//////////////////////////////////////////////////////////////////////
// IAnalysisResults
Float64 CAnalysisAgentImp::GetMoment(PierIDType pierID,xbrTypes::ProductForceType pfType,const xbrPointOfInterest& poi) const
{
   ModelData* pModelData = GetModelData(pierID);

   std::map<PoiIDType,PoiIDType>::iterator found = pModelData->m_PoiMap.find(poi.GetID());
   ATLASSERT(found != pModelData->m_PoiMap.end());
   PoiIDType femPoiID = found->second;

   WBFL::FEA2D::Model& femModel = *pModelData->m_Model;


   LoadCaseIDType lcid = GetLoadCaseID(pfType);

   Float64 FxL, FxR, FyL, FyR, MzL, MzR;
   femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Left,WBFL::FEA2D::LoadOrientation::GlobalProjected,&FxL,&FyL,&MzL);
   femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Right,WBFL::FEA2D::LoadOrientation::GlobalProjected,&FxR,&FyR,&MzR);

   Float64 Mz;
   if ( IsZero(poi.GetDistFromStart()) )
   {
      Mz = -MzR;
   }
   else
   {
      Mz = MzL;
   }
   Mz = IsZero(Mz) ? 0 : Mz;
   return Mz;
}

WBFL::System::SectionValue CAnalysisAgentImp::GetShear(PierIDType pierID,xbrTypes::ProductForceType pfType,const xbrPointOfInterest& poi) const
{
   ModelData* pModelData = GetModelData(pierID);

   std::map<PoiIDType,PoiIDType>::iterator found = pModelData->m_PoiMap.find(poi.GetID());
   ATLASSERT(found != pModelData->m_PoiMap.end());
   PoiIDType femPoiID = found->second;

   WBFL::FEA2D::Model& femModel = *pModelData->m_Model;


   LoadCaseIDType lcid = GetLoadCaseID(pfType);

   Float64 FxL, FxR, FyL, FyR, MzL, MzR;
   femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Left,WBFL::FEA2D::LoadOrientation::GlobalProjected,&FxL,&FyL,&MzL);
   femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Right,WBFL::FEA2D::LoadOrientation::GlobalProjected,&FxR,&FyR,&MzR);

   FyL = IsZero(FyL) ? 0 : FyL;
   FyR = IsZero(FyR) ? 0 : FyR;

   WBFL::System::SectionValue V(-FyL,FyR);
   return V;
}

std::vector<Float64> CAnalysisAgentImp::GetMoment(PierIDType pierID,xbrTypes::ProductForceType pfType,const std::vector<xbrPointOfInterest>& vPoi) const
{
   std::vector<Float64> vM;
   vM.reserve(vPoi.size());
   for (const auto& poi : vPoi)
   {
      Float64 m = GetMoment(pierID,pfType,poi);
      vM.push_back(m);
   }

   return vM;
}

std::vector<WBFL::System::SectionValue> CAnalysisAgentImp::GetShear(PierIDType pierID,xbrTypes::ProductForceType pfType,const std::vector<xbrPointOfInterest>& vPoi) const
{
   std::vector<WBFL::System::SectionValue> vV;
   vV.reserve(vPoi.size());
   for (const auto& poi : vPoi)
   {
      WBFL::System::SectionValue v = GetShear(pierID,pfType,poi);
      vV.push_back(v);
   }

   return vV;
}

Float64 CAnalysisAgentImp::GetMoment(PierIDType pierID,xbrTypes::CombinedForceType lcType,const xbrPointOfInterest& poi) const
{
   std::vector<xbrTypes::ProductForceType> vPFTypes = GetLoads(lcType);
   Float64 M = 0;
   for (const auto& pfType : vPFTypes)
   {
      Float64 m = GetMoment(pierID,pfType,poi);
      M += m;
   }

   return M;
}

WBFL::System::SectionValue CAnalysisAgentImp::GetShear(PierIDType pierID,xbrTypes::CombinedForceType lcType,const xbrPointOfInterest& poi) const
{
   std::vector<xbrTypes::ProductForceType> vPFTypes = GetLoads(lcType);
   WBFL::System::SectionValue V(0,0);
   for (const auto& pfType : vPFTypes)
   {
      WBFL::System::SectionValue v = GetShear(pierID,pfType,poi);
      V += v;
   }

   return V;
}

std::vector<Float64> CAnalysisAgentImp::GetMoment(PierIDType pierID,xbrTypes::CombinedForceType lcType,const std::vector<xbrPointOfInterest>& vPoi) const
{
   std::vector<Float64> vM;
   vM.reserve(vPoi.size());
   for (const auto& poi : vPoi)
   {
      Float64 m = GetMoment(pierID,lcType,poi);
      vM.push_back(m);
   }
   return vM;
}

std::vector<WBFL::System::SectionValue> CAnalysisAgentImp::GetShear(PierIDType pierID,xbrTypes::CombinedForceType lcType,const std::vector<xbrPointOfInterest>& vPoi) const
{
   std::vector<WBFL::System::SectionValue> vV;
   vV.reserve(vPoi.size());
   for (const auto& poi : vPoi)
   {
      WBFL::System::SectionValue v = GetShear(pierID,lcType,poi);
      vV.push_back(v);
   }
   return vV;
}

Float64 CAnalysisAgentImp::GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,const xbrPointOfInterest& poi) const
{
   ModelData* pModelData = GetModelData(pierID);

   WBFL::FEA2D::Model& femModel = *pModelData->m_Model;


   std::map<PoiIDType,PoiIDType>::iterator found = pModelData->m_PoiMap.find(poi.GetID());
   ATLASSERT(found != pModelData->m_PoiMap.end());
   PoiIDType femPoiID = found->second;

   LiveLoadConfiguration& llConfig = pModelData->m_LiveLoadConfigurations[llConfigIdx];

   Float64 FxLeft(0), FxRight(0);
   Float64 FyLeft(0), FyRight(0);
   Float64 MzLeft(0), MzRight(0);

   for (const auto& lcid : llConfig.m_LoadCases)
   {
      Float64 fxLeft, fyLeft, mzLeft;
      femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Left,WBFL::FEA2D::LoadOrientation::Member,&fxLeft,&fyLeft,&mzLeft);

      Float64 fxRight, fyRight, mzRight;
      femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Right,WBFL::FEA2D::LoadOrientation::Member,&fxRight,&fyRight,&mzRight);

      FxLeft += fxLeft;
      FyLeft += fyLeft;
      MzLeft += mzLeft;

      FxRight += fxRight;
      FyRight += fyRight;
      MzRight += mzRight;
   }

   Float64 Mz;
   if ( IsZero(poi.GetDistFromStart()) )
   {
      Mz = -MzRight;
   }
   else
   {
      Mz = MzLeft;
   }

   Mz = IsZero(Mz) ? 0 : Mz;

   GET_IFACE(IXBRProject,pProject);
   Float64 R = pProject->GetLiveLoadReaction(pierID,ratingType,vehicleIdx); // single lane reaction
   IndexType nLoadedLanes = GetLoadedLaneCount(pierID,llConfigIdx);
   Float64 mpf = WBFL::LRFD::Utility::GetMultiplePresenceFactor(nLoadedLanes);

   if ( ::IsPermitRatingType(ratingType) && nLoadedLanes == 1 )
   {
      // MBE 6A.4.5.4.2a ... no MPF for permit cases
      mpf = 1.0;
   }

   Mz *= R*mpf;

   return Mz;
}

WBFL::System::SectionValue CAnalysisAgentImp::GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,const xbrPointOfInterest& poi) const
{
   ModelData* pModelData = GetModelData(pierID);

   std::map<PoiIDType,PoiIDType>::iterator found = pModelData->m_PoiMap.find(poi.GetID());
   ATLASSERT(found != pModelData->m_PoiMap.end());
   PoiIDType femPoiID = found->second;

   WBFL::FEA2D::Model& femModel = *pModelData->m_Model;

   LiveLoadConfiguration& llConfig = pModelData->m_LiveLoadConfigurations[llConfigIdx];

   Float64 FxLeft(0), FxRight(0);
   Float64 FyLeft(0), FyRight(0);
   Float64 MzLeft(0), MzRight(0);

   for (const auto& lcid : llConfig.m_LoadCases)
   {
      Float64 fxLeft, fyLeft, mzLeft;
      femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Left,WBFL::FEA2D::LoadOrientation::Member,&fxLeft,&fyLeft,&mzLeft);

      Float64 fxRight, fyRight, mzRight;
      femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Right,WBFL::FEA2D::LoadOrientation::Member,&fxRight,&fyRight,&mzRight);

      FxLeft += fxLeft;
      FyLeft += fyLeft;
      MzLeft += mzLeft;

      FxRight += fxRight;
      FyRight += fyRight;
      MzRight += mzRight;
   }

   FyLeft  = IsZero(FyLeft)  ? 0 : FyLeft;
   FyRight = IsZero(FyRight) ? 0 : FyRight;
   WBFL::System::SectionValue Fy(-FyLeft,FyRight);

   GET_IFACE(IXBRProject,pProject);
   Float64 R = pProject->GetLiveLoadReaction(pierID,ratingType,vehicleIdx); // single lane reaction

   IndexType nLoadedLanes = GetLoadedLaneCount(pierID,llConfigIdx);
   Float64 mpf = WBFL::LRFD::Utility::GetMultiplePresenceFactor(nLoadedLanes);

   if ( ::IsPermitRatingType(ratingType) && nLoadedLanes == 1 )
   {
      // MBE 6A.4.5.4.2a ... no MPF for permit cases
      mpf = 1.0;
   }

   Fy *= R*mpf;

   return Fy;
}

std::vector<Float64> CAnalysisAgentImp::GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,const std::vector<xbrPointOfInterest>& vPoi) const
{
   std::vector<Float64> vM;
   vM.reserve(vPoi.size());
   for (const auto& poi : vPoi)
   {
      Float64 m = GetMoment(pierID,ratingType,vehicleIdx,llConfigIdx,poi);
      vM.push_back(m);
   }
   return vM;
}

std::vector<WBFL::System::SectionValue> CAnalysisAgentImp::GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,const std::vector<xbrPointOfInterest>& vPoi) const
{
   std::vector<WBFL::System::SectionValue> vV;
   vV.reserve(vPoi.size());
   for (const auto& poi : vPoi)
   {
      WBFL::System::SectionValue v = GetShear(pierID,ratingType,vehicleIdx,llConfigIdx,poi);
      vV.push_back(v);
   }
   return vV;
}

//////////////////////////////////

void CAnalysisAgentImp::GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx,const xbrPointOfInterest& poi,Float64* pMpermit,Float64* pMlegal) const
{
   ATLASSERT(::IsPermitRatingType(ratingType) || ratingType == pgsTypes::lrLegal_Emergency);
#if defined _DEBUG
   GET_IFACE(IXBRRatingSpecification,pRatingSpec);
   if ( ::IsPermitRatingType(ratingType))
      ATLASSERT(pRatingSpec->GetPermitRatingMethod() == xbrTypes::prmWSDOT);
   else
      ATLASSERT(pRatingSpec->GetEmergencyRatingMethod() == xbrTypes::ermWSDOT);
   // This method is only used for WSDOT emergency and permit ratings
#endif

   IndexType nLoadedLanes = GetLoadedLaneCount(pierID,llConfigIdx);
   if ( nLoadedLanes == 1 )
   {
      // if there is only one loaded lane, it is the permit/emergency vehicle... just use the regular implementation
      *pMpermit = GetMoment(pierID,ratingType,vehicleIdx,llConfigIdx,poi);
      *pMlegal = 0;
      return;
   }

   ModelData* pModelData = GetModelData(pierID);

   std::map<PoiIDType,PoiIDType>::iterator found = pModelData->m_PoiMap.find(poi.GetID());
   ATLASSERT(found != pModelData->m_PoiMap.end());
   PoiIDType femPoiID = found->second;

   WBFL::FEA2D::Model& femModel = *pModelData->m_Model;


   // Get the results for all loaded lanes having the same vehicle reaction
   LiveLoadConfiguration& llConfig = pModelData->m_LiveLoadConfigurations[llConfigIdx];
   ATLASSERT(llConfig.m_LoadCases.size() == nLoadedLanes);

   Float64 FxLeftLegal(0), FxRightLegal(0);
   Float64 FyLeftLegal(0), FyRightLegal(0);
   Float64 MzLeftLegal(0), MzRightLegal(0);

   Float64 FxLeftPermit(0), FxRightPermit(0);
   Float64 FyLeftPermit(0), FyRightPermit(0);
   Float64 MzLeftPermit(0), MzRightPermit(0);

   for ( IndexType laneIdx = 0; laneIdx < nLoadedLanes; laneIdx++ )
   {
      // get unit lane load forces
      LoadCaseIDType lcid = llConfig.m_LoadCases[laneIdx];
      Float64 fxLeft, fyLeft, mzLeft;
      femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Left,WBFL::FEA2D::LoadOrientation::Member,&fxLeft,&fyLeft,&mzLeft);

      Float64 fxRight, fyRight, mzRight;
      femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Right,WBFL::FEA2D::LoadOrientation::Member,&fxRight,&fyRight,&mzRight);

      if ( laneIdx == permitLaneIdx )
      {
         FxLeftPermit += fxLeft;
         FyLeftPermit += fyLeft;
         MzLeftPermit += mzLeft;

         FxRightPermit += fxRight;
         FyRightPermit += fyRight;
         MzRightPermit += mzRight;
      }
      else
      {
         FxLeftLegal += fxLeft;
         FyLeftLegal += fyLeft;
         MzLeftLegal += mzLeft;

         FxRightLegal += fxRight;
         FyRightLegal += fyRight;
         MzRightLegal += mzRight;
      }
   }

   if ( IsZero(poi.GetDistFromStart()) )
   {
      *pMpermit = -MzRightPermit;
      *pMlegal  = -MzRightLegal;
   }
   else
   {
      *pMpermit = MzLeftPermit;
      *pMlegal  = MzLeftLegal;
   }

   GET_IFACE(IXBRProject,pProject);
   Float64 Rpermit = pProject->GetLiveLoadReaction(pierID,ratingType,vehicleIdx); // single lane reaction
   Float64 Rlegal  = GetMaxLegalReaction(pierID);

   *pMpermit *= Rpermit;
   *pMlegal  *= Rlegal;

   *pMpermit = IsZero(*pMpermit) ? 0 : *pMpermit;
   *pMlegal  = IsZero(*pMlegal)  ? 0 : *pMlegal;

   Float64 mpf = WBFL::LRFD::Utility::GetMultiplePresenceFactor(nLoadedLanes);
   if ( nLoadedLanes == 1 )
   {
      mpf = 1.0;
   }

   // NOTE: Multiple presense factor is applied to both the permit and legal loads. Consider the following
   // (ignore load factors)
   // Q = DC + DW + mpf(LL)
   // LL = LL_Permit + LL_Legal
   // therefore Q = DC + DW + mpf(LL_Permit + LL_Legal);
   // Re-arrange into the rating factor equation
   // RF = [Q - DC - DW - mpf(LL_Legal)]/[mpf(LL_Permit)]

   *pMlegal  *= mpf;
   *pMpermit *= mpf;
}

void CAnalysisAgentImp::GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx,const std::vector<xbrPointOfInterest>& vPoi,std::vector<Float64>* pvMpermit,std::vector<Float64>* pvMlegal) const
{
   pvMpermit->clear();
   pvMpermit->resize(vPoi.size());
   pvMlegal->clear();
   pvMlegal->resize(vPoi.size());
   for (const auto& poi : vPoi)
   {
      Float64 Mpermit,Mlegal;
      GetMoment(pierID,ratingType,vehicleIdx,llConfigIdx,permitLaneIdx,poi,&Mpermit,&Mlegal);
      pvMpermit->push_back(Mpermit);
      pvMlegal->push_back(Mlegal);
   }
}

void CAnalysisAgentImp::GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx,const xbrPointOfInterest& poi,WBFL::System::SectionValue* pVpermit,WBFL::System::SectionValue* pVlegal) const
{
   ATLASSERT(::IsPermitRatingType(ratingType) || ratingType == pgsTypes::lrLegal_Emergency);
#if defined _DEBUG
   GET_IFACE(IXBRRatingSpecification, pRatingSpec);
   if (::IsPermitRatingType(ratingType))
      ATLASSERT(pRatingSpec->GetPermitRatingMethod() == xbrTypes::prmWSDOT);
   else
      ATLASSERT(pRatingSpec->GetEmergencyRatingMethod() == xbrTypes::ermWSDOT);
   // This method is only used for WSDOT emergency and permit ratings
#endif


   IndexType nLoadedLanes = GetLoadedLaneCount(pierID,llConfigIdx);
   if ( nLoadedLanes == 1 )
   {
      // if there is only one loaded lane, it is the permit vehicle... just use the regular implementation
      *pVpermit = GetShear(pierID,ratingType,vehicleIdx,llConfigIdx,poi);
      *pVlegal = 0;
      return;
   }

   ModelData* pModelData = GetModelData(pierID);

   std::map<PoiIDType,PoiIDType>::iterator found = pModelData->m_PoiMap.find(poi.GetID());
   ATLASSERT(found != pModelData->m_PoiMap.end());
   PoiIDType femPoiID = found->second;

   WBFL::FEA2D::Model& femModel = *pModelData->m_Model;


   // Get the results for all loaded lanes having the same vehicle reaction
   LiveLoadConfiguration& llConfig = pModelData->m_LiveLoadConfigurations[llConfigIdx];
   ATLASSERT(llConfig.m_LoadCases.size() == nLoadedLanes);

   Float64 FxLeftLegal(0), FxRightLegal(0);
   Float64 FyLeftLegal(0), FyRightLegal(0);
   Float64 MzLeftLegal(0), MzRightLegal(0);

   Float64 FxLeftPermit(0), FxRightPermit(0);
   Float64 FyLeftPermit(0), FyRightPermit(0);
   Float64 MzLeftPermit(0), MzRightPermit(0);

   for ( IndexType laneIdx = 0; laneIdx < nLoadedLanes; laneIdx++ )
   {
      // get unit lane load forces
      LoadCaseIDType lcid = llConfig.m_LoadCases[laneIdx];
      Float64 fxLeft, fyLeft, mzLeft;
      femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Left,WBFL::FEA2D::LoadOrientation::Member,&fxLeft,&fyLeft,&mzLeft);

      Float64 fxRight, fyRight, mzRight;
      femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Right,WBFL::FEA2D::LoadOrientation::Member,&fxRight,&fyRight,&mzRight);

      if ( laneIdx == permitLaneIdx )
      {
         FxLeftPermit += fxLeft;
         FyLeftPermit += fyLeft;
         MzLeftPermit += mzLeft;

         FxRightPermit += fxRight;
         FyRightPermit += fyRight;
         MzRightPermit += mzRight;
      }
      else
      {
         FxLeftLegal += fxLeft;
         FyLeftLegal += fyLeft;
         MzLeftLegal += mzLeft;

         FxRightLegal += fxRight;
         FyRightLegal += fyRight;
         MzRightLegal += mzRight;
      }
   }

   *pVpermit = WBFL::System::SectionValue(-FyLeftPermit,FyRightPermit);
   *pVlegal  = WBFL::System::SectionValue(-FyLeftLegal, FyRightLegal );

   GET_IFACE(IXBRProject,pProject);
   Float64 Rpermit = pProject->GetLiveLoadReaction(pierID,ratingType,vehicleIdx); // single lane reaction
   Float64 Rlegal  = GetMaxLegalReaction(pierID);

   *pVpermit *= Rpermit;
   *pVlegal  *= Rlegal;

   pVpermit->Left()  = IsZero(pVpermit->Left())  ? 0 : pVpermit->Left();
   pVpermit->Right() = IsZero(pVpermit->Right()) ? 0 : pVpermit->Right();
   pVlegal->Left()   = IsZero(pVlegal->Left())   ? 0 : pVlegal->Left();
   pVlegal->Right()  = IsZero(pVlegal->Right())  ? 0 : pVlegal->Right();

   Float64 mpf = WBFL::LRFD::Utility::GetMultiplePresenceFactor(nLoadedLanes);
   if ( nLoadedLanes == 1 )
   {
      mpf = 1.0;
   }

   // NOTE: Multiple presense factor is applied to both the permit and legal loads. Consider the following
   // (ignore load factors)
   // Q = DC + DW + mpf(LL)
   // LL = LL_Permit + LL_Legal
   // therefore Q = DC + DW + mpf(LL_Permit + LL_Legal);
   // Re-arrange into the rating factor equation
   // RF = [Q - DC - DW - mpf(LL_Legal)]/[mpf(LL_Permit)]

   *pVlegal  *= mpf;
   *pVpermit *= mpf;
}

void CAnalysisAgentImp::GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx,const std::vector<xbrPointOfInterest>& vPoi,std::vector<WBFL::System::SectionValue>* pvVpermit,std::vector<WBFL::System::SectionValue>* pvVlegal) const
{
   pvVpermit->clear();
   pvVpermit->resize(vPoi.size());
   pvVlegal->clear();
   pvVlegal->resize(vPoi.size());
   for (const auto& poi : vPoi)
   {
      WBFL::System::SectionValue Vpermit,Vlegal;
      GetShear(pierID,ratingType,vehicleIdx,llConfigIdx,permitLaneIdx,poi,&Vpermit,&Vlegal);
      pvVpermit->push_back(Vpermit);
      pvVlegal->push_back(Vlegal);
   }
}

//////////////////////////////////

void CAnalysisAgentImp::GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,const xbrPointOfInterest& poi,Float64* pMin,Float64* pMax,IndexType* pMinLLConfigIdx,IndexType* pMaxLLConfigIdx) const
{
   GET_IFACE(IXBRProject,pProject);
   GET_IFACE(IEAFDisplayUnits,pDisplayUnits);
   GET_IFACE(IEAFProgress,pProgress);
   WBFL::EAF::AutoProgress ap(pProgress);

   CString strProgressMsg;
   strProgressMsg.Format(_T("Getting live load moment for %s, %s, at %s"),
      RatingLibraryEntry::GetLoadRatingType(ratingType),
      pProject->GetLiveLoadName(pierID,ratingType,vehicleIdx).c_str(),
      ::FormatDimension(poi.GetDistFromStart(),pDisplayUnits->GetSpanLengthUnit()));
   pProgress->UpdateMessage(strProgressMsg);

   UnitLiveLoadResult& liveLoadResult = GetUnitLiveLoadResult(pierID,poi);

   // permit rating results is always based on single loaded lane. the load factors make adjustments and account for
   // the presence of vehicles in other lanes (See MBE 6A.4.5.4.2a)
   bool bIsPermitRating = ::IsPermitRatingType(ratingType);
   if ( bIsPermitRating )
   {
      Float64 mpf = WBFL::LRFD::Utility::GetMultiplePresenceFactor(1); // mpf for one loaded lane
      *pMin = liveLoadResult.m_MzMin_SingleLane/mpf;
      *pMax = liveLoadResult.m_MzMax_SingleLane/mpf;
   }
   else
   {
      *pMin = liveLoadResult.m_MzMin;
      *pMax = liveLoadResult.m_MzMax;
   }

   Float64 R = pProject->GetLiveLoadReaction(pierID,ratingType,vehicleIdx); // single lane reaction

   *pMin *= R;
   *pMax *= R;

   *pMin = IsZero(*pMin) ? 0 : *pMin;
   *pMax = IsZero(*pMax) ? 0 : *pMax;

   if ( pMinLLConfigIdx )
   {
      *pMinLLConfigIdx = (bIsPermitRating ? liveLoadResult.m_llConfigIdx_MzMin_SingleLane : liveLoadResult.m_llConfigIdx_MzMin);
   }

   if ( pMaxLLConfigIdx )
   {
      *pMaxLLConfigIdx = (bIsPermitRating ? liveLoadResult.m_llConfigIdx_MzMax_SingleLane : liveLoadResult.m_llConfigIdx_MzMax);
   }
}

void CAnalysisAgentImp::GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,const xbrPointOfInterest& poi,WBFL::System::SectionValue* pMin,WBFL::System::SectionValue* pMax,IndexType* pMinLLConfigIdx,IndexType* pMaxLLConfigIdx) const
{
   GET_IFACE(IXBRProject,pProject);
   GET_IFACE(IEAFDisplayUnits,pDisplayUnits);
   GET_IFACE(IEAFProgress,pProgress);
   WBFL::EAF::AutoProgress ap(pProgress);

   CString strProgressMsg;
   strProgressMsg.Format(_T("Getting live load shear for %s, %s, at %s"),
      RatingLibraryEntry::GetLoadRatingType(ratingType),
      pProject->GetLiveLoadName(pierID,ratingType,vehicleIdx).c_str(),
      ::FormatDimension(poi.GetDistFromStart(),pDisplayUnits->GetSpanLengthUnit()));
   pProgress->UpdateMessage(strProgressMsg);

   UnitLiveLoadResult& liveLoadResult = GetUnitLiveLoadResult(pierID,poi);

   // permit rating results is always based on single loaded lane. the load factors make adjustments and account for
   // the presence of vehicles in other lanes (See MBE 6A.4.5.4.2a)
   bool bIsPermitRating = ::IsPermitRatingType(ratingType);
   if ( bIsPermitRating )
   {
      Float64 mpf = WBFL::LRFD::Utility::GetMultiplePresenceFactor(1); // mpf for one loaded lane
      *pMin = liveLoadResult.m_FyMin_SingleLane/mpf;
      *pMax = liveLoadResult.m_FyMax_SingleLane/mpf;
   }
   else
   {
      *pMin = liveLoadResult.m_FyMin;
      *pMax = liveLoadResult.m_FyMax;
   }

   Float64 R = pProject->GetLiveLoadReaction(pierID,ratingType,vehicleIdx); // single lane reaction

   pMin->Left()  *= R;
   pMin->Right() *= R;
   
   pMax->Left()  *= R;
   pMax->Right() *= R;

   pMin->Left()  = IsZero(pMin->Left())  ? 0 : pMin->Left();
   pMin->Right() = IsZero(pMin->Right()) ? 0 : pMin->Right();
   pMax->Left()  = IsZero(pMax->Left())  ? 0 : pMax->Left();
   pMax->Right() = IsZero(pMax->Right()) ? 0 : pMax->Right();

   if ( pMinLLConfigIdx )
   {
      if ( IsEqual(MaxMagnitude(pMin->Left(),pMin->Right()),pMin->Left()) )
      {
         *pMinLLConfigIdx = (bIsPermitRating ? liveLoadResult.m_llConfigIdx_FyLeftMin_SingleLane : liveLoadResult.m_llConfigIdx_FyLeftMin);
      }
      else
      {
         *pMinLLConfigIdx = (bIsPermitRating ? liveLoadResult.m_llConfigIdx_FyRightMin_SingleLane : liveLoadResult.m_llConfigIdx_FyRightMin);
      }
   }

   if ( pMaxLLConfigIdx )
   {
      if ( IsEqual(MaxMagnitude(pMax->Left(),pMax->Right()),pMax->Left()) )
      {
         *pMaxLLConfigIdx = (bIsPermitRating ? liveLoadResult.m_llConfigIdx_FyLeftMax_SingleLane : liveLoadResult.m_llConfigIdx_FyLeftMax);
      }
      else
      {
         *pMaxLLConfigIdx = (bIsPermitRating ? liveLoadResult.m_llConfigIdx_FyRightMax_SingleLane : liveLoadResult.m_llConfigIdx_FyRightMax);
      }
   }
}

void CAnalysisAgentImp::GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,const std::vector<xbrPointOfInterest>& vPoi,std::vector<Float64>* pvMin,std::vector<Float64>* pvMax,std::vector<IndexType>* pvMinLLConfigIdx,std::vector<IndexType>* pvMaxLLConfigIdx) const
{
   pvMin->clear();
   pvMin->reserve(vPoi.size());
   pvMax->clear();
   pvMax->reserve(vPoi.size());

   if ( pvMinLLConfigIdx )
   {
      pvMinLLConfigIdx->clear();
      pvMinLLConfigIdx->reserve(vPoi.size());
   }

   if ( pvMaxLLConfigIdx )
   {
      pvMaxLLConfigIdx->clear();
      pvMaxLLConfigIdx->reserve(vPoi.size());
   }

   for (const auto& poi : vPoi)
   {
      Float64 min,max;
      //WheelLineConfiguration minConfig, maxConfig;
      IndexType minLLConfigIdx, maxLLConfigIdx;
      GetMoment(pierID,ratingType,vehicleIdx,poi,&min,&max,pvMinLLConfigIdx ? &minLLConfigIdx : nullptr,pvMaxLLConfigIdx ? &maxLLConfigIdx : nullptr);
      pvMin->push_back(min);
      pvMax->push_back(max);
      if ( pvMinLLConfigIdx )
      {
         pvMinLLConfigIdx->push_back(minLLConfigIdx);
      }
      if ( pvMaxLLConfigIdx )
      {
         pvMaxLLConfigIdx->push_back(maxLLConfigIdx);
      }
   }
}

void CAnalysisAgentImp::GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,const std::vector<xbrPointOfInterest>& vPoi,std::vector<WBFL::System::SectionValue>* pvMin,std::vector<WBFL::System::SectionValue>* pvMax,std::vector<IndexType>* pvMinLLConfigIdx,std::vector<IndexType>* pvMaxLLConfigIdx) const
{
   pvMin->clear();
   pvMin->reserve(vPoi.size());
   pvMax->clear();
   pvMax->reserve(vPoi.size());
   if ( pvMinLLConfigIdx )
   {
      pvMinLLConfigIdx->clear();
      pvMinLLConfigIdx->reserve(vPoi.size());
   }
   if ( pvMaxLLConfigIdx )
   {
      pvMaxLLConfigIdx->clear();
      pvMaxLLConfigIdx->reserve(vPoi.size());
   }

   for (const auto& poi : vPoi)
   {
      WBFL::System::SectionValue min,max;
      //WheelLineConfiguration minLeftConfig, minRightConfig, maxLeftConfig, maxRightConfig;
      IndexType minLLConfigIdx, maxLLConfigIdx;
      GetShear(pierID,ratingType,vehicleIdx,poi,&min,&max,pvMinLLConfigIdx ? &minLLConfigIdx : nullptr,pvMaxLLConfigIdx ? &maxLLConfigIdx : nullptr);
      pvMin->push_back(min);
      pvMax->push_back(max);

      if ( pvMinLLConfigIdx )
      {
         pvMinLLConfigIdx->push_back(minLLConfigIdx);
      }

      if ( pvMaxLLConfigIdx )
      {
         pvMaxLLConfigIdx->push_back(maxLLConfigIdx);
      }
   }
}

void CAnalysisAgentImp::GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,const xbrPointOfInterest& poi,Float64* pMin,Float64* pMax,VehicleIndexType* pMinVehicleIdx,VehicleIndexType* pMaxVehicleIdx) const
{
   GET_IFACE(IXBRProject,pProject);
   IndexType nLiveLoadReactions = pProject->GetLiveLoadReactionCount(pierID,ratingType);

   *pMin = DBL_MAX;
   *pMax = -DBL_MAX;
   if ( pMinVehicleIdx )
   {
      *pMinVehicleIdx = INVALID_INDEX;
   }
   
   if ( pMaxVehicleIdx )
   {
      *pMaxVehicleIdx = INVALID_INDEX;
   }

   if ( nLiveLoadReactions == 0 )
   {
      *pMin = 0;
      *pMax = 0;
      return;
   }

   for ( VehicleIndexType vehicleIdx = 0; vehicleIdx < nLiveLoadReactions; vehicleIdx++ )
   {
      Float64 min,max;
      GetMoment(pierID,ratingType,vehicleIdx,poi,&min,&max,nullptr,nullptr);

      if ( min < *pMin )
      {
         *pMin = min;
         if ( pMinVehicleIdx )
         {
            *pMinVehicleIdx = vehicleIdx;
         }
      }

      if ( *pMax < max )
      {
         *pMax = max;
         if ( pMaxVehicleIdx )
         {
            *pMaxVehicleIdx = vehicleIdx;
         }
      }
   }
}

void CAnalysisAgentImp::GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,const xbrPointOfInterest& poi,WBFL::System::SectionValue* pMin,WBFL::System::SectionValue* pMax,VehicleIndexType* pMinLeftVehicleIdx,VehicleIndexType* pMinRightVehicleIdx,VehicleIndexType* pMaxLeftVehicleIdx,VehicleIndexType* pMaxRightVehicleIdx) const
{
   GET_IFACE(IXBRProject,pProject);
   IndexType nLiveLoadReactions = pProject->GetLiveLoadReactionCount(pierID,ratingType);

   *pMin = DBL_MAX;
   *pMax = -DBL_MAX;
   if ( pMinLeftVehicleIdx )
   {
      *pMinLeftVehicleIdx = INVALID_INDEX;
   }
   if ( pMinRightVehicleIdx )
   {
      *pMinRightVehicleIdx = INVALID_INDEX;
   }
   
   if ( pMaxLeftVehicleIdx )
   {
      *pMaxLeftVehicleIdx = INVALID_INDEX;
   }
   if ( pMaxRightVehicleIdx )
   {
      *pMaxRightVehicleIdx = INVALID_INDEX;
   }

   if ( nLiveLoadReactions == 0 )
   {
      *pMin = 0;
      *pMax = 0;
      return;
   }

   for ( VehicleIndexType vehicleIdx = 0; vehicleIdx < nLiveLoadReactions; vehicleIdx++ )
   {
      WBFL::System::SectionValue min,max;
      GetShear(pierID,ratingType,vehicleIdx,poi,&min,&max,nullptr,nullptr);
      if ( min.Left() < (*pMin).Left() )
      {
         pMin->Left() = min.Left();
         if ( pMinLeftVehicleIdx )
         {
            *pMinLeftVehicleIdx = vehicleIdx;
         }
      }

      if ( min.Right() < (*pMin).Right() )
      {
         pMin->Right() = min.Right();
         if ( pMinRightVehicleIdx )
         {
            *pMinRightVehicleIdx = vehicleIdx;
         }
      }

      if ( pMax->Left() < max.Left() )
      {
         pMax->Left() = max.Left();
         if ( pMaxLeftVehicleIdx )
         {
            *pMaxLeftVehicleIdx = vehicleIdx;
         }
      }

      if ( pMax->Right() < max.Right() )
      {
         pMax->Right() = max.Right();
         if ( pMaxRightVehicleIdx )
         {
            *pMaxRightVehicleIdx = vehicleIdx;
         }
      }
   }
}

void CAnalysisAgentImp::GetMoment(PierIDType pierID,pgsTypes::LoadRatingType ratingType,const std::vector<xbrPointOfInterest>& vPoi,std::vector<Float64>* pvMin,std::vector<Float64>* pvMax,std::vector<VehicleIndexType>* pvMinVehicleIdx,std::vector<VehicleIndexType>* pvMaxVehicleIdx) const
{
   pvMin->clear();
   pvMin->reserve(vPoi.size());
   pvMax->clear();
   pvMax->reserve(vPoi.size());
   if ( pvMinVehicleIdx )
   {
      pvMinVehicleIdx->clear();
      pvMinVehicleIdx->reserve(vPoi.size());
   }
   if ( pvMaxVehicleIdx )
   {
      pvMaxVehicleIdx->clear();
      pvMaxVehicleIdx->reserve(vPoi.size());
   }

   for (const auto& poi : vPoi)
   {
      Float64 min,max;
      VehicleIndexType minIdx, maxIdx;
      GetMoment(pierID,ratingType,poi,&min,&max,&minIdx,&maxIdx);
      pvMin->push_back(min);
      pvMax->push_back(max);

      if ( pvMinVehicleIdx )
      {
         pvMinVehicleIdx->push_back(minIdx);
      }

      if ( pvMaxVehicleIdx )
      {
         pvMaxVehicleIdx->push_back(maxIdx);
      }
   }
}

void CAnalysisAgentImp::GetShear(PierIDType pierID,pgsTypes::LoadRatingType ratingType,const std::vector<xbrPointOfInterest>& vPoi,std::vector<WBFL::System::SectionValue>* pvMin,std::vector<WBFL::System::SectionValue>* pvMax,std::vector<VehicleIndexType>* pvMinLeftVehicleIdx,std::vector<VehicleIndexType>* pvMinRightVehicleIdx,std::vector<VehicleIndexType>* pvMaxLeftVehicleIdx,std::vector<VehicleIndexType>* pvMaxRightVehicleIdx) const
{
   pvMin->clear();
   pvMin->reserve(vPoi.size());
   pvMax->clear();
   pvMax->reserve(vPoi.size());
   if ( pvMinLeftVehicleIdx )
   {
      pvMinLeftVehicleIdx->clear();
      pvMinLeftVehicleIdx->reserve(vPoi.size());
   }
   if ( pvMinRightVehicleIdx )
   {
      pvMinRightVehicleIdx->clear();
      pvMinRightVehicleIdx->reserve(vPoi.size());
   }
   if ( pvMaxLeftVehicleIdx )
   {
      pvMaxLeftVehicleIdx->clear();
      pvMaxLeftVehicleIdx->reserve(vPoi.size());
   }
   if ( pvMaxRightVehicleIdx )
   {
      pvMaxRightVehicleIdx->clear();
      pvMaxRightVehicleIdx->reserve(vPoi.size());
   }

   for (const auto& poi : vPoi)
   {
      WBFL::System::SectionValue min,max;
      VehicleIndexType minLeftIdx, minRightIdx, maxLeftIdx, maxRightIdx;
      GetShear(pierID,ratingType,poi,&min,&max,&minLeftIdx,&minRightIdx,&maxLeftIdx,&maxRightIdx);
      pvMin->push_back(min);
      pvMax->push_back(max);

      if ( pvMinLeftVehicleIdx )
      {
         pvMinLeftVehicleIdx->push_back(minLeftIdx);
      }

      if ( pvMinRightVehicleIdx )
      {
         pvMinRightVehicleIdx->push_back(minRightIdx);
      }

      if ( pvMaxLeftVehicleIdx )
      {
         pvMaxLeftVehicleIdx->push_back(maxLeftIdx);
      }

      if ( pvMaxRightVehicleIdx )
      {
         pvMaxRightVehicleIdx->push_back(maxRightIdx);
      }
   }
}

void CAnalysisAgentImp::GetMoment(PierIDType pierID,pgsTypes::LimitState limitState,const xbrPointOfInterest& poi,Float64* pMin,Float64* pMax) const
{
   pgsTypes::LoadRatingType ratingType = RatingTypeFromLimitState(limitState);

   GET_IFACE(IXBRProject,pProject);
   Float64 gDC = pProject->GetDCLoadFactor(limitState);
   Float64 gDW = pProject->GetDWLoadFactor(limitState);
   Float64 gCR = pProject->GetCRLoadFactor(limitState);
   Float64 gSH = pProject->GetSHLoadFactor(limitState);
   Float64 gPS = pProject->GetPSLoadFactor(limitState);
   Float64 gRE = pProject->GetRELoadFactor(limitState);
   Float64 gLL = pProject->GetLiveLoadFactor(pierID,limitState,INVALID_INDEX);

   Float64 DC = 0;
   std::vector<xbrTypes::ProductForceType> vDC = GetLoads(xbrTypes::lcDC);
   for (const auto& pfType : vDC)
   {
      Float64 dc = GetMoment(pierID,pfType,poi);
      DC += dc;
   }

   Float64 DW = 0;
   std::vector<xbrTypes::ProductForceType> vDW = GetLoads(xbrTypes::lcDW);
   for (const auto& pfType : vDW)
   {
      Float64 dw = GetMoment(pierID,pfType,poi);
      DW += dw;
   }

   Float64 CR = 0;
   std::vector<xbrTypes::ProductForceType> vCR = GetLoads(xbrTypes::lcCR);
   for (const auto& pfType : vCR)
   {
      Float64 cr = GetMoment(pierID,pfType,poi);
      CR += cr;
   }

   Float64 SH = 0;
   std::vector<xbrTypes::ProductForceType> vSH = GetLoads(xbrTypes::lcSH);
   for (const auto& pfType : vSH)
   {
      Float64 sh = GetMoment(pierID,pfType,poi);
      SH += sh;
   }

   Float64 PS = 0;
   std::vector<xbrTypes::ProductForceType> vPS = GetLoads(xbrTypes::lcPS);
   for (const auto& pfType : vPS)
   {
      Float64 ps = GetMoment(pierID,pfType,poi);
      PS += ps;
   }

   Float64 RE = 0;
   std::vector<xbrTypes::ProductForceType> vRE = GetLoads(xbrTypes::lcRE);
   for (const auto& pfType : vRE)
   {
      Float64 re = GetMoment(pierID,pfType,poi);
      RE += re;
   }

   Float64 LLIMmin, LLIMmax;
   GetMoment(pierID,ratingType,poi,&LLIMmin,&LLIMmax,nullptr,nullptr);

   *pMin = gDC*DC + gDW*DW + gCR*CR + gSH*SH + gPS*PS + gRE*RE + gLL*LLIMmin;
   *pMax = gDC*DC + gDW*DW + gCR*CR + gSH*SH + gPS*PS + gRE*RE + gLL*LLIMmax;
}

void CAnalysisAgentImp::GetShear(PierIDType pierID,pgsTypes::LimitState limitState,const xbrPointOfInterest& poi,WBFL::System::SectionValue* pMin,WBFL::System::SectionValue* pMax) const
{
   pgsTypes::LoadRatingType ratingType = RatingTypeFromLimitState(limitState);

   GET_IFACE(IXBRProject,pProject);
   Float64 gDC = pProject->GetDCLoadFactor(limitState);
   Float64 gDW = pProject->GetDWLoadFactor(limitState);
   Float64 gCR = pProject->GetCRLoadFactor(limitState);
   Float64 gSH = pProject->GetSHLoadFactor(limitState);
   Float64 gPS = pProject->GetPSLoadFactor(limitState);
   Float64 gRE = pProject->GetRELoadFactor(limitState);
   Float64 gLL = pProject->GetLiveLoadFactor(pierID,limitState,INVALID_INDEX);

   WBFL::System::SectionValue DC = 0;
   std::vector<xbrTypes::ProductForceType> vDC = GetLoads(xbrTypes::lcDC);
   for (const auto& pfType : vDC)
   {
      WBFL::System::SectionValue dc = GetShear(pierID,pfType,poi);
      DC += dc;
   }

   WBFL::System::SectionValue DW = 0;
   std::vector<xbrTypes::ProductForceType> vDW = GetLoads(xbrTypes::lcDW);
   for (const auto& pfType : vDW)
   {
      WBFL::System::SectionValue dw = GetShear(pierID,pfType,poi);
      DW += dw;
   }

   WBFL::System::SectionValue CR = 0;
   std::vector<xbrTypes::ProductForceType> vCR = GetLoads(xbrTypes::lcCR);
   for (const auto& pfType : vCR)
   {
      WBFL::System::SectionValue cr = GetShear(pierID,pfType,poi);
      CR += cr;
   }

   WBFL::System::SectionValue SH = 0;
   std::vector<xbrTypes::ProductForceType> vSH = GetLoads(xbrTypes::lcSH);
   for (const auto& pfType : vSH)
   {
      WBFL::System::SectionValue sh = GetShear(pierID,pfType,poi);
      SH += sh;
   }

   WBFL::System::SectionValue PS = 0;
   std::vector<xbrTypes::ProductForceType> vPS = GetLoads(xbrTypes::lcPS);
   for (const auto& pfType : vPS)
   {
      WBFL::System::SectionValue ps = GetShear(pierID,pfType,poi);
      PS += ps;
   }

   WBFL::System::SectionValue RE = 0;
   std::vector<xbrTypes::ProductForceType> vRE = GetLoads(xbrTypes::lcRE);
   for (const auto& pfType : vRE)
   {
      WBFL::System::SectionValue re = GetShear(pierID,pfType,poi);
      RE += re;
   }

   WBFL::System::SectionValue LLIMmin, LLIMmax;
   GetShear(pierID,ratingType,poi,&LLIMmin,&LLIMmax,nullptr,nullptr,nullptr,nullptr);

   *pMin = gDC*DC + gDW*DW + gCR*CR + gSH*SH + gPS*PS + gRE*RE + gLL*LLIMmin;
   *pMax = gDC*DC + gDW*DW + gCR*CR + gSH*SH + gPS*PS + gRE*RE + gLL*LLIMmax;
}

void CAnalysisAgentImp::GetMoment(PierIDType pierID,pgsTypes::LimitState limitState,const std::vector<xbrPointOfInterest>& vPoi,std::vector<Float64>* pvMin,std::vector<Float64>* pvMax) const
{
   pvMin->clear();
   pvMin->reserve(vPoi.size());
   pvMax->clear();
   pvMax->reserve(vPoi.size());
   for (const auto& poi : vPoi)
   {
      Float64 min,max;
      GetMoment(pierID,limitState,poi,&min,&max);
      pvMin->push_back(min);
      pvMax->push_back(max);
   }
}

void CAnalysisAgentImp::GetShear(PierIDType pierID,pgsTypes::LimitState limitState,const std::vector<xbrPointOfInterest>& vPoi,std::vector<WBFL::System::SectionValue>* pvMin,std::vector<WBFL::System::SectionValue>* pvMax) const
{
   pvMin->clear();
   pvMin->reserve(vPoi.size());
   pvMax->clear();
   pvMax->reserve(vPoi.size());
   for (const auto& poi : vPoi)
   {
      WBFL::System::SectionValue min,max;
      GetShear(pierID,limitState,poi,&min,&max);
      pvMin->push_back(min);
      pvMax->push_back(max);
   }
}

std::vector<xbrTypes::ProductForceType> CAnalysisAgentImp::GetLoads(xbrTypes::CombinedForceType lcType) const
{
   std::vector<xbrTypes::ProductForceType> vPFTypes;
   switch(lcType)
   {
   case xbrTypes::lcDC:
      vPFTypes.push_back(xbrTypes::pftLowerXBeam);
      vPFTypes.push_back(xbrTypes::pftUpperXBeam);
      vPFTypes.push_back(xbrTypes::pftDCReactions);
      break;

   case xbrTypes::lcDW:
      vPFTypes.push_back(xbrTypes::pftDWReactions);
      break;

   case xbrTypes::lcSH:
      vPFTypes.push_back(xbrTypes::pftSHReactions);
      break;

   case xbrTypes::lcCR:
      vPFTypes.push_back(xbrTypes::pftCRReactions);
      break;

   case xbrTypes::lcPS:
      vPFTypes.push_back(xbrTypes::pftPSReactions);
      break;

   case xbrTypes::lcRE:
      vPFTypes.push_back(xbrTypes::pftREReactions);
      break;

   default:
      ATLASSERT(false);
   }

   return vPFTypes;
}

void CAnalysisAgentImp::ComputeLiveLoadLocations(PierIDType pierID,ModelData* pModelData) const
{
   GET_IFACE(IEAFProgress,pProgress);
   WBFL::EAF::AutoProgress ap(pProgress);

   GET_IFACE(IXBRPier,pPier);
   Float64 Wcc = pPier->GetCurbToCurbWidth(pierID); // normal to alignment

   // Get the lane configuration for the curb-to-curb width (measured normal to the alignment)
   Float64 wLane,wLoadedLane;
   IndexType nLanes;
   GetLaneInfo(Wcc,&wLane,&nLanes,&wLoadedLane);


   // Adjust Wcc and wLoadedLane so that they are measured in the plane of the pier
   Float64 skew = pPier->GetSkewAngle(pierID);
   Wcc /= cos(skew);
   wLoadedLane /= cos(skew);

   GET_IFACE(IXBRProject,pProject);
   Float64 maxStepSize = pProject->GetMaxLiveLoadStepSize();

   IndexType nMaxLoadedLanes = pProject->GetMaxLoadedLanes();
   nLanes = Min(nLanes,nMaxLoadedLanes);

   Float64 LCO, RCO;
   pProject->GetCurbLineOffset(pierID,&LCO,&RCO); // this are measured normal to the alignment
   LCO /= cos(skew); // divide by cos(skew) to get measured in plane of pier
   if (pProject->GetCurbLineDatum(pierID) == pgsTypes::omtBridge)
   {
      Float64 BLO = pProject->GetBridgeLineOffset(pierID);
      LCO += BLO;
      RCO += BLO;
   }
   Float64 XcurbLine = pPier->ConvertPierToCrossBeamCoordinate(pierID,LCO);

   CString strProgressMsg;
   for ( IndexType nLoadedLanes = 1; nLoadedLanes <= nLanes; nLoadedLanes++ )
   {
      strProgressMsg.Format(_T("Generating live load placement for %d of %d loaded lanes"),nLoadedLanes,nLanes);
      pProgress->UpdateMessage(strProgressMsg);

      IndexType nLaneGaps = nLoadedLanes-1; // number of "gaps" between loaded lanes
      Float64 Wll = nLoadedLanes*wLoadedLane; // width of live load, measured in the plane of the pier
      IndexType nTotalSteps = (IndexType)ceil((Wcc - Wll)/maxStepSize); // total number of steps to move live load from left to right curb lines
      Float64 stepSize = (0 < nTotalSteps ? (Wcc-Wll)/nTotalSteps : 0); // actual step size

      // The lane configuration (position of the loaded lanes) is defined as a sequence of "gap indicies".
      // The size of the gap between lanes is the gap position index times the step size.
      // The step size of the gaps is equal to the step size for moving the lane configuration transversely
      // along the cross beam
      //
      //  wLoadedLane      wLoadedLane         wLoadedLane
      //  |<------>|       |<------>|          |<------>|
      //  ==========       ==========          ==========
      //            |<--->|          |<------>|
      //              gap               gap
      std::vector<std::vector<IndexType>> vGapPositions;
      IndexType nMaxGapSteps = Min(nMaxLoadedLanes,nTotalSteps); // don't let the gap be any wider than nMaxLoaded lanes. Once the loaded lanes get to far apart, the results don't combine
      GetLaneGapConfiguration(nMaxGapSteps,nLaneGaps,vGapPositions);

      // for each lane configuration, get the wheel line reaction configuration
      // and analyze the structure for the configuration. The configuration, in general,
      // will not be as wide as the curb-to-curb width, so step the configuration until
      // it reaches the right curb line.

      //
      //  wLoadedLanewLoadedLane         wLoadedLane
      //  |<------>|<------>|           |<------>|
      //  ===================           ==========   ---->>> move configuration by (stepIdx)*(step size) until the right lane gets to the right edge of the XBeam
      //           |         |<------->|
      //           gap=0         gap = (gap position idx)*(step size)
      //
      // Increment the gap position and repeat...

      for (const auto& vGapPosition : vGapPositions)
      {
         // location of wheel lines measured from the left curb line... with the first wheel line being 
         // at its left-most position. The position of the second wheel line is governed by
         // the gage spacing of the live load (6', adjusted for skew). The position of the remaining
         // wheel lines are governed by the position of the lane (gap position).
         std::vector<Float64> vWheelLinePositions = GetWheelLinePositions(skew,stepSize,wLoadedLane,vGapPosition);
         ATLASSERT(vWheelLinePositions.size() % 2 == 0); // must be even number... two wheel lines per lane
         ATLASSERT(vWheelLinePositions.size()/2 == nLoadedLanes); // must be 2 times as many wheel lines as loaded lanes

         // get the number of steps used to make the wheel line reaction configuration.
         IndexType nStepsUsed = std::accumulate(vGapPosition.begin(),vGapPosition.end(),(IndexType)0);

         // the number of times we need to step the wheel line reaction configuration towards the right curb line
         // is equal to the total number of steps less the number of steps used by the configuration.
         IndexType nStepsRemaining = nTotalSteps - nStepsUsed;

#if defined _DEBUG
         // 2ft curb line rule doesn't apply for lanes less than 10 ft wide
         if (WBFL::Units::ConvertToSysUnits(10.0, WBFL::Units::Measure::Feet) <= wLoadedLane)
         {
            // at the last step, the right wheel line, in the right-most lane, must be 2' from the right curb line
            Float64 w2 = WBFL::Units::ConvertToSysUnits(2.0, WBFL::Units::Measure::Feet); // 2' shy distance from curb-line
            w2 /= cos(skew);
            ATLASSERT(IsEqual(vWheelLinePositions.back() + stepSize * nStepsRemaining + w2, Wcc));
         }
#endif

         // Step the wheel line reaction configuration towards the right curb line, analyzing the
         // cross beam for each loading position.
         for ( IndexType stepIdx = 0; stepIdx <= nStepsRemaining; stepIdx++ )
         {
            // Xoffset is the amount to shift the wheel line configuration towards the right curb line.
            // Remember the vWheelLinePositions is a configuration with the lanes all shifted to the
            // left curb line. Xoffset moves the entire configuration towards the right curb line
            Float64 Xoffset = stepSize*stepIdx; 
            Xoffset += XcurbLine;

            // Create the information for the wheel line loads that are to be applied to the FEM model.
            // Wheel line position is adjusted from default position by Xoffset.
            LiveLoadConfiguration llConfig;
            llConfig.m_LoadCases = InitializeWheelLineLoads(pModelData,Xoffset,vWheelLinePositions);
            ATLASSERT(llConfig.m_LoadCases.size() == nLoadedLanes);

            pModelData->m_LiveLoadConfigurations.push_back(llConfig);
         } // next step
      } // next gap position
   } // next number of loaded lanes
}

void CAnalysisAgentImp::GetLaneGapConfiguration(IndexType nTotalSteps,IndexType nLaneGaps,std::vector<std::vector<IndexType>>& vGapPositions) const
{
   if ( nLaneGaps == 0 )
   {
      std::vector<IndexType> vDigits;
      vGapPositions.push_back(vDigits);
      return;
   }

   static std::vector<IndexType> vDigits;
   for ( IndexType stepIdx = 0; stepIdx <= nTotalSteps; stepIdx++ )
   {
      vDigits.push_back(stepIdx);
      if ( 1 < nLaneGaps )
      {
         GetLaneGapConfiguration(nTotalSteps-stepIdx,nLaneGaps-1,vGapPositions);
      }
      else
      {
         vGapPositions.push_back(vDigits);
         vDigits.pop_back();
      }
   }

   if ( 0 < vDigits.size() )
   {
      vDigits.pop_back();
   }
}

std::vector<Float64> CAnalysisAgentImp::GetWheelLinePositions(Float64 skew,Float64 stepSize,Float64 wLoadedLane,const std::vector<IndexType>& vGapPosition) const
{
   Float64 w3 = WBFL::Units::ConvertToSysUnits(3.0,WBFL::Units::Measure::Feet); // 6 ft spacing between wheel lines... wheel line is +/-3' from CL lane
   w3 /= cos(skew); // we are working in the plane of the pier, so make skew adjustment

   std::vector<Float64> vLoadPositions;
   // first pair of wheel line loads... at left curb line
   vLoadPositions.push_back(wLoadedLane/2-w3);
   vLoadPositions.push_back(wLoadedLane/2+w3);

   IndexType laneIdx = 1;
   Float64 totalGapWidth = 0;
   for(const auto& gapPosition : vGapPosition)
   {
      Float64 gapWidth = stepSize*gapPosition;
      totalGapWidth += gapWidth;
      
      Float64 XclLane = totalGapWidth + laneIdx*wLoadedLane + wLoadedLane/2;

      Float64 XleftWheelLine  = XclLane - w3;
      Float64 XrightWheelLine = XclLane + w3;

      vLoadPositions.push_back(XleftWheelLine);
      vLoadPositions.push_back(XrightWheelLine);

      laneIdx++;
   }

   return vLoadPositions;
}

std::vector<LoadCaseIDType> CAnalysisAgentImp::InitializeWheelLineLoads(ModelData* pModelData,Float64 Xoffset,const std::vector<Float64>& vWheelLinePositions) const
{
   // keep track of the last load case ID used for one loaded lane
   // vLoadPositions has two point loads per lane, so the number of loaded lanes
   // is half the size of the container
   IndexType nLoadedLanes = vWheelLinePositions.size()/2;

   if ( nLoadedLanes == 1 )
   {
      pModelData->m_LastSingleLaneLLConfigIdx = pModelData->m_LiveLoadConfigurations.size();
   }

   // for each loaded lane, create a FEM2D load case that has the two
   // wheel line loads
   std::vector<LoadCaseIDType> vLoadCases;
   auto iter(vWheelLinePositions.begin());
   auto end(vWheelLinePositions.end());
   for ( ; iter != end; iter += 2 )
   {
      // these are the left curb line justified position of the wheel lines
      Float64 Xleft  = *iter;
      Float64 Xright = *(iter+1);

      // adjust the wheel line positions for the current step
      Xleft  += Xoffset;
      Xright += Xoffset;

      // Check to see if we've previously had a vehicle in this position (stored based on Xcenter)
      Float64 Xcenter = (Xleft + Xright)/2;
      std::map<Float64,LoadCaseIDType,Float64_less>::iterator found = pModelData->m_LiveLoadCases.find(Xcenter);
      if ( found != pModelData->m_LiveLoadCases.end() )
      {
         // there is already a load case for this lane position... simply reference it
         LoadCaseIDType lcid = found->second;
         vLoadCases.push_back(lcid);
         continue;
      }

      // This is a new position... create a FEM2d load case

      LoadCaseIDType lcid = pModelData->m_NextLiveLoadCaseID++;
      vLoadCases.push_back(lcid);

      // save the lane position and load case ID for future reference
      pModelData->m_LiveLoadCases.insert(std::make_pair(Xcenter,lcid));

      // save the lane configuration for future reference (force lane edges to be within the bounds of the model)
      LaneConfiguration laneConfig;
      laneConfig.Xleft  = Max(Xleft,0.0);
      laneConfig.Xright = Min(Xright,pModelData->m_Lmax);
      pModelData->m_LaneConfigurations.insert(std::make_pair(lcid,laneConfig));
   }

   return vLoadCases;
}

void CAnalysisAgentImp::ApplyWheelLineLoadsToFemModel(ModelData* pModelData) const
{
   GET_IFACE(IEAFProgress,pProgress);
   WBFL::EAF::AutoProgress ap(pProgress);
   CString strProgressMsg;

   IndexType nLaneConfigurations = pModelData->m_LaneConfigurations.size();

   WBFL::FEA2D::Model& femModel = *pModelData->m_Model;


   std::map<LoadCaseIDType,LaneConfiguration>::iterator begin(pModelData->m_LaneConfigurations.begin());
   std::map<LoadCaseIDType,LaneConfiguration>::iterator iter(begin);
   std::map<LoadCaseIDType,LaneConfiguration>::iterator end(pModelData->m_LaneConfigurations.end());
   for ( ; iter != end; iter++ )
   {
      IndexType laneConfigIdx = std::distance(begin,iter);
      strProgressMsg.Format(_T("Applying wheel line reactions to lane configuration %d of %d"),laneConfigIdx,nLaneConfigurations);

      LoadCaseIDType lcid = iter->first;
      LaneConfiguration& laneConfig = iter->second;

      // create the load case
      WBFL::FEA2D::Loading& loading = femModel.CreateLoading(lcid);


      // create the loading for each wheel line
      LoadIDType loadID = 0;
      Float64 P = -0.5; // unit lane load = 0.5 per wheel line

      MemberIDType mbrID;
      Float64 mbrLocation;

      // left wheel line load
      GetSuperstructureFemModelLocation(pModelData,laneConfig.Xleft,&mbrID,&mbrLocation);

      loading.CreatePointLoad(loadID++,mbrID,mbrLocation,0.0,P,0.0,WBFL::FEA2D::LoadOrientation::Global);

      // right wheel line load
      GetSuperstructureFemModelLocation(pModelData,laneConfig.Xright,&mbrID,&mbrLocation);

      loading.CreatePointLoad(loadID++,mbrID,mbrLocation,0.0,P,0.0,WBFL::FEA2D::LoadOrientation::Global);
   }
}

std::set<CAnalysisAgentImp::UnitLiveLoadResult>& CAnalysisAgentImp::GetUnitLiveLoadResults(PierIDType pierID) const
{
   std::map<PierIDType,std::set<UnitLiveLoadResult>>::iterator found = m_pUnitLiveLoadResults->find(pierID);
   if ( found == m_pUnitLiveLoadResults->end() )
   {
      std::set<UnitLiveLoadResult> results;
      std::pair<std::map<PierIDType,std::set<UnitLiveLoadResult>>::iterator,bool> result = m_pUnitLiveLoadResults->insert(std::make_pair(pierID,results));
      ATLASSERT(result.second == true);
      found = result.first;
   }

   std::set<UnitLiveLoadResult>& results = (*found).second;
   return results;
}

struct Result
{
   Float64 Value;
   IndexType llConfigIdx;
   Result(Float64 v,IndexType l) : Value(v), llConfigIdx(l) {}
   bool operator<(const Result& result)const { return Value < result.Value; }
};

void CAnalysisAgentImp::ComputeUnitLiveLoadResult(PierIDType pierID,const xbrPointOfInterest& poi) const
{
   GET_IFACE_NOCHECK(IEAFDisplayUnits,pDisplayUnits); // this interface is not used in the rare case that the superstructure width is smaller than a lane, there are no lanes so there aren't any live load results
   GET_IFACE(IEAFProgress,pProgress);
   WBFL::EAF::AutoProgress ap(pProgress);
   CString strProgressMsg;

   Float64 MzMin = DBL_MAX;
   Float64 MzMax = -DBL_MAX;
   IndexType minMz_llConfigIdx = INVALID_INDEX;
   IndexType maxMz_llConfigIdx = INVALID_INDEX;

   Float64 MzMin_SingleLane = DBL_MAX;
   Float64 MzMax_SingleLane = -DBL_MAX;
   IndexType minMz_llConfigIdx_SingleLane = INVALID_INDEX;
   IndexType maxMz_llConfigIdx_SingleLane = INVALID_INDEX;

   WBFL::System::SectionValue FyMin = DBL_MAX;
   WBFL::System::SectionValue FyMax = -DBL_MAX;
   IndexType minFyLeft_llConfigIdx = INVALID_INDEX;
   IndexType maxFyLeft_llConfigIdx = INVALID_INDEX;
   IndexType minFyRight_llConfigIdx = INVALID_INDEX;
   IndexType maxFyRight_llConfigIdx = INVALID_INDEX;

   WBFL::System::SectionValue FyMin_SingleLane = DBL_MAX;
   WBFL::System::SectionValue FyMax_SingleLane = -DBL_MAX;
   IndexType minFyLeft_llConfigIdx_SingleLane = INVALID_INDEX;
   IndexType maxFyLeft_llConfigIdx_SingleLane = INVALID_INDEX;
   IndexType minFyRight_llConfigIdx_SingleLane = INVALID_INDEX;
   IndexType maxFyRight_llConfigIdx_SingleLane = INVALID_INDEX;

   ModelData* pModelData = GetModelData(pierID);

   std::map<PoiIDType,PoiIDType>::iterator found = pModelData->m_PoiMap.find(poi.GetID());
   ATLASSERT(found != pModelData->m_PoiMap.end());
   PoiIDType femPoiID = found->second;

   std::set<Result> moments;
   std::set<Result> shears;

   WBFL::FEA2D::Model& femModel = *pModelData->m_Model;

   IndexType nLiveLoadConfigs = pModelData->m_LiveLoadConfigurations.size();

   IndexType progressMsgIdx = Min((IndexType)100,nLiveLoadConfigs/10); // we don't want to update the progress message on every loop... the progress window flickers
   progressMsgIdx = Max((IndexType)1,progressMsgIdx); // this is a devisor so it can never be zero
   // when the configuration index is a multiple of the above value, update the message, but not more than every 100 times through the loop

   for ( IndexType llConfigIdx = 0; llConfigIdx < nLiveLoadConfigs; llConfigIdx++ )
   {
      LiveLoadConfiguration& llConfig = pModelData->m_LiveLoadConfigurations[llConfigIdx];

      IndexType nLoadedLanes = llConfig.m_LoadCases.size();

      if ( llConfigIdx % progressMsgIdx == 0 )
      {
         strProgressMsg.Format(_T("Computing unit live load response at %s for configuration %d of %d - %d loaded lanes"),::FormatDimension(poi.GetDistFromStart(),pDisplayUnits->GetSpanLengthUnit()),llConfigIdx,nLiveLoadConfigs,nLoadedLanes);
         pProgress->UpdateMessage(strProgressMsg);
      }

      Float64 mpf = WBFL::LRFD::Utility::GetMultiplePresenceFactor(nLoadedLanes);

      Float64 FxLeft(0), FyLeft(0), MzLeft(0);
      Float64 FxRight(0), FyRight(0), MzRight(0);

      for ( IndexType laneIdx = 0; laneIdx < nLoadedLanes; laneIdx++ )
      {
         LoadCaseIDType lcid = llConfig.m_LoadCases[laneIdx];

         Float64 fxLeft, fyLeft, mzLeft;
         femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Left,WBFL::FEA2D::LoadOrientation::Member,&fxLeft,&fyLeft,&mzLeft);

         Float64 fxRight, fyRight, mzRight;
         femModel.ComputePOIForces(lcid,femPoiID,WBFL::FEA2D::MemberFaceType::Right,WBFL::FEA2D::LoadOrientation::Member,&fxRight,&fyRight,&mzRight);

         FxLeft += fxLeft;
         FyLeft += fyLeft;
         MzLeft += mzLeft;

         FxRight += fxRight;
         FyRight += fyRight;
         MzRight += mzRight;
      }

      Float64 Mz;
      if ( IsZero(poi.GetDistFromStart()) )
      {
         Mz = -MzRight;
      }
      else
      {
         Mz = MzLeft;
      }

      Mz = IsZero(Mz) ? 0 : Mz;
      Mz *= mpf;

      moments.insert(Result(Mz,llConfigIdx));

      if ( Mz < MzMin )
      {
         MzMin = Mz;
         minMz_llConfigIdx = llConfigIdx;
      }

      if ( MzMax < Mz )
      {
         MzMax = Mz;
         maxMz_llConfigIdx = llConfigIdx;
      }


      if ( llConfigIdx <= pModelData->m_LastSingleLaneLLConfigIdx )
      {
         if ( Mz < MzMin_SingleLane )
         {
            MzMin_SingleLane = Mz;
            minMz_llConfigIdx_SingleLane = llConfigIdx;
         }

         if ( MzMax_SingleLane < Mz )
         {
            MzMax_SingleLane = Mz;
            maxMz_llConfigIdx_SingleLane = llConfigIdx;
         }
      }

      FyLeft  = IsZero(FyLeft)  ? 0 : FyLeft;
      FyRight = IsZero(FyRight) ? 0 : FyRight;
      WBFL::System::SectionValue Fy(-FyLeft,FyRight);

      Fy *= mpf;

      shears.insert(Result(Max(fabs(Fy.Left()),fabs(Fy.Right())),llConfigIdx)); // want shear with maximum magnitude

      if ( Fy.Left() < FyMin.Left() )
      {
         FyMin.Left() = Fy.Left();
         minFyLeft_llConfigIdx = llConfigIdx;
      }

      if ( Fy.Right() < FyMin.Right() )
      {
         FyMin.Right() = Fy.Right();
         minFyRight_llConfigIdx = llConfigIdx;
      }

      if ( FyMax.Left() < Fy.Left() )
      {
         FyMax.Left() = Fy.Left();
         maxFyLeft_llConfigIdx = llConfigIdx;
      }

      if ( FyMax.Right() < Fy.Right() )
      {
         FyMax.Right() = Fy.Right();
         maxFyRight_llConfigIdx = llConfigIdx;
      }


      if ( llConfigIdx <= pModelData->m_LastSingleLaneLLConfigIdx )
      {
         if ( Fy.Left() < FyMin_SingleLane.Left() )
         {
            FyMin_SingleLane.Left() = Fy.Left();
            minFyLeft_llConfigIdx_SingleLane = llConfigIdx;
         }

         if ( Fy.Right() < FyMin_SingleLane.Right() )
         {
            FyMin_SingleLane.Right() = Fy.Right();
            minFyRight_llConfigIdx_SingleLane = llConfigIdx;
         }

         if ( FyMax_SingleLane.Left() < Fy.Left() )
         {
            FyMax_SingleLane.Left() = Fy.Left();
            maxFyLeft_llConfigIdx_SingleLane = llConfigIdx;
         }

         if ( FyMax_SingleLane.Right() < Fy.Right() )
         {
            FyMax_SingleLane.Right() = Fy.Right();
            maxFyRight_llConfigIdx_SingleLane = llConfigIdx;
         }
      }
   }

   UnitLiveLoadResult liveLoadResult;
   liveLoadResult.m_idPOI = poi.GetID();
   liveLoadResult.m_MzMax = MzMax;
   liveLoadResult.m_MzMin = MzMin;
   liveLoadResult.m_llConfigIdx_MzMin = minMz_llConfigIdx;
   liveLoadResult.m_llConfigIdx_MzMax = maxMz_llConfigIdx;
   liveLoadResult.m_FyMax = FyMax;
   liveLoadResult.m_FyMin = FyMin;
   liveLoadResult.m_llConfigIdx_FyLeftMin = minFyLeft_llConfigIdx;
   liveLoadResult.m_llConfigIdx_FyLeftMax = maxFyLeft_llConfigIdx;
   liveLoadResult.m_llConfigIdx_FyRightMin = minFyRight_llConfigIdx;
   liveLoadResult.m_llConfigIdx_FyRightMax = maxFyRight_llConfigIdx;

   liveLoadResult.m_MzMax_SingleLane = MzMax_SingleLane;
   liveLoadResult.m_MzMin_SingleLane = MzMin_SingleLane;
   liveLoadResult.m_llConfigIdx_MzMin_SingleLane = minMz_llConfigIdx_SingleLane;
   liveLoadResult.m_llConfigIdx_MzMax_SingleLane = maxMz_llConfigIdx_SingleLane;
   liveLoadResult.m_FyMax_SingleLane = FyMax_SingleLane;
   liveLoadResult.m_FyMin_SingleLane = FyMin_SingleLane;
   liveLoadResult.m_llConfigIdx_FyLeftMin_SingleLane = minFyLeft_llConfigIdx_SingleLane;
   liveLoadResult.m_llConfigIdx_FyLeftMax_SingleLane = maxFyLeft_llConfigIdx_SingleLane;
   liveLoadResult.m_llConfigIdx_FyRightMin_SingleLane = minFyRight_llConfigIdx_SingleLane;
   liveLoadResult.m_llConfigIdx_FyRightMax_SingleLane = maxFyRight_llConfigIdx_SingleLane;

   // moments is sorted least to greatest... N minimum moments are
   // at the begining of the sequence... the N maximum moments are at
   // the end of the sequence.
   // Use forward iterator at start of sequence
   int N = MAX_CASES; // using 50 min/max moments
   std::set<Result>::iterator fmIter(moments.begin());
   std::set<Result>::iterator fmEnd(moments.end());
   for ( int i = 0; i < N && fmIter != fmEnd; i++, fmIter++)
   {
      Result& mr(const_cast<Result&>(*fmIter));
      liveLoadResult.m_MzMinLiveLoadConfigs.push_back(mr.llConfigIdx);
   }

   // Use reverse iterator at end of sequence
   std::set<Result>::reverse_iterator rmIter(moments.rbegin());
   std::set<Result>::reverse_iterator rmEnd(moments.rend());
   std::set<Result>::reverse_iterator rvIter(shears.rbegin());
   std::set<Result>::reverse_iterator rvEnd(shears.rend());
   for ( int i = 0; i < N && rmIter != rmEnd && rvIter != rvEnd; i++, rmIter++, rvIter++ )
   {
      Result& mr(const_cast<Result&>(*rmIter));
      liveLoadResult.m_MzMaxLiveLoadConfigs.push_back(mr.llConfigIdx);

      Result& vr(const_cast<Result&>(*rvIter));
      liveLoadResult.m_FyLiveLoadConfigs.push_back(vr.llConfigIdx);
   }

   std::set<UnitLiveLoadResult>& liveLoadResults = GetUnitLiveLoadResults(pierID);
   liveLoadResults.insert(liveLoadResult);
}

CAnalysisAgentImp::UnitLiveLoadResult& CAnalysisAgentImp::GetUnitLiveLoadResult(PierIDType pierID,const xbrPointOfInterest& poi) const
{
   std::set<UnitLiveLoadResult>& liveLoadResults = GetUnitLiveLoadResults(pierID);
   UnitLiveLoadResult key;
   key.m_idPOI = poi.GetID();
   std::set<UnitLiveLoadResult>::iterator found = liveLoadResults.find(key);
   if ( found == liveLoadResults.end() )
   {
      ComputeUnitLiveLoadResult(pierID,poi);
      found = liveLoadResults.find(key);
      ATLASSERT(found != liveLoadResults.end());
   }

   return const_cast<UnitLiveLoadResult&>(*found);
}

Float64 CAnalysisAgentImp::GetMaxLegalReaction(PierIDType pierID) const
{
   GET_IFACE(IXBRProject,pProject);

   Float64 Rlgl = -DBL_MAX;
   for ( int i = 0; i < 2; i++ )
   {
      pgsTypes::LoadRatingType legalRatingType = (i == 0 ? pgsTypes::lrLegal_Routine : pgsTypes::lrLegal_Special);
      VehicleIndexType nVehicles = pProject->GetLiveLoadReactionCount(pierID,legalRatingType);
      for ( VehicleIndexType vehicleIdx = 0; vehicleIdx < nVehicles; vehicleIdx++ )
      {
         Float64 r_legal = pProject->GetLiveLoadReaction(pierID,legalRatingType,vehicleIdx);
         Rlgl = Max(Rlgl,r_legal);
      }
   }
   return Rlgl;
}

std::vector<Float64> CAnalysisAgentImp::GetWheelLineLocations(ModelData* pModelData) const
{
   std::vector<Float64> vWheelLineLocations;

   std::map<LoadCaseIDType,LaneConfiguration>::iterator iter(pModelData->m_LaneConfigurations.begin());
   std::map<LoadCaseIDType,LaneConfiguration>::iterator end(pModelData->m_LaneConfigurations.end());
   for ( ; iter != end; iter++ )
   {
      LaneConfiguration& laneConfig(iter->second);
      vWheelLineLocations.push_back(laneConfig.Xleft);
      vWheelLineLocations.push_back(laneConfig.Xright);
   }
   return vWheelLineLocations;
}

void CAnalysisAgentImp::InvalidateModels(bool bCreateNewDataStructures)
{
   if (m_pModelData)
   {
      std::map<PierIDType, ModelData>* pOldModelData = m_pModelData.release();
      if (bCreateNewDataStructures)
      {
         m_pModelData = std::make_unique<std::map<PierIDType, ModelData>>();
      }

#if defined _USE_MULTITHREADING
      m_ThreadManager.CreateThread(CAnalysisAgentImp::DeleteModels, (LPVOID)(pOldModelData));
#else
      CAnalysisAgentImp::DeleteModels((LPVOID)(pOldModelData));
#endif
   }
}

UINT CAnalysisAgentImp::DeleteModels(LPVOID pParam)
{
   WATCH(_T("Begin: DeleteModels"));
   
   std::map<PierIDType,ModelData>* pModelData = (std::map<PierIDType,ModelData>*)pParam;
   if (pModelData)
   {
      pModelData->clear();
      delete pModelData;
   }

   WATCH(_T("End: DeleteModels"));

   return 0;
}

void CAnalysisAgentImp::InvalidateResults(bool bCreateNewDataStructures)
{
   if (m_pUnitLiveLoadResults)
   {
      std::map<PierIDType, std::set<UnitLiveLoadResult>>* pOldResults = m_pUnitLiveLoadResults.release();
      if (bCreateNewDataStructures)
      {
         m_pUnitLiveLoadResults = std::make_unique<std::map<PierIDType, std::set<UnitLiveLoadResult>>>();
      }

#if defined _USE_MULTITHREADING
      m_ThreadManager.CreateThread(CAnalysisAgentImp::DeleteResults, (LPVOID)(pOldResults));
#else
      CAnalysisAgentImp::DeleteResults((LPVOID)(pOldResults));
#endif
   }
}

UINT CAnalysisAgentImp::DeleteResults(LPVOID pParam)
{
   WATCH(_T("Begin: DeleteResults"));
   
   std::map<PierIDType,std::set<UnitLiveLoadResult>>* pResults = (std::map<PierIDType,std::set<UnitLiveLoadResult>>*)pParam;
   if (pResults)
   {
      pResults->clear();
      delete pResults;
   }

   WATCH(_T("End: DeleteResults"));

   return 0;
}
