#include "Mesh.h"
#include "ThermalState.h"
#include <iostream>

int main(int argc, char const *argv[])
{
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
