#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#include "Math/Math2D.h"

using namespace DAVA;

#define FILE_PATH String("~res:/KeyedArchives/keyed_archive_original.yaml")
#define GENERATED_FILE_PATH String("KeyedArchives/keyed_archive_created.yaml")
//#define GENERATED_FILE_PATH "~res:/KeyedArchives/keyed_archive_created.yaml"
//#define GENERATED_FILE_PATH "/Users/user/Documents/work/gitHub/dava.framework/Projects/UnitTests/Data/KeyedArchives/keyed_archive_created.yaml"

#define BOOLMAPID "mapNamebool"
#define INT32MAPID "mapNameint32"
#define UINT32MAPID "mapNameUInt32"
#define FLOATMAPID "mapNamefloat"
#define STRINGMAPID "mapNameString"
#define WSTRINGMAPID "mapNameWideString"
#define BYTEARRMAPID "mapNameByteArrey"
#define INT64MAPID "mapNameint64"
#define UINT64MAPID "mapNameUInt64"
#define VECTOR2MAPID "mapNamevector2"
#define VECTOR3MAPID "mapNameVector3"
#define VECTOR4MAPID "mapNameVector4"
#define MATRIX2MAPID "mapNameMatrix2"
#define MATRIX3MAPID "mapNameMatrix3"
#define MATRIX4MAPID "mapNameMatrix4"
#define KEYEDARCHMAPID "mapNameKArch"
#define INT8MAPID "mapNameInt8"
#define UINT8MAPID "mapNameUInt8"
#define INT16MAPID "mapNameInt16"
#define UINT16MAPID "mapNameUInt16"
#define FLOAT64MAPID "mapNameFloat64"
#define COLORMAPID "mapNameColor"
#define TESTKEY "testKey"

DAVA_TESTCLASS (KeyedArchiveYamlTest)
{
    RefPtr<KeyedArchive> loadedArchive;
    RefPtr<KeyedArchive> testArchive;

    KeyedArchiveYamlTest()
        : loadedArchive(new KeyedArchive())
        , testArchive(new KeyedArchive())
    {
    }

    DAVA_TEST (TestFunction)
    {
        bool loaded = false;

        loadedArchive->DeleteAllKeys();

        loaded = loadedArchive->LoadFromYamlFile(FILE_PATH);
        TEST_VERIFY(false != loaded);

        FilePath documentsPath = FileSystem::Instance()->GetCurrentDocumentsDirectory();
        FilePath generatedPath = documentsPath + GENERATED_FILE_PATH;

        FileSystem::Instance()->CreateDirectory(generatedPath.GetDirectory(), true);

        loadedArchive->SaveToYamlFile(generatedPath);

        ScopedPtr<KeyedArchive> loadedArchiveFromGeneratedFile(new KeyedArchive());
        loaded = loadedArchiveFromGeneratedFile->LoadFromYamlFile(generatedPath);

        TEST_VERIFY(false != loaded);

        TEST_VERIFY(*loadedArchive->GetVariant(BOOLMAPID) == *loadedArchiveFromGeneratedFile->GetVariant(BOOLMAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(INT32MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(INT32MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(UINT32MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(UINT32MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(FLOATMAPID) == *loadedArchiveFromGeneratedFile->GetVariant(FLOATMAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(STRINGMAPID) == *loadedArchiveFromGeneratedFile->GetVariant(STRINGMAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(WSTRINGMAPID) == *loadedArchiveFromGeneratedFile->GetVariant(WSTRINGMAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(BYTEARRMAPID) == *loadedArchiveFromGeneratedFile->GetVariant(BYTEARRMAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(INT64MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(INT64MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(UINT64MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(UINT64MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(VECTOR2MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(VECTOR2MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(VECTOR3MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(VECTOR3MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(VECTOR4MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(VECTOR4MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(MATRIX2MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(MATRIX2MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(MATRIX3MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(MATRIX3MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(MATRIX4MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(MATRIX4MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(KEYEDARCHMAPID) == *loadedArchiveFromGeneratedFile->GetVariant(KEYEDARCHMAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(INT8MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(INT8MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(UINT8MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(UINT8MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(INT16MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(INT16MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(UINT16MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(UINT16MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(COLORMAPID) == *loadedArchiveFromGeneratedFile->GetVariant(COLORMAPID));

        testArchive->SetFloat(TESTKEY, 999.0f);
        const VariantType* variantFloatPtr = testArchive->GetVariant(TESTKEY);

        testArchive->SetString(TESTKEY, "test string");
        const VariantType* variantStringPtr = testArchive->GetVariant(TESTKEY);

        VariantType variant;
        variant.SetBool(false);
        testArchive->SetVariant(TESTKEY, std::move(variant));
        const VariantType* variantPtr = testArchive->GetVariant(TESTKEY);

        TEST_VERIFY(variantFloatPtr == variantStringPtr);
        TEST_VERIFY(variantPtr == variantStringPtr);
        TEST_VERIFY(variant.GetType() == VariantType::TYPE_NONE);
    }
};
