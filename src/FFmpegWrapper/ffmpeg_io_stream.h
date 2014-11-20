#pragma once

#include "stdafx.h"
#include "ffmpeg.h"
#include "ffmpeg_file_io.h"

class FF_IO_Stream
{
	AVIOContext* avioContext = NULL;
	FF_IO_CONTEXT ioContext;
	BOOL bAsyncIo = FALSE;
	char* pBuffer;
	int nSizeOfBuf;
public:
	FF_IO_Stream(int BufferSize = 0x100000);
	~FF_IO_Stream();
public:
	operator AVIOContext*() const;
public:
	bool CreateIo(HANDLE hFile, BOOL bAsync = FALSE);
	bool CreateIo(IStream* pStream, BOOL bAsync = FALSE);
	bool CreateIo(IMFByteStream* pMFStream, BOOL bAsync = FALSE);
public:
	long long GetFileSize();
public:
	void NotifyFFmpegOpened();
};