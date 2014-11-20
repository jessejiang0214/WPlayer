#pragma once

#include "stdafx.h"
#include "ffmpeg.h"

class CAVPacket : public IUnknown
{
	ULONG RefCount = 1;
	AVPacket Packet;
public:
	CAVPacket(AVPacket* pPacket)
	{
		av_init_packet(&Packet);
		av_copy_packet(&Packet,pPacket);
	}
	~CAVPacket()
	{
		av_free_packet(&Packet);
	}
public:
	AVPacket* GetPacket()
	{
		return &Packet;
	}
public:
	IFACEMETHODIMP QueryInterface(REFIID iid,void** ppv)
	{
		return E_NOINTERFACE;
	}
	IFACEMETHODIMP_(ULONG) AddRef()
	{
		return (ULONG)InterlockedIncrement(&RefCount);
	}
	IFACEMETHODIMP_(ULONG) Release()
	{
		ULONG _RefCount = (ULONG)InterlockedDecrement(&RefCount);
		if (_RefCount == 0)
			delete this;
		return _RefCount;
	}
};