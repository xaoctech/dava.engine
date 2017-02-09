#include "UnitTests/UnitTests.h"
#include "Render/RHI/Common/PreProcessor.h"
#include "Render/RHI/Common/rhi_Utils.h"

DAVA_TESTCLASS (PreprocessorTest)
{
    DAVA_TEST (TestFunction)
    {
        ExpressionEvaluator ev;
        struct
        {
            const char* expr;
            const float result;
        } data[] =
        {
          { "2+2", 4.0f },
          { "bla+7", 20.0f },
          { "(5+3) / (3-1)", 4.0f },
          { "3 + ((1+7)/2) + 1", 8.0f },
          { "SHADING == SHADING_PERVERTEX", 1.0f },
          { "SHADING != SHADING_NONE", 1.0f },
          { "LIGHTING_ENABLED", 1.0f },
          { "!DARKNESS_DISABLED", 1.0f },
          { "!DARKNESS_DISABLED && SHADING != SHADING_NONE", 1.0f },
          { "LIGHTING_ENABLED || !DARKNESS_DISABLED", 1.0f }
        };

        ev.set_variable("bla", 13);
        ev.set_variable("SHADING_NONE", 0);
        ev.set_variable("SHADING_PERVERTEX", 1);
        ev.set_variable("SHADING_PERPIXEL", 2);
        ev.set_variable("SHADING", 1);
        ev.set_variable("LIGHTING_ENABLED", 1);
        ev.set_variable("DARKNESS_DISABLED", 0);
        for (unsigned i = 0; i != countof(data); ++i)
        {
            float res = 0;
            bool success = ev.evaluate(data[i].expr, &res);

            TEST_VERIFY(success);
            TEST_VERIFY(abs(res - data[i].result) < 0.000001f)
        }
    }
};
