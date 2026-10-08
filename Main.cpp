#include "Mesh.h"
#include "ThermalState.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {
Mesh makeTestTetrahedron() {
    std::vector<Vertex> vertices{
        Vertex(0.0f, 1.0f, 0.0f),
        Vertex(0.5f, 0.0f, 0.5f),
        Vertex(-0.5f, 0.0f, 0.5f),
        Vertex(0.5f, 0.0f, -0.5f)
    };
    std::vector<Face> faces{
        Face({1, 2, 3}, {1, 2, 3}),
        Face({2, 0, 3}, {2, 0, 3}),
        Face({3, 0, 1}, {3, 0, 1}),
        Face({1, 0, 2}, {1, 0, 2})
    };

    vertices[0].SetFaceRef(1);
    vertices[1].SetFaceRef(0);
    vertices[2].SetFaceRef(0);
    vertices[3].SetFaceRef(0);
    return Mesh(vertices, faces, -1);
}

Mesh makeSquareMesh() {
    std::vector<Vertex> vertices{
        Vertex(0.0f, 0.0f, 1.0f),
        Vertex(-2.0f, -2.0f, 0.0f),
        Vertex(2.0f, -2.0f, 0.0f),
        Vertex(2.0f, 2.0f, 0.0f),
        Vertex(-2.0f, 2.0f, 0.0f)
    };
    std::vector<Face> faces{
        Face({1, 2, 4}, {1, 5, 2}),
        Face({2, 3, 4}, {4, 0, 3}),
        Face({0, 2, 1}, {0, 5, 3}),
        Face({0, 3, 2}, {1, 2, 4}),
        Face({0, 4, 3}, {1, 3, 5}),
        Face({0, 1, 4}, {0, 4, 2})
    };

    vertices[0].SetFaceRef(2);
    vertices[1].SetFaceRef(0);
    vertices[2].SetFaceRef(0);
    vertices[3].SetFaceRef(1);
    vertices[4].SetFaceRef(0);
    return Mesh(vertices, faces, 0);
}

bool hasEdge(const Face& face, int first, int second) {
    return std::find(face.vertices.begin(), face.vertices.end(), first) != face.vertices.end() &&
           std::find(face.vertices.begin(), face.vertices.end(), second) != face.vertices.end();
}

bool checkMeshTopology(const Mesh& mesh, const std::string& testName) {
    bool valid = true;
    auto error = [&](const std::string& message) {
        std::cerr << "[ERREUR] " << testName << ": " << message << '\n';
        valid = false;
    };

    for (size_t faceId = 0; faceId < mesh.faces.size(); ++faceId) {
        const Face& face = mesh.faces[faceId];
        if (face.vertices.size() != 3 || face.neighbours.size() != 3) {
            error("la face " + std::to_string(faceId) + " n'a pas exactement 3 sommets et 3 voisins");
            continue;
        }

        for (int vertexId : face.vertices) {
            if (vertexId < 0 || vertexId >= static_cast<int>(mesh.vertices.size())) {
                error("la face " + std::to_string(faceId) + " référence le sommet invalide " +
                      std::to_string(vertexId));
            }
        }

        for (int edge = 0; edge < 3; ++edge) {
            int neighbourId = face.neighbours[edge];
            if (neighbourId < 0 || neighbourId >= static_cast<int>(mesh.faces.size())) {
                error("la face " + std::to_string(faceId) + " a un voisin invalide à l'index " +
                      std::to_string(edge));
                continue;
            }

            int first = face.vertices[(edge + 1) % 3];
            int second = face.vertices[(edge + 2) % 3];
            const Face& neighbour = mesh.faces[neighbourId];
            bool reciprocal = false;
            for (int neighbourEdge = 0; neighbourEdge < 3; ++neighbourEdge) {
                if (neighbour.neighbours.size() == 3 &&
                    neighbour.neighbours[neighbourEdge] == static_cast<int>(faceId) &&
                    hasEdge(neighbour, first, second)) {
                    reciprocal = true;
                    break;
                }
            }
            if (!reciprocal) {
                error("voisinage non réciproque entre les faces " + std::to_string(faceId) +
                      " et " + std::to_string(neighbourId));
            }
        }
    }
    return valid;
}

