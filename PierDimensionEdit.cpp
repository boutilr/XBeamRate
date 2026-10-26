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
#include "PierDimensionEdit.h"
#include "ProjectAgent/ScallopGeometry.h"

#include <AgentTools.h>
#include <IFace/Project.h>
#include <DManip/DimensionLine.h>
#include <DManip/DisplayObjectEvents.h>
#include <DManip/EditableUnitValueTextBlock.h>
#include <EAF/Transaction.h>
#include <EAF/TxnManager.h>
#include <Math/Math.h>
#include <WBFLGenericBridge.h>
#include <comdef.h>

namespace
{
// A geometry-only edit preserves bearing counts, reactions, and load factors.
class txnEditPierDimension : public WBFL::EAF::Transaction
{
public:
   txnEditPierDimension(const xbrPierData& before, const xbrPierData& after) :
      m_Before(before), m_After(after) {}

   virtual bool Execute() override { Apply(m_After); return true; }
   virtual void Undo() override { Apply(m_Before); }
   virtual std::unique_ptr<WBFL::EAF::Transaction> CreateClone() const override
      { return std::make_unique<txnEditPierDimension>(m_Before, m_After); }
   virtual std::_tstring Name() const override { return _T("Edit Pier Dimension"); }
   virtual bool IsUndoable() const override { return true; }
   virtual bool IsRepeatable() const override { return false; }

private:
   void Apply(const xbrPierData& data)
   {
      auto pBroker = EAFGetBroker();
      GET_IFACE2(pBroker, IXBRProject, pProject);
      GET_IFACE2(pBroker, IXBREvents, pEvents);
      // Queue UI notifications until all project listeners have invalidated
      // their cached models. Otherwise a view can redraw the previous geometry.
      pEvents->HoldEvents();
      try
      {
         pProject->SetPierData(data);
      }
      catch (...)
      {
         pEvents->CancelPendingEvents();
         throw;
      }
      pEvents->FirePendingEvents();
   }

