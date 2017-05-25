/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "UnitTests/UnitTests.h"
#include "Math/AABBox3.h"
#include "Utils/Random.h"

using namespace DAVA;

DAVA_TESTCLASS (RayMathTest)
{
    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(DavaFramework)
    DECLARE_COVERED_FILES("AABBox3.cpp")
    DECLARE_COVERED_FILES("Ray.cpp")
    END_FILES_COVERED_BY_TESTS()

    DAVA_TEST (AABBox3Test)
    {
        AABBox3 emptyBox;
        Vector3 center(0.0f, 0.0f, 0.0f);
        float largeBoxSize = 10.0f;
        float smallBoxSize = 1.0f;
        AABBox3 largeBox(center, largeBoxSize);
        AABBox3 smallBox(center, smallBoxSize);

        TEST_VERIFY(largeBox.IntersectsWithBox(smallBox));
        TEST_VERIFY(smallBox.IntersectsWithBox(smallBox));
        TEST_VERIFY(smallBox.IntersectsWithBox(smallBox));
        TEST_VERIFY(emptyBox.IsEmpty() == true);
        TEST_VERIFY(smallBox.IsEmpty() == false);

        TEST_VERIFY(largeBox.IsInside(center));
        TEST_VERIFY(smallBox.IsInside(center));

        TEST_VERIFY(largeBox.IsInside(smallBox) == true);
        TEST_VERIFY(smallBox.IsInside(largeBox) == false);

        Vector3 testCenter = largeBox.GetCenter();
        TEST_VERIFY(FLOAT_EQUAL(center.x, testCenter.x) && FLOAT_EQUAL(center.y, testCenter.y) && FLOAT_EQUAL(center.z, testCenter.z));

        Vector3 testSize = largeBox.GetSize();
        TEST_VERIFY(FLOAT_EQUAL(largeBoxSize, testSize.x) && FLOAT_EQUAL(largeBoxSize, testSize.y) && FLOAT_EQUAL(largeBoxSize, testSize.z));

        {
            AABBox3 testBox(center, largeBoxSize);
            TEST_VERIFY(testBox == largeBox);
            TEST_VERIFY(testBox != smallBox);
        }

        AABBox3 transformedBox;
        smallBox.GetTransformedBox(Matrix4::MakeRotation(Vector3::UnitX, PI / 2.0f), transformedBox);
        TEST_VERIFY(FLOAT_EQUAL(transformedBox.min.x, smallBox.min.x));
        TEST_VERIFY(FLOAT_EQUAL(transformedBox.min.y, smallBox.min.y));
        TEST_VERIFY(FLOAT_EQUAL(transformedBox.min.z, smallBox.min.z));
        TEST_VERIFY(FLOAT_EQUAL(transformedBox.max.x, smallBox.max.x));
        TEST_VERIFY(FLOAT_EQUAL(transformedBox.max.y, smallBox.max.y));
        TEST_VERIFY(FLOAT_EQUAL(transformedBox.max.z, smallBox.max.z));
    }

    DAVA_TEST (RayAABBoxCollisionTest)
    {
        {
            AABBox3 box(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.5f, 0.5f, 0.5f));

            Ray3 bordersTest[] =
            {
              // along all sides tests
              Ray3(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 2.0f)),
              Ray3(Vector3(0.5f, 0.5f, -1.0f), Vector3(0.0f, 0.0f, 2.0f)),
              Ray3(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -2.0f)),
              Ray3(Vector3(0.5f, 0.5f, 1.0f), Vector3(0.0f, 0.0f, -2.0f)),
              Ray3(Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, 2.0f, 0.0f)),
              Ray3(Vector3(0.5f, -1.0f, 0.5f), Vector3(0.0f, 2.0f, 0.0f)),
              Ray3(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, -2.0f, 0.0f)),
              Ray3(Vector3(0.5f, 1.0f, 0.5f), Vector3(0.0f, -2.0f, 0.0f)),
              Ray3(Vector3(-1.0f, 0.0f, 0.0f), Vector3(2.0f, 0.0f, 0.0f)),
              Ray3(Vector3(-1.0f, 0.5f, 0.5f), Vector3(2.0f, 0.0f, 0.0f)),
              Ray3(Vector3(1.0f, 0.0f, 0.0f), Vector3(-2.0f, 0.0f, 0.0f)),
              Ray3(Vector3(1.0f, 0.5f, 0.5f), Vector3(-2.0f, 0.0f, 0.0f)),
              Ray3(Vector3(-0.5f, -0.5f, -0.5f), Vector3(2.0f, 2.0f, 2.0f)),
              Ray3(Vector3(1.0f, 1.0f, 1.0f), Vector3(-2.0f, -2.0f, -2.0f)),
            };
            float results[][2]
            {
              { 0.5f, 0.75f },
              { 0.5f, 0.75f },
              { 0.25f, 0.5f },
              { 0.25f, 0.5f },

              { 0.5f, 0.75f },
              { 0.5f, 0.75f },
              { 0.25f, 0.5f },
              { 0.25f, 0.5f },

              { 0.5f, 0.75f },
              { 0.5f, 0.75f },
              { 0.25f, 0.5f },
              { 0.25f, 0.5f },

              { 0.25f, 0.5f },
              { 0.25f, 0.5f },
            };

            // Check borders
            for (uint32 k = 0; k < sizeof(bordersTest) / sizeof(Ray3); ++k)
            {
                float32 tMin, tMax;
                TEST_VERIFY(Intersection::RayBox(bordersTest[k], box) == true);

                TEST_VERIFY(Intersection::RayBox(bordersTest[k], box, tMin) == true);
                TEST_VERIFY(FLOAT_EQUAL(tMin, results[k][0]));

                TEST_VERIFY(Intersection::RayBox(bordersTest[k], box, tMin, tMax) == true);
                TEST_VERIFY(FLOAT_EQUAL(tMin, results[k][0]));
                TEST_VERIFY(FLOAT_EQUAL(tMax, results[k][1]));

                Ray3Optimized rayOpt(bordersTest[k].origin, bordersTest[k].direction);

                TEST_VERIFY(Intersection::RayBox(rayOpt, box, tMin, tMax) == true);
                TEST_VERIFY(FLOAT_EQUAL(tMin, results[k][0]));
                TEST_VERIFY(FLOAT_EQUAL(tMax, results[k][1]));
            }
        }
        {
            AABBox3 box(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));

            Ray3 bordersTest[] =
            {
              // along all sides tests
              Ray3(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 2.0f)),
              Ray3(Vector3(0.5f, 0.5f, -1.0f), Vector3(0.0f, 0.0f, 2.0f)),
            };
            float results[][2]
            {
              { 0.5f, 1.0f },
              { 0.5f, 1.0f },
            };

            // Check borders
            for (uint32 k = 0; k < sizeof(bordersTest) / sizeof(Ray3); ++k)
            {
                float32 tMin, tMax;
                TEST_VERIFY(Intersection::RayBox(bordersTest[k], box, tMin));

                TEST_VERIFY(Intersection::RayBox(bordersTest[k], box, tMin) == true);
                TEST_VERIFY(FLOAT_EQUAL(tMin, results[k][0]));

                TEST_VERIFY(Intersection::RayBox(bordersTest[k], box, tMin, tMax) == true);
                TEST_VERIFY(FLOAT_EQUAL(tMin, results[k][0]));
                TEST_VERIFY(FLOAT_EQUAL(tMax, results[k][1]));

                Ray3Optimized rayOpt(bordersTest[k].origin, bordersTest[k].direction);

                TEST_VERIFY(Intersection::RayBox(rayOpt, box, tMin, tMax) == true);
                TEST_VERIFY(FLOAT_EQUAL(tMin, results[k][0]));
                TEST_VERIFY(FLOAT_EQUAL(tMax, results[k][1]));
            }
        }

        AABBox3 box2(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.3f, 1.2f, 1.5f));

        Ray3 sideRayBase[] =
        {
          Ray3(Vector3(box2.min.x, box2.min.y, box2.min.z - 0.5f), Vector3(0.0f, 0.0f, 1.0f)),
          Ray3(Vector3(box2.min.x, box2.min.y, box2.max.z + 0.5f), Vector3(0.0f, 0.0f, -1.0f)),

          Ray3(Vector3(box2.min.x, box2.min.y - 0.5f, box2.min.z), Vector3(0.0f, 1.0f, 0.0f)),
          Ray3(Vector3(box2.min.x, box2.max.y + 0.5f, box2.min.z), Vector3(0.0f, -1.0f, 0.0f)),

          Ray3(Vector3(box2.min.x - 0.5f, box2.min.y, box2.min.z), Vector3(1.0f, 0.0f, 0.0f)),
          Ray3(Vector3(box2.max.x + 0.5f, box2.min.y, box2.min.z), Vector3(-1.0f, 0.0f, 0.0f)),
        };

        Vector3 randShift[] =
        {
          Vector3(1.0f, 1.0f, 0.0f),
          Vector3(1.0f, 1.0f, 0.0f),
          Vector3(1.0f, 0.0f, 1.0f),
          Vector3(1.0f, 0.0f, 1.0f),
          Vector3(0.0f, 1.0f, 1.0f),
          Vector3(0.0f, 1.0f, 1.0f),
        };
        Vector3 size = box2.GetSize();

        // Random sample each side (inside)
        for (uint32 side = 0; side < 6; ++side)
        {
            for (uint32 t = 0; t < 1000; ++t)
            {
                float32 r = Random::Instance()->RandFloat32InBounds(0.0f, 1.0f);
                Ray3 ray = sideRayBase[side];
                ray.origin += randShift[side] * size * r;
                float32 tMin, tMax;
                TEST_VERIFY(Intersection::RayBox(ray, box2) == true);

                TEST_VERIFY(Intersection::RayBox(ray, box2, tMin) == true);
                TEST_VERIFY(FLOAT_EQUAL(tMin, 0.5f));

                TEST_VERIFY(Intersection::RayBox(ray, box2, tMin, tMax) == true);
                TEST_VERIFY(FLOAT_EQUAL(tMin, 0.5f));

                Ray3Optimized rayOpt(ray.origin, ray.direction);
                TEST_VERIFY(Intersection::RayBox(rayOpt, box2, tMin, tMax) == true);
                TEST_VERIFY(FLOAT_EQUAL(tMin, 0.5f));
            }

            for (uint32 t = 0; t < 1000; ++t)
            {
                float32 r = Random::Instance()->RandFloat32InBounds(0.1f, 1.0f);
                Ray3 ray = sideRayBase[side];
                ray.origin += -randShift[side] * size * r;
                float32 tMin, tMax;
                TEST_VERIFY(Intersection::RayBox(ray, box2) == false);

                TEST_VERIFY(Intersection::RayBox(ray, box2, tMin) == false);

                TEST_VERIFY(Intersection::RayBox(ray, box2, tMin, tMax) == false);

                Ray3Optimized rayOpt(ray.origin, ray.direction);
                TEST_VERIFY(Intersection::RayBox(rayOpt, box2, tMin, tMax) == false);
            }
        }

        // Big distance test

        // Test end of ray inside box

        // Test whole ray inside box
        {
            Ray3 ray(Vector3(0.5f, 0.5f, 0.5f), Vector3(0.1f, 0.0f, 0.0f));
            AABBox3 box(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
            float32 tMin, tMax;
            TEST_VERIFY(Intersection::RayBox(ray, box) == true);

            TEST_VERIFY(Intersection::RayBox(ray, box, tMin) == true);
            TEST_VERIFY(tMin == 5.0f);

            TEST_VERIFY(Intersection::RayBox(ray, box, tMin, tMax) == true);
            TEST_VERIFY(tMin == -5.0f);
            TEST_VERIFY(tMax == 5.0f);
        }

        {
            Ray3 ray(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 0.0f, 0.0f));
            AABBox3 box(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
            float32 tMin, tMax;

            TEST_VERIFY(Intersection::RayBox(ray, box) == true);

            TEST_VERIFY(Intersection::RayBox(ray, box, tMin) == true);
            TEST_VERIFY(tMin == 0.0f);

            TEST_VERIFY(Intersection::RayBox(ray, box, tMin, tMax) == true);
            TEST_VERIFY(tMin == 0.0f);
            TEST_VERIFY(tMax == 0.5f);
        }
    };

    DAVA_TEST (TriangleAABBoxCollisionTest)
    {
        AABBox3 box(Vector3(0.0f, 0.0f, 0.0f), Vector3(10.0f, 10.0f, 10.0f));

        Vector3 p0, p1, p2;
        p0 = Vector3(1.0f, 3.0f, 1.0f);
        p1 = Vector3(3.0f, 1.0f, 1.0f);
        p2 = Vector3(1.0f, 1.0f, 3.0f);
        TEST_VERIFY(Intersection::BoxTriangle(box, p0, p1, p2) == true);

        p0 = Vector3(-1.0f, -3.0f, -1.0f);
        p1 = Vector3(-3.0f, -1.0f, -1.0f);
        p2 = Vector3(-1.0f, -1.0f, -3.0f);
        TEST_VERIFY(Intersection::BoxTriangle(box, p0, p1, p2) == false);

        p0 = Vector3(0.0f, 0.0f, 0.0f);
        p1 = Vector3(-3.0f, -1.0f, -1.0f);
        p2 = Vector3(-1.0f, -1.0f, -3.0f);
        TEST_VERIFY(Intersection::BoxTriangle(box, p0, p1, p2) == true); // Edge should be count as intersection

        p0 = Vector3(-1000.0f, -1000.0f, -1000.0f);
        p1 = Vector3(1000.0f, 1000.0f, 1000.0f);
        p2 = Vector3(1000.0f, 1000.0f, 1000.0f);
        TEST_VERIFY(Intersection::BoxTriangle(box, p0, p1, p2) == true);

        p0 = Vector3(5.0f, 1000.0f, 1000.0f);
        p1 = Vector3(5.0f, -1000.0f, 1000.0f);
        p2 = Vector3(5.0f, 1000.0f, -1000.0f);
        TEST_VERIFY(Intersection::BoxTriangle(box, p0, p1, p2) == true);

        /* p0 = Vector3(0.0f, 0.0f, 0.0f);
         p1 = Vector3(1.0f, 0.0f, 0.0f);
         p2 = Vector3(0.0f, 0.0f, 1.0f);
         TEST_VERIFY(box.IsIntersectsWithTriangle(p0, p1, p2) == true);
         
         p0 = Vector3(0.0f, 0.0f, 0.0f);
         p1 = Vector3(0.0f, 1.0f, 0.0f);
         p2 = Vector3(0.0f, 0.0f, 1.0f);
         TEST_VERIFY(box.IsIntersectsWithTriangle(p0, p1, p2) == true);*/

        p0 = Vector3(-50.0f, 5.0f, 5.0f);
        p1 = Vector3(100.0f, 5.0f, 5.0f);
        p2 = Vector3(101.0f, 5.0f, 5.0f);
        TEST_VERIFY(Intersection::BoxTriangle(box, p0, p1, p2) == true);

        p0 = Vector3(150.0f, 5.0f, 5.0f);
        p1 = Vector3(-50.0f, 5.0f, 5.0f);
        p2 = Vector3(-51.0f, 5.0f, 5.0f);
        TEST_VERIFY(Intersection::BoxTriangle(box, p0, p1, p2) == true);

        // The difference in results is just the math of the intersection test.
        p0 = Vector3(1500.0f, 5.0f, 5.0f);
        p1 = Vector3(10.0003f, 5.0f, 5.0f);
        p2 = Vector3(10.0003f, 5.0f, 5.0f);
        TEST_VERIFY(Intersection::BoxTriangle(box, p0, p1, p2) == false); // result is false because it's outside the box

        p0 = Vector3(1500.0f, 5.0f, 5.0f);
        p1 = Vector3(10.0001f, 5.0f, 5.0f);
        p2 = Vector3(10.0001f, 5.0f, 5.0f);
        TEST_VERIFY(Intersection::BoxTriangle(box, p0, p1, p2) == false);

        p0 = Vector3(20.0f, 5.0f, 5.0f);
        p1 = Vector3(10.0001f, 5.0f, 5.0f);
        p2 = Vector3(10.0001f, 5.0f, 5.0f);
        TEST_VERIFY(Intersection::BoxTriangle(box, p0, p1, p2) == false); // here origin is closer and same outside result tend to be inside.

        p0 = Vector3(-5000.0, -5000.0, -5000.0);
        p1 = Vector3(5000.0f, 0.0f, 5000.0f);
        p1 = Vector3(0.0f, 5000.0f, 5000.0f);
        TEST_VERIFY(Intersection::BoxTriangle(box, p0, p1, p2) == true);

        // Corner cases
        //        {
        //            /*
        //             box -(-5.06715011596680,-5.85103988647461,-0.11697360128164) (0.00000000000000, 0.00000000000000, 7.17094850540161)
        //
        //             (0.00000101618002, -3.77569198608398, 3.90985894203186)
        //             (-0.00000074881905, 3.77569198608398, 3.90985894203186)
        //             (3.26984500885010, 1.88784694671631, 3.90985894203186)
        //             */
        //
        //            AABBox3 box(Vector3(-5.06715011596680f,-5.85103988647461f,-0.11697360128164f),
        //                    Vector3(0.00000000000000f, 0.00000000000000f, 7.17094850540161f));
        //
        //            p0 = Vector3(0.00000101618002f, -3.77569198608398f, 3.90985894203186f);
        //            p1 = Vector3(-0.00000074881905f, 3.77569198608398f, 3.90985894203186f);
        //            p2 = Vector3(3.26984500885010f, 1.88784694671631f, 3.90985894203186f);
        //            TEST_VERIFY(box.IsIntersectsWithTriangle(p0, p1, p2) == true);
        //
        //            Vector3 halfBox = box.GetSize() * 0.5f;
        //
        //            for (uint32 xdiv = 0; xdiv < 2; ++xdiv)
        //            {
        //                for (uint32 ydiv = 0; ydiv < 2; ++ydiv)
        //                {
        //                    for (uint32 zdiv = 0; zdiv < 2; ++zdiv)
        //                    {
        //                        Vector3 childBoxMin(box.min.x + halfBox.x * (float32)xdiv,
        //                                            box.min.y + halfBox.y * (float32)ydiv,
        //                                            box.min.z + halfBox.z * (float32)zdiv);
        //                        AABBox3 childBox(childBoxMin, childBoxMin + halfBox);
        //
        //                        Logger::FrameworkDebug("box -(%.14f,%.14f,%.14f) (%.14f, %.14f, %.14f) %d",
        //                                               childBox.min.x, childBox.min.y, childBox.min.z,
        //                                               childBox.max.x, childBox.max.y, childBox.max.z,
        //                                               childBox.IsIntersectsWithTriangle(p0, p1, p2) ? (1) : (0));
        //
        //                    }
        //                }
        //            }
        //
        //        }
    }

    DAVA_TEST (TriangleAABBoxFloatPrecisionProblemTest)
    {
        AABBox3 box(Vector3(0.0f, 0.0f, 0.0f), Vector3(10.0f, 10.0f, 10.0f));

        Vector3 p0, p1, p2;
        p0 = Vector3(1.0f, 3.0f, 1.0f);
        p1 = Vector3(3.0f, 1.0f, 1.0f);
        p2 = Vector3(1.0f, 1.0f, 3.0f);
        TEST_VERIFY(Intersection::BoxTriangle(box, p0, p1, p2) == true);
    }
};
