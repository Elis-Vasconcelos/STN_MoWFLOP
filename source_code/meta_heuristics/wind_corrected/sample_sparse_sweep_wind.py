"""Gera (angle, wind) por (instancia, algoritmo, run_id) pras 8 instancias
do single-site sparsity sweep (506_e-02 .. 513_e-05, RQ1).

Sitio sintetico novo, sem contrapartida real no CEC -> nao ha dado real
pra reusar (diferente de 178_/101_, que reusam os sorteios reais do CEC).
Sorteio ponderado pela rosa dos ventos RVO_TNW.txt (mesmo arquivo do
benchmark Cazzaro & Pisinger), reimplementando a logica do bloco
comentado em instance_info.cpp -- exatamente o mesmo caminho
"fresh_geometry" de sample_wind_rose.py.

Sem seed fixa -- usa entropia real do sistema (random.SystemRandom). O CSV
gerado e o REGISTRO PERMANENTE: nao e pra ser regenerado depois (cada
regeracao produz sorteios diferentes). Versione o CSV, nao confie em
reproduzir a partir deste script.

Uso: python3 sample_sparse_sweep_wind.py > sparse_wind_map_506-513.csv
"""
import csv
import random
import sys

WIND_FILE = "/home/elis/Projects/TCC/STN_MoWFLOP/wflop_instances/wind/RVO_TNW.txt"
N_RUNS = 20
ALGOS = ["moead", "nsga2"]
INSTANCES = [
    "506_e-02", "507_e-03", "508_e-04", "509_e-05",
    "510_e-02", "511_e-03", "512_e-04", "513_e-05",
]


def load_wind_rose(path):
    rows = []
    with open(path) as f:
        for line in f:
            parts = line.split()
            if len(parts) != 3:
                continue
            angle, wind, p = float(parts[0]), float(parts[1]), float(parts[2])
            rows.append((angle, wind, p))
    total = sum(p for _, _, p in rows)
    assert 0.99 <= total <= 1.01, f"probabilities sum to {total}, expected ~1.0"
    return rows


def sample_one(rows, rng):
    r = rng.random()
    acc = 0.0
    for angle, wind, p in rows:
        acc += p
        if acc > r:
            return angle, wind
    return rows[-1][0], rows[-1][1]


def main():
    rows = load_wind_rose(WIND_FILE)
    rng = random.SystemRandom()  # entropia real, sem seed fixa

    writer = csv.writer(sys.stdout, lineterminator="\n")
    writer.writerow(["instance", "algo", "run_id", "angle", "wind", "source"])

    for instance in INSTANCES:
        for algo in ALGOS:
            for run_id in range(N_RUNS):
                angle, wind = sample_one(rows, rng)
                writer.writerow([
                    instance, algo, run_id,
                    f"{angle:.12f}", f"{wind:.12f}", "sampled_tnw",
                ])


if __name__ == "__main__":
    main()
