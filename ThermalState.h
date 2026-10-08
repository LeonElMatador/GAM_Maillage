#pragma once
#include <iostream>
#include <vector>
#include "Face.h"
#include "Vertex.h"
#include "Mesh.h"
#include <cassert>
#include "vec3.h"

class ThermalState{
    public : 
        std::vector<double> heatValues;
        std::vector<double> laplacian;
        int generation;

        ThermalState(std::vector<double> heatValues) : heatValues(std::move(heatValues)), laplacian(this->heatValues.size(), 0), generation(0){};
        
        void computeLaplacian(const Mesh& m);
        void step(const Mesh& m, double deltaTime);
        
        vec3 getColor(int i);

        void WriteCOFF(const Mesh& mesh, const std::string& filePath);
};