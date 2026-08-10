#ifndef STN_LOGGER_H
#define STN_LOGGER_H

#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "../global_modules/generate_initial_population/generate_rSolution.h"

// Identificador da execução independente, estampado em toda linha do CSV.
// Definido nos main (moead.cpp / nsga2.cpp), lido de argv[5].
extern int stn_run_id;

// Registra, a cada STN_LOGGER_INTERVAL gerações, a solução representativa
// de cada vetor de peso da STN -- os dados crus a partir dos quais uma
// Search Trajectory Network é montada depois.
//
// O logger não conhece o algoritmo: recebe as soluções representativas já
// escolhidas, uma por vetor, na mesma ordem dos vetores de peso -- sempre
// via select_representatives (abaixo), já que os p vetores da STN são um
// conjunto próprio, menor e mais esparso que os SIZE_OF_POPULATION vetores
// internos do MOEA/D, então mesmo lá a representativa de cada vetor da STN
// precisa ser escolhida, não sai direto da população.
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

        void log(int generation, const std::vector<Solution*>& representatives);

    private:
        void write_row(int generation, int vector_id, const Solution& solution);
        std::vector<int> occupied_global_indices(const Solution& solution);
        void write_candidate_table(const std::string& file_path);

        std::ofstream out;
        // índice global de uma posição = zone_offset[zona] + índice na zona
        std::vector<int> zone_offset;
};

// Escolhe, para cada vetor de peso, o membro da população que minimiza a
// escalarização de Chebyshev daquele vetor -- a solução representativa que
// o log da STN registra. Um observador externo em ambos os algoritmos: nem
// MOEA/D nem NSGA-II decompõem o problema nos p vetores próprios da STN
// (o MOEA/D decompõe nos seus SIZE_OF_POPULATION vetores internos, um
// conjunto diferente), então a seleção roda por cima da população corrente
// dos dois do mesmo jeito.
std::vector<Solution*> select_representatives(
    std::vector<Solution>& population,
    std::vector<std::pair<double, double>>& lambda_vector,
    std::pair<double, double>& z_point);

std::vector<Solution*> select_representatives(
    std::vector<Solution*>& population,
    std::vector<std::pair<double, double>>& lambda_vector,
    std::pair<double, double>& z_point);

#endif
