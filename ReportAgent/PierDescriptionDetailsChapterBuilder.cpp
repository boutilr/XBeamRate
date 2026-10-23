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

#include "StdAfx.h"
#include "ReportAgent.h"
#include "PierDescriptionDetailsChapterBuilder.h"
#include <EAF\EAFDisplayUnits.h>

#include <IFace\Project.h>
#include <IFace\Pier.h>

#include "XBeamRateReportSpecification.h"

#include <XBeamRateExt\LongitudinalRebarData.h>
#include <XBeamRateExt\StirrupData.h>

#include <PsgLib\GirderLabel.h>
#include <PGSuperUIUtil.h>


void write_superstructure_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID);
void write_substructure_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID);
void write_concrete_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID);
void write_reinforcement_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID);
void write_longitudinal_reinforcement_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID);
void write_transverse_reinforcement_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID,xbrTypes::Stage stage);

CPierDescriptionDetailsChapterBuilder::CPierDescriptionDetailsChapterBuilder()
{
}

LPCTSTR CPierDescriptionDetailsChapterBuilder::GetName() const
{
   return TEXT("Pier Description Details");
}

rptChapter* CPierDescriptionDetailsChapterBuilder::Build(const std::shared_ptr<const WBFL::ReportMgr::ReportSpecification>& pRptSpec,Uint16 level) const
{
   USES_CONVERSION;

   auto pXBRRptSpec = std::dynamic_pointer_cast<const CXBeamRateReportSpecification>(pRptSpec);

   // This report does not use the passd span and girder parameters
   rptChapter* pChapter = CXBeamRateChapterBuilder::Build(pRptSpec,level);

   auto pBroker = pXBRRptSpec->GetBroker();


   PierIDType pierID = pXBRRptSpec->GetPierID();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   write_superstructure_data(pBroker,pDisplayUnits,pChapter,pierID);
   write_substructure_data(pBroker,pDisplayUnits,pChapter,pierID);
   write_concrete_data(pBroker,pDisplayUnits,pChapter,pierID);
   write_reinforcement_data(pBroker,pDisplayUnits,pChapter,pierID);
   write_longitudinal_reinforcement_data(pBroker,pDisplayUnits,pChapter,pierID);
   write_transverse_reinforcement_data(pBroker,pDisplayUnits,pChapter,pierID,xbrTypes::Stage1);
   write_transverse_reinforcement_data(pBroker,pDisplayUnits,pChapter,pierID,xbrTypes::Stage2);

   return pChapter;
}


