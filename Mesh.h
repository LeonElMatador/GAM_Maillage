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

class Mesh{
    public : 
        std::vector<Vertex> vertices;
        std::vector<Face> faces; 

        Mesh(std::vector<Vertex>& vertices, std::vector<Face>& faces);

        //TODO default constructor
        //TODO copy constructor

        //TODO function that returns if a mesh is coherent (each pair of triangle is connected in both ways)

        static Mesh LoadTetrahedron();
        static void WriteOFF(const Mesh& mesh, const std::string& filePath);
        static Mesh ReadOFF(const std::string& filePath);

        
        std::vector<float> laplacian(const std::vector<float>& values) const ;
        std::vector<int> neighbours(const int& vertex) const;

        void deleteFace(int id);
};