   xbrPierData m_Before, m_After;
};

Float64& DimensionInput(xbrPierData& data, PierDimension dimension, IndexType index)
{
   switch (dimension)
   {
   case PierDimension::H1L: return data.GetH1L();
   case PierDimension::H1R: return data.GetH1R();
   case PierDimension::H2L: return data.GetH2L();
   case PierDimension::H2R: return data.GetH2R();
   case PierDimension::X1L: return data.GetX1L();
   case PierDimension::X1R: return data.GetX1R();
   case PierDimension::X2L: return data.GetX2L();
   case PierDimension::X2R: return data.GetX2R();
   case PierDimension::LowerWidth: return data.GetW();
   case PierDimension::UpperWidth: return data.GetDiaphragmWidth();
   case PierDimension::UpperHeight: return data.GetDiaphragmHeight();
   case PierDimension::LeftOverhang: return data.GetOHL();
   case PierDimension::RightOverhang: return data.GetOHR();
   case PierDimension::ColumnSpacing: return data.GetColumnSpacing(index);
   default: ASSERT(FALSE); return data.GetW(); // Indexed values use their setters below.
   }
}

bool ChangeDimension(xbrPierData& data, PierDimension dimension, IndexType index,
   Float64 value, Float64 previous)
{
   if (!std::isfinite(value) || value < 0.0)
      return false;

   const Float64 delta = value - previous;
   switch (dimension)
   {
   case PierDimension::ColumnHeight:
   {
      if (value <= 0.0) return false;
      auto& column = data.GetColumnData(index);
      // A larger clear height lowers a base specified by elevation.
      const Float64 sign = column.GetColumnHeightMeasurementType() == CColumnData::chtHeight ? 1.0 : -1.0;
      column.SetColumnHeight(column.GetColumnHeight() + sign * delta);
      break;
   }
   case PierDimension::ScallopDepth:
      data.SetColumnScallopDepth(index, data.GetColumnScallopDepth(index) + delta);
      break;
   case PierDimension::BayRadius:
      data.SetColumnScallopRadius(index, value);
      break;
   case PierDimension::LeftRadius:
      data.SetXBeamOverhangRadius(pgsTypes::stLeft, value);
      break;
   case PierDimension::RightRadius:
      data.SetXBeamOverhangRadius(pgsTypes::stRight, value);
      break;
   default:
      // Use the change in the drawn distance, preserving the input's datum.
      DimensionInput(data, dimension, index) += delta;
      break;
   }
   return true;
}

bool ValidateGeometry(xbrPierData& data, CString& message)
{
   for (Float64 value : {data.GetW(), data.GetH1L(), data.GetH1R(), data.GetH2L(), data.GetH2R(),
        data.GetOHL(), data.GetOHR(), data.GetX1L(), data.GetX1R(), data.GetX2L(), data.GetX2R(),
        data.GetDiaphragmHeight(), data.GetDiaphragmWidth()})
      if (!std::isfinite(value)) return false;
   if (data.GetColumnCount() == 0) return false;
   if (data.GetW() <= 0 || data.GetH1L() <= 0 || data.GetH1R() <= 0 ||
       data.GetOHL() < 0 || data.GetOHR() < 0 || data.GetX1L() < 0 || data.GetX1R() < 0 ||
       data.GetDiaphragmHeight() < 0 || data.GetDiaphragmWidth() <= 0)
   {
      message = _T("Cross beam widths and end depths must be positive. Offsets, overhangs and diaphragm height cannot be negative.");
      return false;
   }

   Float64 length = data.GetOHL() + data.GetOHR();
   const auto count = data.GetColumnCount();
   for (ColumnIndexType i = 0; i + 1 < count; ++i)
   {
      if (!std::isfinite(data.GetColumnSpacing(i)) || data.GetColumnSpacing(i) <= 0)
      {
         message = _T("Column spacing must be greater than zero.");
         return false;
      }
      length += data.GetColumnSpacing(i);
   }
   if (!std::isfinite(length)) return false;

   if (data.GetPierLayoutType() == pgsTypes::pltCommon)
   {
      Float64 leftDiameter, rightDiameter, unused;
      data.GetColumnData(0).GetColumnDimensions(&leftDiameter, &unused);
      data.GetColumnData(count - 1).GetColumnDimensions(&rightDiameter, &unused);
      if (data.GetOHL() < leftDiameter / 2 || data.GetOHR() < rightDiameter / 2 ||
          data.GetH2L() < 0 || data.GetH2R() < 0 || data.GetX2L() < 0 || data.GetX2R() < 0 ||
          (IsZero(data.GetX2L()) != IsZero(data.GetH2L())) ||
          (IsZero(data.GetX2R()) != IsZero(data.GetH2R())) ||
          (data.GetX2L() > 0 && data.GetX2L() < data.GetX1L()) ||
          (data.GetX2R() > 0 && data.GetX2R() < data.GetX1R()) ||
          data.GetX2L() + data.GetX2R() > length)
      {
         message = _T("The taper dimensions or overhangs are incompatible. Each taper requires both a height and length; its length must cover the end offset. Tapers cannot overlap and overhangs must reach the outside column faces.");
         return false;
      }
   }

   // Build a private model. Never publish invalid geometry to the project or its caches.
   CComPtr<IPier> model;
   data.CreatePierModel(&model);
   CComPtr<ICrossBeam> crossBeam;
   if (!model || FAILED(model->get_CrossBeam(&crossBeam)) || !crossBeam)
      return false;

   if (data.GetPierLayoutType() == pgsTypes::pltScalloped)
   {
      if (data.GetX1L() > data.GetOHL() || data.GetX1R() > data.GetOHR())
      {
         message = _T("An end slope offset cannot exceed its overhang length.");
         return false;
      }

      CComPtr<IPoint2dCollection> surface;
      if (FAILED(crossBeam->get_TopSurface(0, &surface)) || !surface) return false;
      IndexType pointCount;
      surface->get_Count(&pointCount);
      if (pointCount < 2) return false;
      std::vector<xbrScallop::Point> top, ends;
      WBFL::Math::PiecewiseFunction elevation;
      for (IndexType i = 0; i < pointCount; ++i)
      {
         CComPtr<IPoint2d> point;
         surface->get_Item(i, &point);
         Float64 x, y;
         point->Location(&x, &y);
         top.push_back({x, y});
         elevation.AddPoint(x, y);
      }
      ends.push_back({top.front().x + data.GetX1L(), top.front().y - data.GetH1L()});
      CComPtr<IColumnLayout> columns;
      model->get_ColumnLayout(&columns);
      for (ColumnIndexType i = 0; i < count; ++i)
      {
         const Float64 depth = data.GetColumnScallopDepth(i);
         if (!std::isfinite(depth) || depth <= 0) return false;
         Float64 xxb, xp;
         columns->get_ColumnLocation(i, &xxb);
         model->ConvertCrossBeamToPierCoordinate(xxb, &xp);
         ends.push_back({xp, elevation.Evaluate(xp) - depth});
      }
      ends.push_back({top.back().x - data.GetX1R(), top.back().y - data.GetH1R()});
      for (ColumnIndexType i = 0; i <= count; ++i)
      {
         const Float64 radius = i == 0 ? data.GetXBeamOverhangRadius(pgsTypes::stLeft)
            : i == count ? data.GetXBeamOverhangRadius(pgsTypes::stRight) : data.GetColumnScallopRadius(i - 1);
         if (xbrScallop::ValidateArc(ends[i], ends[i + 1], radius, top) != xbrScallop::ArcError::None)
         {
            message = _T("Invalid scallop geometry: the radius must fit the inclined chord, the arc must remain below the cap top, and it cannot turn back past an endpoint.");
            return false;
         }
      }
   }

   CComPtr<IPoint2dCollection> bottom;
   if (FAILED(crossBeam->get_BottomSurface(0, &bottom)) || !bottom) return false;
   CComPtr<IColumnLayout> columns;
   model->get_ColumnLayout(&columns);
   for (ColumnIndexType i = 0; i < count; ++i)
   {
      CComPtr<IColumn> column;
      columns->get_Column(i, &column);
      Float64 height;
      if (FAILED(column->get_Height(&height)) || !std::isfinite(height) || height <= 0)
      {
         message = _T("Every column must have a positive clear height.");
         return false;
      }
   }
   return true;
}

class CPierDimensionEditEvents : public WBFL::DManip::iDisplayObjectEvents
{
public:
   CPierDimensionEditEvents(PierIDType pierID, PierDimension dimension, IndexType index) :
      m_PierID(pierID), m_Dimension(dimension), m_Index(index) {}