void write_superstructure_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID)
{
   INIT_UV_PROTOTYPE( rptLengthUnitValue, elevation, pDisplayUnits->GetAlignmentLengthUnit(), true );
   INIT_UV_PROTOTYPE( rptLengthUnitValue, length, pDisplayUnits->GetSpanLengthUnit(), true );
   INIT_UV_PROTOTYPE( rptLengthUnitValue, dim, pDisplayUnits->GetComponentDimUnit(), true );

   GET_IFACE2(pBroker,IXBRProject,pProject);
   rptParagraph* pPara = new rptParagraph(rptStyleManager::GetHeadingStyle());
   *pChapter << pPara;
   *pPara << _T("Superstructure Properties") << rptNewLine;

   pPara = new rptParagraph;
   *pChapter << pPara;

   CString strDatumType[] = {_T("Alignment"), _T("BridgeLine") };
   CString strPierTypes[] = {_T("Continuous"),_T("Integral"),_T("Expansion")};

   CString strImageName;
   strImageName.Format(_T("XBR_Superstructure_%s_%s.png"),strPierTypes[pProject->GetPierType(pierID)],strDatumType[pProject->GetCurbLineDatum(pierID)]);
   *pPara << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + std::_tstring(strImageName)) << rptNewLine << rptNewLine;

   *pPara << _T("Horizonal dimensions are normal to the alignment, except for W which is normal to the plane of the pier.") << rptNewLine;

   rptRcTable* pTable = rptStyleManager::CreateTableNoHeading(2);
   pTable->SetColumnStyle(0,rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   pTable->SetStripeRowColumnStyle(0,rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));
   *pPara << pTable << rptNewLine;



   RowIndexType row = 0;
   (*pTable)(row,0) << _T("Pier Type");
   (*pTable)(row,1) << strPierTypes[pProject->GetPierType(pierID)];
   row++;

   (*pTable)(row,0) << _T("Curb line measured from");
   (*pTable)(row,1) << (pProject->GetCurbLineDatum(pierID) == pgsTypes::omtAlignment ? _T("Alignment") : _T("BridgeLine"));
   row++;

   (*pTable)(row,0) << _T("Skew");
   (*pTable)(row,1) << pProject->GetOrientation(pierID);
   row++;

   (*pTable)(row,0) << _T("Deck elevation at alignment");
   (*pTable)(row,1) << elevation.SetValue(pProject->GetDeckElevation(pierID));
   row++;

   (*pTable)(row,0) << _T("Gross Slab Depth (TS)");
   (*pTable)(row,1) << dim.SetValue(pProject->GetDeckThickness(pierID));
   row++;

   (*pTable)(row,0) << _T("Crown Point Offset (CPO)");
   (*pTable)(row,1) << RPT_OFFSET(pProject->GetCrownPointOffset(pierID),length);
   row++;

   (*pTable)(row,0) << _T("Bridge Line Offset (BLO)");
   (*pTable)(row,1) << RPT_OFFSET(pProject->GetBridgeLineOffset(pierID),length);
   row++;

   Float64 LCO, RCO;
   pProject->GetCurbLineOffset(pierID,&LCO,&RCO);
   (*pTable)(row,0) << _T("Left Curb Offset (LCO)");
   (*pTable)(row,1) << RPT_OFFSET(LCO,length);
   row++;

   (*pTable)(row,0) << _T("Right Curb Offset (RCO)");
   (*pTable)(row,1) << RPT_OFFSET(RCO,length);
   row++;

   std::_tstring strSlopeTag = pDisplayUnits->GetAlignmentLengthUnit().UnitOfMeasure.UnitTag();
   Float64 SL, SR;
   pProject->GetCrownSlopes(pierID,&SL,&SR);
   (*pTable)(row,0) << _T("Left Crown Slope (SL)");
   (*pTable)(row,1) << SL << _T(" ") << strSlopeTag << _T("/") << strSlopeTag;
   row++;
   (*pTable)(row,0) << _T("Right Crown Slope (SR)");
   (*pTable)(row,1) << SR << _T(" ") << strSlopeTag << _T("/") << strSlopeTag;
   row++;

   Float64 W,H;
   pProject->GetDiaphragmDimensions(pierID,&H,&W);
   (*pTable)(row,0) << _T("Diaphragm Width (W)");
   (*pTable)(row,1) << length.SetValue(W);
   row++;
   (*pTable)(row,0) << _T("Diaphragm Height (H)");
   (*pTable)(row,1) << length.SetValue(H);
   row++;
}

