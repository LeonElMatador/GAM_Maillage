#include "Mesh.h"
#include <algorithm>

namespace {

    void printFace(const char* state, int faceId, const Face& face) {
        std::cout << "  " << state << " #" << faceId << " : " << face.str() << '\n';
    }   // fonction pour le debug

    int localIndexOf(const Face& face, int vertex) {
        for (int i = 0; i < 3; ++i) {
            if (face.vertices[i] == vertex) return i;
        }
        return -1;
    } // pour renvoyer l'index local d'un sommet dans une face
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Face>& faces, int borderLink) : vertices(vertices), faces(faces) , borderLink(borderLink){}

Mesh Mesh::LoadTetrahedron(){
    Vertex p1(0.5f, 0, 0.5f);
    Vertex p2(-0.5f, 0, 0.5f);
    Vertex p3(0.5f, 0, -0.5f);

    Vertex p0(0, 1, 0);

    std::vector<Vertex> vertices{p0,p1,p2,p3};

    Face f1(std::vector<int>{1, 2, 3});
    Face f2(std::vector<int>{2, 0, 3});
    Face f3(std::vector<int>{3, 0, 1});
    Face f4(std::vector<int>{1, 0, 2});

    std::vector<Face> faces{f1,f2,f3,f4};

    p1.SetFaceRef(0);
    p2.SetFaceRef(0);
    p3.SetFaceRef(0);
    p0.SetFaceRef(2); 

    f1.SetNeighbours(std::vector<int>{1,2,3});
    f2.SetNeighbours(std::vector<int>{2,0,3});
    f3.SetNeighbours(std::vector<int>{3,0,1});
    f4.SetNeighbours(std::vector<int>{1,0,2});

    return Mesh(vertices, faces, -1);
};


void Mesh::WriteOFF(const Mesh& mesh, const std::string& filePath){
    //on associe un nouvel index à chaque sommet, sauf au sommet à l'infini
    std::vector<int> remappedVertexIds(mesh.vertices.size(), -1);
    int finiteVertexCount = 0;
    for(size_t i = 0; i < mesh.vertices.size(); i++){
        if((int)(i) != mesh.borderLink){
            remappedVertexIds[i] = finiteVertexCount++;
        }
    }

    float maxCurvature = 0.0f;
    for(int i = 0; i < mesh.vertices.size(); ++i){
        if(i == mesh.borderLink) continue;
        maxCurvature = std::max(maxCurvature, mesh.curvature[i]);
    }

    //on compte uniquement les faces qui ne contiennent pas le sommet à l'infini
    int finiteFaceCount = 0;
    for(const Face& face : mesh.faces){
        if(std::find(face.vertices.begin(), face.vertices.end(), mesh.borderLink) == face.vertices.end()){
            finiteFaceCount++;
        }
    }

    std::string fileContent = "COFF\n";
    fileContent += std::to_string(finiteVertexCount) + " ";
    fileContent += std::to_string(finiteFaceCount) + " 0\n";

    for(size_t i = 0; i < mesh.vertices.size(); i++){
        if((int)(i) != mesh.borderLink){
            float normalizedCurvature = mesh.curvature[i] / maxCurvature;
            fileContent += mesh.vertices[i].coord.str() + " " + (mesh.curvatureToColor(normalizedCurvature)).str() +"\n";
        }
    }

    for(const Face& face : mesh.faces){
        if(std::find(face.vertices.begin(), face.vertices.end(), mesh.borderLink) != face.vertices.end()){
            continue;
        }
        fileContent += "3";
        for(int vertexId : face.vertices){
            fileContent += " " + std::to_string(remappedVertexIds[vertexId]);
        }
        fileContent += "\n";
    }

    //on ouvre le fichier puis on y écrit le maillage fini
    std::ofstream offFile(filePath);
    if(offFile.is_open()){
        offFile << fileContent;
        std::cout << "OFF file saved successfully as " << filePath << std::endl;
    }
    else{
        std::cerr << "Error while opening file : " << filePath << std::endl;
    }
    offFile.close();
}


