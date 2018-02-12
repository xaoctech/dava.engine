#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"
#include "Render/RHI/Common/Parser/sl_Parser.h"
#include "Render/RHI/Common/Parser/sl_GeneratorHLSL.h"
#include "Render/RHI/Common/Parser/sl_GeneratorGLES.h"
#include "Render/RHI/Common/Parser/sl_GeneratorMSL.h"

char astTestSourceCode[] = R"(
uniform sampler2D unusedTexture;
[auto][a] property float4 unusedAutoVariable;
[auto][b] property float4 unusedAutoVariable1;
[auto][a] property float4 unusedAutoVariable2;
[auto][b] property float4 unusedAutoVariable3;
[material][b] property float4 unusedMaterialVariable1;
[material][a] property float4 unusedMaterialVariable;
[material][b] property float4 unusedMaterialVariable2;
[material][a] property float4 unusedMaterialVariable3;
[instance][c] property float4 unusedBigArray[64] : "bigarray";

fragment_out
{
    float4 color : SV_Target0;
};

fragment_out fp_main()
{
    fragment_out psOut;
    psOut.color = float4(1.0, 0.5, 0.25, 1.0);
    return psOut;
}
)";

DAVA_TESTCLASS (SLParserTest)
{
    DAVA_TEST (ShaderSourceTest)
    {
        using namespace rhi;
        ShaderSource shaderSource("~res:/Materials/Shaders/unit_test_shader");
        TEST_VERIFY(shaderSource.Construct(rhi::ProgType::PROG_FRAGMENT, astTestSourceCode));

        DAVA::String output = shaderSource.GetSourceCode(HostApi());
        DAVA::Logger::Info("Host api output:\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n%s"
                           "\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n",
                           output.c_str());
    }
    DAVA_TEST (ParseTest)
    {
        using namespace rhi;
        static sl::Allocator allocator;

        const char* unusedGlobalDeclarations[] = {
            "unusedTexture",
            "unusedAutoVariable",
            "unusedMaterialVariable",
            "unusedBigArray",
        };

        sl::HLSLTree tree(&allocator);
        sl::HLSLParser parser(&allocator, "none", astTestSourceCode, sizeof(astTestSourceCode));
        TEST_VERIFY(parser.Parse(&tree));

        for (const char* name : unusedGlobalDeclarations)
        {
            sl::HLSLDeclaration* decl = tree.FindGlobalDeclaration(name);
            TEST_VERIFY(decl != nullptr);
            TEST_VERIFY(decl->hidden == false);
        }

        sl::PruneTree(&tree, "fp_main");

        for (const char* name : unusedGlobalDeclarations)
        {
            sl::HLSLDeclaration* decl = tree.FindGlobalDeclaration(name);
            TEST_VERIFY(decl != nullptr);
            TEST_VERIFY(decl->hidden == true);
        }

        sl::HLSLGenerator hlslGenerator(&allocator);
        sl::GLESGenerator glesGenerator(&allocator);
        sl::MSLGenerator mslGenerator(&allocator);

        std::string hlslOutput;
        hlslOutput.reserve(1024);
        TEST_VERIFY(hlslGenerator.Generate(&tree, sl::HLSLGenerator::Mode::MODE_DX9, sl::Target::TARGET_FRAGMENT, "fp_main", &hlslOutput));

        hlslOutput.clear();
        TEST_VERIFY(hlslGenerator.Generate(&tree, sl::HLSLGenerator::Mode::MODE_DX11, sl::Target::TARGET_FRAGMENT, "fp_main", &hlslOutput));

        hlslOutput.clear();
        TEST_VERIFY(glesGenerator.Generate(&tree, sl::Target::TARGET_FRAGMENT, "fp_main", &hlslOutput));

        hlslOutput.clear();
        TEST_VERIFY(mslGenerator.Generate(&tree, sl::Target::TARGET_FRAGMENT, "fp_main", &hlslOutput));
    }
};
