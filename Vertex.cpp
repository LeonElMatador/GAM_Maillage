#include "Vertex.h"

    
Vertex::Vertex(const vec3& p) : coordinates(p){}
Vertex::Vertex(const float& x, const float& y, const float& z) : coordinates(x,y,z) {}
void Vertex::SetFaceRef(const int& faceRef){
    this->faceRef = faceRef;
}

std::string Vertex::str()const {
    return coordinates.str() ;
}
