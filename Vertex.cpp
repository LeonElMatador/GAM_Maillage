#include "Vertex.h"

    
Vertex::Vertex(const vec3& p) : coord(p){}
Vertex::Vertex(const float& x, const float& y, const float& z) : coord(x,y,z) {}
void Vertex::SetFaceRef(const int& faceRef){
    this->faceRef = faceRef;
}

std::string Vertex::str()const {
    return coord.str() ;
}
