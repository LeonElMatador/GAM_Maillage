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

// vérification indépendante (en double, sur tous les sommets) : nombre de couples
// (face finie, sommet) où le sommet est strictement dans le cercle circonscrit de la face
int countDelaunayViolations(const Mesh& mesh) {
    int violations = 0;
    for (size_t f = 0; f < mesh.faces.size(); ++f) {
        if (mesh.isInfiniteFace(f)) continue;
        const vec3& a = mesh.vertices[mesh.faces[f][0]].coord;
        const vec3& b = mesh.vertices[mesh.faces[f][1]].coord;
        const vec3& c = mesh.vertices[mesh.faces[f][2]].coord;
        for (size_t k = 0; k < mesh.vertices.size(); ++k) {
            const int v = (int)k;
            if (v == mesh.borderLink || v == mesh.faces[f][0] || v == mesh.faces[f][1] || v == mesh.faces[f][2]) continue;
            const vec3& d = mesh.vertices[k].coord;
            const double ax = a.x - d.x, ay = a.y - d.y;
            const double bx = b.x - d.x, by = b.y - d.y;
            const double cx = c.x - d.x, cy = c.y - d.y;
            const double det = (ax * ax + ay * ay) * (bx * cy - cx * by)
                             - (bx * bx + by * by) * (ax * cy - cx * ay)
                             + (cx * cx + cy * cy) * (ax * by - bx * ay);
            if (det > 1e-6) violations++;
        }
    }
    return violations;
}

// nombre de faces finies qui ne sont pas orientées dans le sens trigo
int countInvertedFaces(const Mesh& mesh) {
    int inverted = 0;
    for (size_t f = 0; f < mesh.faces.size(); ++f) {
        if (mesh.isInfiniteFace(f)) continue;
        const vec3& a = mesh.vertices[mesh.faces[f][0]].coord;
        const vec3& b = mesh.vertices[mesh.faces[f][1]].coord;
        const vec3& c = mesh.vertices[mesh.faces[f][2]].coord;
        if (vec3::IsTrigoOriented(a, b, c) != 1) inverted++;
    }
    return inverted;
}

bool testDelaunay(int pointCount) {
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
    std::filesystem::create_directories("OFF");
    bool success = true;

    //1. triangulation naïve transformée en triangulation de Delaunay
    Mesh naive = Mesh::Triangulize(points);
    Mesh::WriteOFF(naive, "OFF/delaunay_test_naive.off");
    std::cout << "[naiveToDelaunay] " << pointCount + 3 << " points\n"
              << "  avant : " << countDelaunayViolations(naive) << " violations, "
              << countInvertedFaces(naive) << " faces retournees\n";

    const int flips = naive.naiveToDelaunay();
    const int violations = countDelaunayViolations(naive);
    const int inverted = countInvertedFaces(naive);
    const int secondFlips = naive.naiveToDelaunay(); //déjà Delaunay : doit valoir 0
    std::cout << "  apres : " << flips << " flips, " << violations << " violations, "
              << inverted << " faces retournees, 2e appel : " << secondFlips << " flips\n";
    Mesh::WriteOFF(naive, "OFF/delaunay_test_lawson.off");

    const bool naiveOk = violations == 0 && inverted == 0 && secondFlips == 0;
    std::cout << "  -> " << (naiveOk ? "OK" : "ECHEC") << "\n\n";
    success = success && naiveOk;

    //2. insertions successives dans une triangulation de Delaunay
    Mesh incremental = Mesh::Triangulize({points[0], points[1], points[2]});
    int totalFlips = 0;
    int firstFailure = -1;
    for (size_t i = 3; i < points.size(); ++i) {
        totalFlips += incremental.insertPointDelaunay(points[i]);
        //on vérifie après chaque insertion pour savoir laquelle pose problème
        if (firstFailure == -1 && (countDelaunayViolations(incremental) != 0 || countInvertedFaces(incremental) != 0)) {
            firstFailure = (int)i;
        }
    }
    Mesh::WriteOFF(incremental, "OFF/delaunay_test_incremental.off");

    std::cout << "[insertPointDelaunay] " << pointCount << " insertions\n"
              << "  " << totalFlips << " flips, " << countDelaunayViolations(incremental) << " violations, "
              << countInvertedFaces(incremental) << " faces retournees\n";
    if (firstFailure != -1) {
        std::cout << "  premiere insertion fautive : point " << firstFailure << "\n";
    }

    const bool incrementalOk = firstFailure == -1;
    std::cout << "  -> " << (incrementalOk ? "OK" : "ECHEC") << "\n";
    return success && incrementalOk;
}

void computeCurvature(const std::string& inputPath) {
    Mesh mesh = Mesh::ReadOFF(inputPath);
    mesh.computeCurvature();
    Mesh::WriteOFFWithCurvature(mesh, "./OFF/curvature.off");
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
                  << "  ./tp1 --test-add-outside\n"
                  << "  ./tp1 --test-delaunay [nombre_de_points]\n";
        return 0;
    }

    if (argc == 2 && std::string(argv[1]) == "--test-add-outside") {    // ajouter un point en dehors d'un maillage prédéfini
        addOnePointOutsideTriangle();
        return 0;
    }

    if ((argc == 2 || argc == 3) && std::string(argv[1]) == "--test-delaunay") {  // test de naiveToDelaunay et insertPointDelaunay
        const int pointCount = argc == 3 ? std::atoi(argv[2]) : 50;
        return testDelaunay(pointCount) ? 0 : 1;
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