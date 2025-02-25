/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (https://PooyaEimandar.github.io) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/WolfEngine/Wolf.Engine/issues
	Website			 : https://WolfEngine.App
	Name			 : w_compress_lz4.h
	Description		 : compress stream based on https://github.com/lz4/lz4
	Comment          :
*/

#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include "w_system_export.h"
#include "w_compress_data_type.h"
#include <stdio.h>

	WSYS_EXP int compress_buffer_c(
		/*_In_*/	const char* pSrcBuffer,
		/*_In_*/	w_compress_mode pMode,
		/*_In_*/	int pAcceleration,
		/*_Inout_*/	w_compress_result* pCompressInfo,
		/*_Inout_*/ char* pErrorLog);

	WSYS_EXP int decompress_buffer_c(
		/*_In_*/	const char* pCompressedBuffer,
		/*_Inout_*/	w_compress_result* pDecompressInfo,
		/*_Inout_*/ char* pErrorLog);

	/*WSYS_EXP w_compress_result compress_file_c(FILE* pFileStreamIn, FILE* pCompressedFileOut, _Inout_ char* pErrorLog);
	WSYS_EXP w_compress_result decompress_file_c(FILE* pCompressedFileIn, FILE* pFileStreamOut, _Inout_ char* pErrorLog);*/

#if defined (__cplusplus)
}
#endif