int countFacesWithVertex(const Mesh& mesh, int vertexId) {
    int count = 0;
    for (const Face& face : mesh.faces) {
        if (std::find(face.vertices.begin(), face.vertices.end(), vertexId) != face.vertices.end()) {
            ++count;
        }
    }
    return count;
}

bool writeFiniteMeshOFF(const Mesh& mesh, const std::string& fileName,
                        const std::string& testName) {
    if (mesh.borderLink < 0 || mesh.borderLink >= static_cast<int>(mesh.vertices.size())) {
        std::cerr << "[ERREUR] " << testName << ": index de sommet à l'infini invalide\n";
        return false;
    }

    std::error_code directoryError;
    std::filesystem::create_directories("OFF", directoryError);
    if (directoryError) {
        std::cerr << "[ERREUR] " << testName << ": impossible de créer le dossier OFF: "
                  << directoryError.message() << '\n';
        return false;
    }
    std::ofstream file(std::filesystem::path("OFF") / fileName);
    if (!file) {
        std::cerr << "[ERREUR] " << testName << ": impossible d'écrire OFF/" << fileName << '\n';
        return false;
    }

    std::vector<int> remappedVertexIds(mesh.vertices.size(), -1);
    int finiteVertexCount = 0;
    for (size_t vertexId = 0; vertexId < mesh.vertices.size(); ++vertexId) {
        if (static_cast<int>(vertexId) != mesh.borderLink) {
            remappedVertexIds[vertexId] = finiteVertexCount++;
        }
    }

    int finiteFaceCount = 0;
    for (const Face& face : mesh.faces) {
        if (std::find(face.vertices.begin(), face.vertices.end(), mesh.borderLink) ==
            face.vertices.end()) {
            ++finiteFaceCount;
        }
    }

    file << "OFF\n" << finiteVertexCount << ' ' << finiteFaceCount << " 0\n";
    file << std::setprecision(9);
    for (size_t vertexId = 0; vertexId < mesh.vertices.size(); ++vertexId) {
        if (static_cast<int>(vertexId) == mesh.borderLink) continue;
        const vec3& point = mesh.vertices[vertexId].coord;
        file << point.x << ' ' << point.y << ' ' << point.z << '\n';
    }

    for (const Face& face : mesh.faces) {
        if (std::find(face.vertices.begin(), face.vertices.end(), mesh.borderLink) !=
            face.vertices.end()) {
            continue;
        }
        file << "3";
        for (int vertexId : face.vertices) {
            if (vertexId < 0 || vertexId >= static_cast<int>(remappedVertexIds.size()) ||
                remappedVertexIds[vertexId] < 0) {
                std::cerr << "[ERREUR] " << testName << ": sommet invalide lors de l'export OFF\n";
                return false;
            }
            file << ' ' << remappedVertexIds[vertexId];
        }
        file << '\n';
    }

    if (!file) {
        std::cerr << "[ERREUR] " << testName << ": erreur pendant l'écriture de OFF/"
                  << fileName << '\n';
        return false;
    }
    std::cout << "Maillage exporté : OFF/" << fileName << '\n';
    return true;
}

