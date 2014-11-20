#pragma once

#include "stdafx.h"
#include "ffmpeg.h"
#include "ffmpeg_io_stream.h"

#define FF_AV_MAX_OPEN_TIMEOUT 21000 //21 sec

class FF_Format_Stream_Wrapper
{
	int RefCount = 1;
	AVFormatContext* pFormatContext = NULL;
protected:
	ULONG64 FFOpenTime = 0;
public:
	FF_Format_Stream_Wrapper(AVIOContext* avioContext,char* pszFileName = NULL);
	FF_Format_Stream_Wrapper(FF_IO_Stream* pIoStream,char* pszFileName = NULL);
	~FF_Format_Stream_Wrapper();
public:
	operator AVFormatContext*() const;
public:
	int Ref();
	int Unref();
private:
	static int ff_avio_open_interrupt(void* opaque);
};