void write_substructure_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID)
{
   INIT_UV_PROTOTYPE( rptLengthUnitValue, elevation, pDisplayUnits->GetAlignmentLengthUnit(), false );
   INIT_UV_PROTOTYPE( rptLengthUnitValue, length, pDisplayUnits->GetSpanLengthUnit(), false );
   INIT_UV_PROTOTYPE( rptLengthUnitValue, dim, pDisplayUnits->GetComponentDimUnit(), false );

   GET_IFACE2(pBroker,IXBRProject,pProject);
   rptParagraph* pPara = new rptParagraph(rptStyleManager::GetHeadingStyle());
   *pChapter << pPara;
   *pPara << _T("Substructure Properties") << rptNewLine;

   pPara = new rptParagraph;
   *pChapter << pPara;

   *pPara << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("Lower_XBeam_Dimensions.png")) << rptNewLine << rptNewLine;

   *pPara << _T("Horizonal dimensions are in the plane of the pier.") << rptNewLine;

   rptRcTable* pTable = rptStyleManager::CreateTableNoHeading(4);
   pTable->SetColumnStyle(0,rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   pTable->SetStripeRowColumnStyle(0,rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));
   pTable->SetColumnStyle(2,rptStyleManager::GetTableCellStyle(CB_NONE | CJ_LEFT));
   pTable->SetStripeRowColumnStyle(2,rptStyleManager::GetTableStripeRowCellStyle(CB_NONE | CJ_LEFT));
   *pPara << pTable << rptNewLine;

   Float64 H1,H2,H3,H4,X1,X2,X3,X4,X5,X6,W;
   pProject->GetLowerXBeamDimensions(pierID,&H1,&H2,&H3,&H4,&X1,&X2,&X3,&X4,&W);
   X5 = pProject->GetXBeamLeftOverhang(pierID);
   X6 = pProject->GetXBeamRightOverhang(pierID);

   (*pTable)(0,0) << _T("H1 = ") << length.SetValue(H1);
   (*pTable)(0,1) << _T("H2 = ") << length.SetValue(H2);
   (*pTable)(0,2) << _T("H3 = ") << length.SetValue(H3);
   (*pTable)(0,3) << _T("H4 = ") << length.SetValue(H4);

   (*pTable)(1,0) << _T("X1 = ") << length.SetValue(X1);
   (*pTable)(1,1) << _T("X2 = ") << length.SetValue(X2);
   (*pTable)(1,2) << _T("X3 = ") << length.SetValue(X3);
   (*pTable)(1,3) << _T("X4 = ") << length.SetValue(X4);

   (*pTable)(2,0) << _T("X5 = ") << length.SetValue(X5);
   (*pTable)(2,1) << _T("X5 = ") << length.SetValue(X6);
   (*pTable)(2,2) << _T("");
   (*pTable)(2,3) << _T("W = ") << length.SetValue(W);

   pTable = rptStyleManager::CreateDefaultTable(9,_T("Column Properties"));
   *pPara << pTable << rptNewLine;
   ColumnIndexType col = 0;
   (*pTable)(0,col++) << _T("Col");
   (*pTable)(0,col++) << COLHDR(_T("Top") << rptNewLine << _T("Elev"), rptLengthUnitTag, pDisplayUnits->GetAlignmentLengthUnit());
   (*pTable)(0,col++) << COLHDR(_T("Bottom") << rptNewLine << _T("Elev"), rptLengthUnitTag, pDisplayUnits->GetAlignmentLengthUnit());
   (*pTable)(0,col++) << COLHDR(_T("Height"), rptLengthUnitTag, pDisplayUnits->GetAlignmentLengthUnit());
   (*pTable)(0,col++) << _T("Fixity");
   (*pTable)(0,col++) << _T("Shape");
   (*pTable)(0,col++) << COLHDR(_T("Diameter") << rptNewLine << _T("Width"), rptLengthUnitTag, pDisplayUnits->GetSpanLengthUnit());
   (*pTable)(0,col++) << COLHDR(_T("Depth"), rptLengthUnitTag, pDisplayUnits->GetSpanLengthUnit());
   (*pTable)(0,col++) << COLHDR(_T("Spacing"), rptLengthUnitTag, pDisplayUnits->GetSpanLengthUnit());

   GET_IFACE2(pBroker,IXBRPier,pPier);

   CString strShape[] = {_T("Circle"), _T("Rectangle") };


   RowIndexType row = pTable->GetNumberOfHeaderRows();
   ColumnIndexType nColumns = pProject->GetColumnCount(pierID);
   for ( ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++, row++ )
   {
      col = 0;

      (*pTable)(row,col++) << (colIdx+1);
      (*pTable)(row,col++) << elevation.SetValue(pPier->GetTopColumnElevation(pierID,colIdx));
      (*pTable)(row,col++) << elevation.SetValue(pPier->GetBottomColumnElevation(pierID,colIdx));
      (*pTable)(row,col++) << length.SetValue(pProject->GetColumnHeight(pierID,colIdx));
      (*pTable)(row,col++) << GetTransverseFixityString(pProject->GetColumnFixity(pierID,colIdx));

      CColumnData::ColumnShapeType shapeType;
      CColumnData::ColumnHeightMeasurementType measureType;
      Float64 D1, D2, H;
      pProject->GetColumnProperties(pierID,colIdx,&shapeType,&D1,&D2,&measureType,&H);
      (*pTable)(row,col++) << strShape[shapeType];

      if ( shapeType == CColumnData::cstCircle )
      {
         (*pTable)(row,col++) << length.SetValue(D1);
         (*pTable)(row,col++) << _T("");
      }
      else
      {
         (*pTable)(row,col++) << length.SetValue(D1);
         (*pTable)(row,col++) << length.SetValue(D2);
      }

      if ( colIdx < nColumns-1 )
      {
         (*pTable)(row,col++) << length.SetValue(pProject->GetColumnSpacing(pierID,colIdx));
      }
      else
      {
         (*pTable)(row,col++) << _T("");
      }
   }

   pgsTypes::OffsetMeasurementType columnDatum;
   ColumnIndexType refColumnIdx;
   Float64 refColumnOffset;
   pProject->GetRefColumnLocation(pierID,&columnDatum,&refColumnIdx,&refColumnOffset);
   CString strDatumType[] = {_T("Alignment"), _T("BridgeLine") };

   length.ShowUnitTag(true);

   *pPara << _T("Column ") << LABEL_COLUMN(refColumnIdx) << _T(" is located ") << RPT_OFFSET(refColumnOffset,length) << _T("from the ") << strDatumType[columnDatum] << rptNewLine;
}