bool runTriangulationCase(const std::string& name, const std::vector<vec3>& points,
                          int expectedBorderFaces, const std::string& offFileName) {
    Mesh mesh = Mesh::Triangulize(points);
    bool passed = checkMeshTopology(mesh, name);
    passed = writeFiniteMeshOFF(mesh, offFileName, name) && passed;
    const int borderVertex = mesh.borderLink;
    const int expectedFaceCount = 2 * static_cast<int>(points.size()) - 2;

    if (static_cast<int>(mesh.vertices.size()) != static_cast<int>(points.size()) + 1) {
        std::cerr << "[ERREUR] " << name << ": nombre de sommets attendu "
                  << points.size() + 1 << ", obtenu " << mesh.vertices.size() << '\n';
        passed = false;
    }
    if (static_cast<int>(mesh.faces.size()) != expectedFaceCount) {
        std::cerr << "[ERREUR] " << name << ": nombre de faces attendu "
                  << expectedFaceCount << ", obtenu " << mesh.faces.size() << '\n';
        passed = false;
    }

    int actualBorderFaces = 0;
    for (size_t faceId = 0; faceId < mesh.faces.size(); ++faceId) {
        const Face& face = mesh.faces[faceId];
        bool containsBorder = std::find(face.vertices.begin(), face.vertices.end(), borderVertex) !=
                              face.vertices.end();
        if (containsBorder) {
            ++actualBorderFaces;
        } else if (vec3::IsTrigoOriented(mesh.vertices[face[0]].coord,
                                         mesh.vertices[face[1]].coord,
                                         mesh.vertices[face[2]].coord) != 1) {
            std::cerr << "[ERREUR] " << name << ": la face finie " << faceId
                      << " est dégénérée ou mal orientée\n";
            passed = false;
        }
    }
    if (actualBorderFaces != expectedBorderFaces) {
        std::cerr << "[ERREUR] " << name << ": nombre de faces de bord attendu "
                  << expectedBorderFaces << ", obtenu " << actualBorderFaces << '\n';
        passed = false;
    }

    for (size_t vertexId = 1; vertexId < mesh.vertices.size(); ++vertexId) {
        const int faceRef = mesh.vertices[vertexId].faceRef;
        if (faceRef < 0 || faceRef >= static_cast<int>(mesh.faces.size()) ||
            std::find(mesh.faces[faceRef].vertices.begin(), mesh.faces[faceRef].vertices.end(),
                      static_cast<int>(vertexId)) == mesh.faces[faceRef].vertices.end()) {
            std::cerr << "[ERREUR] " << name << ": faceRef invalide pour le sommet "
                      << vertexId << '\n';
            passed = false;
        }
    }

    for (size_t pointId = 0; pointId < points.size(); ++pointId) {
        const vec3& point = points[pointId];
        bool found = false;
        for (size_t vertexId = 1; vertexId < mesh.vertices.size(); ++vertexId) {
            const vec3& coordinate = mesh.vertices[vertexId].coord;
            if (coordinate.x == point.x && coordinate.y == point.y && coordinate.z == point.z) {
                found = true;
                if (countFacesWithVertex(mesh, static_cast<int>(vertexId)) == 0) {
                    std::cerr << "[ERREUR] " << name << ": le sommet " << pointId
                              << " n'appartient à aucune face\n";
                    passed = false;
                }
                break;
            }
        }
        if (!found) {
            std::cerr << "[ERREUR] " << name << ": le point " << pointId
                      << " est absent du maillage\n";
            passed = false;
        }
    }

    if (passed) std::cout << "[OK] " << name << '\n';
    return passed;
}

