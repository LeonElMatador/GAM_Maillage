#include "Mesh.h"
#include "ThermalState.h"
#include <algorithm>
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

bool runMeshTests() {
    int failures = 0;

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