//Reads only triangles
Mesh Mesh::ReadOFF(const std::string& filePath){
    //on ouvre le fichier OFF et on passe son en-tête
    std::ifstream file(filePath);

    if(!file.is_open()){
        std::cerr << "Error while opening file : " << filePath << std::endl;
    }

    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    //on récupère le nombre de sommets et de faces
    float s, f;
    file >> s >> f;

    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    //on lit les coordonnées et on crée les sommets
    std::vector<Vertex> vertices;
 
    for(size_t i = 0; i<s; i++){
        float x, y, z;
        file >> x >> y >> z;

        vertices.push_back(Vertex(vec3(x,y,z)));

        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    //on lit les faces triangulaires et on initialise leurs voisins
    std::vector<Face> faces;

    //on mémorise les arêtes rencontrées pour retrouver les faces voisines
    std::map<std::array<int, 2>, std::array<int, 2>> map;

    for(size_t i = 0; i<f; i++){
        int p;
        file >> p;

        assert(p==3);

        std::vector<int> faceVertices;
        int v1, v2, v3;
        file >> v1 >> v2 >> v3;    
        faceVertices.push_back(v1);
        faceVertices.push_back(v2);
        faceVertices.push_back(v3);
        
        assert(vec3::Cross(vertices[v2].coord - vertices[v1].coord,  vertices[v3].coord - vertices[v1].coord).norm()!=0);

        std::vector<int> faceNeighbours;
        faceNeighbours.resize(3);

        vertices[v1].SetFaceRef(i);
        vertices[v2].SetFaceRef(i);
        vertices[v3].SetFaceRef(i);
        

        auto find1 = map.find({v2,v1});
        if(find1==map.end()){
            map.insert({{v1,v2}, {(int)(i), 2}});
        }
        else{
            faceNeighbours[2] = find1->second[0];
            faces[find1->second[0]].neighbours[find1->second[1]] = i;
            map.erase({v2,v1});
        }

        auto find2 = map.find({v3,v2});
        if(find2==map.end()){
            map.insert({{v2,v3}, {(int)(i), 0}});
        }
        else{   
            faceNeighbours[0] = find2->second[0];
            faces[find2->second[0]].neighbours[find2->second[1]] = i;
            map.erase({v3,v2});
        }

        auto find3 = map.find({v1,v3});
        if(find3==map.end()){
            map.insert({{v3,v1}, {(int)(i), 1}});
        }
        else{
            faceNeighbours[1] = find3->second[0];
            faces[find3->second[0]].neighbours[find3->second[1]] = i;
            map.erase({v1,v3});
        }

        faces.push_back(Face(faceVertices, faceNeighbours));
    }



    int borderLinkIndex = vertices.size();
    if(!map.empty()){
        //on crée le sommet à l'infini et les faces correspondant aux arêtes de bord
        float inf =  std::numeric_limits<float>::infinity();
        Vertex borderLink = Vertex(vec3(0,0,inf));
        vertices.push_back(borderLink);

        int i = 0;
        for (const auto& edge : map) {
            //on relie chaque nouvelle face de bord à sa face intérieure
            Face f({borderLinkIndex, edge.first[1], edge.first[0]});
            f.neighbours[0] = edge.second[0];
            faces[edge.second[0]].neighbours[edge.second[1]] = faces.size();

            if(i > 0){
                f.neighbours[2] = faces.size()-1;
                faces[faces.size()-1].neighbours[1] = faces.size();
            }
            else{
                borderLink.SetFaceRef(faces.size());
            }

            faces.push_back(f);
            i++;
        }
        if(i>0){
            //on relie les deux extrémités de la bordure pour fermer le parcours
            faces[faces.size()-i].neighbours[2] = faces.size()-1;
            faces[faces.size()-1].neighbours[1] = faces.size()-i;
        }
    }
    else{
        borderLinkIndex=-1;
    }
    
    
    return Mesh(vertices, faces, borderLinkIndex);
}




std::vector<int> Mesh::neighbours(const int& vertex) const{ 
    std::vector<int> neighbours;
    if (vertex < 0 || vertex >= static_cast<int>(this->vertices.size())) return neighbours;
    if (this->borderLink >= 0 && vertex == this->borderLink) return neighbours;

    int firstFace = this->vertices[vertex].faceRef;
    if (firstFace < 0 || firstFace >= static_cast<int>(this->faces.size())) return neighbours;

    int currentFace = firstFace;
    std::vector<int> visitedFaces;
    visitedFaces.push_back(currentFace);

    for (int iter = 0; iter < static_cast<int>(this->faces.size()) * 3; ++iter) {
        int localIndex = -1;
        for (int i = 0; i < 3; ++i) {
            if (this->faces[currentFace].vertices[i] == vertex) {
                localIndex = i;
                break;
            }
        }
        if (localIndex == -1 || currentFace < 0 || currentFace >= static_cast<int>(this->faces.size())) {
            break;
        }

        const int nextLocalIndex = (localIndex + 1) % 3;
        const int nextFace = this->faces[currentFace].neighbours[nextLocalIndex];
        const int nextVertex = this->faces[currentFace].vertices[nextLocalIndex];

        if (nextFace == -1) break;
        neighbours.push_back(nextVertex);

        if (nextFace == firstFace) break;

        currentFace = nextFace;
        if (std::find(visitedFaces.begin(), visitedFaces.end(), currentFace) != visitedFaces.end()) {
            break;
        }
        visitedFaces.push_back(currentFace);
    }

    return neighbours;
}

void Mesh::faceSplit(int faceId, vec3 newSommet) {
    int A = faces[faceId][0];
    int C = faces[faceId][1];
    int B = faces[faceId][2];
    //on récupère les sommets de la face


    Vertex newVertex = Vertex(newSommet);
    vertices.push_back(newVertex);
    int P = vertices.size() - 1;
    //on crée et ajoute le nouveau sommet

    Face PBA = Face({P, B, A});
    Face PAC = Face({P, A, C});
    Face PCB = Face({P, C, B});
    //on crée les trois nouvelles faces

    int PBAindex = faces.size();
    int PACindex = faces.size() + 1;
    int PCBindex = faces.size() + 2;
    //on calcule les nouveaux index des nouvelles faces

    vertices[A].SetFaceRef(PBAindex);
    vertices[B].SetFaceRef(PBAindex);
    vertices[C].SetFaceRef(PACindex);
    vertices[P].SetFaceRef(PBAindex);
    //on change les faces des sommets par une des nouvelles faces créée (peu importe laquelle)

    std::vector<int> voisinsPBA = {faces[faceId].neighbours[1], PACindex, PCBindex};
    std::vector<int> voisinsPAC = {faces[faceId].neighbours[2], PCBindex, PBAindex};
    std::vector<int> voisinsPCB = {faces[faceId].neighbours[0], PBAindex, PACindex};
    //on crée les listes de voisins des nouvelles faces

    PBA.SetNeighbours(voisinsPBA);
    PAC.SetNeighbours(voisinsPAC);
    PCB.SetNeighbours(voisinsPCB);

    faces.push_back(PBA);
    faces.push_back(PAC);
    faces.push_back(PCB);
    //attribution voisins, puis on ajoute les faces à la liste

    int n0 = faces[faceId].neighbours[0];
    int n1 = faces[faceId].neighbours[1];
    int n2 = faces[faceId].neighbours[2];
    //on récupère les voisins de l'ancienne face

    if (n0 != -1) {
        for (int i = 0; i < 3; i++)
            if (faces[n0].neighbours[i] == faceId)
                faces[n0].neighbours[i] = PCBindex;
    }
    if (n1 != -1) {
        for (int i = 0; i < 3; i++)
            if (faces[n1].neighbours[i] == faceId)
                faces[n1].neighbours[i] = PBAindex;
    }
    if (n2 != -1) {
        for (int i = 0; i < 3; i++)
            if (faces[n2].neighbours[i] == faceId)
                faces[n2].neighbours[i] = PACindex;
    }
    //on update les faces opposées des anciens voisins par les nouvelles faces

    if (vertices[A].faceRef == faceId) vertices[A].SetFaceRef(PBAindex); // A ∈ PBA et PAC
    if (vertices[B].faceRef == faceId) vertices[B].SetFaceRef(PBAindex); // B ∈ PBA et PCB
    if (vertices[C].faceRef == faceId) vertices[C].SetFaceRef(PACindex); // C ∈ PAC et PCB
    //si les sommets de la face avaient comme face adjacente la face supprimée, on la modifie

    deleteFace(faceId);
    //appel à la fonction de suppression d'une face
}

void Mesh::edgeSplit(int face1, int face2, vec3 newSommet) {
    if (face1 < 0 || face2 < 0 ||
        face1 >= (int)(faces.size()) ||
        face2 >= (int)(faces.size())) {
        return;
    }
    //ne doit pas arriver, juste au cas où


    int edge1 = -1;
    int edge2 = -1;

    for (int i = 0; i < 3; ++i) {
        if (faces[face1].neighbours[i] == face2) edge1 = i;
        if (faces[face2].neighbours[i] == face1) edge2 = i;
    }
    if (edge1 == -1 || edge2 == -1) return;
    //on cherche l'arête commune aux deux faces

    const int opposé1 = faces[face1].vertices[edge1];
    const int commun1  = faces[face1].vertices[(edge1 + 1) % 3];
    const int commun2  = faces[face1].vertices[(edge1 + 2) % 3];
    const int opposé2 = faces[face2].vertices[edge2];
    //on récupère les sommets des deux faces

    // Nouveau sommet M, sur l'arête (commun1, commun2)
    Vertex newVertex = Vertex(newSommet);
    vertices.push_back(newVertex);
    int M = vertices.size() - 1;
    //on crée et ajoute le nouveau sommet

    int T1index = faces.size();     // (opposé1, commun1, M)
    int T2index = faces.size() + 1; // (opposé1, M, commun2)
    int T3index = faces.size() + 2; // (opposé2, commun2, M)
    int T4index = faces.size() + 3; // (opposé2, M, commun1)
    //on calcule les nouveaux index des nouvelles faces

    vertices[M].SetFaceRef(T1index);
    //on change la face du nouveau sommet par une des nouvelles faces créée (peu importe laquelle)

    int ext_f1_1 = faces[face1].neighbours[(edge1 + 1) % 3]; // opposé commun1, arête (opposé1,commun2)
    int ext_f1_2 = faces[face1].neighbours[(edge1 + 2) % 3]; // opposé commun2, arête (opposé1,commun1)

    int idxcommun1inFace2 = localIndexOf(faces[face2], commun1);
    int idxcommun2inFace2 = localIndexOf(faces[face2], commun2);
    int ext_f2_commun1 = faces[face2].neighbours[idxcommun2inFace2]; // opposé commun2, arête (opposé2,commun1)
    int ext_f2_commun2 = faces[face2].neighbours[idxcommun1inFace2]; // opposé commun1, arête (opposé2,commun2)
    //on récupère les voisins externes des deux anciennes faces

    Face T1 = Face({opposé1, commun1, M});
    Face T2 = Face({opposé1, M, commun2});
    Face T3 = Face({opposé2, commun2, M});
    Face T4 = Face({opposé2, M, commun1});
    //on crée les quatre nouvelles faces

    T1.SetNeighbours({T4index, T2index, ext_f1_2});
    T2.SetNeighbours({T3index, ext_f1_1, T1index});
    T3.SetNeighbours({T2index, T4index, ext_f2_commun2});
    T4.SetNeighbours({T1index, ext_f2_commun1, T3index});

    faces.push_back(T1);
    faces.push_back(T2);
    faces.push_back(T3);
    faces.push_back(T4);
    //on crée les listes de voisins, puis on ajoute les faces à la liste

    //on update les faces opposées des anciens voisins par les nouvelles faces
    if (ext_f1_2 != -1)
        for (int i = 0; i < 3; i++)
            if (faces[ext_f1_2].neighbours[i] == face1) faces[ext_f1_2].neighbours[i] = T1index;
    if (ext_f1_1 != -1)
        for (int i = 0; i < 3; i++)
            if (faces[ext_f1_1].neighbours[i] == face1) faces[ext_f1_1].neighbours[i] = T2index;
    if (ext_f2_commun2 != -1)
        for (int i = 0; i < 3; i++)
            if (faces[ext_f2_commun2].neighbours[i] == face2) faces[ext_f2_commun2].neighbours[i] = T3index;
    if (ext_f2_commun1 != -1)
        for (int i = 0; i < 3; i++)
            if (faces[ext_f2_commun1].neighbours[i] == face2) faces[ext_f2_commun1].neighbours[i] = T4index;

    //si les sommets des anciennes faces avaient comme face adjacente une des faces supprimées, on la modifie
    if (vertices[opposé1].faceRef == face1) vertices[opposé1].SetFaceRef(T1index);
    if (vertices[opposé2].faceRef == face2) vertices[opposé2].SetFaceRef(T3index);
    if (vertices[commun1].faceRef == face1) vertices[commun1].SetFaceRef(T1index);
    if (vertices[commun1].faceRef == face2) vertices[commun1].SetFaceRef(T4index);
    if (vertices[commun2].faceRef == face1) vertices[commun2].SetFaceRef(T2index);
    if (vertices[commun2].faceRef == face2) vertices[commun2].SetFaceRef(T3index);

    // Suppression de l'index le plus grand en premier, pour ne pas décaler l'autre
    if (face1 > face2) {
        deleteFace(face1);
        deleteFace(face2);
    } else {
        deleteFace(face2);
        deleteFace(face1);
    }
    //appel à la fonction de suppression des deux anciennes faces
    const int firstNewFace = (int)(faces.size()) - 4;
}

void Mesh::edgeFlip(int face1, int face2) {
    if (face1 == face2 || face1 < 0 || face2 < 0 ||
        face1 >= (int)(faces.size()) ||
        face2 >= (int)(faces.size())) {
        return;
    }
    //ne doit pas arriver, juste au cas où

    int edge1 = -1, edge2 = -1;
    for (int i = 0; i < 3; ++i) {
        if (faces[face1].neighbours[i] == face2) edge1 = i;
        if (faces[face2].neighbours[i] == face1) edge2 = i;
    }
    if (edge1 == -1 || edge2 == -1) return;
    //on cherche l'arête commune aux deux faces

    const int opposé1 = faces[face1].vertices[edge1];
    const int commun1  = faces[face1].vertices[(edge1 + 1) % 3];
    const int commun2  = faces[face1].vertices[(edge1 + 2) % 3];
    const int opposé2 = faces[face2].vertices[edge2];
    //on récupère les sommets des deux faces

    int ext_opposé1_commun1 = faces[face1].neighbours[localIndexOf(faces[face1], commun2)];
    int ext_commun2_opposé1 = faces[face1].neighbours[localIndexOf(faces[face1], commun1)];
    int ext_commun1_opposé2 = faces[face2].neighbours[localIndexOf(faces[face2], commun2)];
    int ext_opposé2_commun2 = faces[face2].neighbours[localIndexOf(faces[face2], commun1)];
    //on récupère les voisins externes des deux faces

    Face newT1 = Face({opposé1, commun1, opposé2});
    Face newT2 = Face({opposé1, opposé2, commun2});
    //on crée les deux nouvelles faces en remplaçant l'arête commune

    newT1.SetNeighbours({ext_commun1_opposé2, face2, ext_opposé1_commun1});
    newT2.SetNeighbours({ext_opposé2_commun2, ext_commun2_opposé1, face1});
    //on crée les listes de voisins des nouvelles faces

    if (ext_commun2_opposé1 != -1)
        for (int i = 0; i < 3; i++)
            if (faces[ext_commun2_opposé1].neighbours[i] == face1) faces[ext_commun2_opposé1].neighbours[i] = face2;
    if (ext_commun1_opposé2 != -1)
        for (int i = 0; i < 3; i++)
            if (faces[ext_commun1_opposé2].neighbours[i] == face2) faces[ext_commun1_opposé2].neighbours[i] = face1;
    //on update les faces opposées des anciens voisins par les nouvelles faces

    if (vertices[commun1].faceRef == face1 || vertices[commun1].faceRef == face2)
        vertices[commun1].SetFaceRef(face1);
    if (vertices[commun2].faceRef == face1 || vertices[commun2].faceRef == face2)
        vertices[commun2].SetFaceRef(face2);
    //si les sommets de l'arête commune avaient comme face adjacente une des faces modifiées, on la modifie

    faces[face1] = newT1;
    faces[face2] = newT2;
    //on remplace les deux anciennes faces par les nouvelles faces
}


void Mesh::deleteFace(int id){
    //on parcours toutes les faces pour corriger les index des voisins
    for(int i = 0; i<faces.size(); i++){
        for(int j = 0; j<3; j++){
            if(faces[i].neighbours[j]>id){
                faces[i].neighbours[j]--;
            } 
        }
    }
    //on parcours tous les sommets pour corriger les références de face
    for(int i = 0; i<vertices.size(); i++){
        if(vertices[i].faceRef>id) vertices[i].faceRef--;
    }
    //on supprime la face de la liste
    faces.erase(faces.begin() + id);
}

void Mesh::addVerticesToTriangulation(const vec3& p){
    //on parcours toutes les faces pour savoir où se trouve le nouveau sommet
    for(int i = 0; i < faces.size(); i++){
        int edgeRelatedFace = 0;
        int relativePos = isInside(p, i, edgeRelatedFace);
        if(relativePos==1){
            faceSplit(i, p);
            return;
        }
        if(relativePos==0){
            edgeSplit(i, edgeRelatedFace, p);
            return;
        }
    }
    //si le sommet est à l'extérieur, on récupère la première face de bord
    int firstFace = this->vertices[borderLink].faceRef;
    int isFirstFaceVisible = -1;
    int currentFace = firstFace;
    bool hasSplit = false;
    int it = -1;
    //on parcours le bord dans un premier sens
    do{
        it++;
        if (currentFace < 0 || currentFace >= (int)(faces.size())) break;
        int v1 = -1; 
        int v2 = -1; 
        for(size_t i = 0; i < 3; i++){
            if(this->faces[currentFace].vertices[i] == borderLink){
                v1 = (i+1)%3;
                v2 = (i+2)%3;
            }
        }
        if (v1 == -1 || v2 == -1) break;
        //on récupère les deux sommets de l'arête de bord et la prochaine face à parcourir
        int nextFace = this->faces[currentFace].neighbours[v1];
        const int endpoint1 = this->faces[currentFace].vertices[v1];
        const int endpoint2 = this->faces[currentFace].vertices[v2];
        //on vérifie si la nouvelle face est visible depuis le point ajouté
        if(vec3::IsTrigoOriented(p,vertices[endpoint1].coord, vertices[endpoint2].coord) == 1){//Visible
            if(firstFace == currentFace) isFirstFaceVisible = this->faces[currentFace].neighbours[v2];//on enregistre le voisin gauche pour pouvoir repartir a gauche
            if(!hasSplit){
                faceSplit(currentFace, p);
                //une face vient d'etre supprimée
                if(nextFace>currentFace)nextFace--;
                if(isFirstFaceVisible>currentFace)isFirstFaceVisible--;
                hasSplit=true;
            }
            else{
                //edge flip
                edgeFlip(currentFace, this->faces[currentFace].neighbours[v2]);//on flip avec la face a gauche
            }
        }
        else if(hasSplit){//si la face n'est pas visible, on a fini de parcourir le bord visible
            break;
        }
        currentFace = nextFace;
    }while(it<faces.size());//arret d'urgence

    it=-1;
    if(isFirstFaceVisible > -1){//on repart dans l'autre sens si la première face était déjà visible
        currentFace = isFirstFaceVisible; //on avait enregistré le voisin gauche
        //on parcours le reste du bord dans l'autre sens
        do{
            it++;
            if (currentFace < 0 || currentFace >= (int)(faces.size())) break;
            int v1 = -1; 
            int v2 = -1; 
            for(size_t i = 0; i < 3; i++){
                if(this->faces[currentFace].vertices[i] == borderLink){
                    v1 = (i+1)%3;
                    v2 = (i+2)%3;
                }
            }
            if (v1 == -1 || v2 == -1) break;
            //on récupère les deux sommets de l'arête de bord et la prochaine face à parcourir
            int nextFace = this->faces[currentFace].neighbours[v2];
            const int endpoint1 = this->faces[currentFace].vertices[v1];
            const int endpoint2 = this->faces[currentFace].vertices[v2];
            //on vérifie si la nouvelle face est visible depuis le point ajouté
            if(vec3::IsTrigoOriented(p,vertices[endpoint1].coord, vertices[endpoint2].coord) == 1){//Visible
                edgeFlip(currentFace, this->faces[currentFace].neighbours[v1]);//on flip avec la face a gauche
            }
            else {//si la face n'est pas visible, on a fini de parcourir le bord visible
                break;
            }
            currentFace = nextFace;
        }while(it<faces.size());//arret d'urgence
    }
}

Mesh Mesh::Triangulize(const std::vector<vec3>& points){
    //on crée le sommet à l'infini et les trois premiers sommets du maillage
    Vertex infini(vec3(0,0,1));
    Vertex v1(points[0]);
    Vertex v2(points[1]);
    Vertex v3(points[2]);

    std::vector<Vertex> vertices{infini,v1,v2,v3};
    //on initialise les faces de référence des sommets
    vertices[1].SetFaceRef(0);
    vertices[2].SetFaceRef(0);
    vertices[3].SetFaceRef(0);
    vertices[0].SetFaceRef(2);

    //on crée les quatre faces initiales, dont trois faces de bord
    std::vector<Face> faces{
        Face({1, 2, 3}, {1, 2, 3}),
        Face({2, 0, 3}, {2, 0, 3}),
        Face({3, 0, 1}, {3, 0, 1}),
        Face({1, 0, 2}, {1, 0, 2})
    };

    Mesh mesh(vertices, faces, 0);

    //on ajoute les autres sommets un par un à la triangulation
    for(int i = 3; i < points.size(); i++){
        mesh.addVerticesToTriangulation(points[i]);
    }
    return mesh;
}

const float EPSILON = 0.000001f;
int Mesh::isInside(const vec3& p, int faceIndex, int& edgeFace)const {
    //on récupère la face et on calcule les orientations du point par rapport à ses trois arêtes
    Face f = faces[faceIndex];
    float airPAB = vec3::Cross(vertices[f[0]].coord - p, vertices[f[1]].coord - p).z;
    float airPBC = vec3::Cross(vertices[f[1]].coord - p, vertices[f[2]].coord - p).z;
    float airPCA = vec3::Cross(vertices[f[2]].coord - p, vertices[f[0]].coord - p).z;

    //si le point est du côté extérieur d'une arête, il est à l'extérieur de la face
    if(airPAB<-EPSILON||airPBC<-EPSILON||airPCA<-EPSILON){
        return -1;
    }
    //si le point est à l'intérieur des trois arêtes, il est dans la face
    if(airPAB > EPSILON && airPBC > EPSILON && airPCA > EPSILON){
        return 1;
    }
    //si le point est sur une arête, on enregistre la face voisine correspondante
    if(airPAB < EPSILON && airPAB >= -EPSILON) edgeFace = f.neighbours[2];
    if(airPBC < EPSILON && airPBC >= -EPSILON) edgeFace = f.neighbours[0];
    if(airPCA < EPSILON && airPCA >= -EPSILON) edgeFace = f.neighbours[1];
    //on retourne 0 si le point est sur une arête ou sur un sommet
    return 0;
}


std::vector<int> Mesh::getBorder() const{
    std::vector<int> borderFaces; 
    //on récupère une face de bord pour commencer le parcours
    int firstFace = this->vertices[borderLink].faceRef;
    int currentFace = firstFace;
    //on parcours les faces de bord jusqu'à revenir à la première
    while(borderFaces.size()==0 || currentFace!=firstFace){
        int v = -1; 
        for(size_t i = 0; i < 3; i++){
            if(this->faces[currentFace].vertices[i] == borderLink){
                v = (i+1)%3;
            }
        }
        assert(v!=-1);
        //on ajoute la face courante puis on passe à sa face voisine sur le bord
        borderFaces.push_back(currentFace);

        currentFace = this->faces[currentFace].neighbours[v];
    }

    //on retourne la liste des faces de bord
    return borderFaces;
}


void Mesh::computeCurvature(){
    curvature.resize(vertices.size());
    laplacien.resize(vertices.size());

    for(int i = 0; i < static_cast<int>(vertices.size()); ++i){
        if (i == borderLink) {
            laplacien[i] = vec3(0.0f, 0.0f, 0.0f);
            curvature[i] = 0.0f;
            continue;
        }

        vec3 sum(0.0f, 0.0f, 0.0f);
        float surfaceSum = 0.0f;
        std::vector<int> neighbours = this->neighbours(i);
        if (neighbours.size() < 2) {
            laplacien[i] = vec3(0.0f, 0.0f, 0.0f);
            curvature[i] = 0.0f;
            continue;
        }

        const int size = static_cast<int>(neighbours.size());
        for(int j = 0; j < size; ++j){
            const vec3 A = vertices[i].coord;
            const vec3 B = vertices[neighbours[j]].coord;
            const vec3 C = vertices[neighbours[(j - 1 + size) % size]].coord;
            const vec3 D = vertices[neighbours[(j + 1) % size]].coord;

            const vec3 CA = A - C;
            const vec3 CB = B - C;
            const vec3 DA = A - D;
            const vec3 DB = B - D;

            const float crossCABnorm = vec3::Cross(CA, CB).norm();
            const float crossDABnorm = vec3::Cross(DA, DB).norm();

            if (crossCABnorm <= 1e-8f || crossDABnorm <= 1e-8f) {
                continue;
            }

            const float dotCAB = vec3::Dot(CA, CB);
            const float dotDAB = vec3::Dot(DA, DB);
            const float leftSurface = crossCABnorm / 2.0f;
            surfaceSum += leftSurface;

            const float cotanC = dotCAB / crossCABnorm;
            const float cotanD = dotDAB / crossDABnorm;

            sum = sum + (B - A) * (cotanC + cotanD);
        }

        if (surfaceSum <= 1e-8f) {
            laplacien[i] = vec3(0.0f, 0.0f, 0.0f);
            curvature[i] = 0.0f;
            continue;
        }

        laplacien[i] = sum / (2.0f * surfaceSum);
        curvature[i] = laplacien[i].norm() / 2.0f;
        if (!std::isfinite(curvature[i]) || !std::isfinite(laplacien[i].x) ||
            !std::isfinite(laplacien[i].y) || !std::isfinite(laplacien[i].z)) {
            laplacien[i] = vec3(0.0f, 0.0f, 0.0f);
            curvature[i] = 0.0f;
        }
    }
}


vec3 Mesh::curvatureToColor(float c)const{
    const double value = c;
    const vec3 blue(0.0f, 0.0f, 1.0f);
    const vec3 cyan(0.0f, 1.0f, 1.0f);
    const vec3 green(0.0f, 1.0f, 0.0f);
    const vec3 yellow(1.0f, 1.0f, 0.0f);
    const vec3 red(1.0f, 0.0f, 0.0f);

    if (value < 0.08f) return vec3::Lerp(blue, cyan, value / 0.08f);
    if (value < 0.25f) return vec3::Lerp(cyan, green, (value - 0.08f) / 0.17f);
    if (value < 0.55f) return vec3::Lerp(green, yellow, (value - 0.25f) / 0.30f);
    if (value < 0.90f) return vec3::Lerp(yellow, red, (value - 0.55f) / 0.35f);
    return red;
}