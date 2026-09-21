#pragma once
#include <iostream>
#include <vector>
#include <cmath>

class vec3{
    public : 
        float x;
        float y; 
        float z;

        vec3(const float& x, const float& y, const float& z);

        vec3();

        vec3(const vec3& p);

        /*friend std::string& operator+=(std::string& text, const Point& p);

        operator std::string() const{ //probleme si je fais s = point + "\n" ca bug pcq pas d'opérateur + donc c'est mieux un to_string
            return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z);
        }*/

        std::string str() const;

        float laplacian(const float& value) const;

        vec3  operator +(const vec3& v)const{
            return vec3(x+v.x, y+v.y, z+v.z);
        };

        vec3  operator *(float a)const{
            return vec3(a*x, a*y, a*z);
        };
        
        vec3  operator -(const vec3& v)const{
            return vec3(x-v.x, y-v.y, z-v.z);
        };
        
        static vec3 Cross(const vec3& v1, const vec3& v2);

        static float Dot(const vec3& v1, const vec3& v2);

        float norm()const;
        float norm2() const;

        static vec3 Lerp(const vec3& v1, const vec3& v2, float t);

};