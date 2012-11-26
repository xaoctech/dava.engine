#include "Render/dxtHelper.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

#include "Render/RenderManager.h"
#include "Render/2D/Sprite.h"
#include "Render/Texture.h"
#include "Render/Image.h"
#include "FileSystem/FileSystem.h"

using namespace DAVA;
using namespace nvtt;


void DxtWrapper::WriteDxtFile(const char* file_name, int32 width, int32 height, uint8 * data, PixelFormat format)
{
	printf("* Writing DXT(*.DDS) file (%d x %d): %s\n", width, height, file_name);
	/*
	png_color_8 sig_bit;
	
	png_structp png_ptr;
	png_infop info_ptr;
    
    
	png_byte color_type = PNG_COLOR_TYPE_RGBA;
	png_byte bit_depth = 8;
    
    int32 bytes_for_color = 4;
    if(FORMAT_A8 == format)
    {
        color_type = PNG_COLOR_TYPE_GRAY;
        bytes_for_color = 1;
    }
    else if(FORMAT_A16 == format)
    {
        bit_depth = 16;
        color_type = PNG_COLOR_TYPE_GRAY;
        bytes_for_color = 2;
    }
    
	png_bytep * row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    
    
	for (int y = 0; y < height; y++)
	{
//		row_pointers[y] = (png_byte*) &data[y * width * 4];
		row_pointers[y] = (png_byte*) &data[y * width * bytes_for_color];
	}
	
	*/
	/* create file */
	FILE *fp = fopen(file_name, "wb");
	if (!fp)
	{
		Logger::Error("[LibPngWrapper::WritePngFile] File %s could not be opened for writing", file_name);
		//abort_("[write_png_file] File %s could not be opened for writing", file_name);
		return;
	}
	
	InputOptions inputOptions;
	inputOptions.setTextureLayout(TextureType_2D, width, height);
	inputOptions.setMipmapData(data, width, height);

	OutputOptions outputOptions;
	outputOptions.setFileName("testt.dds");
	
	CompressionOptions compressionOptions;
	compressionOptions.setFormat(Format_DXT1);
	
	Compressor compressor;
	compressor.process(inputOptions, compressionOptions, outputOptions);

	///* initialize stuff */
	//png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	//
	//
	//
	//info_ptr = png_create_info_struct(png_ptr);
	//if (!info_ptr)
	//{
	//	Logger::Error("[LibPngWrapper::WritePngFile] png_create_info_struct failed");
	//
	//	//abort_("[write_png_file] png_create_info_struct failed");
	//	return;
	//}
	//
	//if (setjmp(png_jmpbuf(png_ptr)))
	//{
	//	Logger::Error("[LibPngWrapper::WritePngFile] Error during init_io");
	//	//abort_("[write_png_file] Error during init_io");
	//	return;
	//}
	//
	//png_init_io(png_ptr, fp);
	
	
	///* write header */
	//if (setjmp(png_jmpbuf(png_ptr)))
	//{
	//	Logger::Error("[LibPngWrapper::WritePngFile] Error during writing header");
	//	//abort_("[write_png_file] Error during writing header");
	//	return;
	//}
    //
	//
	//png_set_IHDR(png_ptr, info_ptr, width, height,
	//			 bit_depth, color_type, PNG_INTERLACE_NONE,
	//			 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	//
	//
    //if(FORMAT_A8 == format)
    //{
    //    sig_bit.red = 0;
    //    sig_bit.green = 0;
    //    sig_bit.blue = 0;
    //    
    //    sig_bit.gray = 8;
    //    sig_bit.alpha = 0;
    //}
    //else if(FORMAT_A16 == format)
    //{
    //    sig_bit.red = 0;
    //    sig_bit.green = 0;
    //    sig_bit.blue = 0;
    //    
    //    sig_bit.gray = 16;
    //    sig_bit.alpha = 0;
    //}
    //else 
    //{
    //    sig_bit.red = 8;
    //    sig_bit.green = 8;
    //    sig_bit.blue = 8;
    //    sig_bit.alpha = 8;
    //}
	//
	//png_set_sBIT(png_ptr, info_ptr, &sig_bit);
	//
    //
	//png_write_info(png_ptr, info_ptr);
	//png_set_shift(png_ptr, &sig_bit);
	//png_set_packing(png_ptr);
	
	///* write bytes */
	//if (setjmp(png_jmpbuf(png_ptr)))
	//{
	//	Logger::Error("[LibPngWrapper::WritePngFile] Error during writing bytes");
	//	//abort_("[write_png_file] Error during writing bytes");
	//	return;
	//}
	//
	//png_write_image(png_ptr, row_pointers);
	//
	//
	///* end write */
	//if (setjmp(png_jmpbuf(png_ptr)))
	//{
	//	Logger::Error("[LibPngWrapper::WritePngFile] Error during end of write");
	//	//abort_("[write_png_file] Error during end of write");
	//	return;
	//}
	//
	//png_write_end(png_ptr, NULL);
	//
	//free(row_pointers);
	
	fclose(fp);
}