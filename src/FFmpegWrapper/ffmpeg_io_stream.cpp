#include "stdafx.h"
#include "ffmpeg_io_stream.h"

FF_IO_Stream::FF_IO_Stream(int BufferSize)
{
	nSizeOfBuf = BufferSize;
	pBuffer = (char*)av_mallocz(nSizeOfBuf);
	RtlZeroMemory(&ioContext, sizeof FF_IO_CONTEXT);
}

FF_IO_Stream::~FF_IO_Stream()
{
	switch (ioContext.type)
	{
	case FF_IO_DATA_TYPE_HFILE:
		CloseHandle(ioContext.hFile);
		break;
	case FF_IO_DATA_TYPE_ISTREAM:
		ioContext.pStream->Release();
		break;
	case FF_IO_DATA_TYPE_MFBYTESTREAM:
		ioContext.pMFStream->Release();
		break;
	}
	if (avioContext)
		av_free(avioContext);
	if (pBuffer)
		av_free(pBuffer);
}

FF_IO_Stream::operator AVIOContext*() const
{
	return avioContext;
}

bool FF_IO_Stream::CreateIo(HANDLE hFile, BOOL bAsync)
{
	if (pBuffer == NULL)
		return false;
	if (avioContext)
		return false;
	if (!DuplicateHandle(GetCurrentProcess(), hFile, GetCurrentProcess(), &ioContext.hFile, 0, FALSE, DUPLICATE_SAME_ACCESS))
		return false;
	ioContext.type = FF_IO_DATA_TYPE_HFILE;
	avioContext = avio_alloc_context((unsigned char*)pBuffer, nSizeOfBuf, 0, &ioContext, &ff_read_data, &ff_write_data, &ff_seek_pointer);
	bAsyncIo = bAsync;
	return avioContext ? true : false;
}

bool FF_IO_Stream::CreateIo(IStream* pStream, BOOL bAsync)
{
	if (pBuffer == NULL || pStream == NULL)
		return false;
	if (avioContext)
		return false;
	pStream->AddRef();
	ioContext.pStream = pStream;
	ioContext.type = FF_IO_DATA_TYPE_ISTREAM;
	avioContext = avio_alloc_context((unsigned char*)pBuffer, nSizeOfBuf, 0, &ioContext, &ff_read_data, &ff_write_data, &ff_seek_pointer);
	bAsyncIo = bAsync;
	return avioContext ? true : false;
}

bool FF_IO_Stream::CreateIo(IMFByteStream* pMFStream, BOOL bAsync)
{
	if (pBuffer == NULL || pMFStream == NULL)
		return false;
	if (avioContext)
		return false;
	pMFStream->AddRef();
	ioContext.pMFStream = pMFStream;
	ioContext.type = FF_IO_DATA_TYPE_MFBYTESTREAM;
	avioContext = avio_alloc_context((unsigned char*)pBuffer, nSizeOfBuf, 0, &ioContext, &ff_read_data, &ff_write_data, &ff_seek_pointer);
	bAsyncIo = bAsync;
	return avioContext ? true : false;
}

long long FF_IO_Stream::GetFileSize()
{
	return ff_get_total_size(&ioContext);
}

void FF_IO_Stream::NotifyFFmpegOpened()
{
	pBuffer = NULL;
}