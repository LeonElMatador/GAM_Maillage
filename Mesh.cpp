#include "Mesh.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Face>& faces, int borderLink) : vertices(vertices), faces(faces) , borderLink(borderLink){}

//TODO default constructor
//TODO copy constructor

//TODO function that returns if a mesh is coherent (each pair of triangle is connected in both ways)


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
    std::string fileContent = "OFF\n";
    fileContent += std::to_string(mesh.vertices.size()) + " ";
    fileContent += std::to_string(mesh.faces.size()) + " 0\n";

    for(size_t i = 0; i < mesh.vertices.size(); i++){
        fileContent += mesh.vertices[i].coord.str() + "\n";
    }

    for(size_t i = 0; i < mesh.faces.size(); i++){
        fileContent += mesh.faces[i].str() +"\n";
    }

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
    std::ifstream file(filePath);

    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    float s, f;
    file >> s >> f;

    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::vector<Vertex> vertices;
 
    for(size_t i = 0; i<s; i++){
        float x, y, z;
        file >> x >> y >> z;

        vertices.push_back(Vertex(vec3(x,y,z)));

        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::vector<Face> faces;

    
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
            map.insert({{v1,v2}, {static_cast<int>(i), 2}});
        }
        else{
            faceNeighbours[2] = find1->second[0];
            faces[find1->second[0]].neighbours[find1->second[1]] = i;
            map.erase({v2,v1});
        }

        auto find2 = map.find({v3,v2});
        if(find2==map.end()){
            map.insert({{v2,v3}, {static_cast<int>(i), 0}});
        }
        else{   
            faceNeighbours[0] = find2->second[0];
            faces[find2->second[0]].neighbours[find2->second[1]] = i;
            map.erase({v3,v2});
        }

        auto find3 = map.find({v1,v3});
        if(find3==map.end()){
            map.insert({{v3,v1}, {static_cast<int>(i), 1}});
        }
        else{
            faceNeighbours[1] = find3->second[0];
            faces[find3->second[0]].neighbours[find3->second[1]] = i;
            map.erase({v1,v3});
        }

        faces.push_back(Face(faceVertices, faceNeighbours));
    }


    //Building border
    float inf =  std::numeric_limits<float>::infinity();
    Vertex borderLink = Vertex(vec3(0,0,inf));
    int borderLinkIndex = vertices.size();
    vertices.push_back(borderLink);

    int i = 0;
    for (const auto& edge : map) {
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

    //on raccroche le premier et le dernier de la bordure 
    faces[faces.size()-i].neighbours[2] = faces.size()-1;
    faces[faces.size()-1].neighbours[1] = faces.size()-i;
    
    return Mesh(vertices, faces, borderLinkIndex);
}




std::vector<int> Mesh::neighbours(const int& vertex) const{ 
    std::vector<int> neighbours; 
    int firstFace = this->vertices[vertex].faceRef;
    int currentFace = firstFace;
    while(neighbours.size()==0 || currentFace!=firstFace){
        int neigh = -1; 
        for(size_t i = 0; i < 3; i++){
            if(this->faces[currentFace].vertices[i] == vertex){
                neigh = (i+1)%3;
            }
        }
        assert(neigh!=-1);
        neighbours.push_back(this->faces[currentFace].vertices[neigh]);

        currentFace = this->faces[currentFace].neighbours[neigh];
    }

    return neighbours;
}

void Mesh::faceSplit(int faceId, vec3 newSommet) {
    int A = faces[faceId][0];
    int C = faces[faceId][1];
    int B = faces[faceId][2];

    Vertex newVertex = Vertex(newSommet);
    vertices.push_back(newVertex);
    int P = vertices.size() - 1;

    Face PBA = Face({P, B, A});
    Face PAC = Face({P, A, C});
    Face PCB = Face({P, C, B});

    int PBAindex = faces.size();
    int PACindex = faces.size() + 1;
    int PCBindex = faces.size() + 2;

    vertices[A].SetFaceRef(PBAindex);
    vertices[B].SetFaceRef(PBAindex);
    vertices[C].SetFaceRef(PACindex);
    newVertex.SetFaceRef(PBAindex);  

    std::vector<int> voisinsPBA = {faces[faceId].neighbours[1], PACindex, PCBindex};
    std::vector<int> voisinsPAC = {faces[faceId].neighbours[2], PCBindex, PBAindex};
    std::vector<int> voisinsPCB = {faces[faceId].neighbours[0], PBAindex, PACindex};

    PBA.SetNeighbours(voisinsPBA);
    PAC.SetNeighbours(voisinsPAC);
    PCB.SetNeighbours(voisinsPCB);

    faces.push_back(PBA);
    faces.push_back(PAC);
    faces.push_back(PCB);


    int n0 = faces[faceId].neighbours[0];
    int n1 = faces[faceId].neighbours[1];
    int n2 = faces[faceId].neighbours[2];

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

    if (vertices[A].faceRef == faceId) vertices[A].SetFaceRef(PBAindex); // A ∈ PBA et PAC
    if (vertices[B].faceRef == faceId) vertices[B].SetFaceRef(PBAindex); // B ∈ PBA et PCB
    if (vertices[C].faceRef == faceId) vertices[C].SetFaceRef(PACindex); // C ∈ PAC et PCB

    deleteFace(faceId);
}

