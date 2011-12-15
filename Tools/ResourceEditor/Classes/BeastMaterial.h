#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_MATERIAL__
#define __BEAST_MATERIAL__

#include "DAVAEngine.h"
#include "BeastTypes.h"
#include "BeastNameGenerator.h"

class BeastMaterial
{
public:
	DECLARE_BEAST_NAME(BeastMaterial);

	BeastMaterial(ILBManagerHandle manager);
	virtual ~BeastMaterial();

private:
	ILBMeshHandle mesh;
	ILBManagerHandle manager;
};

#endif //__BEAST_MATERIAL__

#endif //__DAVAENGINE_BEAST__