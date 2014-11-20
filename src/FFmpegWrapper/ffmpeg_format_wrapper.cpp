#include "stdafx.h"
#include "ffmpeg_format_wrapper.h"

FF_Format_Stream_Wrapper::FF_Format_Stream_Wrapper(AVIOContext* avioContext,char* pszFileName)
{
	if (avioContext == NULL)
		return;
	pFormatContext = avformat_alloc_context();
	pFormatContext->pb = avioContext;
	pFormatContext->interrupt_callback = {ff_avio_open_interrupt,this};
	pFormatContext->flags = AVFMT_VARIABLE_FPS|AVFMT_FLAG_CUSTOM_IO;
	FFOpenTime = GetTickCount64();
	if (avformat_open_input(&pFormatContext,pszFileName,NULL,NULL) != 0)
	{
		avformat_free_context(pFormatContext);
		pFormatContext = NULL;
	}
	FFOpenTime = 0;
}

FF_Format_Stream_Wrapper::FF_Format_Stream_Wrapper(FF_IO_Stream* pIoStream,char* pszFileName)
{
	if (pIoStream == NULL)
		return;
	pFormatContext = avformat_alloc_context();
	pFormatContext->pb = pIoStream->operator AVIOContext *();
	pFormatContext->interrupt_callback = {ff_avio_open_interrupt,this};
	pFormatContext->flags = AVFMT_VARIABLE_FPS|AVFMT_FLAG_CUSTOM_IO;
	FFOpenTime = GetTickCount64();
	if (avformat_open_input(&pFormatContext,pszFileName,NULL,NULL) != 0)
	{
		avformat_free_context(pFormatContext);
		pFormatContext = NULL;
	}
	FFOpenTime = 0;
}

FF_Format_Stream_Wrapper::~FF_Format_Stream_Wrapper()
{
	if (pFormatContext)
	{
		avformat_close_input(&pFormatContext);
		if (pFormatContext)
			avformat_free_context(pFormatContext);
	}
}

FF_Format_Stream_Wrapper::operator AVFormatContext*() const
{
	return pFormatContext;
}

int FF_Format_Stream_Wrapper::Ref()
{
	return InterlockedIncrement((PULONG)&RefCount);
}

int FF_Format_Stream_Wrapper::Unref()
{
	int _RefCount = InterlockedDecrement((PULONG)&RefCount);
	if (_RefCount == 0)
		delete this;
	return _RefCount;
}

int FF_Format_Stream_Wrapper::ff_avio_open_interrupt(void* opaque)
{
	FF_Format_Stream_Wrapper* p = (FF_Format_Stream_Wrapper*)opaque;
	if (p->FFOpenTime == 0)
		return 0;
	if ((GetTickCount64() - p->FFOpenTime) > FF_AV_MAX_OPEN_TIMEOUT)
		return 1;
	else
		return 0;
}