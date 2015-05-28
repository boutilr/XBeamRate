///////////////////////////////////////////////////////////////////////
// PGSuper - Prestressed Girder SUPERstructure Design and Analysis
// Copyright � 1999-2015  Washington State Department of Transportation
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

#include <WBFLCore.h>
#include <XBeamRateExt\PierData.h>

class txnDeadLoadReaction
{
public:
   txnDeadLoadReaction();
   Float64 m_DC;
   Float64 m_DW;
};

class txnLiveLoadReactions
{
public:
   txnLiveLoadReactions();
   std::vector<std::pair<std::_tstring,Float64>> m_LLIM;
};

class txnEditPierData
{
public:
   txnEditPierData();

   xbrPierData m_PierData;

   Float64 m_gDC;
   Float64 m_gDW;
   Float64 m_gLL[6]; // use pgsTypes::LoadRatingType to access array

   std::vector<txnDeadLoadReaction> m_DeadLoadReactions[2];

   txnLiveLoadReactions m_DesignLiveLoad;
   txnLiveLoadReactions m_LegalRoutineLiveLoad;
   txnLiveLoadReactions m_LegalSpecialLiveLoad;
   txnLiveLoadReactions m_PermitRoutineLiveLoad;
   txnLiveLoadReactions m_PermitSpecialLiveLoad;
};

class txnEditPier :
   public txnTransaction
{
public:
   txnEditPier(const txnEditPierData& oldPierData,const txnEditPierData& newPierData);
   ~txnEditPier(void);

   virtual bool Execute();
   virtual void Undo();
   virtual txnTransaction* CreateClone() const;
   virtual std::_tstring Name() const;
   virtual bool IsUndoable();
   virtual bool IsRepeatable();

private:
   void Execute(int i);

   txnEditPierData m_PierData[2];

};
