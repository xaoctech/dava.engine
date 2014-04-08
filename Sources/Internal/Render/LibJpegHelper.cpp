/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "Render/LibJpegHelper.h"

#include "Render/Image.h"
#include "Render/Texture.h"
#include "Render/RenderManager.h"

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

#include <stdlib.h>
#include <stdio.h>

#include "libjpeg/jpeglib.h"
#include <setjmp.h>

namespace DAVA
{
//
struct jpegErrorManager
{
    /* "public" fields */
    struct jpeg_error_mgr pub;
    /* for return to caller */
    jmp_buf setjmp_buffer;
};
    
char jpegLastErrorMsg[JMSG_LENGTH_MAX];
    
void jpegErrorExit (j_common_ptr cinfo)
{
    /* cinfo->err actually points to a jpegErrorManager struct */
    jpegErrorManager* myerr = (jpegErrorManager*) cinfo->err;
    /* note : *(cinfo->err) is now equivalent to myerr->pub */
    
    /* output_message is a method to print an error message */
    /*(* (cinfo->err->output_message) ) (cinfo);*/
    
    /* Create the message */
    ( *(cinfo->err->format_message) ) (cinfo, jpegLastErrorMsg);
    
    /* Jump to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}

bool LibJpegWrapper::IsFileImage(File *file)
{
    String path = file->GetFilename().GetAbsolutePathname();
    struct jpeg_decompress_struct cinfo;
    struct jpegErrorManager jerr;
    
    FILE *infile = fopen( path.c_str(), "rb" );
    
    if ( !infile )
    {
        Logger::Error("[LibJpegWrapper::IsJpegFile] File %s could not be opened for reading", path.c_str());
        return false;
    }
    cinfo.err = jpeg_std_error( &jerr.pub );
    
    jerr.pub.error_exit = jpegErrorExit;
    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        Logger::Error("[LibJpegWrapper::IsJpegFile] File %s has wrong jpeg header", path.c_str());
        return false;
    }
    jpeg_create_decompress( &cinfo );
    jpeg_stdio_src( &cinfo, infile );
    jpeg_read_header( &cinfo, TRUE );
    jpeg_destroy_decompress( &cinfo );
    fclose( infile );
    return true;
}

bool LibJpegWrapper::ReadFile(const FilePath & fileName, Vector<Image *> &imageSet, int32 baseMipMap)
{
    struct jpeg_decompress_struct cinfo;
    struct jpegErrorManager jerr;
    
    FILE *infile = fopen( fileName.GetAbsolutePathname().c_str(), "rb" );
    if ( !infile )
    {
        Logger::Error("[LibJpegWrapper::ReadJpegFile] File %s could not be opened for reading", fileName.GetAbsolutePathname().c_str());
        return false;
    }
    
    cinfo.err = jpeg_std_error( &jerr.pub );
    jerr.pub.error_exit = jpegErrorExit;
    Image* image = NULL;
    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        SafeRelease(image);
        Logger::Error("[LibJpegWrapper::ReadJpegFile] File %s has wrong jpeg header", fileName.GetAbsolutePathname().c_str());
        return false;
    }
    
    jpeg_create_decompress( &cinfo );
    jpeg_stdio_src( &cinfo, infile );
    jpeg_read_header( &cinfo, TRUE );
    jpeg_start_decompress( &cinfo );
    
    PixelFormat format = FORMAT_RGB888;
    if(cinfo.jpeg_color_space == JCS_GRAYSCALE)
    {
        format = FORMAT_A8;
    }
    
    image = Image::Create(cinfo.image_width, cinfo.image_height, format);

    JSAMPROW output_data;
    unsigned int scanline_len = cinfo.output_width * cinfo.output_components;
    
    unsigned int scanline_count = 0;
    while (cinfo.output_scanline < cinfo.output_height)
    {
        output_data = (image->data + (scanline_count * scanline_len));
        jpeg_read_scanlines(&cinfo, &output_data, 1);
        scanline_count++;
    }
        
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    imageSet.push_back(image);
    return true;
}

bool LibJpegWrapper::ReadFile(File *infile, Vector<Image *> &imageSet, int32 baseMipMap)
{
    return ReadFile(infile->GetFilename(), imageSet);
}
    
bool LibJpegWrapper::WriteFile(const FilePath & file_name, int32 width, int32 height, uint8 * raw_image, PixelFormat format, bool generateMipmaps)
{
    DVASSERT(format == FORMAT_A8 || format == FORMAT_RGB888);
    struct jpeg_compress_struct cinfo;
    struct jpegErrorManager jerr;
    
    JSAMPROW row_pointer[1];
    FILE *outfile = fopen( file_name.GetAbsolutePathname().c_str(), "wb" );
    
    if ( !outfile )
    {
        Logger::Error("[LibJpegWrapper::WriteJpegFile] File %s could not be opened for writing", file_name.GetAbsolutePathname().c_str());
        return false;
    }
    cinfo.err = jpeg_std_error( &jerr.pub );
    
    jerr.pub.error_exit = jpegErrorExit;
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_compress( &cinfo );
        fclose(outfile);
        Logger::Error("[LibJpegWrapper::WriteJpegFile] Error during compression of jpeg into file %s.", file_name.GetAbsolutePathname().c_str());
        return false;
    }
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);
    
    /* Setting the parameters of the output file here */
    cinfo.image_width = width;
    cinfo.image_height = height;
    
    cinfo.in_color_space = JCS_RGB;
    int colorComponents = 3;
    if(format != FORMAT_RGB888)
    {
        cinfo.in_color_space = JCS_GRAYSCALE;
        colorComponents = 1;
    }
    
    cinfo.input_components = colorComponents;
    
    jpeg_set_defaults( &cinfo );
    cinfo.num_components = colorComponents;
    //cinfo.data_precision = 4;
    cinfo.dct_method = JDCT_FLOAT;
    
    /*The quality value ranges from 0..100. If "force_baseline" is TRUE, the computed quantization table entries are limited to 1..255 for JPEG baseline compatibility.*/
    jpeg_set_quality(&cinfo, 100, TRUE);

    jpeg_start_compress( &cinfo, TRUE );
    while( cinfo.next_scanline < cinfo.image_height )
    {
        row_pointer[0] = &raw_image[ cinfo.next_scanline * cinfo.image_width * cinfo.input_components];
        jpeg_write_scanlines( &cinfo, row_pointer, 1 );
    }
    jpeg_finish_compress( &cinfo );
    jpeg_destroy_compress( &cinfo );
    fclose( outfile );
    return true;
}
   
    
};
