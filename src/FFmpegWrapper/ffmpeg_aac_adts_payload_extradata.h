/*
	Project:FFmpeg Media Source for Windows Media Foundation
	Part:FFmpeg Wrapper
	File:ffmpeg_aac_adts_payload_extradata.h

	Creator:ShanYe (2014-06-13)
	Last Change:ShanYe (2014-06-13)
	Rev:1.0.0
*/

#pragma once

#include "stdafx.h"
#include "ffmpeg.h"

bool ffmpeg_aac_adts_frame_header_read(AVFormatContext* pFormatContext,AVStream* pStream,AVPacket* pPacket,byte* pADTSHeader);