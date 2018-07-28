#include <iostream>
#include <fstream>
#include <climits>
#include <vector>
#include <memory>
#include <utility>
#include <string>
#include <cstdlib>
#include <cmath>

#include "vector3.hpp"

using namespace std;

using Vector3f = Vector3<float>;

using Point = Vector3f;
using Color = Vector3f;

struct Ray
{
    Point org;    // origin
    Vector3f dir; // direction
};

struct Object;
struct Hit
{
    Point pos;     // position
    Vector3f norm; // normal
    const Object *obj;
};

struct Object
{
    Point pos; // position
    Color surface_color;
    Color emission_color;

    Object(Point pos, Color surface_color, Color emission_color = Vector3f(0))
        : pos(pos), surface_color(surface_color), emission_color(emission_color) {}

    virtual unique_ptr<Hit> intersect(const Ray &ray) const = 0;
};

struct Sphere : Object
{
    float radius;

    Sphere(Point pos, float radius, Color surface_color, Color emission_color = Vector3f(0))
        : Object(pos, surface_color, emission_color), radius(radius) {}

    unique_ptr<Hit> intersect(const Ray &ray) const
    {
        /*
        -----------------------

                    * pos (sphere)
                  / |
             l  /   | l_sin 
              /     |
        org *-------|--------> ray
              l_cos 

        -----------------------
        */

        auto hit = make_unique<Hit>();
        auto l = pos - ray.org;

        float l_cos = l.dot(ray.dir);

        if (l_cos <= 0)
        {
            return nullptr;
        }

        float l_sin_sqr = l.sqr_length() - l_cos * l_cos;

        if (l_sin_sqr > radius * radius)
        {
            return nullptr;
        }

        float distance = l_cos - sqrt(radius * radius - l_sin_sqr);

        hit->pos = ray.org + ray.dir * distance;
        hit->norm = (hit->pos - pos).normalized();
        hit->obj = this;

        return hit;
    }
};

struct Camera
{
    float factor;
    Point pos;

    Camera(float fov)
    {
        /*
                cvs
        
                _
                /|
               / | 
              /  | 
             / a 
       cam  -----|
             \ a 
              \  |
               \ |
               _\|

        fov = 2a

        */
        factor = ::tan(M_PI * 0.5 * fov / 180);
    }

    Camera(Point pos, float fov)
        : pos(pos)
    {
        factor = ::tan(M_PI * 0.5 * fov / 180);
    }

    Ray primary_ray(float x, float y) const
    {
        x *= factor;
        y *= factor;
        return {pos, Vector3f(x, y, -1).normalized()};
    }
};

struct Scene
{
    vector<unique_ptr<Object>> objs;
};

struct Canvas
{
    int width;
    int height;
    vector<vector<Color>> pixel;

    Canvas(int width, int height)
        : width(width),
          height(height)
    {
        pixel = vector<vector<Color>>(width);
        for (auto &v : pixel)
        {
            v.resize(height);
        }
    }

    void draw(function<Color(float, float)> render)
    {
        float _aspect_ratio = aspect_ratio();
        float inv_width = 1.0 / width;
        float inv_height = 1.0 / height;
        for (int i = 0; i < width; ++i)
        {
            for (int j = 0; j < height; ++j)
            {
                float x = ((i * inv_width) * 2 - 1) * _aspect_ratio;
                float y = 1 - 2 * (j * inv_height);
                pixel[i][j] = render(x, y);
            }
        }
    };

    void save(string filename) const
    {
        ofstream ofs(filename, ios::binary);
        ofs << "P6" << endl
            << width << " " << height << endl
            << "255" << endl;
        for (int j = 0; j < height; ++j)
        {
            for (int i = 0; i < width; ++i)
            {
                ofs << (unsigned char)(::min(1.0f, pixel[i][j].x) * 255)
                    << (unsigned char)(::min(1.0f, pixel[i][j].y) * 255)
                    << (unsigned char)(::min(1.0f, pixel[i][j].z) * 255);
            }
        }
        ofs.close();
    }

    float aspect_ratio() const
    {
        return width / float(height);
    }
};

Color trace(const Ray &ray, const Scene &scene, int max_depth)
{
    float min_distance = numeric_limits<float>::infinity();

    unique_ptr<Hit> first_hit = nullptr;

    for (const auto &obj : scene.objs)
    {
        auto hit = obj->intersect(ray);
        if (hit != nullptr)
        {
            float distance = ::distance(ray.org, hit->pos);
            if (distance < min_distance)
            {
                first_hit = move(hit);
                min_distance = distance;
            }
        }
    }

    if (first_hit != nullptr)
    {
        if (ray.dir.dot(first_hit->norm) > 0)
        {
            first_hit->norm = -first_hit->norm;
        }

        Color surface_color;

        if (max_depth > 0)
        {
            float facing_ratio = ray.dir.dot(first_hit->norm);

            /*
                _
                /|
           b   /
              /   
             /
            - - - - -> n
             \   |
              \  |
           a   \ 
               _\|
                
            (a + b) / 2 = (a . n / |n|) * (n / |n|)
            */

            Ray reflect_ray = {first_hit->pos, ray.dir - first_hit->norm * 2 * ray.dir.dot(first_hit->norm)};

            Color reflection = trace(reflect_ray, scene, max_depth - 1);

            // I use this weird configuration to achieve the effect.
            surface_color = reflection * first_hit->obj->surface_color + Vector3f(0.3);
        }

        return surface_color + first_hit->obj->emission_color;
    }

    return Color(0);
}

auto create_renderer(const Scene &scene, const Camera &camera, int max_depth)
{
    return [&](float x, float y) {
        auto primary_ray = camera.primary_ray(x, y);
        return trace(primary_ray, scene, max_depth);
    };
}

int main()
{
    Canvas canvas(4096, 2160);

    auto scene = Scene();

    scene.objs.push_back(make_unique<Sphere>(Point(0.0, -10004, -20), 10000, Color(0.20, 0.20, 0.20)));
    scene.objs.push_back(make_unique<Sphere>(Point(0.0, 0, -20), 4, Color(1.00, 0.32, 0.36)));
    scene.objs.push_back(make_unique<Sphere>(Point(5.0, -1, -15), 2, Color(0.90, 0.76, 0.46)));
    scene.objs.push_back(make_unique<Sphere>(Point(5.0, 0, -25), 3, Color(0.65, 0.77, 0.97)));
    scene.objs.push_back(make_unique<Sphere>(Point(-5.5, 0, -15), 3, Color(0.90, 0.90, 0.90)));

    scene.objs.push_back(make_unique<Sphere>(Point(0.0, 20, -30), 3, Color(0, 0, 0), Color(3)));

    auto camera = Camera(30);
    auto renderer = create_renderer(scene, camera, 5);
    canvas.draw(renderer);
    canvas.save("./fig/gift.ppm");

    return 0;
}