void write_concrete_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID)
{
   rptParagraph* pPara = new rptParagraph;
   *pChapter << pPara;

   bool bK1 = (WBFL::LRFD::BDSManager::Edition::ThirdEditionWith2005Interims <= WBFL::LRFD::BDSManager::GetEdition());
   bool bLambda = (WBFL::LRFD::BDSManager::Edition::SeventhEditionWith2016Interims <= WBFL::LRFD::BDSManager::GetEdition());


   ColumnIndexType nColumns = 7;
   if ( bK1 )
   {
      nColumns += 6;
   }
   if ( bLambda )
   {
      nColumns++;
   }

   rptRcTable* pTable = rptStyleManager::CreateDefaultTable(nColumns,_T("Concrete Properties"));
   pTable->SetColumnStyle(0, rptStyleManager::GetTableCellStyle( CB_NONE | CJ_LEFT) );
   pTable->SetStripeRowColumnStyle(0, rptStyleManager::GetTableStripeRowCellStyle( CB_NONE | CJ_LEFT) );
   pTable->SetColumnStyle(1, rptStyleManager::GetTableCellStyle( CB_NONE | CJ_LEFT) );
   pTable->SetStripeRowColumnStyle(1, rptStyleManager::GetTableStripeRowCellStyle( CB_NONE | CJ_LEFT) );

   *pPara << pTable << rptNewLine;

   ColumnIndexType col = 0;
   RowIndexType row = 0;
   (*pTable)(row,col++) << _T("Type");
   (*pTable)(row,col++) << COLHDR(RPT_FC,  rptStressUnitTag, pDisplayUnits->GetStressUnit() );
   (*pTable)(row,col++) << COLHDR(RPT_EC,  rptStressUnitTag, pDisplayUnits->GetStressUnit() );
   (*pTable)(row,col++) << COLHDR(Sub2(symbol(gamma),_T("w")), rptDensityUnitTag, pDisplayUnits->GetDensityUnit() );
   (*pTable)(row,col++) << COLHDR(Sub2(symbol(gamma),_T("s")), rptDensityUnitTag, pDisplayUnits->GetDensityUnit() );
   (*pTable)(row,col++) << COLHDR(Sub2(_T("D"),_T("agg")), rptLengthUnitTag, pDisplayUnits->GetComponentDimUnit() );
   (*pTable)(row,col++) << COLHDR(RPT_STRESS(_T("ct")),  rptStressUnitTag, pDisplayUnits->GetStressUnit() );
   if ( bK1 )
   {
      pTable->SetNumberOfHeaderRows(2);
      for ( ColumnIndexType i = 0; i < col; i++ )
      {
         pTable->SetRowSpan(0,i,2); 
      }

      pTable->SetColumnSpan(0,col,2);
      (*pTable)(0,col) << Sub2(_T("E"),_T("c"));
      (*pTable)(1,col++) << Sub2(_T("K"),_T("1"));
      (*pTable)(1,col++) << Sub2(_T("K"),_T("2"));

      pTable->SetColumnSpan(0,col,2);
      (*pTable)(0,col) << _T("Creep");
      (*pTable)(1,col++) << Sub2(_T("K"),_T("1"));
      (*pTable)(1,col++) << Sub2(_T("K"),_T("2"));

      pTable->SetColumnSpan(0,col,2);
      (*pTable)(0,col) << _T("Shrinkage");
      (*pTable)(1,col++) << Sub2(_T("K"),_T("1"));
      (*pTable)(1,col++) << Sub2(_T("K"),_T("2"));
   }

   if ( bLambda )
   {
      if ( bK1 )
      {
         pTable->SetRowSpan(0,col,2);
         (*pTable)(0,col++) << symbol(lambda);
      }
      else
      {
         (*pTable)(row,col++) << symbol(lambda);
      }
   }


   row = pTable->GetNumberOfHeaderRows();

   GET_IFACE2(pBroker,IXBRProject,pProject);
   const CConcreteMaterial& concrete = pProject->GetConcrete(pierID);

   INIT_UV_PROTOTYPE( rptLengthUnitValue,  cmpdim,  pDisplayUnits->GetComponentDimUnit(), false );
   INIT_UV_PROTOTYPE( rptStressUnitValue,  stress,  pDisplayUnits->GetStressUnit(),       false );
   INIT_UV_PROTOTYPE( rptDensityUnitValue, density, pDisplayUnits->GetDensityUnit(),      false );
   INIT_UV_PROTOTYPE( rptStressUnitValue,  modE,    pDisplayUnits->GetModEUnit(),         false );

   col = 0;

   (*pTable)(row,col++) << WBFL::LRFD::ConcreteUtil::GetTypeName( (WBFL::Materials::ConcreteType)concrete.Type, true );

   GET_IFACE2(pBroker,IXBRMaterial,pMaterial);
   Float64 Ec = pMaterial->GetXBeamEc(pierID);
   Float64 lambda = pMaterial->GetXBeamLambda(pierID);

   (*pTable)(row,col++) << stress.SetValue( concrete.Fc );
   (*pTable)(row,col++) << modE.SetValue( Ec );
   (*pTable)(row,col++) << density.SetValue( concrete.WeightDensity );

   if ( concrete.bUserEc )
   {
      (*pTable)(row,col++) << RPT_NA;
   }
   else
   {
      (*pTable)(row,col++) << density.SetValue( concrete.StrengthDensity );
   }

   (*pTable)(row,col++) << cmpdim.SetValue( concrete.MaxAggregateSize );

   if ( concrete.bHasFct )
   {
      (*pTable)(row,col++) << stress.SetValue( concrete.Fct );
   }
   else
   {
      (*pTable)(row,col++) << RPT_NA;
   }


   if (bK1)
   {
      (*pTable)(row,col++) << concrete.EcK1;
      (*pTable)(row,col++) << concrete.EcK2;
      (*pTable)(row,col++) << concrete.CreepK1;
      (*pTable)(row,col++) << concrete.CreepK2;
      (*pTable)(row,col++) << concrete.ShrinkageK1;
      (*pTable)(row,col++) << concrete.ShrinkageK2;
   }

   if ( bLambda )
   {
      (*pTable)(row,col++) << lambda;
   }
}

