"""Gera (angle, wind) por (instancia, algoritmo, run_id) pras instancias
esparsas.

Duas fontes, dependendo da familia da instancia:

- "same_geometry" (178_r1e-04/05, 101_r1e-04/05): reusa os sorteios REAIS
  que o CEC2026 usou pra ns178/ns101 (20 runs cada, extraidos direto de
  raw_results/wflopcec26_results/ -- dado real, nao inventado). As duas
  variantes de densidade (r1e-04/r1e-05) do mesmo site reusam o MESMO
  conjunto de 20 sorteios do site original, mantendo a densidade como
  unica variavel entre elas (mesma logica de "same_geometry" documentada
  em wflop_instances/README.md).

- "fresh_geometry" (23t_*, 63t_*): sites sinteticos novos, sem
  contrapartida real no CEC -- nao ha dado real pra reusar. Sorteio
  ponderado pela rosa dos ventos (RVO_TNW.txt, mesmo arquivo documentado
  por Cazzaro & Pisinger como usado no benchmark do paper -- ver
  wflop_instances/README.md), reimplementando a logica do bloco comentado
  em instance_info.cpp (nenhuma versao ativa existe em lugar nenhum que
  checamos). Sem seed fixa -- usa entropia real do sistema; o CSV gerado
  vira o registro permanente (nao é pra ser regenerado depois, igual o
  dado do CEC tambem nao é reprodutivel a partir do codigo deles, que
  nao existe mais).

Uso: python3 sample_wind_rose.py > sparse_wind_map.csv
"""
import csv
import random
import sys
from pathlib import Path

WIND_FILE = "/home/elis/Projects/TCC/STN_MoWFLOP/wflop_instances/wind/RVO_TNW.txt"
CEC_ROOT = Path("/home/elis/Projects/TCC/STNs-MOCO-MoWFLOP/raw_results/wflopcec26_results")
N_RUNS = 20
ALGOS = ["moead", "nsga2"]

SAME_GEOMETRY = {
    "178_r1e-04": "178",
    "178_r1e-05": "178",
    "101_r1e-04": "101",
    "101_r1e-05": "101",
}
FRESH_GEOMETRY = [
    "23t_01_r1e-04", "23t_02_r1e-04", "23t_03_r1e-04",
    "23t_01_r1e-05", "23t_02_r1e-05", "23t_03_r1e-05",
    "63t_01_r1e-04", "63t_02_r1e-04", "63t_03_r1e-04",
    "63t_01_r1e-05", "63t_02_r1e-05", "63t_03_r1e-05",
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


def cec_real_draw(algo, real_instance, run_id):
    """run_id 0-19 -> pasta de run 1-20 do CEC (dado real, nao inventado)."""
    log = CEC_ROOT / algo / f"ns{real_instance}" / str(run_id + 1) / "log.txt"
    angle = wind = None
    with open(log) as f:
        for line in f:
            if line.startswith("Wind:"):
                wind = float(line.split(":", 1)[1].strip())
            elif line.startswith("Angle:"):
                angle = float(line.split(":", 1)[1].strip())
    assert angle is not None and wind is not None, f"could not parse {log}"
    return angle, wind


def main():
    rows = load_wind_rose(WIND_FILE)
    rng = random.SystemRandom()  # entropia real, sem seed fixa

    writer = csv.writer(sys.stdout, lineterminator="\n")
    writer.writerow(["instance", "algo", "run_id", "angle", "wind", "source"])

    for instance, real_instance in SAME_GEOMETRY.items():
        for algo in ALGOS:
            for run_id in range(N_RUNS):
                angle, wind = cec_real_draw(algo, real_instance, run_id)
                writer.writerow([instance, algo, run_id, f"{angle:.12f}", f"{wind:.12f}", f"cec_real_ns{real_instance}"])

    for instance in FRESH_GEOMETRY:
        for algo in ALGOS:
            for run_id in range(N_RUNS):
                angle, wind = sample_one(rows, rng)
                writer.writerow([instance, algo, run_id, f"{angle:.12f}", f"{wind:.12f}", "sampled_tnw"])


if __name__ == "__main__":
    main()
