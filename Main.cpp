#include "Mesh.h"
#include "ThermalState.h"
#include <filesystem>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
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
}

int main(int argc, char const *argv[])
{
    if (argc == 3 && std::string(argv[1]) == "--random") {
        size_t parsedCharacters = 0;
        const int pointCount = std::stoi(argv[2], &parsedCharacters);
        randomizeTriangulation(pointCount);
        return 0;

    }


    // tes trucs de con pour la chaleur là
    Mesh mesh = Mesh::ReadOFF(std::string(argv[1]));
    ThermalState thermalState(std::vector<float>(mesh.vertices.size(), 0));
    thermalState.heatValues[0] = 100;

    const int totalSteps = 10000;
    std::cout << "Progression : 0%" << std::flush;
    for (int i = 1; i <= totalSteps; ++i) {
        thermalState.step(mesh, 0.00000010f);

        if (i % 100 == 0) {
            const int percent = (i * 100) / totalSteps;
            std::cout << "\rProgression : " << percent << "%" << std::flush;
        }

        if (i % 2000 == 0) thermalState.WriteCOFF(mesh, std::string(argv[1]));
    }

    std::cout << "\rProgression : 100%" << std::endl;
    return 0;
}
