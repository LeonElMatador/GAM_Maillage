#include "Mesh.h"
#include "ThermalState.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {

void addOnePointOutsideTriangle() {
    const std::vector<vec3> trianglePoints{
        vec3(-1.0f, -1.0f, 0.0f),
        vec3(1.0f, -1.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f)
    };

    Mesh mesh = Mesh::Triangulize(trianglePoints);
    mesh.addVerticesToTriangulation(vec3(0.0f, -2.0f, 0.0f));

    std::filesystem::create_directories("OFF");
    Mesh::WriteOFF(mesh, "OFF/triangle_plus_outside_point.off");
}

void randomizeTriangulation(int pointCount) {
    std::vector<vec3> points{
        vec3(-1.0f, -1.0f, 0.0f),
        vec3(1.0f, -1.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f)
    };

    std::mt19937 generator(20261008u);
    std::uniform_real_distribution<float> coordinate(-10.0f, 10.0f);
    for (int i = 0; i < pointCount; ++i) {
        points.emplace_back(coordinate(generator), coordinate(generator), 0.0f);
    }

    Mesh mesh = Mesh::Triangulize(points);
    const std::string fileName = "random_" + std::to_string(pointCount) + "_points.off";
    std::filesystem::create_directories("OFF");
    Mesh::WriteOFF(mesh, (std::filesystem::path("OFF") / fileName).string());
}

void computeCurvature(const std::string& inputPath) {
    Mesh mesh = Mesh::ReadOFF(inputPath);
    mesh.computeCurvature();
    Mesh::WriteOFFWithCurvature(mesh, "curvature.off");
}

void runHeatSimulation(const std::string& inputPath, int totalSteps) {
    Mesh mesh = Mesh::ReadOFF(inputPath);
    ThermalState thermalState(std::vector<double>(mesh.vertices.size(), 0));
    thermalState.heatValues[0] = 100;

    std::cout << "Progression : 0%" << std::flush;
    for (int i = 1; i <= totalSteps; ++i) {
        thermalState.step(mesh, 0.0000005f);
        thermalState.heatValues[0] = 100;

        if (i % 100 == 0 || i == totalSteps) {
            const int percent = (i* 100) / totalSteps;
            std::cout << "\rProgression : " << percent << "%" << std::flush;
        }

        if (i % 2000 == 0) thermalState.WriteCOFF(mesh, inputPath);
    }

    if (totalSteps == 0 || totalSteps % 2000 != 0) {
        thermalState.WriteCOFF(mesh, inputPath);
    }
    std::cout << "\rProgression : 100%" << std::endl;
}
}

int main(int argc, char const *argv[])
{
    if (argc == 2 && std::string(argv[1]) == "--help") {
        std::cout << "Usage:\n"
                  << "  ./tp1 --random <nombre_de_points>\n"
                  << "  ./tp1 --curvature <fichier.off>\n"
                  << "  ./tp1 --heat <fichier.off> <nombre_d_iterations>\n"
                  << "  ./tp1 --test-add-outside\n";
        return 0;
    }

    if (argc == 2 && std::string(argv[1]) == "--test-add-outside") {    // ajouter un point en dehors d'un maillage prédéfini
        addOnePointOutsideTriangle();
        return 0;
    }

    if (argc == 3 && std::string(argv[1]) == "--random") {  // triangulisation aléatoire avec n points
        const int pointCount = std::atoi(argv[2]);
        std::cerr << pointCount << std::endl;
        randomizeTriangulation(pointCount);
        return 0;
    }

    if (argc == 3 && std::string(argv[1]) == "--curvature") {   // appel fonction de courbure
        computeCurvature(argv[2]);
        return 0;
    }

    if (argc == 4 && std::string(argv[1]) == "--heat") {    // chaleur
        const int totalSteps = std::atoi(argv[3]);
        runHeatSimulation(argv[2], totalSteps);
        return 0;
    }

    std::cerr << "Arguments invalides. Utilisez './tp1 --help' pour voir les options.\n";
    return 1;
}