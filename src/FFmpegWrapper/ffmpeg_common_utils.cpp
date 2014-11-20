/*
	Project:FFmpeg Media Source for Windows Media Foundation
	Part:FFmpeg Wrapper
	File:ffmpeg_common_utils.cpp

	Creator:ShanYe (2014-06-13)
	Last Change:ShanYe (2014-06-13)
	Rev:1.0.0
*/

#include "stdafx.h"
#include "ffmpeg_common_utils.h"

void make_ffmpeg_codec_audio_info_to_WAVEFORMATEX(AVCodecContext* codec,WAVEFORMATEX* pwfx)
{
	pwfx->wFormatTag = WAVE_FORMAT_UNKNOWN;
	pwfx->nChannels = codec->channels;
	pwfx->nSamplesPerSec = codec->sample_rate;
	pwfx->wBitsPerSample = av_get_bytes_per_sample(codec->sample_fmt) << 3;
	pwfx->nAvgBytesPerSec = (pwfx->nChannels * (pwfx->wBitsPerSample / 8)) * pwfx->nSamplesPerSec;
	pwfx->nBlockAlign = pwfx->nAvgBytesPerSec / pwfx->nSamplesPerSec;
	pwfx->cbSize = 0;
}

void make_ffmpeg_codec_audio_16bPCM_info_to_WAVEFORMATEX(AVCodecContext* codec,WAVEFORMATEX* pwfx)
{
	pwfx->wFormatTag = WAVE_FORMAT_PCM;
	pwfx->nChannels = 2;
	pwfx->nSamplesPerSec = codec->sample_rate;
	pwfx->wBitsPerSample = 16;
	pwfx->nAvgBytesPerSec = (pwfx->nChannels * (pwfx->wBitsPerSample / 8)) * pwfx->nSamplesPerSec;
	pwfx->nBlockAlign = pwfx->nAvgBytesPerSec / pwfx->nSamplesPerSec;
	pwfx->cbSize = 0;
}

int get_ffmpeg_mpeg4_h263_profile_level(AVCodecContext* codec)
{
	if (VID_WH_E(codec->width,codec->height,176,144))
		return 1;
	else if (VID_WH_E(codec->width,codec->height,352,288))
		return 2;
	else if (VID_WH_E(codec->width,codec->height,352,756))
		return 4;
	else if (VID_WH_E(codec->width,codec->height,720,576))
		return 5;
	else if (VID_WH_E(codec->width,codec->height,640,480))
		return 0x4A;
	return 0;
}

bool query_ffmpeg_metadata_desc_string(AVDictionary* metadata,char* name,char* pstr,int cch)
{
	if (metadata == NULL || pstr == NULL)
		return false;
	memset(pstr,0,cch);
	AVDictionaryEntry* entry = av_dict_get(metadata,name,NULL,0);
	if (!entry)
		return false;
	if (!entry->value)
		return false;
	if (cch < strlen(entry->value))
		return true;
	strcpy(pstr,entry->value);
	return true;
}

long long convert_ffmpeg_pts(AVPacket* packet,AVRational time_base,long long start_time,long long new_tb)
{
	long long pts = packet->pts;
	if (pts == AV_NOPTS_VALUE)
		pts = packet->dts;
	if (pts == AV_NOPTS_VALUE)
		return 0;
	if (start_time != 0)
		pts -= av_rescale(start_time,time_base.den,AV_TIME_BASE * (long long)time_base.num);
	return av_rescale(pts,(long long)time_base.num * new_tb,time_base.den);
}