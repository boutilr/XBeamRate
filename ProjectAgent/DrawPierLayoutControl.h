#pragma once

#include <DManip/DManip.h>
#include <WBFLGenericBridge.h>
#include <XBeamRateExt/PierData.h>

extern UINT WM_PIER_LAYOUT_POPOUT_CLOSED;

// Uses the same WBFL display objects as CXBeamRateView, bound to a pending pier snapshot.
class CDrawPierLayoutControl : public CDisplayWnd
{
    DECLARE_DYNAMIC(CDrawPierLayoutControl)
public:
    CDrawPierLayoutControl();
    ~CDrawPierLayoutControl() override;
    void OnDraw(CDC* pDC) override;
    void CustomInit(const xbrPierData& pierData);
    BOOL CreatePopout(const xbrPierData& pierData, CWnd* pOwner);
    void Refresh(const xbrPierData& pierData, bool fit = false);
protected:
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
    DECLARE_MESSAGE_MAP()
private:
    HWND m_hPopoutOwner = nullptr;
    xbrPierData m_PierData;
    IDType m_DisplayObjectID = 0;
    CComPtr<IPier> m_PierModel;
    CComPtr<ICrossBeam> m_CrossBeam;
    CComPtr<IColumnLayout> m_Columns;
    void UpdateDisplayObjects();
    void UpdateRoadwayDisplayObjects();
    void UpdateXBeamDisplayObjects();
    void UpdateColumnDisplayObjects();
    void UpdateDimensionsDisplayObjects();
    std::shared_ptr<WBFL::DManip::iLineDisplayObject> CreateLineDisplayObject(const WBFL::Geometry::Point2d& start, const WBFL::Geometry::Point2d& end);
    void BuildDimensionLine(std::shared_ptr<WBFL::DManip::iDisplayList> list, const WBFL::Geometry::Point2d& from, const WBFL::Geometry::Point2d& to, bool omitZero = true);
    void BuildDimensionLine(std::shared_ptr<WBFL::DManip::iDisplayList> list, const WBFL::Geometry::Point2d& from, const WBFL::Geometry::Point2d& to, Float64 dimension);
    void GetXBeamProfile(pgsTypes::Stage stage, IShape** shape) const;
    void GetXBeamShape(pgsTypes::Stage stage, Float64 Xxb, IShape** shape) const;
    Float64 GetBottomY(Float64 Xxb) const;
    Float64 GetMaxColumnHeight() const;
    Float64 ConvertCrossBeamToPierCoordinate(Float64 coordinate) const;
    Float64 ConvertPierToCrossBeamCoordinate(Float64 coordinate) const;
    Float64 ConvertPierToCurbLineCoordinate(Float64 coordinate) const;
    Float64 GetElevation(Float64 coordinate) const;
    Float64 GetColumnLocation(IndexType index) const;
    Float64 GetTopColumnElevation(IndexType index) const;
    Float64 GetBottomColumnElevation(IndexType index) const;
};
