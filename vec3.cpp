#include "vec3.h"

vec3::vec3(const float& x, const float& y, const float& z) : x(x), y(y), z(z) {}

vec3::vec3() :  x(0), y(0), z(0) {} 

vec3::vec3(const vec3& p) : x(p.x), y(p.y), z(p.z) {}

/*std::string& operator+=(std::string& s, const vec3& p){ //TODO remove this exemple
    s+= std::to_string(p.x) + " " + std::to_string(p.y) + " " + std::to_string(p.z);
    return s;
}*/

std::string vec3::str() const {
    return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z);
}

vec3 vec3::Cross(const vec3& v1, const vec3& v2){
    float x = v1.y * v2.z - v2.y * v1.z ;
    float y = v1.z*v2.x - v1.x * v2.z;
    float z = v1.x*v2.y - v1.y*v2.x;
    return vec3(x,y,z);
}

float vec3::Dot(const vec3& v1, const vec3& v2){
    return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

float vec3::norm2()const{
    return x*x + y*y + z*z;
}

float vec3::norm()const{
    return std::sqrt(x*x + y*y + z*z);
}

vec3 vec3::Lerp(const vec3& v1, const vec3& v2, float t){
    return v1*(1-t) + v2*t;
}

bool vec3::IsTrigoOriented(const vec3& v1, const vec3& v2, const vec3& v3){
    return Cross(v2-v1, v3-v1).z > 0;
}

int vec3::IsInside(const vec3& p, const vec3& a, const vec3& b, const vec3& c){
    float airPAB = Cross(a-p, b-p).z;
    float airPBC = Cross(b-p, c-p).z;
    float airPCA = Cross(c-p, a-p).z;

    if(airPAB<0||airPBC<0||airPCA<0){
        return -1;
    }
    else if(airPAB==0||airPBC==0||airPCA==0){
        return 0;
    }
    else return 1;
}