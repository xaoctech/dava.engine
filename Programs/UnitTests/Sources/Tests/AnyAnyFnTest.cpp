#include "Base/Any.h"
#include "Base/AnyFn.h"
#include "Base/FastName.h"
#include "Base/TypeInheritance.h"
#include "Base/RefPtr.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/KeyedArchive.h"
#include "Math/Vector.h"
#include "Math/Color.h"
#include "UnitTests/UnitTests.h"
#include <numeric>

namespace DAVA
{
DAVA_TESTCLASS (AnyAnyFnTest)
{
    struct Stub
    {
    };

    template <typename T>
    struct NoCompareTemplate
    {
        T t;
    };

    struct Trivial
    {
        bool operator==(const Trivial& t) const
        {
            return (a == t.a && b == t.b && c == t.c);
        }

        bool operator<(const Trivial& t) const
        {
            return (a <= t.a && b <= t.b && c < t.c);
        }

        int a;
        int b;
        int c;
    };

    struct NotTrivial
    {
        virtual ~NotTrivial()
        {
        }
    };

    struct A
    {
        enum E1
        {
            E1_1,
            E1_2
        };

        enum class E2
        {
            E2_1,
            E2_2
        };

        virtual ~A() = default;

        int a = 1;

        virtual Vector3 TestFn(Vector3 v1, Vector3 v2)
        {
            return (v1 * v2) * static_cast<float32>(a);
        }

        virtual Vector3 TestFnConst(Vector3 v1, Vector3 v2) const
        {
            return (v1 * v2) * static_cast<float32>(a);
        }

        float32 TestFnFloat(float32 b)
        {
            return static_cast<float32>(a) - b;
        }

        static int32 StaticTestFn(int32 a, int32 b)
        {
            return a + b;
        }

        template <typename R, typename... T>
        R TestSum(const T&... args)
        {
            auto fn = [](const std::array<R, sizeof...(args)>& a) -> R
            {
                return std::accumulate(a.begin(), a.end(), R(0));
            };

            return fn({ args... });
        }
    };

    class A1
    {
    public:
        int a1;
    };

    class B : public A
    {
    public:
        B() = default;

        B(int b_)
            : b(b_)
        {
            a = 0;
        }

        int b;

        Vector3 TestFn(Vector3 v1, Vector3 v2) override
        {
            return (v1 + v2) * static_cast<float32>(b);
        }

        Vector3 TestFnConst(Vector3 v1, Vector3 v2) const override
        {
            return (v1 + v2) * static_cast<float32>(b);
        }
    };

    class D : public A, public A1
    {
    public:
        int d;
    };

    class E : public D
    {
    public:
        int e;
    };

    template <typename T>
    void DoAutoStorageSimpleTest(const T& initialValue)
    {
        T simpleValue(initialValue);

        AutoStorage<> as;
        TEST_VERIFY(as.IsEmpty());

        as.SetSimple(simpleValue);
        TEST_VERIFY(!as.IsEmpty());
        TEST_VERIFY(as.IsSimple());
        TEST_VERIFY(as.GetSimple<T>() == simpleValue);
        TEST_VERIFY(as.GetSimple<const T&>() == simpleValue);

        const T& simpleRef = simpleValue;
        as.SetSimple(simpleRef);
        TEST_VERIFY(!as.IsEmpty());
        TEST_VERIFY(as.IsSimple());
        TEST_VERIFY(as.GetSimple<T>() == simpleRef);
        TEST_VERIFY(as.GetSimple<const T&>() == simpleRef);

        as.Clear();
        TEST_VERIFY(as.IsEmpty());
    }

    template <typename T>
    void DoAutoStorageSharedTest(const T& initialValue)
    {
        T sharedValue(initialValue);

        AutoStorage<> as;
        TEST_VERIFY(as.IsEmpty());

        as.SetShared(sharedValue);
        TEST_VERIFY(!as.IsEmpty());
        TEST_VERIFY(!as.IsSimple());
        TEST_VERIFY(as.GetShared<T>() == sharedValue);
        TEST_VERIFY(as.GetShared<const T&>() == sharedValue);

        const T& sharedRef = sharedValue;
        as.SetShared(sharedRef);
        TEST_VERIFY(!as.IsEmpty());
        TEST_VERIFY(!as.IsSimple());
        TEST_VERIFY(as.GetShared<T>() == sharedRef);
        TEST_VERIFY(as.GetShared<const T&>() == sharedRef);

        as.Clear();
        TEST_VERIFY(as.IsEmpty());
    }

    template <typename T>
    void DoAnyTest(const T& initialValue)
    {
        T value(initialValue);
        T& valueRef(value);
        const T& valueConstRef(value);

        Any a;
        TEST_VERIFY(a.IsEmpty());

        a.Set(value);
        TEST_VERIFY(!a.IsEmpty());
        TEST_VERIFY(value == a.Get<T>());
        TEST_VERIFY(value == a.Get<const T&>());

        TEST_VERIFY(std::hash<typename std::decay<T>::type>()(value) == a.Hash());

        a.Set(valueRef);
        TEST_VERIFY(!a.IsEmpty());
        TEST_VERIFY(value == a.Get<T>());
        TEST_VERIFY(value == a.Get<const T&>());

        a.Set(valueConstRef);
        TEST_VERIFY(!a.IsEmpty());
        TEST_VERIFY(value == a.Get<T>());
        TEST_VERIFY(value == a.Get<const T&>());

        a.Clear();
        TEST_VERIFY(a.IsEmpty());
        TEST_VERIFY(!a.CanGet<int>());
        TEST_VERIFY(!a.CanGet<void*>());

        TEST_VERIFY(123 == a.GetSafely<int>(123));

        try
        {
            a.Get<int>();
            TEST_VERIFY(false && "Shouldn't be able to get int from empty Any");
        }
        catch (const Exception& e)
        {
            TEST_VERIFY(e.callstack.size() > 0);
        }
    }

#ifdef _MSVC_LANG
#pragma warning(push)
#pragma warning(disable : 4756) // ignore "overflow in constant arithmetic" warning
#endif

    template <typename T1, typename T2>
    void DoAnyCastHashTest()
    {
        T1 t1Lowest = std::numeric_limits<T1>::lowest();
        T1 t1Max = std::numeric_limits<T1>::max();
        T1 t1Min = std::numeric_limits<T1>::min();
        T1 t1 = static_cast<T1>(123);
        T1 t1_0 = static_cast<T1>(0);
        T1 t1_plus = static_cast<T1>(1);
        T1 t1_minus = static_cast<T1>(-1);

        T2 t2Lowest = std::numeric_limits<T2>::lowest();
        T2 t2Max = std::numeric_limits<T2>::max();
        T2 t2Min = std::numeric_limits<T2>::min();
        T2 t2 = static_cast<T2>(123);
        T2 t2_0 = static_cast<T2>(0);
        T2 t2_plus = static_cast<T2>(1);
        T2 t2_minus = static_cast<T2>(-1);

        auto castT1toT2 = [](const T1& in) -> T2 {
            return static_cast<T2>(in);
        };

        auto castT2toT1 = [](const T2& in) -> T1 {
            return static_cast<T1>(in);
        };

        {
            TEST_VERIFY(Any(t1Lowest).Cast<T2>() == castT1toT2(t1Lowest));
            TEST_VERIFY(Any(t1Max).Cast<T2>() == castT1toT2(t1Max));
            TEST_VERIFY(Any(t1Min).Cast<T2>() == castT1toT2(t1Min));
            TEST_VERIFY(Any(t1).Cast<T2>() == castT1toT2(t1));
            TEST_VERIFY(Any(t1_0).Cast<T2>() == castT1toT2(t1_0));
            TEST_VERIFY(Any(t1_plus).Cast<T2>() == castT1toT2(t1_plus));
            TEST_VERIFY(Any(t1_minus).Cast<T2>() == castT1toT2(t1_minus));

            TEST_VERIFY(Any(t2Lowest).Cast<T1>() == castT2toT1(t2Lowest));
            TEST_VERIFY(Any(t2Max).Cast<T1>() == castT2toT1(t2Max));
            TEST_VERIFY(Any(t2Min).Cast<T1>() == castT2toT1(t2Min));
            TEST_VERIFY(Any(t2).Cast<T1>() == castT2toT1(t2));
            TEST_VERIFY(Any(t2_0).Cast<T1>() == castT2toT1(t2_0));
            TEST_VERIFY(Any(t2_plus).Cast<T1>() == castT2toT1(t2_plus));
            TEST_VERIFY(Any(t2_minus).Cast<T1>() == castT2toT1(t2_minus));
        }

        {
            Any a(t1);
            T2 t2 = a.Cast<T2>();

            Any b(t2);
            T1 t3 = a.Cast<T1>();

            TEST_VERIFY(t3 == t1);
            TEST_VERIFY(!a.CanCast<Stub>());
            TEST_VERIFY(!b.CanCast<Stub>());
        }

        {
            auto x1 = std::hash<T1>()(t1Lowest);
            TEST_VERIFY(Any(t1Lowest).Hash() == x1);

            auto x2 = std::hash<T1>()(t1Max);
            TEST_VERIFY(Any(t1Max).Hash() == x2);

            auto x3 = std::hash<T1>()(t1Min);
            TEST_VERIFY(Any(t1Min).Hash() == x3);

            auto x4 = std::hash<T1>()(t1);
            TEST_VERIFY(Any(t1).Hash() == x4);

            auto x5 = std::hash<T1>()(t1_0);
            TEST_VERIFY(Any(t1_0).Hash() == x5);

            auto x6 = std::hash<T1>()(t1_plus);
            TEST_VERIFY(Any(t1_plus).Hash() == x6);

            auto x7 = std::hash<T1>()(t1_minus);
            TEST_VERIFY(Any(t1_minus).Hash() == x7);
        }
    }

#ifdef _MSVC_LANG
#pragma warning(pop) 
#endif

    template <typename T>
    void DoAnyCompareEqualTest(T v1, T v2)
    {
        Any a(v1);
        Any b(v1);
        Any c(v2);

        Any i(35245225);
        Any s("tmpstr23545");
        Any ss(String("fw2452458"));

        TEST_VERIFY(a == b);
        TEST_VERIFY(a != c);
        TEST_VERIFY(b != c);

        TEST_VERIFY(a != i);
        TEST_VERIFY(a != s);
        TEST_VERIFY(a != ss);
    }

    DAVA_TEST (AutoStorageTest)
    {
        int32 v = 10203040;

        DoAutoStorageSimpleTest<int32>(v);
        DoAutoStorageSimpleTest<int64>(10203040506070);
        DoAutoStorageSimpleTest<float32>(0.123456f);
        DoAutoStorageSimpleTest<void*>(nullptr);
        DoAutoStorageSimpleTest<int32*>(&v);
        DoAutoStorageSimpleTest<const int32*>(&v);

        String s("10203040");

        DoAutoStorageSharedTest<int32>(v);
        DoAutoStorageSharedTest<int64>(10203040506070);
        DoAutoStorageSharedTest<float32>(0.123456f);
        DoAutoStorageSharedTest<String>(s);
        DoAutoStorageSharedTest<void*>(nullptr);
        DoAutoStorageSharedTest<String*>(&s);
        DoAutoStorageSharedTest<const String*>(&s);
    }

    DAVA_TEST (TypeTest)
    {
        struct ccc
        {
        };

        ccc c1;
        ccc c2;

        const Type* t1 = Type::Instance<ccc&>();
        const Type* t2 = Type::Instance<ccc const&>();
        const Type* t3 = Type::Instance<const ccc&>();

        const Type* tp1 = Type::Instance<ccc*>();
        const Type* tp2 = Type::Instance<ccc const*>();
        const Type* tp3 = Type::Instance<const ccc* const>();

        TEST_VERIFY(t1 != t2);
        TEST_VERIFY(t2 == t3);
        TEST_VERIFY(t1->GetTypeFlags() != t2->GetTypeFlags());

        TEST_VERIFY(tp1 != tp2);
        TEST_VERIFY(tp1 != tp3);
        TEST_VERIFY(tp2 != tp3);
        TEST_VERIFY(tp1->GetTypeFlags() != tp2->GetTypeFlags());
        TEST_VERIFY(tp1->GetTypeFlags() != tp3->GetTypeFlags());
        TEST_VERIFY(tp2->GetTypeFlags() != tp3->GetTypeFlags());

        uint32_t customIndex = Type::AllocUserData();

        TEST_VERIFY(nullptr == Type::Instance<int>()->GetUserData(customIndex));
        TEST_VERIFY(nullptr == Type::Instance<float>()->GetUserData(customIndex));

        Type::Instance<int>()->SetUserData(customIndex, &c1);
        Type::Instance<float>()->SetUserData(customIndex, &c2);

        TEST_VERIFY(&c1 == Type::Instance<int>()->GetUserData(customIndex));
        TEST_VERIFY(&c1 != Type::Instance<float>()->GetUserData(customIndex));
        TEST_VERIFY(&c2 != Type::Instance<int>()->GetUserData(customIndex));
        TEST_VERIFY(&c2 == Type::Instance<float>()->GetUserData(customIndex));

        Type::Instance<int>()->SetUserData(customIndex, nullptr);
        Type::Instance<float>()->SetUserData(customIndex, nullptr);

        TEST_VERIFY(nullptr == Type::Instance<int>()->GetUserData(customIndex));
        TEST_VERIFY(nullptr == Type::Instance<float>()->GetUserData(customIndex));
    }

    DAVA_TEST (TypeArrayTest)
    {
        const size_t N1 = 1000;
        const size_t N2 = 100;
        const size_t N3 = 10;

        using A1 = Array<float, N1>;
        using A2 = Array<uint32, N2>;
        using A3 = char[N3];

        const Type* t1 = Type::Instance<A1>();
        const Type* t2 = Type::Instance<A2>();
        const Type* t3 = Type::Instance<A3>();

        TEST_VERIFY(nullptr != t1);
        TEST_VERIFY(nullptr != t2);
        TEST_VERIFY(nullptr != t3);

        TEST_VERIFY(t1->IsArray());
        TEST_VERIFY(t2->IsArray());
        TEST_VERIFY(t3->IsArray());

        TEST_VERIFY(t1->GetSize() == sizeof(A1));
        TEST_VERIFY(t2->GetSize() == sizeof(A2));
        TEST_VERIFY(t3->GetSize() == sizeof(A3));

        TEST_VERIFY(t1->GetArrayDimension() == N1);
        TEST_VERIFY(t2->GetArrayDimension() == N2);
        TEST_VERIFY(t3->GetArrayDimension() == N3);

        TEST_VERIFY(t1->GetArrayElementType() == Type::Instance<float>());
        TEST_VERIFY(t2->GetArrayElementType() == Type::Instance<uint32>());
        TEST_VERIFY(t3->GetArrayElementType() == Type::Instance<char>());
    }

    DAVA_TEST (AnyMove)
    {
        int32 v = 516273;
        String s("516273");

        Any a(v);
        Any b(std::move(a));
        TEST_VERIFY(a.IsEmpty());
        TEST_VERIFY(!b.IsEmpty());
        TEST_VERIFY(b.Get<decltype(v)>() == v);

        a.Set(s);
        b.Set(std::move(a));
        TEST_VERIFY(a.IsEmpty());
        TEST_VERIFY(!b.IsEmpty());
        TEST_VERIFY(b.Get<decltype(s)>() == s);
    }

    DAVA_TEST (AnyTestSimple)
    {
        int32 v = 50607080;
        String s("50607080");

        DoAnyTest<int32>(v);
        DoAnyTest<int64>(80706050403020);
        DoAnyTest<float32>(0.123456f);
        DoAnyTest<void*>(nullptr);
        DoAnyTest<const void*>(nullptr);
        DoAnyTest<const int32&>(v);
        DoAnyTest<int32*>(&v);
        DoAnyTest<const int32*>(&v);
        DoAnyTest<String>(s);
        DoAnyTest<const String&>(s);
        DoAnyTest<String*>(&s);
        DoAnyTest<const String*>(&s);

        try
        {
            Any a(10);
            a.Get<String>();
            TEST_VERIFY(false && "Shouldn't be able to ge String");
        }
        catch (const Exception&)
        {
            TEST_VERIFY(true);
        }
    }

    DAVA_TEST (EmptyAnyCastGetTest)
    {
        Any a;
        TEST_VERIFY(a.CanGet<int32>() == false);
        TEST_VERIFY(a.CanCast<int32>() == false);

        TEST_VERIFY(a != Any(3));
        TEST_VERIFY(Any(3) != a);
        TEST_VERIFY(Any() == Any()); //-V501
        TEST_VERIFY((Any() != Any()) == false); //-V501
    }

    DAVA_TEST (AnyCastGetPtrTest)
    {
        B b;
        D d;
        E e;

        B* bPtr = &b;
        A* baPtr = static_cast<A*>(bPtr);

        D* dPtr = &d;
        A* daPtr = static_cast<A*>(dPtr);
        A1* da1Ptr = static_cast<A1*>(dPtr);

        E* ePtr = &e;
        A1* ea1Ptr = static_cast<A1*>(ePtr);

        Any a;
        a.Set(bPtr);

        TypeInheritance::RegisterBases<B, A>();
        TypeInheritance::RegisterBases<D, A, A1>();
        TypeInheritance::RegisterBases<E, D>();

        // simple
        TEST_VERIFY(bPtr == a.Get<void*>());
        TEST_VERIFY(bPtr == a.Get<const void*>());
        TEST_VERIFY(bPtr == a.Get<B*>());
        TEST_VERIFY(bPtr == a.Get<const B*>());
        TEST_VERIFY(a.CanCast<A*>());
        TEST_VERIFY(a.CanCast<const A*>());
        TEST_VERIFY(baPtr == a.Cast<A*>());
        TEST_VERIFY(baPtr == a.Cast<const A*>());

        TEST_VERIFY(!a.CanCast<Stub*>());
        TEST_VERIFY(!a.CanCast<const Stub*>());
        TEST_VERIFY(!a.CanCast<int>());
        TEST_VERIFY(!a.CanCast<void>());
        TEST_VERIFY(!a.CanCast<Stub>());

        // multiple inheritance
        a.Set(dPtr);
        TEST_VERIFY(a.CanCast<A*>());
        TEST_VERIFY(a.CanCast<A1*>());
        TEST_VERIFY(dPtr == a.Get<void*>());
        TEST_VERIFY(daPtr == a.Cast<A*>());
        TEST_VERIFY(da1Ptr == a.Cast<A1*>());

        // hierarchy down cast
        a.Set(ePtr);
        TEST_VERIFY(a.CanCast<A1*>());
        TEST_VERIFY(ea1Ptr == a.Cast<A1*>());

        // hierarchy up cast
        a.Set(ea1Ptr);
        TEST_VERIFY(ePtr == a.Cast<E*>());

        try
        {
            a.Cast<int>();
            TEST_VERIFY(false && "Shouldn't be able to cast to int");
        }
        catch (const Exception&)
        {
            TEST_VERIFY(true);
        }
    }

    DAVA_TEST (AnyCastGetEnumTest)
    {
        Any a;

        a.Set(A::E1_1);
        TEST_VERIFY(A::E1_1 == a.Get<A::E1>());
        TEST_VERIFY(A::E1_2 != a.Get<A::E1>());

        a.Set(A::E2::E2_2);
        TEST_VERIFY(A::E2::E2_1 != a.Get<A::E2>());
        TEST_VERIFY(A::E2::E2_2 == a.Get<A::E2>());

        TEST_VERIFY(Any(A::E1_1).Cast<int8>() == static_cast<int8>(A::E1_1));
        TEST_VERIFY(Any(A::E1_1).Cast<int16>() == static_cast<int16>(A::E1_1));
        TEST_VERIFY(Any(A::E1_1).Cast<int32>() == static_cast<int32>(A::E1_1));
        TEST_VERIFY(Any(A::E1_1).Cast<int64>() == static_cast<int64>(A::E1_1));
        TEST_VERIFY(Any(A::E1_1).Cast<uint8>() == static_cast<uint8>(A::E1_1));
        TEST_VERIFY(Any(A::E1_1).Cast<uint16>() == static_cast<uint16>(A::E1_1));
        TEST_VERIFY(Any(A::E1_1).Cast<uint32>() == static_cast<uint32>(A::E1_1));
        TEST_VERIFY(Any(A::E1_1).Cast<uint64>() == static_cast<uint64>(A::E1_1));

        int v = static_cast<int>(A::E1_2);

        TEST_VERIFY(Any(static_cast<int8>(v)).Cast<A::E1>() == static_cast<A::E1>(static_cast<int8>(v)));
        TEST_VERIFY(Any(static_cast<int16>(v)).Cast<A::E1>() == static_cast<A::E1>(static_cast<int16>(v)));
        TEST_VERIFY(Any(static_cast<int32>(v)).Cast<A::E1>() == static_cast<A::E1>(static_cast<int32>(v)));
        TEST_VERIFY(Any(static_cast<int64>(v)).Cast<A::E1>() == static_cast<A::E1>(static_cast<int64>(v)));
        TEST_VERIFY(Any(static_cast<uint8>(v)).Cast<A::E1>() == static_cast<A::E1>(static_cast<uint8>(v)));
        TEST_VERIFY(Any(static_cast<uint16>(v)).Cast<A::E1>() == static_cast<A::E1>(static_cast<uint16>(v)));
        TEST_VERIFY(Any(static_cast<uint32>(v)).Cast<A::E1>() == static_cast<A::E1>(static_cast<uint32>(v)));
        TEST_VERIFY(Any(static_cast<uint64>(v)).Cast<A::E1>() == static_cast<A::E1>(static_cast<uint64>(v)));
    }

    DAVA_TEST (AnyCastFundamentalTest)
    {
        DoAnyCastHashTest<int8, int16>();
        DoAnyCastHashTest<int8, int32>();
        DoAnyCastHashTest<int8, int64>();
        DoAnyCastHashTest<int8, uint8>();
        DoAnyCastHashTest<int8, uint16>();
        DoAnyCastHashTest<int8, uint32>();
        DoAnyCastHashTest<int8, uint64>();
        DoAnyCastHashTest<int8, float32>();
        DoAnyCastHashTest<int8, float64>();
        DoAnyCastHashTest<int8, size_t>();

        DoAnyCastHashTest<int16, int32>();
        DoAnyCastHashTest<int16, int64>();
        DoAnyCastHashTest<int16, uint8>();
        DoAnyCastHashTest<int16, uint16>();
        DoAnyCastHashTest<int16, uint32>();
        DoAnyCastHashTest<int16, uint64>();
        DoAnyCastHashTest<int16, float32>();
        DoAnyCastHashTest<int16, float64>();
        DoAnyCastHashTest<int16, size_t>();

        DoAnyCastHashTest<int32, int64>();
        DoAnyCastHashTest<int32, uint8>();
        DoAnyCastHashTest<int32, uint16>();
        DoAnyCastHashTest<int32, uint32>();
        DoAnyCastHashTest<int32, uint64>();
        DoAnyCastHashTest<int32, float32>();
        DoAnyCastHashTest<int32, float64>();
        DoAnyCastHashTest<int32, size_t>();

        DoAnyCastHashTest<int64, uint8>();
        DoAnyCastHashTest<int64, uint16>();
        DoAnyCastHashTest<int64, uint32>();
        DoAnyCastHashTest<int64, uint64>();
        DoAnyCastHashTest<int64, float32>();
        DoAnyCastHashTest<int64, float64>();
        DoAnyCastHashTest<int64, size_t>();

        DoAnyCastHashTest<uint8, uint16>();
        DoAnyCastHashTest<uint8, uint32>();
        DoAnyCastHashTest<uint8, uint64>();
        DoAnyCastHashTest<uint8, float32>();
        DoAnyCastHashTest<uint8, float64>();
        DoAnyCastHashTest<uint8, size_t>();

        DoAnyCastHashTest<uint16, uint32>();
        DoAnyCastHashTest<uint16, uint64>();
        DoAnyCastHashTest<uint16, float32>();
        DoAnyCastHashTest<uint16, float64>();
        DoAnyCastHashTest<uint16, size_t>();

        DoAnyCastHashTest<uint32, uint64>();
        DoAnyCastHashTest<uint32, float32>();
        DoAnyCastHashTest<uint32, float64>();
        DoAnyCastHashTest<uint32, size_t>();

        DoAnyCastHashTest<uint64, float32>();
        DoAnyCastHashTest<uint64, float64>();
        DoAnyCastHashTest<uint64, size_t>();

        DoAnyCastHashTest<float32, float64>();
        DoAnyCastHashTest<float32, size_t>();
        DoAnyCastHashTest<float64, size_t>();
    }

    DAVA_TEST (AnyLoadStoreTest)
    {
        int v1 = 11223344;
        int v2 = 321;

        int* iptr1 = &v1;
        int* iptr2 = nullptr;

        Any a;

        // load test
        a.LoadData(&v1, Type::Instance<int>());
        TEST_VERIFY(a.Get<int>() == v1);

        // store test
        a.StoreData(&v2, sizeof(v2));
        TEST_VERIFY(v1 == v2);

        // load/store pointers
        a.LoadData(&iptr1, Type::Instance<int*>());
        a.StoreData(&iptr2, sizeof(iptr2));
        TEST_VERIFY(iptr1 == iptr2);

        // load/store trivial types
        Trivial triv;
        Trivial triv1{ 11, 22 };
        a.LoadData(&triv, Type::Instance<Trivial>());
        TEST_VERIFY(a.Get<Trivial>() == triv);
        a.StoreData(&triv1, sizeof(triv1));
        TEST_VERIFY(triv1 == triv);

        // load/store fail cases
        NotTrivial not_triv;
        TEST_VERIFY(!a.LoadData(&not_triv, Type::Instance<NotTrivial>()));
        TEST_VERIFY(!a.StoreData(&not_triv, sizeof(not_triv)));
        TEST_VERIFY(!a.StoreData(&triv, sizeof(triv) / 2));
    }

    DAVA_TEST (AnyCompareTest)
    {
        // simple
        {
            bool b1 = TypeDetails::IsEqualComparable<bool>::value;

            DoAnyCompareEqualTest<bool>(true, false);
            DoAnyCompareEqualTest<int32>(10, 20);
            DoAnyCompareEqualTest<uint32>(10, 20);
            DoAnyCompareEqualTest(10.0, 20.0);
        }

        // base
        {
            DoAnyCompareEqualTest(Color::Blue, Color::Red);
            DoAnyCompareEqualTest(Vector3(1.0f, 2.0f, 3.0f), Vector3(3.0f, 2.0f, 1.0f));

            Matrix4 m;
            m.BuildScale(Vector3(1.0, 2.0f, 3.0f));
            DoAnyCompareEqualTest(m, Matrix4::IDENTITY);
        }

        // strings
        {
            const char* str1 = "str1";
            const char* str2 = "str2";

            DoAnyCompareEqualTest(str1, str2);
            DoAnyCompareEqualTest(String(str1), String(str2));
            DoAnyCompareEqualTest(FastName(str1), FastName(str2));
            DoAnyCompareEqualTest(FilePath(str1), FilePath(str2));
        }

        // pointer
        {
            int i = 11223344;
            int* iptr = &i;
            void* vptr = static_cast<void*>(iptr);
            void* nptr = nullptr;

            Any a(iptr);
            Any b(vptr);
            Any a0(static_cast<int*>(nptr));
            Any b0(nptr);

            DoAnyCompareEqualTest<int*>(iptr, nullptr);

            TEST_VERIFY(a == b);
            TEST_VERIFY(a0 == b0);
            TEST_VERIFY(a != a0);
            TEST_VERIFY(b != b0);
        }

        // RefPtr
        {
            RefPtr<KeyedArchive> ka1(new KeyedArchive());
            RefPtr<KeyedArchive> ka2(new KeyedArchive());
            DoAnyCompareEqualTest(ka1, ka2);
        }

        // Vector
        {
            Vector<int> vec1{ 1, 2, 3 };
            Vector<int> vec2;

            Vector<Trivial> vecTrivial1;
            Vector<Trivial> vecTrivial2;
            vecTrivial1.push_back(Trivial());
            vecTrivial1.back().a = 10;

            DoAnyCompareEqualTest(vec1, vec2);
            DoAnyCompareEqualTest(vecTrivial1, vecTrivial2);
        }

        // no compare or less
        {
            NoCompareTemplate<int> i;
            TEST_VERIFY(!Any(i).IsEmpty());
        }

        // empty
        {
            Any a;
            Any b;

            // empty != value
            a.Clear();
            b.Set(123321);
            TEST_VERIFY(a != b);

            // empty != pointer
            a.Clear();
            b.Set(&a);
            TEST_VERIFY(a != b);

            // empty != nullptr
            a.Clear();
            b.Set(nullptr);
            TEST_VERIFY(a != b);

            // two empties ==
            a.Clear();
            b.Clear();
            TEST_VERIFY(a == b);
        }
    }

    DAVA_TEST (AnyFnTest)
    {
        A a;

        AnyFn fn;
        TEST_VERIFY(!fn.IsValid())

        fn = AnyFn(&A::StaticTestFn);
        TEST_VERIFY(fn.IsValid());
        TEST_VERIFY(fn.IsStatic());
        TEST_VERIFY(A::StaticTestFn(10, 20) == fn.Invoke(Any(10), Any(20)).Get<int32>());

        try
        {
            fn.Invoke();
            TEST_VERIFY(false && "Shouldn't be invoked with bad arguments");
        }
        catch (const Exception&)
        {
            TEST_VERIFY(true);
        }

        try
        {
            fn.BindThis(&a);
            TEST_VERIFY(false && "This shouldn't be binded to static function");
        }
        catch (const Exception&)
        {
            TEST_VERIFY(true);
        }

        fn = AnyFn(&A::TestFn);
        TEST_VERIFY(fn.IsValid());
        TEST_VERIFY(!fn.IsStatic());

        Vector3 op1(1.0f, 2.0f, 3.0f);
        Vector3 op2(7.0f, 0.7f, 0.0f);
        Vector3 op3(4.0f, 3.5f, 2.1f);

        Any res;

        fn.Invoke(Any(&a), Any(op1), Any(op2));
        res = fn.Invoke(Any(static_cast<const A*>(&a)), Any(op1), Any(op2));
        TEST_VERIFY(a.TestFn(op1, op2) == res.Get<Vector3>());

        // check compilation if pointer is const
        TEST_VERIFY(a.TestFn(op1, op2) == res.Get<Vector3>());

        fn.BindThis(&a);
        fn = fn.BindThis(static_cast<const A*>(&a));
        TEST_VERIFY(fn.IsValid());
        TEST_VERIFY(fn.IsStatic());

        a.a = 5;
        res = fn.Invoke(Any(op1), Any(op3));
        TEST_VERIFY(a.TestFn(op1, op3) == res.Get<Vector3>());

        fn = AnyFn(&A::TestFnConst);
        res = fn.Invoke(Any(&a), Any(op2), Any(op3));
        TEST_VERIFY(a.TestFnConst(op2, op3) == res.Get<Vector3>());
    }

    DAVA_TEST (AnyFnWithCastTest)
    {
        A a;
        a.a = 13;

        A* b = new B(31);

        // check args casting
        {
            AnyFn aFnInt(&A::StaticTestFn);
            AnyFn aFnFloat(&A::TestFnFloat);

            float32 farg1 = 111.5445346f;
            float32 farg2 = 46768.12543f;
            Any anyRet = aFnInt.InvokeWithCast(farg1, farg2);
            int32 iret = A::StaticTestFn(static_cast<int32>(farg1), static_cast<int32>(farg2));
            TEST_VERIFY(iret == anyRet.Get<int32>());

            anyRet = aFnInt.InvokeWithCast(farg1, static_cast<int32>(farg2));
            TEST_VERIFY(iret == anyRet.Get<int32>());

            int32 arg1 = 543;
            anyRet = aFnFloat.InvokeWithCast(&a, arg1);
            float32 fret = a.TestFnFloat(static_cast<float32>(arg1));
            TEST_VERIFY(fret == anyRet.Get<float32>());

            anyRet = aFnFloat.InvokeWithCast(&a, static_cast<float32>(arg1));
            TEST_VERIFY(fret == anyRet.Get<float32>());
        }

        // check first argument 'this' casting
        {
            AnyFn aFn(&A::TestFn);
            AnyFn aFnConst(&A::TestFnConst);

            AnyFn bFn(&B::TestFn);
            AnyFn bFnConst(&B::TestFnConst);

            Vector3 arg1(11.0f, 22.44f, 3.38f);
            Vector3 arg2(71.15f, 0.7f, 130.0f);

            A tmpA;
            tmpA.a = 43;

            // call A functions with that = A
            Any res = aFn.InvokeWithCast(tmpA, arg1, arg2);
            TEST_VERIFY(res.Get<Vector3>() == tmpA.TestFn(arg1, arg2));

            res = aFnConst.InvokeWithCast(tmpA, arg1, arg2);
            TEST_VERIFY(res.Get<Vector3>() == tmpA.TestFnConst(arg1, arg2));

            // call A functions with that = A*, that points on B
            res = aFn.InvokeWithCast(b, arg1, arg2);
            TEST_VERIFY(res.Get<Vector3>() == b->TestFn(arg1, arg2));

            res = aFnConst.InvokeWithCast(b, arg1, arg2);
            TEST_VERIFY(res.Get<Vector3>() == b->TestFnConst(arg1, arg2));

            // call B functions that = A*, that points on B
            res = bFn.InvokeWithCast(b, arg1, arg2);
            TEST_VERIFY(res.Get<Vector3>() == b->TestFn(arg1, arg2));

            res = bFnConst.InvokeWithCast(b, arg1, arg2);
            TEST_VERIFY(res.Get<Vector3>() == b->TestFnConst(arg1, arg2));
        }
    }

    template <typename R, typename... T>
    void DoAnyFnInvokeTest(const T&... args)
    {
        A a;
        AnyFn fn(&A::TestSum<R, T...>);

        auto& invokeParams = fn.GetInvokeParams();
        TEST_VERIFY(invokeParams.retType == Type::Instance<R>());
        TEST_VERIFY(invokeParams.argsType.at(0) == Type::Instance<A*>());
        TEST_VERIFY(invokeParams.argsType.size() == (sizeof...(args) + 1));

        Any res = fn.Invoke(&a, args...);
        TEST_VERIFY(res.Get<R>() == a.TestSum<R>(args...));

        try
        {
            fn.Invoke(nullptr, nullptr);
            TEST_VERIFY(false && "AnyFn shouldn't invoke with bad arguments");
        }
        catch (const Exception&)
        {
            TEST_VERIFY(true);
        }

        // now bind this, and test once again
        fn = fn.BindThis(&a);
        auto& invokeParams1 = fn.GetInvokeParams();
        TEST_VERIFY(invokeParams1.retType == Type::Instance<R>());
        TEST_VERIFY(invokeParams1.argsType.size() == sizeof...(args));

        res = fn.Invoke(args...);
        TEST_VERIFY(res.Get<R>() == a.TestSum<R>(args...));
    }

    DAVA_TEST (AnyFnInvokeTest)
    {
        DoAnyFnInvokeTest<int>(1);
        DoAnyFnInvokeTest<int>(1, 2);
        DoAnyFnInvokeTest<int>(1, 2, 3);
        DoAnyFnInvokeTest<int>(1, 2, 3, 4);
        DoAnyFnInvokeTest<int>(1, 2, 3, 4, 5);

        DoAnyFnInvokeTest<float>(10.0f);
        DoAnyFnInvokeTest<float>(10.0f, 0.2f);
        DoAnyFnInvokeTest<float>(10.0f, 0.2f, 3.0f);
        DoAnyFnInvokeTest<float>(10.0f, 0.2f, 3.0f, 0.04f);
        DoAnyFnInvokeTest<float>(10.0f, 0.2f, 3.0f, 0.04f, 500.0f);
    }
};

} // namespace DAVA
