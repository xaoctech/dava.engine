#include "UnitTests/UnitTests.h"
#include "Render/RHI/Common/PreProcessor.h"
#include "Render/RHI/Common/rhi_Utils.h"
#include "Logger/Logger.h"

#include <stdlib.h>

static float EV_OneMore(float x)
{
    return x + 1.0f;
}

static void
DumpBytes(const void* mem, unsigned count)
{
    const uint8_t* byte = reinterpret_cast<const uint8_t*>(mem);
    const uint8_t* end = byte + count;
    static char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    while (byte < end)
    {
        unsigned byte_cnt = (end - byte < 8) ? unsigned(end - byte) : 8;
        char line[64];

        memset(line, ' ', countof(line));
        for (unsigned b = 0; b < byte_cnt; ++b)
        {
            char* l1 = line + 3 * b + ((b < 4) ? 0 : 2);
            char* l2 = line + 8 * 3 + 2 + 1 + 2 + b;

            l1[0] = hex[(byte[b] >> 4) & 0x0F];
            l1[1] = hex[byte[b] & 0x0F];
            l1[2] = ' ';

            l2[0] = (byte[b] > 0x20 && byte[b] < 0xAF) ? char(byte[b]) : '.';
            l2[1] = '\0';
        }
        line[8 * 3 + 2 + 1] = '|';
        line[8 * 3 + 2 + 1 + 2 + 8] = '\0';
        DAVA::Logger::Info("%s", line);

        byte += byte_cnt;
    }
}

