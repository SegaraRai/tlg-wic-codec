﻿#pragma once

#define __MAKE_VERSION_STRING(S) #S
#define _MAKE_VERSION_STRING(A, B, C, D) __MAKE_VERSION_STRING(A##.##B##.##C##.##D)
#define MAKE_VERSION_STRING(A, B, C, D) _MAKE_VERSION_STRING(A, B, C, D)

#define TLG_WIC_CODEC_PRODUCTNAME "tlg-wic-codec"
#define TLG_WIC_CODEC_FILENAME TLG_WIC_CODEC_PRODUCTNAME ".dll"
#define TLG_WIC_CODEC_DESCRIPTION "KIRIKIRI TLG codecs for Windows Imaging Component"

#define TLG_WIC_CODEC_AUTHOR "Go Watanabe, SegaraRai"
#define TLG_WIC_CODEC_COPYRIGHT "(C) 2020 " TLG_WIC_CODEC_AUTHOR

#define TLG_WIC_CODEC_VERSION_MAJOR 1
#define TLG_WIC_CODEC_VERSION_MINOR 1
#define TLG_WIC_CODEC_VERSION_PATCH 0
#define TLG_WIC_CODEC_VERSION_BUILD 0

#define TLG_WIC_CODEC_VERSION MAKE_VERSION_STRING(TLG_WIC_CODEC_VERSION_MAJOR, TLG_WIC_CODEC_VERSION_MINOR, TLG_WIC_CODEC_VERSION_PATCH, TLG_WIC_CODEC_VERSION_BUILD)
