// OTHER DEALINGS IN THE SOFTWARE.

#ifndef NV_TT_DECOMPRESSOR_H
#define NV_TT_DECOMPRESSOR_H

#include <nvcore/Ptr.h>

#include "nvtt.h"
#include <nvimage/DirectDrawSurface.h>

namespace nvtt
{
	struct Decompressor::Private
	{
		Private() 
		{
			m_dds = NULL;
		}

		bool initWithDDSFile(const char * pathToDDSFile);
		
		void erase();

		bool getDecompressedSize(unsigned int * width, unsigned int * height) const;

		bool decompress(void * data, unsigned int size) const;

		bool getMipMapCount(unsigned int * mipmapCount) const;

		bool getCompressionFormat(Format * comprFormat) const;

		private:

		nv::DirectDrawSurface * m_dds;

	};

} // nvtt namespace


#endif // NV_TT_DECOMPRESSOR_H