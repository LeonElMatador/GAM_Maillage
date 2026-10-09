#include "ThermalState.h"
#include <algorithm>
#include <filesystem>



void ThermalState::computeLaplacian(const Mesh& m){
    //on calcule le laplacien de chaleur de chaque sommet
    for(int i = 0; i < m.vertices.size(); i++){
        double sum = 0;
        double surfaceSum = 0;
        //on récupère les voisins du sommet courant
        std::vector<int> neighbours = m.neighbours(i);
        assert(!neighbours.empty());

        //on calcule la contribution de chaque voisin
        for(int j = 0; j< neighbours.size(); j++){
            vec3 A = m.vertices[i].coord;
            vec3 B = m.vertices[neighbours[j]].coord;

            //on récupère le voisin précédent pour calculer la première cotangente
            int size = static_cast<int>(neighbours.size());
            vec3 C = m.vertices[neighbours[(j-1+size)%size]].coord;
            vec3 CA = A - C;
            vec3 CB = B - C;

            double crossCABnorm = vec3::Cross(CA, CB).norm();
            double dotCAB = vec3::Dot(CA,CB);
            
            //on ajoute l'aire du triangle de gauche à l'aire totale
            double leftSurface = crossCABnorm/2.0f;
            surfaceSum+=leftSurface;

            assert(crossCABnorm!=0);
            double cotanC = dotCAB/crossCABnorm;

            //on récupère le voisin suivant pour calculer la deuxième cotangente
            vec3 D = m.vertices[neighbours[(j+1)%size]].coord;
            vec3 DA = A - D;
            vec3 DB = B - D;

            double crossDABnorm = vec3::Cross(DA, DB).norm();
            double dotDAB = vec3::Dot(DA,DB);

            
            assert(crossDABnorm!=0);
            double cotanD = dotDAB/crossDABnorm;

            //on ajoute la différence de chaleur pondérée par les deux cotangentes
            sum+=(cotanC + cotanD) * (heatValues[neighbours[j]] - heatValues[i]);
        }

        //on calcule le laplacien en divisant la somme par l'aire autour du sommet
        surfaceSum/=3.0f;
        assert(surfaceSum!=0);
        laplacian[i] = sum/(2.0f*surfaceSum);
    }
}

void ThermalState::step(const Mesh& m,  double deltaTime){
    computeLaplacian(m);
    for(size_t i =0; i<heatValues.size(); i++){
        heatValues[i] = heatValues[i] + laplacian[i] *  deltaTime;
        //if(heatValues[i]!=0)std::cout <<heatValues[i]<<std::endl;
    }
    generation++;
}

void ThermalState::WriteCOFF(const Mesh& mesh, const std::string& filePath){
    std::string fileContent = "COFF\n";
    fileContent += std::to_string(mesh.vertices.size()) + " ";
    fileContent += std::to_string(mesh.faces.size()) + " 0\n";

    for(size_t i = 0; i < mesh.vertices.size(); i++){
        fileContent += mesh.vertices[i].coord.str() + " " + getColor(i).str() + "\n";
    }

    for(size_t i = 0; i < mesh.faces.size(); i++){
        fileContent += mesh.faces[i].str() + "\n";
    }

    const std::filesystem::path inputPath(filePath);
    const std::filesystem::path outputPath = inputPath.parent_path() /
        (inputPath.stem().string() + "_" + std::to_string(generation) + ".off");
    std::ofstream coffFile(outputPath);

    if(coffFile.is_open()){
        coffFile << fileContent;
        std::cout << "COFF file saved successfully as " << outputPath.string() << std::endl;
    }
    else{
        std::cerr << "Error while opening file : " << outputPath.string() << std::endl;
    }
    coffFile.close();
}

vec3 ThermalState::getColor(int i){//Using only values between 1000 and 0 degrees
    const double value = std::clamp(heatValues[i],0.0,100.0);
    const vec3 blue(0.0f, 0.0f, 1.0f);
    const vec3 cyan(0.0f, 1.0f, 1.0f);
    const vec3 green(0.0f, 1.0f, 0.0f);
    const vec3 yellow(1.0f, 1.0f, 0.0f);
    const vec3 red(1.0f, 0.0f, 0.0f);

    if (value < 25.0f) return vec3::Lerp(blue, cyan, value / 25.0f);
    if (value < 50.0f) return vec3::Lerp(cyan, green, (value - 25.0f) / 25.0f);
    if (value < 75.0f) return vec3::Lerp(green, yellow, (value - 50.0f) / 25.0f);
    return vec3::Lerp(yellow, red, (value - 75.0f) / 25.0f);
}