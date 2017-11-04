#ifndef DATASECTION_HPP
#define DATASECTION_HPP

#include <DAVAEngine.h>
#include <Base/BaseTypes.h>

typedef DAVA::Vector<DAVA::uint8> * BinaryPtr;
typedef DAVA::Vector<DAVA::uint8> BinaryBlock;

class DataSection
{
public:
	DataSection(DAVA::String _name)
		:name(_name)
	{

	};

	const DAVA::String & sectionName()
	{
		return name;
	};

	BinaryPtr asBinary()
	{
		return &binary;
	};

	void SetBinarySize(DAVA::uint32 newSize)
	{
		binary.resize(newSize);
	}

	void * GetBuffer(void)
	{
		return (void*)&(binary[0]);
	}

	bool write(void * data, DAVA::int32 dataSize)
	{
        DAVA::int32 offset = binary.size();
		binary.resize(offset + dataSize);
		::memcpy((void*)((&binary[0]) + offset), data, dataSize);
		return true;
	}

private:
	const DAVA::String name;
    BinaryBlock binary;
};

#endif
