#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<Face>& faces) : vertices(vertices), faces(faces) {}

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

    return Mesh(vertices, faces);
};


void Mesh::WriteOFF(const Mesh& mesh, const std::string& filePath){
    std::string fileContent = "OFF\n";
    fileContent += std::to_string(mesh.vertices.size()) + " ";
    fileContent += std::to_string(mesh.faces.size()) + " 0\n";

    for(size_t i = 0; i < mesh.vertices.size(); i++){
        fileContent += mesh.vertices[i].coordinates.str() + "\n";
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
        
        assert(vec3::Cross(vertices[v2].coordinates - vertices[v1].coordinates,  vertices[v3].coordinates - vertices[v1].coordinates).norm()!=0);

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


    return Mesh(vertices, faces);
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
    int A = faces[faceId].vertices[0]; 
    int C = faces[faceId].vertices[1]; 
    int B = faces[faceId].vertices[2]; 

    Vertex newVertex = Vertex(newSommet);
    newVertex.SetFaceRef(faceId);
    vertices.push_back(newVertex);
    int P = vertices.size()-1;

    Face PBA = Face({P, B, A});
    Face PAC = Face({P,A,C});
    Face PCB = Face({P,C,B});

    int PBAindex = faces.size();
    int PACindex = faces.size()+1;
    int PCBindex = faces.size()+2;


    std::vector<int> voisinsPBA = {faces[faceId].neighbours[1], PACindex, PCBindex};
    std::vector<int> voisinsPAC = {faces[faceId].neighbours[2], PCBindex, PBAindex};
    std::vector<int> voisinsPCB = {faces[faceId].neighbours[0], PBAindex, PACindex};

    PBA.SetNeighbours(voisinsPBA);
    PAC.SetNeighbours(voisinsPAC);
    PCB.SetNeighbours(voisinsPCB);

    faces.push_back(PBA);
    faces.push_back(PAC);
    faces.push_back(PCB);
    for (int i=0; i<3; i++) {
        if (faces[faces[faceId].neighbours[0]].neighbours[i] == faceId)
            faces[faces[faceId].neighbours[0]].neighbours[i] = PCBindex;
    }
    for (int i=0; i<3; i++) {
        if (faces[faces[faceId].neighbours[1]].neighbours[i] == faceId)
            faces[faces[faceId].neighbours[1]].neighbours[i] = PBAindex;
    }
    for (int i=0; i<3; i++) {
        if (faces[faces[faceId].neighbours[2]].neighbours[i] == faceId)
            faces[faces[faceId].neighbours[2]].neighbours[i] = PACindex;
    }
    
    // appel fonction delete faceId

}

void Mesh::edgeSplit(int face1, int face2) {
    if (face1 < 0 || face2 < 0 ||
        face1 >= static_cast<int>(faces.size()) ||
        face2 >= static_cast<int>(faces.size())) {
        return;
    }

    int edge1 = -1;
    int edge2 = -1;

    for (int i = 0; i < 3; ++i) {
        if (faces[face1].neighbours[i] == face2) {
            edge1 = i;
        }
        if (faces[face2].neighbours[i] == face1) {
            edge2 = i;
        }
    }

    if (edge1 == -1 || edge2 == -1) {
        return;
    }

    const int commonVertex1 = faces[face1].vertices[(edge1 + 1) % 3];
    const int commonVertex2 = faces[face1].vertices[(edge1 + 2) % 3];
    const int oppositeVertex1 = faces[face1].vertices[edge1];
    const int oppositeVertex2 = faces[face2].vertices[edge2];

    (void)commonVertex1;
    (void)commonVertex2;
    (void)oppositeVertex1;
    (void)oppositeVertex2;
}