void write_reinforcement_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID)
{
   GET_IFACE2(pBroker,IXBRProject,pProject);
   WBFL::Materials::Rebar::Type type;
   WBFL::Materials::Rebar::Grade grade;
   pProject->GetRebarMaterial(pierID,&type,&grade);

   std::_tstring strName = WBFL::LRFD::RebarPool::GetMaterialName(type,grade);

   GET_IFACE2(pBroker,IXBRMaterial,pMaterial);
   Float64 E, fy, fu;
   pMaterial->GetRebarProperties(pierID,&E,&fy,&fu);

   INIT_UV_PROTOTYPE( rptStressUnitValue,  modE,    pDisplayUnits->GetModEUnit(),         false );
   INIT_UV_PROTOTYPE( rptStressUnitValue,  stress,  pDisplayUnits->GetStressUnit(),       false );

   rptParagraph* pPara = new rptParagraph;
   *pChapter << pPara;

   rptRcTable* pTable = rptStyleManager::CreateDefaultTable(3,_T("Reinforcement Properties"));
   *pPara << pTable << rptNewLine;

   (*pTable)(0,0) << _T("Name");
   (*pTable)(1,0) << strName;

   (*pTable)(0,1) << COLHDR(RPT_ES,rptStressUnitTag,pDisplayUnits->GetModEUnit());
   (*pTable)(1,1) << modE.SetValue(E);

   (*pTable)(0,2) << COLHDR(RPT_FY,rptStressUnitTag,pDisplayUnits->GetStressUnit());
   (*pTable)(1,2) << stress.SetValue(fy);
}

