#include "./headers/main.h"
#include "./headers/globals.h"

#include <iostream>
#include <string>
using namespace std;

int countRevalue = 0;

BoundedParetoSet * pareto = new BoundedParetoSet();
int stop_criteria = 1000000;
int stn_run_id = 0;
string algorithm = "nsga2";
string instance = "A";
string root_folder = "./";

int main(int argc, char* argv[]){

    if(argc == 2){
        instance = argv[1];
    } else if (argc > 2){
        instance = argv[1];
        root_folder = argv[2];
    }

    // argv[3] e argv[4] (ângulo e vento) são lidos por get_instance_info
    if(argc >= 6){
        stn_run_id = stoi(argv[5]);
    }

    // baixar o critério de parada permite validar localmente o log da STN
    // sem esperar o milhão de avaliações da execução completa
    if(argc >= 7){
        stop_criteria = stoi(argv[6]);
    }

    string path;

    int num_neighbors = 10;

    get_instance_info(argc, argv);

    int sum = 0;
    for(auto elem : turbines_per_zone)
        sum += elem;

    cout << endl;
    cout << "Number of subproblems: " << SIZE_OF_POPULATION << endl;
    cout << "Number of neighbors: " << num_neighbors << endl;
    cout << "Number of fixed turbines: " << fixd.size() << endl;
    cout << "Number of mobile turbines: " << sum << endl;
    cout << "Wind: " << wind << endl;
    cout << "Angle: " << angle << endl;
    cout << "Run id: " << stn_run_id << endl;
    cout << "Stop criteria: " << stop_criteria << endl << endl;

    cout << "Run time:" << endl;
    
    auto population = create_initial_population(SIZE_OF_POPULATION);
    nsga2(population);
}