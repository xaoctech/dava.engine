#include "DAVAEngine.h"

void android_main(struct android_app* app)
{
    // Make sure glue isn't stripped.
    app_dummy();
    
    DAVA::Core::Run(0, NULL, app);
}