void write_longitudinal_reinforcement_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID)
{
   rptParagraph* pPara = new rptParagraph(rptStyleManager::GetHeadingStyle());
   *pChapter << pPara;
   *pPara << _T("Longitudinal Reinforcement") << rptNewLine;

   pPara = new rptParagraph;
   *pChapter << pPara;

   rptRcTable* pTable = rptStyleManager::CreateDefaultTable(11);
   *pPara << pTable << rptNewLine;

   INIT_UV_PROTOTYPE( rptLengthUnitValue, length, pDisplayUnits->GetSpanLengthUnit(), false );
   INIT_UV_PROTOTYPE( rptLengthUnitValue, dim, pDisplayUnits->GetComponentDimUnit(), false );

   ColumnIndexType col = 0;
   (*pTable)(0,col++) << _T("Row");
   (*pTable)(0,col++) << _T("Face");
   (*pTable)(0,col++) << _T("Datum");
   (*pTable)(0,col++) << COLHDR(_T("Start"),rptLengthUnitTag,pDisplayUnits->GetSpanLengthUnit());
   (*pTable)(0,col++) << COLHDR(_T("Length"),rptLengthUnitTag,pDisplayUnits->GetSpanLengthUnit());
   (*pTable)(0,col++) << COLHDR(_T("Cover"),rptLengthUnitTag,pDisplayUnits->GetComponentDimUnit());
   (*pTable)(0,col++) << _T("Bar") << rptNewLine << _T("Size");
   (*pTable)(0,col++) << _T("#") << rptNewLine << _T("Bars");
   (*pTable)(0,col++) << COLHDR(_T("Spacing"),rptLengthUnitTag,pDisplayUnits->GetComponentDimUnit());
   (*pTable)(0,col++) << _T("Start") << rptNewLine << _T("Hook");
   (*pTable)(0,col++) << _T("End") << rptNewLine << _T("Hook");

   GET_IFACE2(pBroker,IXBRProject,pProject);
   const xbrLongitudinalRebarData& rebarData = pProject->GetLongitudinalRebar(pierID);

   CString strFace[] = { _T("Top"), _T("Top Lower XBeam"), _T("Bottom") };
   CString strDatum[] = { _T("Left End"), _T("Right End"), _T("Full Length") };


   RowIndexType row = pTable->GetNumberOfHeaderRows();
   for (const auto& rebarRow : rebarData.RebarRows)
   {
      col = 0;

      (*pTable)(row,col++) << row;
      (*pTable)(row,col++) << strFace[rebarRow.Datum];
      (*pTable)(row,col++) << strDatum[rebarRow.LayoutType];

      if ( rebarRow.LayoutType == xbrTypes::blFullLength )
      {
         (*pTable)(row,col++) << _T("");
         (*pTable)(row,col++) << _T("");
      }
      else
      {
         (*pTable)(row,col++) << length.SetValue(rebarRow.Start);
         (*pTable)(row,col++) << length.SetValue(rebarRow.Length);
      }

      (*pTable)(row,col++) << dim.SetValue(rebarRow.Cover);
      (*pTable)(row,col++) << WBFL::LRFD::RebarPool::GetBarSize(rebarRow.BarSize);
      (*pTable)(row,col++) << rebarRow.NumberOfBars;
      (*pTable)(row,col++) << dim.SetValue(rebarRow.BarSpacing);

      if ( rebarRow.bHookStart )
      {
         (*pTable)(row,col++) << _T("Yes");
      }
      else
      {
         (*pTable)(row,col++) << _T("No");
      }

      if ( rebarRow.bHookEnd )
      {
         (*pTable)(row,col++) << _T("Yes");
      }
      else
      {
         (*pTable)(row,col++) << _T("No");
      }

      row++;
   }
}