void Mesh::edgeSplit(int face1, int face2, vec3 newSommet) {
    if (face1 < 0 || face2 < 0 ||
        face1 >= static_cast<int>(faces.size()) ||
        face2 >= static_cast<int>(faces.size())) {
        return;
    }

    int edge1 = -1;
    int edge2 = -1;

    for (int i = 0; i < 3; ++i) {
        if (faces[face1].neighbours[i] == face2) edge1 = i;
        if (faces[face2].neighbours[i] == face1) edge2 = i;
    }
    if (edge1 == -1 || edge2 == -1) return;

    const int opp1 = faces[face1].vertices[edge1];
    const int cv1  = faces[face1].vertices[(edge1 + 1) % 3];
    const int cv2  = faces[face1].vertices[(edge1 + 2) % 3];
    const int opp2 = faces[face2].vertices[edge2];

    //index d'un sommet donné dans une face
    auto localIndexOf = [](const Face& f, int v) {
        for (int i = 0; i < 3; i++){
            if (f.vertices[i] == v) 
                return i;
        }
        return -1;
    };

    // Nouveau sommet M, sur l'arête (cv1, cv2)
    Vertex newVertex = Vertex(newSommet);
    vertices.push_back(newVertex);
    int M = vertices.size() - 1;

    int T1index = faces.size();     // (opp1, cv1, M)
    int T2index = faces.size() + 1; // (opp1, M, cv2)
    int T3index = faces.size() + 2; // (opp2, cv2, M)
    int T4index = faces.size() + 3; // (opp2, M, cv1)

    vertices[M].SetFaceRef(T1index);


    int ext_f1_1 = faces[face1].neighbours[(edge1 + 1) % 3]; // opposé cv1, arête (opp1,cv2)
    int ext_f1_2 = faces[face1].neighbours[(edge1 + 2) % 3]; // opposé cv2, arête (opp1,cv1)

    int idxCv1inFace2 = localIndexOf(faces[face2], cv1);
    int idxCv2inFace2 = localIndexOf(faces[face2], cv2);
    int ext_f2_cv1 = faces[face2].neighbours[idxCv2inFace2]; // opposé cv2, arête (opp2,cv1)
    int ext_f2_cv2 = faces[face2].neighbours[idxCv1inFace2]; // opposé cv1, arête (opp2,cv2)

    Face T1 = Face({opp1, cv1, M});
    Face T2 = Face({opp1, M, cv2});
    Face T3 = Face({opp2, cv2, M});
    Face T4 = Face({opp2, M, cv1});

    T1.SetNeighbours({T4index, T2index, ext_f1_2});
    T2.SetNeighbours({T3index, ext_f1_1, T1index});
    T3.SetNeighbours({T2index, T4index, ext_f2_cv2});
    T4.SetNeighbours({T1index, ext_f2_cv1, T3index});

    faces.push_back(T1);
    faces.push_back(T2);
    faces.push_back(T3);
    faces.push_back(T4);

    // Maj des voisins externes (garde -1 pour les bords)
    if (ext_f1_2 != -1)
        for (int i = 0; i < 3; i++)
            if (faces[ext_f1_2].neighbours[i] == face1) faces[ext_f1_2].neighbours[i] = T1index;
    if (ext_f1_1 != -1)
        for (int i = 0; i < 3; i++)
            if (faces[ext_f1_1].neighbours[i] == face1) faces[ext_f1_1].neighbours[i] = T2index;
    if (ext_f2_cv2 != -1)
        for (int i = 0; i < 3; i++)
            if (faces[ext_f2_cv2].neighbours[i] == face2) faces[ext_f2_cv2].neighbours[i] = T3index;
    if (ext_f2_cv1 != -1)
        for (int i = 0; i < 3; i++)
            if (faces[ext_f2_cv1].neighbours[i] == face2) faces[ext_f2_cv1].neighbours[i] = T4index;

    // Réassignation des faces opposées des sommets,
    // avant suppression de face1/face2
    if (vertices[opp1].faceRef == face1) vertices[opp1].SetFaceRef(T1index);
    if (vertices[opp2].faceRef == face2) vertices[opp2].SetFaceRef(T3index);
    if (vertices[cv1].faceRef == face1) vertices[cv1].SetFaceRef(T1index);
    if (vertices[cv1].faceRef == face2) vertices[cv1].SetFaceRef(T4index);
    if (vertices[cv2].faceRef == face1) vertices[cv2].SetFaceRef(T2index);
    if (vertices[cv2].faceRef == face2) vertices[cv2].SetFaceRef(T3index);

    // Suppression de l'index le plus grand en premier, pour ne pas décaler l'autre
    if (face1 > face2) {
        deleteFace(face1);
        deleteFace(face2);
    } else {
        deleteFace(face2);
        deleteFace(face1);
    }
}

