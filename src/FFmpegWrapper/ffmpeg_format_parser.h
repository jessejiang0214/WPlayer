#pragma once

#include "stdafx.h"
#include "ffmpeg.h"
#include "ffmpeg_format_wrapper.h"

#define FF_AV_MAX_AUDIO_COUNT 8
#define FF_AV_MAX_VIDEO_COUNT 2
#define FF_AV_MAX_SUBTITLE_COUNT 10

class FF_Format_Stream_Parser
{
	AVFormatContext* pFormatContext = NULL;
	FF_Format_Stream_Wrapper* pMyWrapper = NULL;
	int nDefaultStreamIndex = -1;
	int nBestAudioStreamIndex = -1,nBestVideoStreamIndex = -1,nBestSubtitleStreamIndex = -1;
	int nAudioStreamCount = 0,nVideoStreamCount = 0,nSubtitleStreamCount = 0;
	int *nAudioStreams = NULL,*nVideoStreams = NULL,*nSubtitleStreams = NULL;
public:
	FF_Format_Stream_Parser(AVFormatContext* pContext);
	FF_Format_Stream_Parser(FF_Format_Stream_Wrapper* pWrapper);
	~FF_Format_Stream_Parser();
public:
	bool InitializeFormatStreams(int* find_err = NULL);
public:
	int GetAudioStreamCount();
	int GetVideoStreamCount();
	int GetSubtitleStreamCount();
	int GetAudioStreamIndex(int n = 0);
	int GetVideoStreamIndex(int n = 0);
	int GetSubtitleStreamIndex(int n = 0);
	int GetBestAudioStreamIndex();
	int GetBestVideoStreamIndex();
	int GetBestSubtitleStreamIndex();
public:
	bool IsAudioStreamMFSupported(int n = 0);
	bool IsVideoStreamMFSupported(int n = 0);
public:
	bool IsAudioStreamWinStoreMFSupported(int n = 0);
	bool IsAudioStreamWinPhoneMFSupported(int n = 0);
	bool IsVideoStreamWinStoreMFSupported(int n = 0);
	bool IsVideoStreamWinPhoneMFSupported(int n = 0);
};