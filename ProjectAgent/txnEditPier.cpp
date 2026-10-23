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
#include "ProjectAgent.h"
#include "txnEditPier.h"
#include <XBeamRateExt\XBeamRateUtilities.h>

#include <IFace\Project.h>



txnDeadLoadReaction::txnDeadLoadReaction()
{
   m_DC = 0;
   m_DW = 0;
   m_CR = 0;
   m_SH = 0;
   m_PS = 0;
   m_RE = 0;
   m_W  = 0;
}

txnLiveLoadReactions::txnLiveLoadReactions()
{
}

/////////////////////////////////////////////////////////////////
txnEditPierData::txnEditPierData()
{
   m_DeadLoadReactionType[0] = xbrTypes::rltConcentrated;
   m_DeadLoadReactionType[1] = xbrTypes::rltConcentrated;
}

/////////////////////////////////////////////////////////////////
txnEditPier::txnEditPier(const txnEditPierData& oldPierData,const txnEditPierData& newPierData)
{
   m_PierData[0] = oldPierData;
   m_PierData[1] = newPierData;
}

txnEditPier::~txnEditPier(void)
{
}

bool txnEditPier::Execute()
{
   Execute(1);
   return true;
}

void txnEditPier::Undo()
{
   Execute(0);
}

void txnEditPier::Execute(int i)
{
   auto pBroker = EAFGetBroker();


   GET_IFACE2(pBroker,IXBREvents, pEvents);
   pEvents->HoldEvents(); // don't fire any changed events until all changes are done

   PierIDType pierID = INVALID_ID;

   GET_IFACE2(pBroker,IXBRProject,pProject);
   ATLASSERT(m_PierData[i].m_PierData.GetID() == pierID);
   pProject->SetPierData(m_PierData[i].m_PierData);

   IndexType nBearingLines = m_PierData[i].m_PierData.GetBearingLineCount();
   for ( IndexType brgLineIdx = 0; brgLineIdx < nBearingLines; brgLineIdx++)
   {
      xbrTypes::ReactionLoadType brgReactionType = m_PierData[i].m_DeadLoadReactionType[brgLineIdx];
      pProject->SetBearingReactionType(pierID,brgLineIdx,brgReactionType);
      IndexType nBearings = m_PierData[i].m_PierData.GetBearingCount(brgLineIdx);
      for ( IndexType brgIdx = 0; brgIdx < nBearings; brgIdx++ )
      {
         Float64 DC = m_PierData[i].m_DeadLoadReactions[brgLineIdx][brgIdx].m_DC;
         Float64 DW = m_PierData[i].m_DeadLoadReactions[brgLineIdx][brgIdx].m_DW;
         Float64 CR = m_PierData[i].m_DeadLoadReactions[brgLineIdx][brgIdx].m_CR;
         Float64 SH = m_PierData[i].m_DeadLoadReactions[brgLineIdx][brgIdx].m_SH;
         Float64 PS = m_PierData[i].m_DeadLoadReactions[brgLineIdx][brgIdx].m_PS;
         Float64 RE = m_PierData[i].m_DeadLoadReactions[brgLineIdx][brgIdx].m_RE;
         Float64 W  = m_PierData[i].m_DeadLoadReactions[brgLineIdx][brgIdx].m_W;
         pProject->SetBearingReactions(pierID,brgLineIdx,brgIdx,DC,DW,CR,SH,PS,RE,W);
      }
   }

   pProject->SetDCLoadFactor(pgsTypes::StrengthI_Inventory,m_PierData[i].m_gDC_StrengthI);
   pProject->SetDWLoadFactor(pgsTypes::StrengthI_Inventory,m_PierData[i].m_gDW_StrengthI);
   pProject->SetCRLoadFactor(pgsTypes::StrengthI_Inventory,m_PierData[i].m_gCR_StrengthI);
   pProject->SetSHLoadFactor(pgsTypes::StrengthI_Inventory,m_PierData[i].m_gSH_StrengthI);
   pProject->SetRELoadFactor(pgsTypes::StrengthI_Inventory,m_PierData[i].m_gCR_StrengthI); // using creep factor for relaxation
   pProject->SetPSLoadFactor(pgsTypes::StrengthI_Inventory,m_PierData[i].m_gPS_StrengthI);

   pProject->SetDCLoadFactor(pgsTypes::StrengthI_Operating,m_PierData[i].m_gDC_StrengthI);
   pProject->SetDWLoadFactor(pgsTypes::StrengthI_Operating,m_PierData[i].m_gDW_StrengthI);
   pProject->SetCRLoadFactor(pgsTypes::StrengthI_Operating,m_PierData[i].m_gCR_StrengthI);
   pProject->SetSHLoadFactor(pgsTypes::StrengthI_Operating,m_PierData[i].m_gSH_StrengthI);
   pProject->SetRELoadFactor(pgsTypes::StrengthI_Operating,m_PierData[i].m_gCR_StrengthI); // using creep factor for relaxation
   pProject->SetPSLoadFactor(pgsTypes::StrengthI_Operating,m_PierData[i].m_gPS_StrengthI);

   pProject->SetDCLoadFactor(pgsTypes::StrengthI_LegalRoutine,m_PierData[i].m_gDC_StrengthI);
   pProject->SetDWLoadFactor(pgsTypes::StrengthI_LegalRoutine,m_PierData[i].m_gDW_StrengthI);
   pProject->SetCRLoadFactor(pgsTypes::StrengthI_LegalRoutine,m_PierData[i].m_gCR_StrengthI);
   pProject->SetSHLoadFactor(pgsTypes::StrengthI_LegalRoutine,m_PierData[i].m_gSH_StrengthI);
   pProject->SetRELoadFactor(pgsTypes::StrengthI_LegalRoutine,m_PierData[i].m_gCR_StrengthI); // using creep factor for relaxation
   pProject->SetPSLoadFactor(pgsTypes::StrengthI_LegalRoutine,m_PierData[i].m_gPS_StrengthI);

   pProject->SetDCLoadFactor(pgsTypes::StrengthI_LegalSpecial, m_PierData[i].m_gDC_StrengthI);
   pProject->SetDWLoadFactor(pgsTypes::StrengthI_LegalSpecial, m_PierData[i].m_gDW_StrengthI);
   pProject->SetCRLoadFactor(pgsTypes::StrengthI_LegalSpecial, m_PierData[i].m_gCR_StrengthI);
   pProject->SetSHLoadFactor(pgsTypes::StrengthI_LegalSpecial, m_PierData[i].m_gSH_StrengthI);
   pProject->SetRELoadFactor(pgsTypes::StrengthI_LegalSpecial, m_PierData[i].m_gCR_StrengthI); // using creep factor for relaxation
   pProject->SetPSLoadFactor(pgsTypes::StrengthI_LegalSpecial, m_PierData[i].m_gPS_StrengthI);

   pProject->SetDCLoadFactor(pgsTypes::StrengthI_LegalEmergency, m_PierData[i].m_gDC_StrengthI);
   pProject->SetDWLoadFactor(pgsTypes::StrengthI_LegalEmergency, m_PierData[i].m_gDW_StrengthI);
   pProject->SetCRLoadFactor(pgsTypes::StrengthI_LegalEmergency, m_PierData[i].m_gCR_StrengthI);
   pProject->SetSHLoadFactor(pgsTypes::StrengthI_LegalEmergency, m_PierData[i].m_gSH_StrengthI);
   pProject->SetRELoadFactor(pgsTypes::StrengthI_LegalEmergency, m_PierData[i].m_gCR_StrengthI); // using creep factor for relaxation
   pProject->SetPSLoadFactor(pgsTypes::StrengthI_LegalEmergency, m_PierData[i].m_gPS_StrengthI);

   pProject->SetDCLoadFactor(pgsTypes::StrengthII_PermitRoutine,m_PierData[i].m_gDC_StrengthI);
   pProject->SetDWLoadFactor(pgsTypes::StrengthII_PermitRoutine,m_PierData[i].m_gDW_StrengthI);
   pProject->SetCRLoadFactor(pgsTypes::StrengthII_PermitRoutine,m_PierData[i].m_gCR_StrengthI);
   pProject->SetSHLoadFactor(pgsTypes::StrengthII_PermitRoutine,m_PierData[i].m_gSH_StrengthI);
   pProject->SetRELoadFactor(pgsTypes::StrengthII_PermitRoutine,m_PierData[i].m_gCR_StrengthI); // using creep factor for relaxation
   pProject->SetPSLoadFactor(pgsTypes::StrengthII_PermitRoutine,m_PierData[i].m_gPS_StrengthI);

   pProject->SetDCLoadFactor(pgsTypes::StrengthII_PermitSpecial,m_PierData[i].m_gDC_StrengthI);
   pProject->SetDWLoadFactor(pgsTypes::StrengthII_PermitSpecial,m_PierData[i].m_gDW_StrengthI);
   pProject->SetCRLoadFactor(pgsTypes::StrengthII_PermitSpecial,m_PierData[i].m_gCR_StrengthI);
   pProject->SetSHLoadFactor(pgsTypes::StrengthII_PermitSpecial,m_PierData[i].m_gSH_StrengthI);
   pProject->SetRELoadFactor(pgsTypes::StrengthII_PermitSpecial,m_PierData[i].m_gCR_StrengthI); // using creep factor for relaxation
   pProject->SetPSLoadFactor(pgsTypes::StrengthII_PermitSpecial,m_PierData[i].m_gPS_StrengthI);

   pProject->SetDCLoadFactor(pgsTypes::ServiceI_PermitRoutine,m_PierData[i].m_gDC_ServiceI);
   pProject->SetDWLoadFactor(pgsTypes::ServiceI_PermitRoutine,m_PierData[i].m_gDW_ServiceI);
   pProject->SetCRLoadFactor(pgsTypes::ServiceI_PermitRoutine,m_PierData[i].m_gCR_ServiceI);
   pProject->SetSHLoadFactor(pgsTypes::ServiceI_PermitRoutine,m_PierData[i].m_gSH_ServiceI);
   pProject->SetRELoadFactor(pgsTypes::ServiceI_PermitRoutine,m_PierData[i].m_gCR_ServiceI); // using creep factor for relaxation
   pProject->SetPSLoadFactor(pgsTypes::ServiceI_PermitRoutine,m_PierData[i].m_gPS_ServiceI);

   pProject->SetDCLoadFactor(pgsTypes::ServiceI_PermitSpecial,m_PierData[i].m_gDC_ServiceI);
   pProject->SetDWLoadFactor(pgsTypes::ServiceI_PermitSpecial,m_PierData[i].m_gDW_ServiceI);
   pProject->SetCRLoadFactor(pgsTypes::ServiceI_PermitSpecial,m_PierData[i].m_gCR_ServiceI);
   pProject->SetSHLoadFactor(pgsTypes::ServiceI_PermitSpecial,m_PierData[i].m_gSH_ServiceI);
   pProject->SetRELoadFactor(pgsTypes::ServiceI_PermitSpecial,m_PierData[i].m_gCR_ServiceI); // using creep factor for relaxation
   pProject->SetPSLoadFactor(pgsTypes::ServiceI_PermitSpecial,m_PierData[i].m_gPS_ServiceI);

   pProject->SetLiveLoadFactor(pierID,pgsTypes::StrengthI_Inventory,      m_PierData[i].m_gLL[0]);
   pProject->SetLiveLoadFactor(pierID,pgsTypes::StrengthI_Operating,      m_PierData[i].m_gLL[1]);
   pProject->SetLiveLoadFactor(pierID,pgsTypes::StrengthI_LegalRoutine,   m_PierData[i].m_gLL[2]);
   pProject->SetLiveLoadFactor(pierID, pgsTypes::StrengthI_LegalSpecial, m_PierData[i].m_gLL[3]);
   pProject->SetLiveLoadFactor(pierID, pgsTypes::StrengthI_LegalEmergency, m_PierData[i].m_gLL[4]);
   pProject->SetLiveLoadFactor(pierID,pgsTypes::StrengthII_PermitRoutine, m_PierData[i].m_gLL[5]);
   pProject->SetLiveLoadFactor(pierID,pgsTypes::StrengthII_PermitSpecial, m_PierData[i].m_gLL[6]);
   pProject->SetLiveLoadFactor(pierID,pgsTypes::ServiceI_PermitRoutine,   m_PierData[i].m_gLL[7]);
   pProject->SetLiveLoadFactor(pierID,pgsTypes::ServiceI_PermitSpecial,   m_PierData[i].m_gLL[8]);

   pProject->SetLiveLoadReactions(pierID,pgsTypes::lrDesign_Inventory, m_PierData[i].m_DesignLiveLoad.m_LLIM);
   pProject->SetLiveLoadReactions(pierID,pgsTypes::lrDesign_Operating, m_PierData[i].m_DesignLiveLoad.m_LLIM);
   pProject->SetLiveLoadReactions(pierID,pgsTypes::lrLegal_Routine,    m_PierData[i].m_LegalRoutineLiveLoad.m_LLIM);
   pProject->SetLiveLoadReactions(pierID, pgsTypes::lrLegal_Special, m_PierData[i].m_LegalSpecialLiveLoad.m_LLIM);
   pProject->SetLiveLoadReactions(pierID, pgsTypes::lrLegal_Emergency, m_PierData[i].m_LegalEmergencyLiveLoad.m_LLIM);
   pProject->SetLiveLoadReactions(pierID,pgsTypes::lrPermit_Routine,   m_PierData[i].m_PermitRoutineLiveLoad.m_LLIM);
   pProject->SetLiveLoadReactions(pierID,pgsTypes::lrPermit_Special,   m_PierData[i].m_PermitSpecialLiveLoad.m_LLIM);

   pEvents->FirePendingEvents();
}

std::unique_ptr<WBFL::EAF::Transaction> txnEditPier::CreateClone() const
{
   return std::make_unique<txnEditPier>(m_PierData[0],m_PierData[1]);
}

std::_tstring txnEditPier::Name() const
{
   return _T("Edit Pier");
}

bool txnEditPier::IsUndoable() const
{
   return true;
}

bool txnEditPier::IsRepeatable() const
{
   return false;
}