void write_transverse_reinforcement_data(std::shared_ptr<WBFL::EAF::Broker> pBroker,std::shared_ptr<IEAFDisplayUnits> pDisplayUnits,rptChapter* pChapter,PierIDType pierID,xbrTypes::Stage stage)
{
   rptParagraph* pPara = new rptParagraph(rptStyleManager::GetHeadingStyle());
   *pChapter << pPara;
   *pPara << _T("Stirrups");
   if ( stage == xbrTypes::Stage1 )
   {
      *pPara << _T(" - Lower Cross Beam") << rptNewLine;
   }
   else
   {
      *pPara << _T(" - Full Depth Cross Beam") << rptNewLine;
   }

   GET_IFACE2(pBroker,IXBRProject,pProject);
   const xbrStirrupData* pStirrups;
   if ( stage == xbrTypes::Stage1 )
   {
      pStirrups = &(pProject->GetLowerXBeamStirrups(pierID));
   }
   else
   {
      pStirrups = &(pProject->GetFullDepthStirrups(pierID));
   }

   if ( pStirrups->Symmetric )
   {
      pPara = new rptParagraph;
      *pChapter << pPara;

      *pPara << _T("Stirrups are symmetric about centerline of cross beam") << rptNewLine;
   }

   pPara = new rptParagraph;
   *pChapter << pPara;

   rptRcTable* pTable = rptStyleManager::CreateDefaultTable(5);
   *pPara << pTable << rptNewLine;

   INIT_UV_PROTOTYPE( rptLengthUnitValue, length, pDisplayUnits->GetSpanLengthUnit(), false );
   INIT_UV_PROTOTYPE( rptLengthUnitValue, dim, pDisplayUnits->GetComponentDimUnit(), false );

   ColumnIndexType col = 0;
   (*pTable)(0,col++) << _T("Zone");
   (*pTable)(0,col++) << COLHDR(_T("Zone") << rptNewLine << _T("Length"), rptLengthUnitTag, pDisplayUnits->GetSpanLengthUnit());
   (*pTable)(0,col++) << _T("Bar") << rptNewLine << _T("Size");
   (*pTable)(0,col++) << COLHDR(_T("Spacing"), rptLengthUnitTag, pDisplayUnits->GetComponentDimUnit());
   (*pTable)(0,col++) << _T("# of") << rptNewLine << _T("Legs");


   RowIndexType row = pTable->GetNumberOfHeaderRows();
   IndexType zoneIdx = 0;
   for (const auto& stirrupZone : pStirrups->Zones)
   {
      col = 0;

      (*pTable)(row,col++) << LABEL_INDEX(zoneIdx);
      if ( zoneIdx == pStirrups->Zones.size()-1 )
      {
         if ( pStirrups->Symmetric )
         {
            (*pTable)(row,col++) << _T("to center");
         }
         else
         {
            (*pTable)(row,col++) << _T("to end");
         }
      }
      else
      {
         (*pTable)(row,col++) << length.SetValue(stirrupZone.Length);
      }
      (*pTable)(row,col++) << WBFL::LRFD::RebarPool::GetBarSize(stirrupZone.BarSize);
      (*pTable)(row,col++) << dim.SetValue(stirrupZone.BarSpacing);
      (*pTable)(row,col++) << stirrupZone.nBars;

      row++;
      zoneIdx++;
   }
      
}
