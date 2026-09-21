#include "ThermalState.h"
#include <filesystem>


void ThermalState::computeLaplacian(const Mesh& m){
    for(int i = 0; i < m.vertices.size(); i++){
        float sum = 0;
        float surfaceSum = 0;
        //Pour chaque voisin
        std::vector<int> neighbours = m.neighbours(i);
        assert(!neighbours.empty());

        for(int j = 0; j< neighbours.size(); j++){
            vec3 A = m.vertices[i].coordinates;
            vec3 B = m.vertices[neighbours[j]].coordinates;

            //calcule cotangeante de voisin -1 
            //BUG probleme de type entre size et le modulo + modulo d'unn bail negatif
            int size = static_cast<int>(neighbours.size());
            vec3 C = m.vertices[neighbours[(j-1+size)%size]].coordinates;
            vec3 CA = A - C;
            vec3 CB = B - C;

            float crossCABnorm = vec3::Cross(CA, CB).norm();
            float dotCAB = vec3::Dot(CA,CB);
            
            //on prend l'air de la face de gauche au passage 
            float leftSurface = crossCABnorm/2.0f;
            surfaceSum+=leftSurface;

            assert(crossCABnorm!=0);
            float cotanC = dotCAB/crossCABnorm;

            //calcule cotangeante de voisin +1 
            vec3 D = m.vertices[neighbours[(j+1)%size]].coordinates;
            vec3 DA = A - D;
            vec3 DB = B - D;

            float crossDABnorm = vec3::Cross(DA, DB).norm();
            float dotDAB = vec3::Dot(DA,DB);

            
            assert(crossDABnorm!=0);
            float cotanD = dotDAB/crossDABnorm;

            //Difference entre les deux mult la diff entre les deux values
            sum+=(cotanC + cotanD) * (heatValues[neighbours[j]] - heatValues[i]);
            if(sum!=0)std::cout << heatValues[neighbours[j]] << " "<<  heatValues[i] << " " << sum << std::endl;
        }
        surfaceSum/=3.0f;
        assert(surfaceSum!=0);
        laplacian[i] = sum/(2.0f*surfaceSum);
        if(sum!=0) std::cout <<"ahaha "<<laplacian[i]<<std::endl;
    }
}

void ThermalState::step(const Mesh& m,  float deltaTime){
    computeLaplacian(m);
    for(size_t i =0; i<heatValues.size(); i++){
        heatValues[i] = heatValues[i] + laplacian[i] *  deltaTime;
        if(heatValues[i]!=0)std::cout <<heatValues[i]<<std::endl;
    }
    generation++;
}

void ThermalState::WriteCOFF(const Mesh& mesh, const std::string& filePath){
    std::string fileContent = "COFF\n";
    fileContent += std::to_string(mesh.vertices.size()) + " ";
    fileContent += std::to_string(mesh.faces.size()) + " 0\n";

    for(size_t i = 0; i < mesh.vertices.size(); i++){
        fileContent += mesh.vertices[i].coordinates.str() + " " + getColor(i).str() + "\n";
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

vec3 ThermalState::getColor(int i){//Using only values between 100 and 0 degrees
    float value = heatValues[i];
    if(value<25){
        return vec3::Lerp(vec3(0,0,0), vec3(64/255.0f,0,0), value/25.0f);
    }
    else if(value<50){
        return vec3::Lerp(vec3(64.0f/255.0f,0,0), vec3(128.0f/255.0f,0,0), (value-25.0f)/25.0f);
    }    
    else if(value<75){
        return vec3::Lerp(vec3(128.0f/255.0f,0,0), vec3(191.0f/255.0f,0,0), (value-50.0f)/25.0f);
    }    
    else{
        return vec3::Lerp(vec3(191.0f/255.0f,0,0), vec3(255.0f/255.0f,0,0), (value-75.0f)/25.0f);
    }
}