#pragma once
#include <iostream>
#include <vector>

class Face{
    public : 
        std::vector<int> vertices;
        std::vector<int> neighbours;  

        Face(const std::vector<int>& vertices);
        Face(const std::vector<int>& vertices, const std::vector<int>& neighbours);

        //TODO default constructor
        //TODO copy constructor

        void SetNeighbours(const std::vector<int>& neighbours);

        std::string str() const;
};