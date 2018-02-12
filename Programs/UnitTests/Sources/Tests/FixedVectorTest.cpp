#include <Base/FixedVector.h>
#include <UnitTests/UnitTests.h>

#include <algorithm>
#include <numeric>

using namespace DAVA;

DAVA_TESTCLASS (FixedVectorTest)
{
    DAVA_TEST (TestConstructors)
    {
        FixedVector<int> v1(10);
        TEST_VERIFY(v1.size() == 0);
        TEST_VERIFY(v1.empty() == true);
        TEST_VERIFY(v1.max_size() == 10);
        TEST_VERIFY(v1.capacity() == v1.max_size());

        FixedVector<int> v2(10, 3, 42);
        TEST_VERIFY(v2.size() == 3);
        TEST_VERIFY(v2.empty() == false);
        TEST_VERIFY(v2.max_size() == 10);
        TEST_VERIFY(v2[0] == 42 && v2[1] == 42 && v2[2] == 42);

        const FixedVector<int> v3(10, { 1, 2, 3, 4 });
        FixedVector<int> v4(v3);
        v1 = v3;
        TEST_VERIFY(v3.size() == 4);
        TEST_VERIFY(v3.empty() == false);
        TEST_VERIFY(v3.max_size() == 10);
        TEST_VERIFY(v3[0] == 1 && v3[1] == 2 && v3[2] == 3 && v3[3] == 4);

        TEST_VERIFY(v4.size() == 4);
        TEST_VERIFY(v4.empty() == false);
        TEST_VERIFY(v4.max_size() == 10);
        TEST_VERIFY(v4[0] == 1 && v4[1] == 2 && v4[2] == 3 && v4[3] == 4);

        TEST_VERIFY(v1.size() == 4);
        TEST_VERIFY(v1.empty() == false);
        TEST_VERIFY(v1.max_size() == 10);
        TEST_VERIFY(v1[0] == 1 && v1[1] == 2 && v1[2] == 3 && v1[3] == 4);
    }

    DAVA_TEST (TestAccessorsAndModifiers)
    {
        FixedVector<String> v1(5, { "n1", "n2", "n3" });

        TEST_VERIFY(v1.front() == "n1");
        TEST_VERIFY(v1.back() == "n3");

        String* data = v1.data();
        TEST_VERIFY(data[0] == v1.front() && data[v1.size() - 1] == v1.back());
        const String* cdata = v1.data();
        TEST_VERIFY(cdata[0] == v1.front() && cdata[v1.size() - 1] == v1.back());

        String n42 = "n42";
        v1.push_back(n42); // push_back(const T&)
        TEST_VERIFY(v1.size() == 4);
        TEST_VERIFY(v1.back() == "n42");
        TEST_VERIFY(data == v1.data());

        v1.pop_back();
        TEST_VERIFY(v1.size() == 3);
        TEST_VERIFY(v1.back() == "n3");

        v1.push_back(std::move(n42)); // push_back(T&&)
        TEST_VERIFY(v1.size() == 4);
        TEST_VERIFY(v1.back() == "n42");

        v1.resize(2);
        TEST_VERIFY(v1.size() == 2);
        TEST_VERIFY(v1.front() == "n1" && v1.back() == "n2");
        TEST_VERIFY(cdata == v1.data());

        String& newval = v1.emplace_back(3, 'X'); // string(count, char)
        TEST_VERIFY(v1.size() == 3);
        TEST_VERIFY(v1.back() == "XXX");
        TEST_VERIFY(newval == "XXX");
        TEST_VERIFY(newval.c_str() == v1.back().c_str());

        v1.resize(5, "Ka");
        TEST_VERIFY(v1.size() == 5);
        TEST_VERIFY(v1[0] == "n1" && v1[1] == "n2" && v1[2] == "XXX" && v1[3] == "Ka" && v1[4] == "Ka");
        TEST_VERIFY(data == v1.data());

        v1.resize(0);
        TEST_VERIFY(v1.size() == 0);
        TEST_VERIFY(v1.empty() == true);
        TEST_VERIFY(v1.max_size() == 5);

        v1.resize(3, "test");
        v1.resize(0, "XXX");
        TEST_VERIFY(v1.size() == 0);
        TEST_VERIFY(v1.empty() == true);
        TEST_VERIFY(v1.max_size() == 5);

        v1.resize(3, "test");
        v1.clear();
        TEST_VERIFY(v1.size() == 0);
        TEST_VERIFY(v1.empty() == true);
        TEST_VERIFY(v1.max_size() == 5);
    }

    DAVA_TEST (TestIterators)
    {
        int reference[] = { 4, 3, 1, 5, 2 };
        FixedVector<int> v{ 5, { 4, 3, 1, 5, 2 } };

        int k = 0;
        for (auto i = v.begin(); i != v.end(); ++i, ++k)
        {
            TEST_VERIFY(*i == reference[k]);
        }

        k = 0;
        for (auto i = v.cbegin(); i != v.cend(); ++i, ++k)
        {
            TEST_VERIFY(*i == reference[k]);
        }

        k = 0;
        for (int i : v)
        {
            TEST_VERIFY(i == reference[k]);
            k += 1;
        }

        TEST_VERIFY(std::equal(std::begin(v), std::end(v), std::begin(reference)));

        std::sort(v.begin(), v.end());
        std::sort(std::begin(reference), std::end(reference));
        TEST_VERIFY(std::equal(std::begin(v), std::end(v), std::begin(reference)));

        TEST_VERIFY(std::accumulate(v.cbegin(), v.cend(), 0) == 15);
    }

    DAVA_TEST (TestComparison)
    {
        FixedVector<int> v1(3, { 3, 2, 1 });
        FixedVector<int> v2(v1);

        TEST_VERIFY(v1 == v2);

        std::sort(v1.begin(), v1.end());
        TEST_VERIFY(v1 != v2);
    }
};