void Mesh::edgeFlip(int face1, int face2) {
    if (face1 < 0 || face2 < 0 ||
        face1 >= static_cast<int>(faces.size()) ||
        face2 >= static_cast<int>(faces.size())) {
        return;
    }

    int edge1 = -1, edge2 = -1;
    for (int i = 0; i < 3; ++i) {
        if (faces[face1].neighbours[i] == face2) edge1 = i;
        if (faces[face2].neighbours[i] == face1) edge2 = i;
    }
    if (edge1 == -1 || edge2 == -1) return;

    auto localIndexOf = [](const Face& f, int v) {
        for (int i = 0; i < 3; i++) if (f.vertices[i] == v) return i;
        return -1;
    };

    const int opp1 = faces[face1].vertices[edge1];
    const int cv1  = faces[face1].vertices[(edge1 + 1) % 3];
    const int cv2  = faces[face1].vertices[(edge1 + 2) % 3];
    const int opp2 = faces[face2].vertices[edge2];

    // Les 4 arêtes de bord du rectangle (opp1, cv1, opp2, cv2),
    int ext_opp1_cv1 = faces[face1].neighbours[localIndexOf(faces[face1], cv2)];
    int ext_cv2_opp1 = faces[face1].neighbours[localIndexOf(faces[face1], cv1)];
    int ext_cv1_opp2 = faces[face2].neighbours[localIndexOf(faces[face2], cv2)];
    int ext_opp2_cv2 = faces[face2].neighbours[localIndexOf(faces[face2], cv1)];

    // Nouvelle diagonale : opp1-opp2 (remplace cv1-cv2)
    Face newT1 = Face({opp1, cv1, opp2});   
    Face newT2 = Face({opp1, opp2, cv2}); 

    newT1.SetNeighbours({ext_cv1_opp2, face2, ext_opp1_cv1});
    newT2.SetNeighbours({ext_opp2_cv2, ext_cv2_opp1, face1});

    // Mise à jour des 2 voisins externes qui "changent de côté".
    // Les 2 autres (ext_opp1_cv1, ext_opp2_cv2) restent sur le même index, rien à faire.
    if (ext_cv2_opp1 != -1)
        for (int i = 0; i < 3; i++)
            if (faces[ext_cv2_opp1].neighbours[i] == face1) faces[ext_cv2_opp1].neighbours[i] = face2;
    if (ext_cv1_opp2 != -1)
        for (int i = 0; i < 3; i++)
            if (faces[ext_cv1_opp2].neighbours[i] == face2) faces[ext_cv1_opp2].neighbours[i] = face1;

    // cv1 n'existe plus que dans newT1 (face1), cv2 que dans newT2 (face2)
    if (vertices[cv1].faceRef == face1 || vertices[cv1].faceRef == face2)
        vertices[cv1].SetFaceRef(face1);
    if (vertices[cv2].faceRef == face1 || vertices[cv2].faceRef == face2)
        vertices[cv2].SetFaceRef(face2);
    // opp1 et opp2 apparaissent dans les deux nouveaux triangles : rien à changer

    faces[face1] = newT1;
    faces[face2] = newT2;
}


