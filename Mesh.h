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
        std::vector<vec3> laplacien;
        std::vector<float> curvature;
        int borderLink;//point à l'infini

        Mesh(const std::vector<Vertex>& vertices,const std::vector<Face>& faces, int borderLink);

        //TODO default constructor
        //TODO copy constructor

        //TODO function that returns if a mesh is coherent (each pair of triangle is connected in both ways)

        static Mesh LoadTetrahedron();
        static void WriteOFF(const Mesh& mesh, const std::string& filePath);
        static void WriteOFFWithCurvature(const Mesh& mesh, const std::string& filePath);
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

        //Delaunay
        //outils
        bool isInfiniteFace(int faceId) const;
        bool isInCircumcircle(int faceId, const vec3& p) const;
        //prédicat : l'arête commune à face1 et face2 est-elle localement de Delaunay ?
        bool isLocallyDelaunay(int face1, int face2) const;
        //algorithme de Lawson sur une liste d'arêtes (couples de faces adjacentes)
        int lawson(std::vector<std::pair<int,int>> edges);
        //triangulation naïve -> triangulation de Delaunay (Lawson sur toutes les arêtes)
        int naiveToDelaunay();
        //insertion d'un point dans une triangulation de Delaunay (Lawson sur quelques arêtes seulement)
        int insertPointDelaunay(const vec3& p);

        void computeCurvature();
        vec3 curvatureToColor(float c)const;
};

