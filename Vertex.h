#pragma once
#include <iostream>
#include "vec3.h"
#include "Face.h"

class Vertex{
    public : 
        vec3 coordinates;
        int faceRef;
        
        Vertex(const vec3& p);
        Vertex(const float& x, const float& y, const float& z);
        
        //TODO default constructor
        //TODO copy constructor

        void SetFaceRef(const int& );

        std::string str()const ;

                
};