bool runMeshTests() {
    int failures = 0;

    {
        const std::string name = "IsTrigoOriented";
        const vec3 a(0.0f, 0.0f, 0.0f);
        const vec3 b(4.0f, 0.0f, 0.0f);
        const vec3 c(0.0f, 4.0f, 0.0f);
        bool passed = true;
        if (!vec3::IsTrigoOriented(a, b, c)) {
            std::cerr << "[ERREUR] " << name << ": un triangle antihoraire doit être orienté trigo\n";
            passed = false;
        }
        if (vec3::IsTrigoOriented(a, c, b) >= 0 ||
            vec3::IsTrigoOriented(a, b, vec3(8.0f, 0.0f, 0.0f)) != 0) {
            std::cerr << "[ERREUR] " << name << ": un triangle horaire ou colinéaire ne doit pas être orienté trigo\n";
            passed = false;
        }
        if (passed) std::cout << "[OK] " << name << '\n';
        else ++failures;
    }

    {
        const std::string name = "IsInside";
        const vec3 a(0.0f, 0.0f, 0.0f);
        const vec3 b(4.0f, 0.0f, 0.0f);
        const vec3 c(0.0f, 4.0f, 0.0f);
        Mesh triangle({Vertex(a), Vertex(b), Vertex(c)},
                      {Face({0, 1, 2}, {0, 0, 0})}, -1);
        bool passed = true;
        int edgeFace = -1;
        if (triangle.isInside(vec3(1.0f, 1.0f, 0.0f), 0, edgeFace) != 1) {
            std::cerr << "[ERREUR] " << name << ": le point intérieur doit renvoyer 1\n";
            passed = false;
        }
        if (triangle.isInside(vec3(3.0f, 3.0f, 0.0f), 0, edgeFace) != -1) {
            std::cerr << "[ERREUR] " << name << ": le point extérieur doit renvoyer -1\n";
            passed = false;
        }
        if (triangle.isInside(vec3(2.0f, 0.0f, 0.0f), 0, edgeFace) != 0 ||
            triangle.isInside(a, 0, edgeFace) != 0) {
            std::cerr << "[ERREUR] " << name << ": un point sur une arête ou un sommet doit renvoyer 0\n";
            passed = false;
        }
        if (passed) std::cout << "[OK] " << name << '\n';
        else ++failures;
    }

    {
        const std::string name = "faceSplit";
        Mesh mesh = makeTestTetrahedron();
        const int oldVertexCount = static_cast<int>(mesh.vertices.size());
        const int oldFaceCount = static_cast<int>(mesh.faces.size());
        mesh.faceSplit(0, vec3(0.0f, 0.0f, 0.0f));

        bool passed = checkMeshTopology(mesh, name);
        if (static_cast<int>(mesh.faces.size()) != oldFaceCount + 2) {
            std::cerr << "[ERREUR] " << name << ": nombre de faces attendu " << oldFaceCount + 2
                      << ", obtenu " << mesh.faces.size() << '\n';
            passed = false;
        }
        if (static_cast<int>(mesh.vertices.size()) != oldVertexCount + 1 ||
            countFacesWithVertex(mesh, oldVertexCount) != 3) {
            std::cerr << "[ERREUR] " << name << ": le nouveau sommet doit appartenir aux 3 faces créées\n";
            passed = false;
        }
        if (passed) std::cout << "[OK] " << name << '\n';
        else ++failures;
    }

    {
        const std::string name = "edgeSplit";
        Mesh mesh = makeTestTetrahedron();
        const int oldVertexCount = static_cast<int>(mesh.vertices.size());
        const int oldFaceCount = static_cast<int>(mesh.faces.size());
        mesh.edgeSplit(0, 1, vec3(0.0f, 0.0f, 0.5f));

        bool passed = checkMeshTopology(mesh, name);
        if (static_cast<int>(mesh.faces.size()) != oldFaceCount + 2) {
            std::cerr << "[ERREUR] " << name << ": nombre de faces attendu " << oldFaceCount + 2
                      << ", obtenu " << mesh.faces.size() << '\n';
            passed = false;
        }
        if (static_cast<int>(mesh.vertices.size()) != oldVertexCount + 1 ||
            countFacesWithVertex(mesh, oldVertexCount) != 4) {
            std::cerr << "[ERREUR] " << name << ": le nouveau sommet doit appartenir aux 4 faces créées\n";
            passed = false;
        }
        if (passed) std::cout << "[OK] " << name << '\n';
        else ++failures;
    }

    {
        const std::string name = "edgeFlip";
        Mesh mesh = makeTestTetrahedron();
        const int oldFaceCount = static_cast<int>(mesh.faces.size());
        const int oldVertexCount = static_cast<int>(mesh.vertices.size());
        mesh.edgeFlip(0, 1);

        bool passed = checkMeshTopology(mesh, name);
        if (static_cast<int>(mesh.faces.size()) != oldFaceCount ||
            static_cast<int>(mesh.vertices.size()) != oldVertexCount) {
            std::cerr << "[ERREUR] " << name << ": le nombre de sommets ou de faces a changé\n";
            passed = false;
        }
        if (hasEdge(mesh.faces[0], 2, 3) || hasEdge(mesh.faces[1], 2, 3)) {
            std::cerr << "[ERREUR] " << name << ": l'ancienne diagonale (2, 3) est encore présente\n";
            passed = false;
        }
        if (!hasEdge(mesh.faces[0], 0, 1) || !hasEdge(mesh.faces[1], 0, 1)) {
            std::cerr << "[ERREUR] " << name << ": la nouvelle diagonale (0, 1) est absente\n";
            passed = false;
        }
        if (passed) std::cout << "[OK] " << name << '\n';
        else ++failures;
    }

    {
        if (!runTriangulationCase(
                "Triangulize triangle + 1 point",
                {vec3(-2.0f, -2.0f, 0.0f), vec3(2.0f, -2.0f, 0.0f),
                 vec3(0.0f, 2.0f, 0.0f), vec3(0.0f, -0.5f, 0.0f)},
                3, "triangle_plus_point.off")) {
            ++failures;
        }
    }

    {
        const std::string name = "Ajout d'un point au carré";
        Mesh mesh = makeSquareMesh();
        mesh.addVerticesToTriangulation(vec3(0.0f, 0.5f, 0.0f));
        bool passed = checkMeshTopology(mesh, name);
        passed = writeFiniteMeshOFF(mesh, "square_plus_point.off", name) && passed;

        if (mesh.vertices.size() != 6 || mesh.faces.size() != 8 ||
            countFacesWithVertex(mesh, 5) != 3) {
            std::cerr << "[ERREUR] " << name
                      << ": le point ajouté doit créer un sommet et trois faces incidentes\n";
            passed = false;
        }
        if (passed) std::cout << "[OK] " << name << '\n';
        else ++failures;
    }

    {
        if (!runTriangulationCase(
                "Triangulize 10 points",
                {vec3(-10.0f, -10.0f, 0.0f), vec3(10.0f, -10.0f, 0.0f),
                 vec3(0.0f, 10.0f, 0.0f), vec3(-4.0f, -5.0f, 0.0f),
                 vec3(4.0f, -5.0f, 0.0f), vec3(0.0f, -4.0f, 0.0f),
                 vec3(-3.0f, 0.0f, 0.0f), vec3(3.0f, 0.0f, 0.0f),
                 vec3(-2.0f, 3.0f, 0.0f), vec3(2.0f, 3.0f, 0.0f)},
                3, "10_points.off")) {
            ++failures;
        }
    }

    if (failures == 0) {
        std::cout << "Tous les tests du maillage ont réussi.\n";
    } else {
        std::cerr << failures << " test(s) du maillage en échec.\n";
    }
    return failures == 0;
}
}

