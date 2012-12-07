#include "ParamToPtrConvertionTest.h"

ParamToPtrConvertionTest::ParamToPtrConvertionTest()
    : TestTemplate<ParamToPtrConvertionTest>("ParamToPtrConvertionTest")
{
    ObjectFactory::Instance()->Dump();
    RegisterFunction( this, &ParamToPtrConvertionTest::CheckConvertation, "CheckConvertation", NULL );
}

void ParamToPtrConvertionTest::CheckConvertation( PerfFuncData * data )
{
    void *ptr = NULL;
//test int8
    int8 value_int8 = -123, value_int8_converted = 10;
    ptr = ParamToPtr( value_int8 );

    value_int8_converted = PtrToParam<int8>( ptr );
    TEST_VERIFY( value_int8 == value_int8_converted );

//test uint8
    uint8 value_uint8 = 123, value_uint8_converted = 10;
    ptr = ParamToPtr( value_uint8 );

    value_uint8_converted = PtrToParam<uint8>( ptr );
    TEST_VERIFY( value_uint8 == value_uint8_converted );

//test int16
    int16 value_int16 = -21845, value_int16_converted = 10;
    ptr = ParamToPtr( value_int16 );

    value_int16_converted = PtrToParam<int16>( ptr );
    TEST_VERIFY( value_int16 == value_int16_converted );

//test uint16
    int16 value_uint16 = 21845, value_uint16_converted = 10;
    ptr = ParamToPtr( value_uint16 );

    value_uint16_converted = PtrToParam<uint16>( ptr );
    TEST_VERIFY( value_uint16 == value_uint16_converted );

//test int32
    int16 value_int32 = -1431655765, value_int32_converted = 10;
    ptr = ParamToPtr( value_int32 );

    value_int32_converted = PtrToParam<int32>( ptr );
    TEST_VERIFY( value_int32 == value_int32_converted );

//test uint32
    int16 value_uint32 = 1431655765, value_uint32_converted = 10;
    ptr = ParamToPtr( value_int32 );

    value_uint32_converted = PtrToParam<uint32>( ptr );
    TEST_VERIFY( value_uint32 == value_uint32_converted );

//test enum
    enum eTestEnum
    {
        ZERO = 0,
        VALUE = -1431655765
    };

    eTestEnum value_enum = VALUE, value_enum_converted = ZERO;
    ptr = ParamToPtr( value_enum );

    value_enum_converted = PtrToParam<eTestEnum>( ptr );
    TEST_VERIFY( value_enum == value_enum_converted );
}

