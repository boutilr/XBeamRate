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
#include "txnEditReinforcement.h"

#include <AgentTools.h>
#include <IFace\Project.h>



txnEditReinforcement::txnEditReinforcement(PierIDType pierID,const xbrEditReinforcementData& oldReinforcement,const xbrEditReinforcementData& newReinforcement)
{
   ATLASSERT(pierID != INVALID_ID);
   m_PierID = pierID;
   m_Reinforcement[0] = oldReinforcement;
   m_Reinforcement[1] = newReinforcement;
}

txnEditReinforcement::~txnEditReinforcement(void)
{
}

bool txnEditReinforcement::Execute()
{
   Execute(1);
   return true;
}

void txnEditReinforcement::Undo()
{
   Execute(0);
}

void txnEditReinforcement::Execute(int i)
{
   auto pBroker = EAFGetBroker();


   GET_IFACE2(pBroker,IXBREvents, pEvents);
   pEvents->HoldEvents(); // don't fire any changed events until all changes are done

   GET_IFACE2(pBroker,IXBRProject,pProject);
   pProject->SetRebarMaterial(m_PierID,m_Reinforcement[i].Type,m_Reinforcement[i].Grade);
   pProject->SetLongitudinalRebar(m_PierID,m_Reinforcement[i].LongitudinalRebar);
   pProject->SetLowerXBeamStirrups(m_PierID,m_Reinforcement[i].LowerXBeamStirrups);
   pProject->SetFullDepthStirrups(m_PierID,m_Reinforcement[i].FullDepthStirrups);
   pProject->SetConditionFactor(m_PierID,m_Reinforcement[i].ConditionFactorType,m_Reinforcement[i].ConditionFactor);

   pEvents->FirePendingEvents();
}

std::unique_ptr<WBFL::EAF::Transaction> txnEditReinforcement::CreateClone() const
{
   return std::make_unique<txnEditReinforcement>(m_PierID,m_Reinforcement[0],m_Reinforcement[1]);
}

std::_tstring txnEditReinforcement::Name() const
{
   return _T("Edit Cross Beam Reinforcement");
}

bool txnEditReinforcement::IsUndoable() const
{
   return true;
}

bool txnEditReinforcement::IsRepeatable() const
{
   return false;
}