int main(int argc, char const *argv[])
{
    if (argc == 2 && std::string(argv[1]) == "--test") {
        return runMeshTests() ? 0 : 1;
    }

    //Mesh m = Mesh::LoadTetrahedron();

    if(argc>2){
        std::cerr << "too many arguments" << std::endl;
        return 1;
    }
    else if(argc<2){
        std::cerr << "missing argument" << std::endl;
        return 1;
    }
    //Mesh::WriteOFF(m, std::string(argv[1]));
    Mesh m = Mesh::ReadOFF(std::string(argv[1]));
    ThermalState ts(std::vector<float>(m.vertices.size(), 0));
    ts.heatValues[0] = 100;

    const int totalSteps = 10000;
    std::cout << "Progression : 0%" << std::flush;
    //TODO en gros le probleme c'est que ca diminue ultra vite sans que rien ne monte en temperature
    for(int i = 1; i <= totalSteps; i++){
        ts.step(m, 0.00000010f);

        if(i % 100 == 0){
            int percent = (i * 100) / totalSteps;
            std::cout << "\rProgression : " << percent << "%" << std::flush;
        }

        if(i % 2000 == 0) ts.WriteCOFF(m, std::string(argv[1]));
    }

    std::cout << "\rProgression : 100%" << std::endl;
    return 0;
}
