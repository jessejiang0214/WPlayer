#include "stdafx.h"
#include "ffmpeg_format_parser.h"

FF_Format_Stream_Parser::FF_Format_Stream_Parser(AVFormatContext* pContext)
{
	if (pContext == NULL)
		return;
	nAudioStreams = new int[FF_AV_MAX_AUDIO_COUNT];
	nVideoStreams = new int[FF_AV_MAX_VIDEO_COUNT];
	nSubtitleStreams = new int[FF_AV_MAX_SUBTITLE_COUNT];
	pFormatContext = pContext;
}

FF_Format_Stream_Parser::FF_Format_Stream_Parser(FF_Format_Stream_Wrapper* pWrapper)
{
	if (pWrapper == NULL)
		return;
	nAudioStreams = new int[FF_AV_MAX_AUDIO_COUNT];
	nVideoStreams = new int[FF_AV_MAX_VIDEO_COUNT];
	nSubtitleStreams = new int[FF_AV_MAX_SUBTITLE_COUNT];
	pFormatContext = pWrapper->operator AVFormatContext *();
	pMyWrapper = pWrapper;
	pMyWrapper->Ref();
}

FF_Format_Stream_Parser::~FF_Format_Stream_Parser()
{
	if (nAudioStreams)
		delete nAudioStreams;
	if (nVideoStreams)
		delete nVideoStreams;
	if (nSubtitleStreams)
		delete nSubtitleStreams;
	if (pMyWrapper)
		pMyWrapper->Unref();
}

bool FF_Format_Stream_Parser::InitializeFormatStreams(int* find_err)
{
	int av_ret = avformat_find_stream_info(pFormatContext,NULL);
	if (find_err)
		*find_err = av_ret;
	if (av_ret < 0)
		return false;
	av_dump_format(pFormatContext,-1,NULL,0);

	nDefaultStreamIndex = av_find_default_stream_index(pFormatContext);
	nBestAudioStreamIndex = av_find_best_stream(pFormatContext,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);
	nBestVideoStreamIndex = av_find_best_stream(pFormatContext,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
	nBestSubtitleStreamIndex = av_find_best_stream(pFormatContext,AVMEDIA_TYPE_SUBTITLE,-1,-1,NULL,0);

	if (pFormatContext->nb_streams == 0)
		return false;
	for (int i = 0;i < pFormatContext->nb_streams;i++)
	{
		if (pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			if (nAudioStreamCount < FF_AV_MAX_AUDIO_COUNT)
			{
				nAudioStreams[nAudioStreamCount] = i;
				nAudioStreamCount++;
			}
		}else if (pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			if (nVideoStreamCount < FF_AV_MAX_VIDEO_COUNT)
			{
				nVideoStreams[nVideoStreamCount] = i;
				nVideoStreamCount++;
			}
		}else if (pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
		{
			if (nSubtitleStreamCount < FF_AV_MAX_SUBTITLE_COUNT)
			{
				nSubtitleStreams[nSubtitleStreamCount] = i;
				nSubtitleStreamCount++;
			}
		}
	}
	return true;
}

int FF_Format_Stream_Parser::GetAudioStreamCount()
{
	return nAudioStreamCount;
}

int FF_Format_Stream_Parser::GetVideoStreamCount()
{
	return nVideoStreamCount;
}

int FF_Format_Stream_Parser::GetSubtitleStreamCount()
{
	return nSubtitleStreamCount;
}

int FF_Format_Stream_Parser::GetAudioStreamIndex(int n)
{
	if (n > nAudioStreamCount || nAudioStreamCount == 0)
		return -1;
	return nAudioStreams[n];
}

int FF_Format_Stream_Parser::GetVideoStreamIndex(int n)
{
	if (n > nVideoStreamCount || nVideoStreamCount == 0)
		return -1;
	return nVideoStreams[n];
}

int FF_Format_Stream_Parser::GetSubtitleStreamIndex(int n)
{
	if (n > nSubtitleStreamCount || nSubtitleStreamCount == 0)
		return -1;
	return nSubtitleStreams[n];
}

int FF_Format_Stream_Parser::GetBestAudioStreamIndex()
{
	return nBestAudioStreamIndex;
}

int FF_Format_Stream_Parser::GetBestVideoStreamIndex()
{
	return nBestVideoStreamIndex;
}

int FF_Format_Stream_Parser::GetBestSubtitleStreamIndex()
{
	return nBestSubtitleStreamIndex;
}

bool FF_Format_Stream_Parser::IsAudioStreamMFSupported(int n)
{
	int i = GetAudioStreamIndex(n);
	if (i == -1)
		return false;
	AVCodecID id = pFormatContext->streams[i]->codec->codec_id;
	if (id == AV_CODEC_ID_MP3 || id == AV_CODEC_ID_AAC || 
		id == AV_CODEC_ID_WMALOSSLESS || id == AV_CODEC_ID_WMAPRO || 
		id == AV_CODEC_ID_WMAV1 || id == AV_CODEC_ID_WMAV2)
		return true;
	else
		return false;
}

bool FF_Format_Stream_Parser::IsVideoStreamMFSupported(int n)
{
	int i = GetVideoStreamIndex(n);
	if (i == -1)
		return false;
	if (pFormatContext->streams[i]->codec->bits_per_raw_sample > 8) //10bit is not supported.
		return false;
	AVCodecID id = pFormatContext->streams[i]->codec->codec_id;
	if (id == AV_CODEC_ID_H264 || id == AV_CODEC_ID_VC1 || id == AV_CODEC_ID_MPEG4 ||
		id == AV_CODEC_ID_WMV1 || id == AV_CODEC_ID_WMV2 || id == AV_CODEC_ID_WMV3)
		return true;
	else
		return false;
}

bool FF_Format_Stream_Parser::IsAudioStreamWinStoreMFSupported(int n)
{
	int i = GetAudioStreamIndex(n);
	if (i == -1)
		return false;
	if (pFormatContext->streams[i]->codec->channels > 6)
		return false;
	AVCodecID id = pFormatContext->streams[i]->codec->codec_id;
	if (id == AV_CODEC_ID_AC3)
		return true;
	else
		return false;
}

bool FF_Format_Stream_Parser::IsAudioStreamWinPhoneMFSupported(int n)
{
	return false;
	/*
	int i = GetAudioStreamIndex(n);
	if (i == -1)
		return false;
	AVCodecID id = pFormatContext->streams[i]->codec->codec_id;
	if (id == AV_CODEC_ID_AMR_NB)
		return true;
	else
		return false;
	*/
}

bool FF_Format_Stream_Parser::IsVideoStreamWinStoreMFSupported(int n)
{
	int i = GetVideoStreamIndex(n);
	if (i == -1)
		return false;
	AVCodecID id = pFormatContext->streams[i]->codec->codec_id;
	if (id == AV_CODEC_ID_H263 || id == AV_CODEC_ID_MJPEG)
		return true;
	else
		return false;
}

bool FF_Format_Stream_Parser::IsVideoStreamWinPhoneMFSupported(int n)
{
	int i = GetVideoStreamIndex(n);
	if (i == -1)
		return false;
	AVCodecID id = pFormatContext->streams[i]->codec->codec_id;
	if (id == AV_CODEC_ID_H263)
		return true;
	else
		return false;
}