   virtual void OnChanged(std::shared_ptr<WBFL::DManip::iDisplayObject> object) override
   {
      auto line = std::dynamic_pointer_cast<WBFL::DManip::DimensionLine>(object);
      auto text = std::dynamic_pointer_cast<WBFL::DManip::EditableLengthUnitValueTextBlock>(line ? line->GetTextBlock() : object);
      if (!text) return;
      const auto previousText = text->GetText();
      const Float64 value = text->GetEditedValue();
      if (IsEqual(value, text->GetValue())) return;

      auto pBroker = EAFGetBroker();
      GET_IFACE2(pBroker, IXBRProject, pProject);
      const xbrPierData before = pProject->GetPierData(m_PierID);
      xbrPierData after = before;
      CString message = _T("Invalid pier geometry. Enter a finite, nonnegative dimension and maintain positive depths, radii and column heights.");
      bool valid = false;
      try
      {
         valid = ChangeDimension(after, m_Dimension, m_Index, value, text->GetValue()) && ValidateGeometry(after, message);
      }
      catch (const _com_error&) {}
      catch (HRESULT) {}
      if (!valid)
      {
         text->SetText(previousText);
         AfxMessageBox(message, MB_OK | MB_ICONEXCLAMATION);
         return;
      }
      WBFL::EAF::TxnManager::GetInstance().Execute(std::make_unique<txnEditPierDimension>(before, after));
   }

   virtual bool OnLButtonDblClk(std::shared_ptr<WBFL::DManip::iDisplayObject> object, UINT, const POINT& point) override
   {
      auto line = std::dynamic_pointer_cast<WBFL::DManip::DimensionLine>(object);
      auto text = line ? line->GetTextBlock() : object;
      return text && text->HitTest(point);
   }
   virtual bool OnLButtonDown(std::shared_ptr<WBFL::DManip::iDisplayObject>, UINT, const POINT&) override { return false; }
   virtual bool OnLButtonUp(std::shared_ptr<WBFL::DManip::iDisplayObject>, UINT, const POINT&) override { return false; }
   virtual bool OnRButtonDblClk(std::shared_ptr<WBFL::DManip::iDisplayObject>, UINT, const POINT&) override { return false; }
   virtual bool OnRButtonDown(std::shared_ptr<WBFL::DManip::iDisplayObject>, UINT, const POINT&) override { return false; }
   virtual bool OnRButtonUp(std::shared_ptr<WBFL::DManip::iDisplayObject>, UINT, const POINT&) override { return false; }
   virtual bool OnMouseMove(std::shared_ptr<WBFL::DManip::iDisplayObject>, UINT, const POINT&) override { return false; }
   virtual bool OnMouseWheel(std::shared_ptr<WBFL::DManip::iDisplayObject>, UINT, short, const POINT&) override { return false; }
   virtual bool OnKeyDown(std::shared_ptr<WBFL::DManip::iDisplayObject>, UINT, UINT, UINT) override { return false; }
   virtual bool OnContextMenu(std::shared_ptr<WBFL::DManip::iDisplayObject>, CWnd*, const POINT&) override { return false; }
   virtual void OnDragMoved(std::shared_ptr<WBFL::DManip::iDisplayObject>, const WBFL::Geometry::Size2d&) override {}
   virtual void OnMoved(std::shared_ptr<WBFL::DManip::iDisplayObject>) override {}
   virtual void OnCopied(std::shared_ptr<WBFL::DManip::iDisplayObject>) override {}
   virtual void OnSelect(std::shared_ptr<WBFL::DManip::iDisplayObject>) override {}
   virtual void OnUnselect(std::shared_ptr<WBFL::DManip::iDisplayObject>) override {}

private:
   PierIDType m_PierID;
   PierDimension m_Dimension;
   IndexType m_Index;
};
}

void RegisterPierDimensionEditEvents(std::shared_ptr<WBFL::DManip::iDisplayObject> object,
   PierIDType pierID, PierDimension dimension, IndexType index)
{
   object->RegisterEventSink(std::make_shared<CPierDimensionEditEvents>(pierID, dimension, index));
}