void Mesh::deleteFace(int id){//TODO should be called before adding triangle or should add triangle without taking into account the deletion
    for(int i = 0; i<faces.size(); i++){
        for(int j = 0; j<3; j++){
            if(faces[i].neighbours[j]>id){
                faces[i].neighbours[j]--;
            } 
        }
    }
    for(int i = 0; i<vertices.size(); i++){
        if(vertices[i].faceRef>id) vertices[i].faceRef--;
    }
    faces.erase(faces.begin() + id);
}

void Mesh::addVerticesToTriangulation(const vec3& p){
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
    //en dehors
    int firstFace = this->vertices[borderLink].faceRef;
    int isFirstFaceVisible = -1;
    int currentFace = firstFace;
    bool hasSplit = false;
    do{
        int v1 = -1; 
        int v2 = -1; 
        for(size_t i = 0; i < 3; i++){
            if(this->faces[currentFace].vertices[i] == borderLink){
                v1 = (i+1)%3;
                v2 = (i+2)%3;
            }
        }
        assert(v1!=-1);
        assert(v2!=-1);
        int nextFace = this->faces[currentFace].neighbours[v1];
        if(vec3::IsTrigoOriented(p,vertices[v1].coord, vertices[v2].coord)){//Visible
            if(firstFace == currentFace) isFirstFaceVisible = this->faces[currentFace].neighbours[v2];;//on enregistre le voisin gauche pour pouvoir repartir a gauche 
            if(!hasSplit){
                faceSplit(currentFace, p);//! BUG ca devrait nous renvoyer les faces crées 
                hasSplit=true;
            }
            else{
                //edge flip
                edgeFlip(currentFace, this->faces[currentFace].neighbours[v2]);//on flip avec la face a gauche
            }
        }
        else if(hasSplit){//si pas visible
            break;
        }
        currentFace = nextFace;
    }while(currentFace!=firstFace);


    if(isFirstFaceVisible > -1){//on repart dans l'autre sens si la premier face etait deja visible
        currentFace = isFirstFaceVisible; //on avait enregistré le voisin gauche
        do{
            int v1 = -1; 
            int v2 = -1; 
            for(size_t i = 0; i < 3; i++){
                if(this->faces[currentFace].vertices[i] == borderLink){
                    v1 = (i+1)%3;
                    v2 = (i+2)%3;
                }
            }
            assert(v1!=-1);
            assert(v2!=-1);
            int nextFace = this->faces[currentFace].neighbours[v1];
            if(vec3::IsTrigoOriented(p,vertices[v1].coord, vertices[v2].coord)){//Visible
                edgeFlip(currentFace, this->faces[currentFace].neighbours[v1]);//on flip avec la face a gauche
            }
            else {//si pas visible
                break;
            }
            currentFace = this->faces[currentFace].neighbours[v2];
        }while(currentFace!=firstFace);

    }
}

const float EPSILON = 0.000001f;
int Mesh::isInside(const vec3& p, int faceIndex, int& edgeFace)const {
    Face f = faces[faceIndex];
    float airPAB = vec3::Cross(vertices[f[0]].coord - p, vertices[f[1]].coord - p).z;
    float airPBC = vec3::Cross(vertices[f[1]].coord - p, vertices[f[2]].coord - p).z;
    float airPCA = vec3::Cross(vertices[f[2]].coord - p, vertices[f[0]].coord - p).z;

    if(airPAB<-EPSILON||airPBC<-EPSILON||airPCA<-EPSILON){
        return -1;
    }
    if(airPAB > EPSILON && airPBC > EPSILON && airPCA > EPSILON){
        return 1;
    }
    if(airPAB < EPSILON && airPAB >= -EPSILON) edgeFace = f.neighbours[2];
    if(airPBC < EPSILON && airPBC >= -EPSILON) edgeFace = f.neighbours[0];
    if(airPCA < EPSILON && airPCA >= -EPSILON) edgeFace = f.neighbours[1];
    return 0;
}


std::vector<int> Mesh::getBorder() const{
    std::vector<int> borderFaces; 
    int firstFace = this->vertices[borderLink].faceRef;
    int currentFace = firstFace;
    while(borderFaces.size()==0 || currentFace!=firstFace){
        int v = -1; 
        for(size_t i = 0; i < 3; i++){
            if(this->faces[currentFace].vertices[i] == borderLink){
                v = (i+1)%3;
            }
        }
        assert(v!=-1);
        borderFaces.push_back(currentFace);

        currentFace = this->faces[currentFace].neighbours[v];
    }

    return borderFaces;
}

