#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "../../headers/utils/stn_logger.h"

using namespace std;

STNLogger::STNLogger(const string& file_prefix){

  // calcula offset de cada zona para guardar índice global de posições ocupadas
  zone_offset.resize(num_zones);

  int offset = 0;
  for(int z = 0; z < num_zones; z++){
    zone_offset[z] = offset;
    // o offset da próxima zona é o offset atual mais o número de posições da zona atual
    offset += (int) foundations[z].size();
  }

  // o conteúdo só depende da instância (foundations), então é igual em toda
  // run desse mesmo instance+algorithm -- gerar de novo a cada run é
  // trabalho redundante, então só escreve se ainda não existir
  string candidates_path = file_prefix + "_candidates.csv";
  if(!ifstream(candidates_path).good()){
    write_candidate_table(candidates_path);
  }

  out.open(file_prefix + "_stn.csv");

  if(!out.is_open()){
    cerr << "STN File Path Error: " << file_prefix << "_stn.csv" << endl;
    return;
  }

  // "\n" em vez de endl: este é o arquivo de maior volume da execução e não
  // vale dar flush a cada linha; o destrutor fecha o stream
  out << "run_id,vector_id,generation,f_cost,f_power,occupied" << "\n";
  out << fixed << setprecision(6);
}

STNLogger::~STNLogger(){
  out.close();
}

void STNLogger::log(int generation, const vector<Solution>& representatives){
  last_occupied.resize(representatives.size());

  for(int j = 0; j < (int) representatives.size(); j++){
    write_row(generation, j, representatives[j]);
  }
}

void STNLogger::log(int generation, const vector<Solution*>& representatives){
  last_occupied.resize(representatives.size());

  // escreve uma linha csv pra cada solução representante
  for(int j = 0; j < (int) representatives.size(); j++){
    write_row(generation, j, *representatives[j]);
  }
}

void STNLogger::write_row(int generation, int vector_id, const Solution& solution){

  vector<int> occupied = occupied_global_indices(solution);

  // uma solução representante inalterada ocupa a mesma localização da linha
  // anterior daquele vetor: não acrescenta nada à trajetória e é omitida
  if(occupied == last_occupied[vector_id]){
    return;
  }

  last_occupied[vector_id] = occupied;

  // fitness.first é o custo negado (os dois objetivos são maximizados
  // internamente); o CSV grava custo positivo, a ser minimizado
  out << stn_run_id << "," << vector_id << "," << generation << ","
      << -solution.fitness.first << "," << solution.fitness.second << ",";

  
  // localização (índices globais) das turbinas alocadas dessa solução
  // para identificar x,y precisa ler em candidates.csv
  for(int i = 0; i < (int) occupied.size(); i++){
    if(i > 0){
      out << " ";
    }
    out << occupied[i];
  }

  out << "\n";
}

vector<int> STNLogger::occupied_global_indices(const Solution& solution){

  vector<int> occupied;

  for(int z = 0; z < num_zones; z++){
    for(int i = 0; i < (int) solution.turbines[z].size(); i++){
      const Turbine& turbine = solution.turbines[z][i];
      occupied.push_back(zone_offset[turbine.zone] + turbine.index);
    }
  }

  // ordenar torna a coluna diretamente comparável entre linhas, tanto para
  // omitir repetições aqui quanto para agrupar layouts no pós-processamento
  sort(occupied.begin(), occupied.end());

  return occupied;
}

void STNLogger::write_candidate_table(const string& file_path){

  ofstream file(file_path);

  if(!file.is_open()){
    cerr << "STN File Path Error: " << file_path << endl;
    return;
  }

  file << "global_index,zone,zone_index,x,y" << "\n";
  file << fixed << setprecision(6);

  for(int z = 0; z < num_zones; z++){
    for(int i = 0; i < (int) foundations[z].size(); i++){
      file << zone_offset[z] + i << "," << z << "," << i << ","
           << foundations[z][i].x << "," << foundations[z][i].y << "\n";
    }
  }

  file.close();
}
