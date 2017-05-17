#include "UnitTests/UnitTests.h"
#include "Render/Highlevel/GeometryOctTree.h"
#include "Render/Highlevel/GeometryGenerator.h"
#include <random>

#include <stdlib.h>
#include <math.h>

using namespace DAVA;

#define RAND48_SEED_0 (0x330e)
#define RAND48_SEED_1 (0xabcd)
#define RAND48_SEED_2 (0x1234)
#define RAND48_MULT_0 (0xe66d)
#define RAND48_MULT_1 (0xdeec)
#define RAND48_MULT_2 (0x0005)
#define RAND48_ADD (0x000b)

unsigned short _rand48_seed[3] = {
    RAND48_SEED_0,
    RAND48_SEED_1,
    RAND48_SEED_2
};
unsigned short _rand48_mult[3] = {
    RAND48_MULT_0,
    RAND48_MULT_1,
    RAND48_MULT_2
};
unsigned short _rand48_add = RAND48_ADD;

void _dorand48(unsigned short xseed[3])
{
    unsigned long accu;
    unsigned short temp[2];

    accu = (unsigned long)_rand48_mult[0] * (unsigned long)xseed[0] +
    (unsigned long)_rand48_add;
    temp[0] = (unsigned short)accu; /* lower 16 bits */
    accu >>= sizeof(unsigned short) * 8;
    accu += (unsigned long)_rand48_mult[0] * (unsigned long)xseed[1] +
    (unsigned long)_rand48_mult[1] * (unsigned long)xseed[0];
    temp[1] = (unsigned short)accu; /* middle 16 bits */
    accu >>= sizeof(unsigned short) * 8;
    accu += _rand48_mult[0] * xseed[2] + _rand48_mult[1] * xseed[1] + _rand48_mult[2] * xseed[0];
    xseed[0] = temp[0];
    xseed[1] = temp[1];
    xseed[2] = (unsigned short)accu;
}

double erand48(unsigned short xseed[3])
{
    _dorand48(xseed);
    return ldexp((double)xseed[0], -48) +
    ldexp((double)xseed[1], -32) +
    ldexp((double)xseed[2], -16);
}

struct Vec
{ // Usage: time ./smallpt 5000 && xv image.ppm
    double x, y, z; // position, also color (r,g,b)
    Vec(double x_ = 0, double y_ = 0, double z_ = 0)
    {
        x = x_;
        y = y_;
        z = z_;
    }
    //Vec operator=(Vec &b) { this->x = b.x; this->y = b.y; this->z = b.z; return *this;}
    Vec operator+(const Vec& b) const
    {
        return Vec(x + b.x, y + b.y, z + b.z);
    }
    Vec operator-(const Vec& b) const
    {
        return Vec(x - b.x, y - b.y, z - b.z);
    }
    Vec operator*(double b) const
    {
        return Vec(x * b, y * b, z * b);
    }
    Vec mult(const Vec& b) const
    {
        return Vec(x * b.x, y * b.y, z * b.z);
    }
    Vec& norm()
    {
        return * this = *this * (1 / sqrt(x * x + y * y + z * z));
    }
    double dot(const Vec& b) const
    {
        return x * b.x + y * b.y + z * b.z;
    } // cross:
    Vec operator%(Vec& b)
    {
        return Vec(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
    }
};

struct Ray
{
    Vec o, d;
    Ray(Vec o_, Vec d_)
        : o(o_)
        , d(d_)
    {
    }
};

enum Refl_t
{
    DIFF,
    SPEC,
    REFR
}; // material types, used in radiance()
struct GeoObject
{
    double rad; // radius
    Vec p, e, c, x, n; // position, emission, color, normal
    Vec normalIntersection;
    Vec positionIntersection;

    Refl_t refl; // reflection type (DIFFuse, SPECular, REFRactive)
    PolygonGroup* pgroup = nullptr;
    GeoObject(double rad_, Vec p_, Vec e_, Vec c_, Refl_t refl_)
        :
        rad(rad_)
        , p(p_)
        , e(e_)
        , c(c_)
        , refl(refl_)
    {
    }

