#pragma once
#include <iostream>
#include <vector>
#include "Face.h"
#include "Vertex.h"
#include <fstream>
#include <limits>
#include <map>
#include <array>
#include <cassert>
#include "vec3.h"

class Mesh{
    public : 
        std::vector<Vertex> vertices;
        std::vector<Face> faces; 
        int borderLink;//point à l'infini

        Mesh(const std::vector<Vertex>& vertices,const std::vector<Face>& faces, int borderLink);

        //TODO default constructor
        //TODO copy constructor

        //TODO function that returns if a mesh is coherent (each pair of triangle is connected in both ways)

        static Mesh LoadTetrahedron();
        static void WriteOFF(const Mesh& mesh, const std::string& filePath);
        static Mesh ReadOFF(const std::string& filePath);
        static Mesh Triangulize(const std::vector<vec3>& points);

        
        std::vector<float> laplacian(const std::vector<float>& values) const ;
        std::vector<int> neighbours(const int& vertex) const;

        void faceSplit(int faceId, vec3 newSommet);
        void edgeSplit(int face1, int face2, vec3 newSommet);
        void edgeFlip(int face1, int face2);

        void deleteFace(int id);

        void addVerticesToTriangulation(const vec3& p);

        int isInside(const vec3& p, int f, int& edgeFace)const;

        std::vector<int> getBorder()const;
};

