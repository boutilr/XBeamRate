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

#define EVT_PROJECTPROPERTIES    0x0001
#define EVT_PROJECT              0x0002

//////////////////////////////////////////////////////////////////////////////
// CProxyIXBRProjectPropertiesEventSink
template <class T>
class CProxyIXBRProjectPropertiesEventSink : public WBFL::EAF::EventSinkManager<IXBRProjectPropertiesEventSink>
{
public:

//IXBRProjectPropertiesEventSink : IUnknown
public:
	HRESULT Fire_OnProjectPropertiesChanged()
	{
		T* pT = (T*)this;

      if ( 0 < pT->m_EventHoldCount )
      {
         WBFL::System::Flags<Uint32>::Set(&pT->m_PendingEvents,EVT_PROJECTPROPERTIES);
         return S_OK;
      }

      //pT->Lock();
		HRESULT ret = S_OK;
		for(auto& [id,sink] : this->m_EventSinks)
		{
		   auto callback = sink.lock();
		   callback->OnProjectPropertiesChanged();
		}
		//pT->Unlock();
		return ret;
	}
};

//////////////////////////////////////////////////////////////////////////////
// CProxyIXBRProjectEventSink
template <class T>
class CProxyIXBRProjectEventSink : public WBFL::EAF::EventSinkManager<IXBRProjectEventSink>
{
public:

//IXBRProjectEventSink : IUnknown
public:
	HRESULT Fire_OnProjectChanged()
	{
		T* pT = (T*)this;

      if ( 0 < pT->m_EventHoldCount )
      {
         WBFL::System::Flags<Uint32>::Set(&pT->m_PendingEvents,EVT_PROJECT);
         return S_OK;
      }

      //pT->Lock();
		HRESULT ret = S_OK;
		for(auto& [id,sink] : this->m_EventSinks)
		{
		   auto callback = sink.lock();
		   callback->OnProjectChanged();
		}
		//pT->Unlock();
		return ret;
	}
};

//////////////////////////////////////////////////////////////////////////////
// CProxyIXBREventsEventSink
template <class T>
class CProxyIXBREventsEventSink : public WBFL::EAF::EventSinkManager<IXBREventsSink>
{
public:

//IXBREventsSink : IUnknown
public:
	HRESULT Fire_OnHoldEvents()
	{
		T* pT = (T*)this;

      //pT->Lock();
		HRESULT ret = S_OK;
		for(auto& [id,sink] : this->m_EventSinks)
		{
		   auto callback = sink.lock();
		   callback->OnHoldEvents();
		}
		//pT->Unlock();
		return ret;
	}

	HRESULT Fire_OnFirePendingEvents()
	{
		T* pT = (T*)this;

      //pT->Lock();
		HRESULT ret = S_OK;
		for (auto& [id, sink] : this->m_EventSinks)
		{
		   auto callback = sink.lock();
		   callback->OnFirePendingEvents();
		}
		//pT->Unlock();
		return ret;
	}

	HRESULT Fire_OnCancelPendingEvents()
	{
		T* pT = (T*)this;

      //pT->Lock();
		HRESULT ret = S_OK;
		for (auto& [id, sink] : this->m_EventSinks)
		{
		   auto callback = sink.lock();
		   callback->OnCancelPendingEvents();
		}
		//pT->Unlock();
		return ret;
	}
};
