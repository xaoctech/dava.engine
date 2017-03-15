#include "UnitTests/UnitTests.h"
#include "Render/RHI/Common/PreProcessor.h"
#include "Render/RHI/Common/rhi_Utils.h"

#include <malloc.h>

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
          { "!defined RANDOM_BULLSHIT", 1.0f },
          { "SHADING != SHADING_NONE  &&  !defined RANDOM_BULLSHIT", 1.0f }
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

    DAVA_TEST (TestPreprocessor)
    {
        struct
        {
            const char* inputFileName;
            const char* resultFileName;
        }
        test[] = 
        {
            { "00-input.txt", "00-output.txt" },
            { "01-input.txt", "01-output.txt" },
            { "02-input.txt", "02-output.txt" },
            { "03-input.txt", "03-output.txt" },
            { "04-input.txt", "04-output.txt" },
            { "05-input.txt", "05-output.txt" }
        };
        static const char* BaseDir = "~res:/TestData/PreProcessor";


        class
        TestFileCallback
          : public PreProc::FileCallback
        {
        public:
                                TestFileCallback( const char* base_dir )
                                  : _base_dir(base_dir)
                                {}

            virtual bool        open( const char* file_name )
                                {
                                    char    fname[2048];
                                    
                                    Snprintf( fname, countof(fname), "%s/%s", _base_dir, file_name );
                                    _in = DAVA::File::Create( fname, DAVA::File::READ|DAVA::File::OPEN );

                                    return (_in)  ? true  : false;
                                }
            virtual void        close()
                                {
                                    DVASSERT(_in);
                                    _in->Release();
                                    _in = nullptr;
                                }
            virtual unsigned    size() const
                                {
                                    return (_in)  ? unsigned(_in->GetSize())  : 0;
                                }
            virtual unsigned    read( unsigned max_sz, void* dst )  
                                { 
                                    return (_in)  ? _in->Read( dst, max_sz )  : 0; 
                                }

        private:

            DAVA::File*         _in;
            const char* const   _base_dir;
        };


        for( int i=0; i!=countof(test); ++i )
        {
            TestFileCallback    fc(BaseDir);
            PreProc             pp(&fc);
            std::vector<char>   output;
            char                fname[2048];    Snprintf( fname, countof(fname), "%s/%s", BaseDir, test[i].resultFileName );
            DAVA::File*         expected_file   = DAVA::File::Create( fname, DAVA::File::READ|DAVA::File::OPEN ); 
            size_t              expected_sz     = size_t(expected_file->GetSize());
            char*               expected_data   = (char*)(::malloc( expected_sz + 1 ));

            TEST_VERIFY(pp.process_file( test[i].inputFileName, &output ));

            expected_file->Read( expected_data, expected_sz );
            expected_data[expected_sz] = 0;

            #if 1
            {
            char    aname[2048] = "~res:/Data/TestData/PreProcessor/";

            strcat( aname, test[i].resultFileName );
            strcat( aname, ".actual" );
            DAVA::File* out = DAVA::File::Create( aname, DAVA::File::CREATE|DAVA::File::WRITE ); 
            out->Write( &output[0], output.size() );
            out->Release();
            }
            #endif

            const char* actual_data = &output[0];
            TEST_VERIFY(output.size()==expected_sz);
            TEST_VERIFY(strncmp(expected_data,actual_data,output.size())==0);

            ::free( expected_data );
            expected_file->Release();
        }
    }

};