DAVA_TESTCLASS (PreprocessorTest)
{
    static bool CompareStringBuffers();

    DAVA_TEST (TestExpressionEvaluator)
    {
        ExpressionEvaluator ev;
        struct
        {
            const char* expr;
            const float result;
        } data[] =
        {
#if 1
          { "2+2", 4.0f },
          { "bla+7", 20.0f },
          { "(5+3) / (3-1)", 4.0f },
          { "3 + ((1+7)/2) + 1", 8.0f },
          { "SHADING == SHADING_PERVERTEX", 1.0f },
          { "SHADING != SHADING_NONE", 1.0f },
          { "LIGHTING_ENABLED", 1.0f },
          { "!DARKNESS_DISABLED", 1.0f },
          { "!DARKNESS_DISABLED && SHADING != SHADING_NONE", 1.0f },
          { "LIGHTING_ENABLED || !DARKNESS_DISABLED", 1.0f },
          { "defined DARKNESS_DISABLED", 1.0f },
          { "!defined DARKNESS_DISABLED", 0.0f },
          { "!defined RANDOM_BULLSHIT", 1.0f },
          { "defined RANDOM_BULLSHIT", 0.0f },
          { "defined(RANDOM_BRACED_BULLSHIT)", 0.0f },
          { "!defined(RANDOM_BRACED_BULLSHIT)", 1.0f },
          { "SHADING != SHADING_NONE  &&  !defined RANDOM_BULLSHIT", 1.0f },
          { "!(LIGHTING_ENABLED)", 0.0f },
          { "!(LIGHTING_ENABLED && DARKNESS_ENABLED)", 1.0f },
          { "abs(-7)", 7.0f },
          { "one_more(4)", 5.0f },
          { "one_more(one_more(4))", 6.0f }
#else
//          { "!(LIGHTING_ENABLED && DARKNESS_ENABLED)", 1.0f }
#endif
        };

        ev.SetVariable("bla", 13);
        ev.SetVariable("SHADING_NONE", 0);
        ev.SetVariable("SHADING_PERVERTEX", 1);
        ev.SetVariable("SHADING_PERPIXEL", 2);
        ev.SetVariable("SHADING", 1);
        ev.SetVariable("LIGHTING_ENABLED", 1);
        ev.SetVariable("DARKNESS_ENABLED", 0);
        ev.SetVariable("DARKNESS_DISABLED", 0);
        ExpressionEvaluator::RegisterCommonFunctions();
        ExpressionEvaluator::RegisterFunction("one_more", &EV_OneMore);

        for (unsigned i = 0; i != countof(data); ++i)
        {
            float res = 0;
            bool success = ev.Evaluate(data[i].expr, &res);

            TEST_VERIFY(success);
            TEST_VERIFY(FLOAT_EQUAL(res, data[i].result))
        }

        const char* err_expr[] =
        {
          "BULLSHIT+2"
        };

        for (unsigned i = 0; i != countof(err_expr); ++i)
        {
            float res = 0;
            bool success = ev.Evaluate(err_expr[i], &res);
            char err[256] = "";

            TEST_VERIFY(!success);
            ev.GetLastError(err, countof(err));
            DAVA::Logger::Info("(expected) expr.eval error : %s", err);
        }
    }

    DAVA_TEST (TestPreprocessor)
    {
        struct
        {
            const char* inputFileName;
            const char* resultFileName;
        }
        test[] =
        {
#if 1
          { "00-input.txt", "00-output.txt" },
          { "01-input.txt", "01-output.txt" },
          { "02-input.txt", "02-output.txt" },
          { "03-input.txt", "03-output.txt" },
          { "04-input.txt", "04-output.txt" },
          { "05-input.txt", "05-output.txt" },
          { "06-input.txt", "06-output.txt" },
          { "07-input.txt", "07-output.txt" },
          { "08-input.txt", "08-output.txt" },
          { "09-input.txt", "09-output.txt" },
          { "10-input.txt", "10-output.txt" },
          { "11-input.txt", "11-output.txt" },
          { "12-input.txt", "12-output.txt" },
          { "CC01-input.txt", "CC01-output.txt" },
          { "CC02-input.txt", "CC02-output.txt" },
          { "CC03-input.txt", "CC03-output.txt" },
          { "CC04-input.txt", "CC04-output.txt" },
          { "CC05-input.txt", "CC05-output.txt" },
          { "CC06-input.txt", "CC06-output.txt" },
          { "CC07-input.txt", "CC07-output.txt" },
          { "CC08-input.txt", "CC08-output.txt" },
          { "CC09-input.txt", "CC09-output.txt" },
          { "CC10-input.txt", "CC10-output.txt" }
#else
          { "CC10-input.txt", "CC10-output.txt" }
#endif
        };
        static const char* BaseDir = "~res:/TestData/PreProcessor";

        class
        TestFileCallback
        : public PreProc::FileCallback
        {
        public:
            TestFileCallback(const char* base_dir)
                : _base_dir(base_dir)
                ,
                _in(nullptr)
            {
            }

            virtual bool Open(const char* file_name)
            {
                char fname[2048];

                Snprintf(fname, countof(fname), "%s/%s", _base_dir, file_name);
                _in = DAVA::File::Create(fname, DAVA::File::READ | DAVA::File::OPEN);

                return (_in) ? true : false;
            }
            virtual void Close()
            {
                DVASSERT(_in);
                _in->Release();
                _in = nullptr;
            }
            virtual unsigned Size() const
            {
                return (_in) ? unsigned(_in->GetSize()) : 0;
            }
            virtual unsigned Read(unsigned max_sz, void* dst)
            {
                return (_in) ? _in->Read(dst, max_sz) : 0;
            }

        private:
            DAVA::File* _in;
            const char* const _base_dir;
        };

        for (int i = 0; i != countof(test); ++i)
        {
            DAVA::Logger::Info("pre-proc test %i  \"%s\"...", i, test[i].inputFileName);

            TestFileCallback fc(BaseDir);
            PreProc pp(&fc);
            std::vector<char> output;
            char fname[2048];
            Snprintf(fname, countof(fname), "%s/%s", BaseDir, test[i].resultFileName);
            DAVA::File* expected_file = DAVA::File::Create(fname, DAVA::File::READ | DAVA::File::OPEN);
            DVASSERT(expected_file);
            size_t expected_sz = size_t(expected_file->GetSize());
            char* expected_data = reinterpret_cast<char*>(::malloc(expected_sz + 1));

            TEST_VERIFY(pp.ProcessFile(test[i].inputFileName, &output));

            expected_file->Read(expected_data, uint32(expected_sz));
            expected_data[expected_sz] = 0;

            #if 0
            {
                char aname[2048] = "~res:/Data/TestData/PreProcessor/";

                strcat(aname, test[i].resultFileName);
                strcat(aname, ".actual");
                DAVA::File* out = DAVA::File::Create(aname, DAVA::File::CREATE | DAVA::File::WRITE);
                out->Write(&output[0], uint32(output.size()));
                out->Release();
            }
            #endif

            const char* actual_data = &output[0];
            bool size_match = output.size() == expected_sz;
            bool content_match = strncmp(expected_data, actual_data, output.size()) == 0;
            if (!size_match || !content_match)
            {
                DAVA::Logger::Error("output-data mismatch");
                DAVA::Logger::Info("actual data (%u) :", unsigned(output.size()));
                DumpBytes(&output[0], unsigned(output.size()));
                DAVA::Logger::Info("expected data (%u) :", unsigned(expected_sz));
                DumpBytes(expected_data, unsigned(expected_sz));
            }
            TEST_VERIFY(size_match);
            TEST_VERIFY(content_match);

            ::free(expected_data);
            expected_file->Release();

            DAVA::Logger::Info("  OK");
        }

        const char* err_test[] =
        {
#if 1
          "E01-input.txt",
          "E02-input.txt",
          "E03-input.txt",
          "E04-input.txt"
#else
          "E04-input.txt"
#endif
        };

        for (int i = 0; i != countof(err_test); ++i)
        {
            DAVA::Logger::Info("pre-proc error-test %i  \"%s\"...", i, err_test[i]);

            TestFileCallback fc(BaseDir);
            PreProc pp(&fc);
            std::vector<char> output;

            TEST_VERIFY(pp.ProcessFile(err_test[i], &output) == false);
            DAVA::Logger::Info("  OK");
        }

        DAVA::Logger::Info("pre-proc tests PASSED");
    }
};
