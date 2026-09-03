#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "../../headers/globals.h"
#include "../../headers/global_modules/generate_initial_population/population.h"
#include "../../headers/metaheuristics/moead/modules/tchebycheff.h"
#include "../../headers/utils/stn_logger.h"

using namespace std;

STNLogger::STNLogger(const string& file_prefix, const vector<pair<double, double>>& lambda_vector) : lambda_vector(lambda_vector){

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
  //
  // algorithm/instance/iteration adicionados pra bater com os campos
  // mínimos da Seção 10.2 do STN_MoWFLOP.pdf (antes só existiam via nome
  // de arquivo, não como coluna literal). generation continua além do
  // mínimo exigido -- registra a geração bruta, útil pra depuração e pro
  // cálculo de nGen por run no lado R -- "iteration" é o índice sequencial
  // de gravação que o documento pede especificamente (não a geração bruta).
  out << "algorithm,instance,run_id,vector_id,generation,iteration,f_cost,f_power,weight1,weight2,occupied" << "\n";
  out << fixed << setprecision(6);
}

STNLogger::~STNLogger(){
  out.close();
}

void STNLogger::log(int generation, const vector<Solution*>& representatives){

  // amostragem por intervalo: só registra 1 em cada STN_LOGGER_INTERVAL
  // gerações -- generation 0 (população inicial) sempre cai aqui
  if(generation % STN_LOGGER_INTERVAL != 0){
    return;
  }

  // escreve uma linha csv pra cada solução representante
  for(int j = 0; j < (int) representatives.size(); j++){
    write_row(generation, j, *representatives[j]);
  }

  // incrementa só depois de gravar essa geração inteira (todos os
  // vetores) -- record_index conta gravações, não linhas
  record_index++;
}

void STNLogger::write_row(int generation, int vector_id, const Solution& solution){

  vector<int> occupied = occupied_global_indices(solution);

  // fitness.first é o custo negado (os dois objetivos são maximizados
  // internamente); o CSV grava custo positivo, a ser minimizado
  out << algorithm << "," << instance << "," << stn_run_id << "," << vector_id << "," << generation << "," << record_index << ","
      << -solution.fitness.first << "," << solution.fitness.second << ","
      << lambda_vector[vector_id].first << "," << lambda_vector[vector_id].second << ",";

  
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

// overload para MOEA/D, cuja população é vector<Solution>
vector<Solution*> select_representatives(vector<Solution>& population, vector<pair<double, double>>& lambda_vector, pair<double, double>& z_point){

  vector<Solution*> representatives(lambda_vector.size());

  // um representante por vetor de peso (lambda) da STN
  for(int j = 0; j < (int) lambda_vector.size(); j++){

    // busca linear pelo indivíduo com menor tchebycheff (calculate_gte)
    // para esse lambda -- esse é o "melhor" sob aquele vetor de peso
    Solution* best = &population[0];
    double best_gte = calculate_gte(best->fitness, lambda_vector[j], z_point);

    for(int i = 1; i < (int) population.size(); i++){
      double gte = calculate_gte(population[i].fitness, lambda_vector[j], z_point);
      if(gte < best_gte){
        best_gte = gte;
        best = &population[i];
      }
    }

    representatives[j] = best;
  }

  return representatives;
}

// overload idêntico ao de cima, mas para NSGA-II, cuja população é vector<Solution*> em vez de vector<Solution>.
vector<Solution*> select_representatives(vector<Solution*>& population, vector<pair<double, double>>& lambda_vector, pair<double, double>& z_point){

  vector<Solution*> representatives(lambda_vector.size());

  for(int j = 0; j < (int) lambda_vector.size(); j++){

    Solution* best = population[0];
    double best_gte = calculate_gte(best->fitness, lambda_vector[j], z_point);

    for(int i = 1; i < (int) population.size(); i++){
      double gte = calculate_gte(population[i]->fitness, lambda_vector[j], z_point);
      if(gte < best_gte){
        best_gte = gte;
        best = population[i];
      }
    }

    representatives[j] = best;
  }

  return representatives;
}
