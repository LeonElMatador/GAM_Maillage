#include "Face.h"

Face::Face(const std::vector<int>& vertices) : vertices(std::move(vertices)){}

Face::Face(const std::vector<int>& vertices, const std::vector<int>& neighbours) : vertices(std::move(vertices)), neighbours(std::move(neighbours)){}

void Face::SetNeighbours(const std::vector<int>& neighbours){
    this->neighbours = std::move(neighbours);
};

std::string Face::str() const {
    std::string s = std::to_string(vertices.size());
    
    for(size_t i = 0; i<vertices.size(); i++){
        s += " " + std::to_string(vertices[i]);
    }
    return s;
}

