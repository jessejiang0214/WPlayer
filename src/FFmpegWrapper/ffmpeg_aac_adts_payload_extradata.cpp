/*
	Project:FFmpeg Media Source for Windows Media Foundation
	Part:FFmpeg Wrapper
	File:ffmpeg_aac_adts_payload_extradata.cpp

	Creator:ShanYe (2014-06-13)
	Last Change:ShanYe (2014-06-13)
	Rev:1.0.0
*/

#include "stdafx.h"
#include "ffmpeg_aac_adts_payload_extradata.h"

bool ffmpeg_aac_adts_frame_header_read(AVFormatContext* pFormatContext,AVStream* pStream,AVPacket* pPacket,byte* pADTSHeader)
{
	bool ret = false;
	for (int i = 0;i < 65536;i++)
	{
		if (av_read_frame(pFormatContext,pPacket) < 0)
			break;
		if (pPacket->stream_index == pStream->index)
		{
			if (pPacket->size > 7)
			{
				memcpy(pADTSHeader,pPacket->data,7);
				av_free_packet(pPacket);
				ret = true;
				break;
			}
		}
		av_free_packet(pPacket);
	}
	avformat_seek_file(pFormatContext,-1,0,0,0,AVSEEK_FLAG_BYTE);
	return ret;
}