#include "stdafx.h"
#include "ffmpeg_format_decoder.h"

FF_Format_Stream_Decoder::FF_Format_Stream_Decoder(AVFormatContext* pContext)
{
	pFormatContext = pContext;
}

FF_Format_Stream_Decoder::FF_Format_Stream_Decoder(FF_Format_Stream_Wrapper* pWrapper)
{
	pFormatContext = pWrapper->operator AVFormatContext *();
	pMyWrapper = pWrapper;
	pMyWrapper->Ref();
}

FF_Format_Stream_Decoder::~FF_Format_Stream_Decoder()
{
	if (pCodec)
	{
		if (avcodec_is_open(pCodecContext) >= 0)
			avcodec_close(pCodecContext);
	}
	if (pMyWrapper)
		pMyWrapper->Unref();
}

FF_Format_Stream_Decoder::operator AVCodecContext*() const
{
	return pCodecContext;
}

FF_Format_Stream_Decoder::operator AVCodec*() const
{
	return pCodec;
}

FF_Format_Stream_Decoder::operator AVStream*() const
{
	return pStream;
}

int FF_Format_Stream_Decoder::Ref()
{
	return InterlockedIncrement((PULONG)&RefCount);
}

int FF_Format_Stream_Decoder::Unref()
{
	int _RefCount = InterlockedDecrement((PULONG)&RefCount);
	if (_RefCount == 0)
		delete this;
	return _RefCount;
}

bool FF_Format_Stream_Decoder::InitializeDecoder(int index,int threads)
{
	if (index == -1 || pFormatContext == NULL)
		return false;
	if (index > pFormatContext->nb_streams)
		return false;
	if (pCodec != NULL)
		if (avcodec_is_open(pCodecContext) >= 0)
			return false;
	pStream = pFormatContext->streams[index];
	if (pStream == NULL)
		return false;
	pCodecContext = pStream->codec;
	if (pCodecContext == NULL)
		return false;
	pCodec = avcodec_find_decoder(pCodecContext->codec_id);
	if (pCodec == NULL)
		return false;
	if (threads > 0)
	{
		pCodecContext->thread_count = threads;
		pCodecContext->active_thread_type = FF_THREAD_FRAME;
	}
	int result = avcodec_open2(pCodecContext,pCodec,NULL);
	return result < 0 ? false:true;
}