#include "DAVAEngine.h"

void android_main(struct android_app* app)
{
    DAVA::Core::Run(0, NULL, app);
}
