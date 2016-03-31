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

#ifndef __DAVAENGINE_MOVIEVIEWCONTROL_H__
#define __DAVAENGINE_MOVIEVIEWCONTROL_H__

#include "UI/IMovieViewControl.h"
#include "UI/UIControl.h"
#include "Render\PixelFormatDescriptor.h"

namespace AV
{
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/rational.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
};
#endif
}

namespace DAVA
{
    class MovieViewControl : public IMovieViewControl, public UIControl
    {
    public:

        ~MovieViewControl() override;

        // IMovieViewControl Interface implementation

        // Initialize the control.
        void Initialize(const Rect& rect) override;

        // Position/visibility.
        void SetRect(const Rect& rect) override;
        void SetVisible(bool isVisible) override;

        // Open the Movie.
        void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params) override;

        // Start/stop the video playback.
        void Play() override;
        void Stop() override;

        // Pause/resume the playback.
        void Pause() override;
        void Resume() override;

        // Whether the movie is being played?
        bool IsPlaying() const override;


        // UIControl update and draw implementation
        void Update(float32 timeElapsed) override;

    private:
        static bool isFFMGEGInited;
        bool isPlaying = false;
        Texture * videoTexture = nullptr;
        float32 framerate = 0.f;

        uint32 textureWidth = 0;
        uint32 textureHeight = 0;
        const PixelFormat textureFormat = PixelFormat::FORMAT_RGBA8888;
        uint32 textureBufferSize = 0;
        char8* decodedFrameBuffer = nullptr;


        uint32_t len = 0;
        int32 index = 0;
        int64_t in_channel_layout = -1;
        struct SwrContext *au_convert_ctx = nullptr;

        unsigned int videoindex = -1;
        AV::AVFormatContext	*pFormatCtx = nullptr;       
        AV::AVCodecContext	*pCodecCtx = nullptr;
        AV::AVCodec			*pCodec = nullptr;
        AV::AVFrame	*pFrame = nullptr;
        AV::AVFrame	*pFrameYUV = nullptr;
        uint8 *out_buffer = nullptr;
        AV::AVPacket *packet = nullptr;
        int32 y_size;
        AV::SwsContext *img_convert_ctx = nullptr;

        char8 * filepath = "D:/Projects/Win10/wot.blitz/Data/Video/WG_Logo.m4v";
    };
}

#endif