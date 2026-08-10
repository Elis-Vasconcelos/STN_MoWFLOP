#ifndef STN_LOGGER_H
#define STN_LOGGER_H

#include <fstream>
#include <string>
#include <vector>

#include "../global_modules/generate_initial_population/generate_rSolution.h"

// Identificador da execução independente, estampado em toda linha do CSV.
// Definido nos main (moead.cpp / nsga2.cpp), lido de argv[5].
extern int stn_run_id;

// Registra, a cada geração, a solução representativa de cada vetor de peso
// da decomposição -- os dados crus a partir dos quais uma Search Trajectory
// Network é montada depois.
//
// O logger não conhece o algoritmo: recebe as soluções representativas já
// escolhidas, uma por vetor, na mesma ordem dos vetores de peso. No MOEA/D
// isso é a própria população (population[j] é a melhor solução do
// subproblema j, alinhada com lambda_vector[j]). Em algoritmos sem vetores
// de peso, como o NSGA-II, o chamador seleciona as representativas antes.
//
// Cada linha é um (run, vetor, geração): os dois objetivos mais os índices
// globais das posições ocupadas (geometria crua, um índice por turbina
// colocada). A assinatura de ocupação que particiona o espaço de busca em
// nós da STN, parametrizada pelo lado da célula, é deliberadamente não
// calculada aqui -- é escolha de pós-processamento, e guardar as posições
// cruas permite recalculá-la em qualquer resolução sem reexecutar.
class STNLogger {
    public:
        // Escreve <file_prefix>_stn.csv (a trajetória) e, de uma vez,
        // <file_prefix>_candidates.csv (a tabela índice global -> x, y que
        // decodifica a coluna occupied).
        STNLogger(const std::string& file_prefix);
        ~STNLogger();

        void log(int generation, const std::vector<Solution>& representatives);
        void log(int generation, const std::vector<Solution*>& representatives);

    private:
        void write_row(int generation, int vector_id, const Solution& solution);
        std::vector<int> occupied_global_indices(const Solution& solution);
        void write_candidate_table(const std::string& file_path);

        std::ofstream out;
        // índice global de uma posição = zone_offset[zona] + índice na zona
        std::vector<int> zone_offset;
        // última ocupação escrita por vetor, para omitir linhas repetidas
        std::vector<std::vector<int>> last_occupied;
};

#endif
