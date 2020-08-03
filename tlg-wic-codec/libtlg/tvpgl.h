/*

	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000-2009 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"


*/
/* core C routines for graphics operations */
/* this file is always generated by gengl.pl rev. 0.1 */
#ifndef _TVPGL_H_
#  define _TVPGL_H_

#  include "tjs.h"

/*[*/
#  ifdef __cplusplus
extern "C" {
#  endif
/*]*/

#  ifdef _WIN32
#    define TVP_GL_FUNC_DECL(rettype, funcname, arg) rettype __cdecl funcname arg
#    define TVP_GL_FUNC_EXTERN_DECL(rettype, funcname, arg) extern rettype __cdecl funcname arg
#    define TVP_GL_FUNC_PTR_DECL(rettype, funcname, arg) rettype __cdecl(*funcname) arg
#    define TVP_GL_FUNC_PTR_EXTERN_DECL_(rettype, funcname, arg) extern rettype __cdecl(*funcname) arg
#    define TVP_GL_FUNC_PTR_EXTERN_DECL TVP_GL_FUNC_PTR_EXTERN_DECL_
#  endif

TVP_GL_FUNC_DECL(void, TVPFillARGB, (tjs_uint32 * dest, tjs_int len, tjs_uint32 value));

#  define TVP_TLG6_H_BLOCK_SIZE 8
#  define TVP_TLG6_W_BLOCK_SIZE 8

TVP_GL_FUNC_DECL(void, TVPTLG5ComposeColors3To4, (tjs_uint8 * outp, const tjs_uint8* upper, tjs_uint8* const* buf, tjs_int width));
TVP_GL_FUNC_DECL(void, TVPTLG5ComposeColors4To4, (tjs_uint8 * outp, const tjs_uint8* upper, tjs_uint8* const* buf, tjs_int width));
TVP_GL_FUNC_DECL(tjs_int, TVPTLG5DecompressSlide, (tjs_uint8 * out, const tjs_uint8* in, tjs_int insize, tjs_uint8* text, tjs_int initialr));
TVP_GL_FUNC_DECL(void, TVPTLG6DecodeGolombValuesForFirst, (tjs_int8 * pixelbuf, tjs_int pixel_count, tjs_uint8* bit_pool));
TVP_GL_FUNC_DECL(void, TVPTLG6DecodeGolombValues, (tjs_int8 * pixelbuf, tjs_int pixel_count, tjs_uint8* bit_pool));
TVP_GL_FUNC_DECL(void,
                 TVPTLG6DecodeLineGeneric,
                 (tjs_uint32 * prevline,
                  tjs_uint32* curline,
                  tjs_int width,
                  tjs_int start_block,
                  tjs_int block_limit,
                  tjs_uint8* filtertypes,
                  tjs_int skipblockbytes,
                  tjs_uint32* in,
                  tjs_uint32 initialp,
                  tjs_int oddskip,
                  tjs_int dir));
TVP_GL_FUNC_DECL(
  void,
  TVPTLG6DecodeLine,
  (tjs_uint32 * prevline, tjs_uint32* curline, tjs_int width, tjs_int block_count, tjs_uint8* filtertypes, tjs_int skipblockbytes, tjs_uint32* in, tjs_uint32 initialp, tjs_int oddskip, tjs_int dir));

/*[*/
#  ifdef __cplusplus
}
#  endif
/*]*/

#endif
/* end of the file */