    double intersect(const Ray& r)
    { // returns distance, 0 if nohit

        if (pgroup)
        {
            Ray3Optimized ray(Vector3((float)r.o.x, (float)r.o.y, (float)r.o.z), Vector3((float)r.d.x, (float)r.d.y, (float)r.d.z));
            float32 result;
            uint32 resultTriIndex;
            if (pgroup->GetGeometryOctTree()->IntersectionWithRay(ray, result, resultTriIndex))
            {
                x = r.o + r.d * result;
                n = (x - p).norm();
                return result;
            }

            return 0.0;
        }
        else
        {
            Vec op = p - r.o; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
            double t, eps = 1e-4, b = op.dot(r.d), det = b * b - op.dot(op) + rad * rad;
            if (det < 0)
                return 0;
            else
                det = sqrt(det);

            t = b - det;
            if (t <= eps)
            {
                t = b + det;
                if (t <= eps)
                    return 0.0;
            }
            x = r.o + r.d * t;
            n = (x - p).norm();
            return t;
        }
        return 0.0;
    }
};

GeoObject spheres[] =
{
  //Scene: radius, position, emission, color, material
  GeoObject(1e5, Vec(1e5 + 1, 40.8, 81.6), Vec(), Vec(.75, .25, .25), DIFF), //Left
  GeoObject(1e5, Vec(-1e5 + 99, 40.8, 81.6), Vec(), Vec(.25, .25, .75), DIFF), //Rght
  GeoObject(1e5, Vec(50, 40.8, 1e5), Vec(), Vec(.75, .75, .75), DIFF), //Back
  GeoObject(1e5, Vec(50, 40.8, -1e5 + 170), Vec(), Vec(), DIFF), //Frnt
  GeoObject(1e5, Vec(50, 1e5, 81.6), Vec(), Vec(.75, .75, .75), DIFF), //Botm
  GeoObject(1e5, Vec(50, -1e5 + 81.6, 81.6), Vec(), Vec(.75, .75, .75), DIFF), //Top

  GeoObject(16.5, Vec(27, 16.5, 47), Vec(), Vec(.75, .25, .25), DIFF), //Mirr
  GeoObject(16.5, Vec(73, 16.5, 78), Vec(), Vec(.25, .25, .75), DIFF), //Glas

  //    GeoObject(16.5,Vec(27,16.5,47),       Vec(),Vec(1,1,1)*.999, SPEC),//Mirr
  //    GeoObject(16.5,Vec(73,16.5,78),       Vec(),Vec(1,1,1)*.999, REFR),//Glas
  GeoObject(600, Vec(50, 681.6 - .27, 81.6), Vec(12, 12, 12), Vec(), DIFF) //Lite
};
inline double clamp(double x)
{
    return x < 0 ? 0 : x > 1 ? 1 : x;
}
inline int toInt(double x)
{
    return int(pow(clamp(x), 1 / 2.2) * 255 + .5);
}

inline bool intersect(const Ray& r, double& t, int& id)
{
    double n = sizeof(spheres) / sizeof(GeoObject), d, inf = t = 1e20;
    for (int i = int(n); i--;)
    {
        if ((d = spheres[i].intersect(r)) && d < t)
        {
            t = d;
            id = i;
        }
    }
    return t < inf;
}
/* Vec radiance(const Ray& r, int depth, unsigned short* Xi)
{
    double t; // distance to intersection
    int id = 0; // id of intersected object
    if (!intersect(r, t, id))
        return Vec(); // if miss, return black

    const GeoObject& obj = spheres[id]; // the hit object
    Vec x = obj.x;
    Vec n = obj.n;
    Vec nl = n.dot(r.d) < 0.0f ? n : n * -1.0f;
    Vec f = obj.c;
    //    const GeoObject &obj = spheres[id];        // the hit object
    //    Vec x=r.o+r.d*t, n=(x-obj.p).norm(), nl=n.dot(r.d)<0?n:n*-1, f=obj.c;

    double p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y : f.z; // max refl
    if (++depth > 5)
        if (erand48(Xi) < p)
        {
            f = f * (1 / p);
        }
        else
        {
            return obj.e; //R.R.
        }
    if (obj.refl == DIFF)
    { // Ideal DIFFUSE reflection
        double r1 = 2 * PI * erand48(Xi), r2 = erand48(Xi), r2s = sqrt(r2);
        Vec w = nl, u = ((fabs(w.x) > .1 ? Vec(0, 1) : Vec(1)) % w).norm(), v = w % u;
        Vec d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm();
        return obj.e + f.mult(radiance(Ray(x, d), depth, Xi));
    }
    else if (obj.refl == SPEC) // Ideal SPECULAR reflection
        return obj.e + f.mult(radiance(Ray(x, r.d - n * 2 * n.dot(r.d)), depth, Xi));
    Ray reflRay(x, r.d - n * 2 * n.dot(r.d)); // Ideal dielectric REFRACTION
    bool into = n.dot(nl) > 0; // Ray from outside going in?
    double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = r.d.dot(nl), cos2t;
    if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0) // Total internal reflection
        return obj.e + f.mult(radiance(reflRay, depth, Xi));
    Vec tdir = (r.d * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).norm();
    double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : tdir.dot(n));
    double Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = .25 + .5 * Re, RP = Re / P, TP = Tr / (1 - P);
    return obj.e + f.mult(depth > 2 ? (erand48(Xi) < P ? // Russian roulette
                                       radiance(reflRay, depth, Xi) * RP :
                                       radiance(Ray(x, tdir), depth, Xi) * TP) :
                                      radiance(reflRay, depth, Xi) * Re + radiance(Ray(x, tdir), depth, Xi) * Tr);
}
int SmallPtMain()
{
    int w = 400, h = 400, samps = 1024; // # samples
    Ray cam(Vec(0, 0, 200), Vec(0, 0, -1).norm());
    Vec cx = Vec(w * .5135 / h), cy = (cx % cam.d).norm() * .5135, r, *c = new Vec[w * h];
    for (int y = 0; y < h; y++)
    { // Loop over image rows
        fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps * 4, 100. * y / (h - 1));
        for (unsigned short x = 0, Xi[3] = { 0, 0, static_cast<unsigned short>(y * y * y) }; x < w; x++) // Loop cols
            for (int sy = 0, i = (h - y - 1) * w + x; sy < 2; sy++) // 2x2 subpixel rows
                for (int sx = 0; sx < 2; sx++, r = Vec())
                { // 2x2 subpixel cols
                    for (int s = 0; s < samps; s++)
                    {
                        double r1 = 2 * erand48(Xi), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                        double r2 = 2 * erand48(Xi), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                        Vec d = cx * (((sx + .5 + dx) / 2 + x) / w - .5) +
                        cy * (((sy + .5 + dy) / 2 + y) / h - .5) + cam.d;
                        r = r + radiance(Ray(cam.o + d * 140, d.norm()), 0, Xi) * (1. / samps);
                    } // Camera rays are pushed ^^^^^ forward to start in interior
                    c[i] = c[i] + Vec(clamp(r.x), clamp(r.y), clamp(r.z)) * .25;
                }
    }
    FILE* f = fopen("image.ppm", "w"); // Write image to PPM file.
    fprintf(f, "P3\n%d %d\n%d\n", w, h, 255);
    for (int i = 0; i < w * h; i++)
        fprintf(f, "%d %d %d ", toInt(c[i].x), toInt(c[i].y), toInt(c[i].z));

    return 0;
};*/

DAVA_TESTCLASS (GeometryOctTreeTest)
{
    DAVA_TEST (GenerateRayTracedImage)
    {
        uint32 segmentsCount = 1;

        // 16.5,Vec(27,16.5,47)
        PolygonGroup* left = GeometryGenerator::GenerateBox(AABBox3(Vector3(-400.0f, 0.0f, 0.0f), 500.0f),
                                                            { { FastName("segments.x"), (float)segmentsCount },
                                                              { FastName("segments.y"), (float)segmentsCount },
                                                              { FastName("segments.z"), (float)segmentsCount } });
        left->GenerateGeometryOctTree();
        spheres[0].pgroup = left;

        PolygonGroup* right = GeometryGenerator::GenerateBox(AABBox3(Vector3(400.0f, 0.0f, 0.0f), 500.0f),
                                                             { { FastName("segments.x"), (float)segmentsCount },
                                                               { FastName("segments.y"), (float)segmentsCount },
                                                               { FastName("segments.z"), (float)segmentsCount } });
        right->GenerateGeometryOctTree();
        spheres[1].pgroup = right;

        PolygonGroup* back = GeometryGenerator::GenerateBox(AABBox3(Vector3(0.0f, 0.0f, -400.0f), 500.0f),
                                                            { { FastName("segments.x"), (float)segmentsCount },
                                                              { FastName("segments.y"), (float)segmentsCount },
                                                              { FastName("segments.z"), (float)segmentsCount } });
        back->GenerateGeometryOctTree();
        spheres[2].pgroup = back;

        PolygonGroup* front = GeometryGenerator::GenerateBox(AABBox3(Vector3(0.0f, 0.0f, 400.0f), Vector3(1.0f, 1.0f, 1.0f)),
                                                             { { FastName("segments.x"), (float)segmentsCount },
                                                               { FastName("segments.y"), (float)segmentsCount },
                                                               { FastName("segments.z"), (float)segmentsCount } });
        front->GenerateGeometryOctTree();
        spheres[3].pgroup = front;

        PolygonGroup* top = GeometryGenerator::GenerateBox(AABBox3(Vector3(0.0f, 400.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)),
                                                           { { FastName("segments.x"), (float)segmentsCount },
                                                             { FastName("segments.y"), (float)segmentsCount },
                                                             { FastName("segments.z"), (float)segmentsCount } });

        top->GenerateGeometryOctTree();
        spheres[4].pgroup = top;

        PolygonGroup* bottom = GeometryGenerator::GenerateBox(AABBox3(Vector3(0.0f, -400.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)),
                                                              { { FastName("segments.x"), (float)segmentsCount },
                                                                { FastName("segments.y"), (float)segmentsCount },
                                                                { FastName("segments.z"), (float)segmentsCount } });
        bottom->GenerateGeometryOctTree();
        spheres[5].pgroup = bottom;

        PolygonGroup* box1 = GeometryGenerator::GenerateBox(AABBox3(Vector3(27, 16.5, 47), 16.5),
                                                            { { FastName("segments.x"), (float)segmentsCount },
                                                              { FastName("segments.y"), (float)segmentsCount },
                                                              { FastName("segments.z"), (float)segmentsCount } });
        box1->GenerateGeometryOctTree();
        spheres[6].pgroup = box1;

        PolygonGroup* light = GeometryGenerator::GenerateBox(AABBox3(Vector3(27, 87, 87), 16.5),
                                                             { { FastName("segments.x"), (float)segmentsCount },
                                                               { FastName("segments.y"), (float)segmentsCount },
                                                               { FastName("segments.z"), (float)segmentsCount } });
        light->GenerateGeometryOctTree();
        spheres[8].pgroup = light;

        // SmallPtMain();
    };

#pragma warning(disable : 4723)

    DAVA_TEST (BasicOctTreeRayTest)
    {
        for (uint32 t = 0; t < 5; ++t)
        {
            const uint32 segments[5] = { 1, 2, 3, 30, 50 }; // Check 100, 100 is failing

            Map<FastName, float32> options = {
                { FastName("segments.x"), (float)segments[t] },
                { FastName("segments.y"), (float)segments[t] },
                { FastName("segments.z"), (float)segments[t] }
            };

            PolygonGroup* geometry = GeometryGenerator::GenerateBox(AABBox3(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)), options);

            //            for (uint32 k = 0; k < geometry->GetVertexCount(); ++k)
            //            {
            //                Vector3 coord;
            //                geometry->GetCoord(k, coord);
            //                Logger::FrameworkDebug("%f %f %f", coord.x, coord.y, coord.z);
            //            }
            //
            //            for (uint32 k = 0; k < geometry->GetIndexCount(); ++k)
            //            {
            //                int32 index;
            //                geometry->GetIndex(k, index);
            //                Logger::FrameworkDebug("%d", index);
            //            }
            geometry->GenerateGeometryOctTree();
            GeometryOctTree* geoOctTree = geometry->GetGeometryOctTree();

            Ray3Optimized rays[] =
            {
              Ray3Optimized(Vector3(0.49999f, 0.49999f, -1.0f), Vector3(0.0f, 0.0f, 2.0f)),
              Ray3Optimized(Vector3(0.5f, 0.5f, -1.0f), Vector3(0.0f, 0.0f, 2.0f)),
              Ray3Optimized(Vector3(0.50001f, 0.50001f, -1.0f), Vector3(0.0f, 0.0f, 2.0f)),
              Ray3Optimized(Vector3(0.49999f, 0.50001f, -1.0f), Vector3(0.0f, 0.0f, 2.0f)),
              Ray3Optimized(Vector3(0.50001f, 0.49999f, -1.0f), Vector3(0.0f, 0.0f, 2.0f)),
              Ray3Optimized(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 2.0f)),
              Ray3Optimized(Vector3(1.0f, 1.0f, -1.0f), Vector3(0.0f, 0.0f, 2.0f)),

              Ray3Optimized(Vector3(0.49999f, 0.49999f, 2.0f), Vector3(0.0f, 0.0f, -2.0f)),
              Ray3Optimized(Vector3(0.5f, 0.5f, 2.0f), Vector3(0.0f, 0.0f, -2.0f)),
              Ray3Optimized(Vector3(0.50001f, 0.50001f, 2.0f), Vector3(0.0f, 0.0f, -2.0f)),
              Ray3Optimized(Vector3(0.49999f, 0.50001f, 2.0f), Vector3(0.0f, 0.0f, -2.0f)),
              Ray3Optimized(Vector3(0.50001f, 0.49999f, 2.0f), Vector3(0.0f, 0.0f, -2.0f)),
              Ray3Optimized(Vector3(0.0f, 0.0f, 2.0f), Vector3(0.0f, 0.0f, -2.0f)),
              Ray3Optimized(Vector3(1.0f, 1.0f, 2.0f), Vector3(0.0f, 0.0f, -2.0f)),
            };

            Vector<Ray3Optimized> finalRays;
            for (auto ray : rays)
            {
                finalRays.push_back(ray);
                finalRays.push_back(Ray3Optimized(ray.origin.xzy(), ray.direction.xzy()));
                finalRays.push_back(Ray3Optimized(ray.origin.zxy(), ray.direction.zxy()));
            }

            for (auto ray : finalRays)
            {
                uint32 triIndex;
                float32 resultT;
                TEST_VERIFY(geoOctTree->IntersectionWithRay(ray, resultT, triIndex) == true);
                TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

                TEST_VERIFY(geoOctTree->IntersectionWithRay2(ray, resultT, triIndex) == true);
                TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));
            }

            uint32 triIndex;
            float32 resultT;

            Ray3Optimized rayf(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 2.0f));
            TEST_VERIFY(geoOctTree->IntersectionWithRay(rayf, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            TEST_VERIFY(geoOctTree->IntersectionWithRay2(rayf, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            Ray3Optimized ray(Vector3(0.49999f, 0.49999f, -1.0f), Vector3(0.0f, 0.0f, 2.0f));
            TEST_VERIFY(geoOctTree->IntersectionWithRay(ray, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            TEST_VERIFY(geoOctTree->IntersectionWithRay2(ray, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            Ray3Optimized ray2(Vector3(0.5f, 0.5f, -1.0f), Vector3(0.0f, 0.0f, 2.0f));
            TEST_VERIFY(geoOctTree->IntersectionWithRay(ray2, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            TEST_VERIFY(geoOctTree->IntersectionWithRay2(ray2, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            Ray3Optimized ray3(Vector3(0.51f, 0.51f, -1.0f), Vector3(0.0f, 0.0f, 2.0f));
            TEST_VERIFY(geoOctTree->IntersectionWithRay(ray3, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            TEST_VERIFY(geoOctTree->IntersectionWithRay2(ray3, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            Ray3Optimized ray4(Vector3(0.49f, 0.51f, -1.0f), Vector3(0.0f, 0.0f, 2.0f));
            TEST_VERIFY(geoOctTree->IntersectionWithRay(ray4, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            TEST_VERIFY(geoOctTree->IntersectionWithRay2(ray4, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            Ray3Optimized ray5(Vector3(0.51f, 0.49f, -1.0f), Vector3(0.0f, 0.0f, 2.0f));
            TEST_VERIFY(geoOctTree->IntersectionWithRay(ray5, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            TEST_VERIFY(geoOctTree->IntersectionWithRay2(ray5, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            Ray3Optimized ray6(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 2.0f));
            TEST_VERIFY(geoOctTree->IntersectionWithRay(ray6, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            TEST_VERIFY(geoOctTree->IntersectionWithRay2(ray6, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            Ray3Optimized ray7(Vector3(1.0f, 1.0f, -1.0f), Vector3(0.0f, 0.0f, 2.0f));
            TEST_VERIFY(geoOctTree->IntersectionWithRay(ray7, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            TEST_VERIFY(geoOctTree->IntersectionWithRay2(ray7, resultT, triIndex) == true);
            TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));

            //Ray3Optimized ray2(Vector3(0.5f, 0.5f, -1.0f), Vector3(0.0f, 0.0f, 2.0f));
            //TEST_VERIFY(geoOctTree->IntersectionWithRay(ray2, resultT, triIndex, 0.0f, 1.000f) == true);
            //TEST_VERIFY(FLOAT_EQUAL(resultT, 0.5f));
            SafeRelease(geometry);
        }
    }
};
