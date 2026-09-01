# Campanha esparsa 506–513 no servidor Bambu — instruções de execução

**RQ1 / sweep de densidade single-site.** Rodar MOEA/D e NSGA-II nas 8
instâncias `506_e-02 … 513_e-05` (geometria única compartilhada, só τ e
densidade τ/|P| variam), com `(angle, wind)` sorteado da rosa dos ventos
por execução, e coletar as trajetórias STN.

Tudo aqui é pra **você** rodar no servidor — eu não acesso o Bambu. Onde
tiver `<...>` troque pelo valor real.

---

## 0. Parâmetros da campanha (fixados)

| | |
|---|---|
| Instâncias | `506_e-02 507_e-03 508_e-04 509_e-05 510_e-02 511_e-03 512_e-04 513_e-05` |
| Algoritmos | `moead`, `nsga2` |
| Execuções por (instância, algo) | **20** (`run_id` 0–19) |
| `stop_criteria` | **1 000 000** avaliações |
| STN `stn_p` (nº de vetores observadores) | **100** |
| STN `stn_interval` (generations entre amostras) | **50** |
| `(angle, wind)` | pré-sorteado, uma tupla por (instância, algo, run_id) — arquivo `sparse_wind_map_506-513.csv` |
| Total de combinações | 8 × 2 × 20 = **320** |
| Servidor | **bambu1** — `bambu-server1.freeddns.org`, SSH porta **4522**, auth por senha |
| Saída | `raw_results/meta_heuristics_stn_windcorrected/<algo>/<instance>/p100_i50/<run_id>/` |

`stn_p=100` é **um único valor** (a campanha STN10 original varria 10/50/100;
esta não — P não afeta a busca, só a densidade do instrumento, e 100 é o
teto já usado). Então são 320 combinações, não 960.

### Origem do `(angle, wind)`

Sorteio ponderado pela rosa dos ventos `wflop_instances/wind/RVO_TNW.txt`
(mesmo arquivo do benchmark Cazzaro & Pisinger), uma tupla independente
por (instância, algo, run_id) — exatamente o caminho "fresh_geometry" do
`sample_wind_rose.py`: site sintético novo, sem contrapartida real no
CEC2026 pra reusar. Gerado **sem seed fixa** (`random.SystemRandom`), então
**o CSV `sparse_wind_map_506-513.csv` é o registro permanente** — não é pra
regenerar depois (cada regeração dá sorteios diferentes). Já está no bundle;
versione junto com os resultados.

Gerador: `source_code/meta_heuristics/wind_corrected/sample_sparse_sweep_wind.py`
(só pra rastreabilidade do método — **não rode de novo**).

---

## 1. Montar o bundle de transferência (na sua máquina local)

O servidor não precisa dos 20 GB do repositório. Só de:

- `source_code/meta_heuristics/` (código C++ + `scripts/` + `wind_corrected/`) — ~3 MB
- as 8 pastas de instância sob `wflop_instances/sparse_instances/single_site_sweep/` — ~21 MB

```bash
cd /home/elis/Projects/TCC/STN_MoWFLOP

# garante que o wind map está gerado (321 linhas: 1 header + 320)
wc -l source_code/meta_heuristics/wind_corrected/sparse_wind_map_506-513.csv

tar czf /tmp/sparse_campaign_bundle.tar.gz \
  --exclude='source_code/meta_heuristics/moead' \
  --exclude='source_code/meta_heuristics/nsga2' \
  --exclude='*.o' \
  source_code/meta_heuristics \
  wflop_instances/sparse_instances/single_site_sweep

ls -lh /tmp/sparse_campaign_bundle.tar.gz   # ~25 MB
```

Transferir (porta 4522, mesma senha do SSH):

```bash
sftp -P 4522 <user>@bambu-server1.freeddns.org
# no prompt do sftp:
put /tmp/sparse_campaign_bundle.tar.gz
bye
```

---

## 2. Preparar o layout no servidor

```bash
ssh -p 4522 <user>@bambu-server1.freeddns.org

mkdir -p ~/STN_MoWFLOP && cd ~/STN_MoWFLOP
tar xzf ~/sparse_campaign_bundle.tar.gz

# dirs que os scripts esperam existir (symlinks e saída são criados on-demand)
mkdir -p instances/site raw_results source_code/logs

# confere a árvore mínima
find . -maxdepth 3 -type d | sort
```

Deve resultar em, no mínimo:

```
./instances/site
./raw_results
./source_code/logs
./source_code/meta_heuristics
./source_code/meta_heuristics/scripts
./source_code/meta_heuristics/wind_corrected
./wflop_instances/sparse_instances/single_site_sweep
./wflop_instances/sparse_instances/single_site_sweep/506_e-02
...  (508..513)
```

`run_one_windcorrected.sh` acha cada instância com
`find ../wflop_instances -maxdepth 4 -type d -name "<instance>"` e cria o
symlink `instances/site/<instance>` na primeira vez que ela roda — por isso
`instances/site/` só precisa existir vazio.

---

## 3. Container (podman) — build do C++

Host é Debian 6.1, **só podman** (docker proibido), **sem Python de sistema,
sem instalar pacote no host**. Todo o build e execução acontece dentro de um
container. Precisa só de `g++` + `make` (o `Makefile` usa
`g++ -std=c++17 -O2 -Werror`; nenhuma lib externa).

`~/STN_MoWFLOP/Containerfile`:

```dockerfile
FROM debian:bookworm-slim
RUN apt-get update && \
    apt-get install -y --no-install-recommends build-essential ca-certificates && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /work/source_code
```

Build da imagem:

```bash
cd ~/STN_MoWFLOP
podman build -t sparse-campaign -f Containerfile .
```

Compilar os binários (dentro do container, montando o repo; `:Z` reetiqueta
SELinux, inofensivo se não houver):

```bash
podman run --rm -v ~/STN_MoWFLOP:/work:Z -w /work/source_code/meta_heuristics \
  sparse-campaign make rebuild
```

Confere:

```bash
ls -l ~/STN_MoWFLOP/source_code/meta_heuristics/{moead,nsga2}
file ~/STN_MoWFLOP/source_code/meta_heuristics/moead   # ELF 64-bit executable
```

---

## 4. Smoke test antes da campanha inteira

Roda a instância mais pesada (`513_e-05`, 185 054 posições) com um
`stop_criteria` minúsculo, os dois algoritmos, 1 run — pra (a) confirmar que
o pipeline funciona ponta-a-ponta no container e (b) medir tempo e
extrapolar.

```bash
podman run --rm -v ~/STN_MoWFLOP:/work:Z -w /work/source_code sparse-campaign \
  bash -c '
    time ./meta_heuristics/scripts/run_one_windcorrected.sh 513_e-05 moead 0 210 11 2000 100 50
    time ./meta_heuristics/scripts/run_one_windcorrected.sh 513_e-05 nsga2 0 210 11 2000 100 50
  '
```

(`210 11` = um `(angle, wind)` qualquer pro teste — o smoke test não vai pro
resultado final; apaga a pasta depois.)

Validação do CSV STN gerado (rode de `source_code/`, dentro do container ou
com `podman run ... bash -c`):

```bash
f=raw_results/meta_heuristics_stn_windcorrected/moead/513_e-05/p100_i50/0/513_e-05_moead_stn.csv

# coluna 11: cada linha deve ter exatamente τ turbinas ocupadas
#   τ = 5  pra 506–509 ;  τ = 15 pra 510–513   (aqui 513 -> 15)
awk -F',' 'NR>1{n=split($11,a," "); if(n!=15) print "linha "NR": "n" turbinas"}' "$f"

# coluna 5: generations amostradas devem ser 0,50,100,...
awk -F',' 'NR>1{print $5}' "$f" | sort -n -u | head

# coluna 6: iteration sequencial 0,1,2,...
awk -F',' 'NR>1{print $6}' "$f" | sort -n -u | head

# nenhum vector_id (col 4) com generation (col 5) repetida ou fora de ordem
awk -F',' 'NR>1{print $4","$5}' "$f" | sort -t, -k1,1n -k2,2n | \
  awk -F',' 'BEGIN{pv=-1} pv==$1 && $2<=pg {print "ordem quebrada no vetor "$1} {pv=$1; pg=$2}'
```

Silêncio em todos = log consistente. Depois:

```bash
rm -rf raw_results/meta_heuristics_stn_windcorrected/{moead,nsga2}/513_e-05
```

### Extrapolação de tempo (referência)

Medição local: `nsga2` em `513_e-05` ≈ **75 s / 2000 avaliações** → ~10 h por
run completo de 1e6 (escala ~linear). MOEA/D historicamente ~3× mais rápido
que NSGA-II nessas instâncias.

| Instância | posições | ~tempo/run (1e6 evals) |
|---|---|---|
| 506/507/510/511 (e-02, e-03) | 60–2 548 | segundos a poucos minutos |
| 508_e-04 | 6 000 | ~0,3–0,5 h |
| 512_e-04 | 19 505 | ~1–1,5 h |
| 509_e-05 | 60 000 | ~3–4 h |
| 513_e-05 | 185 054 | ~10 h (MOEA/D) / ~30 h (NSGA-II) |

`513_e-05` domina o custo: 40 runs (20 × 2 algos) ≈ 700–900 CPU-h só nele.
Campanha inteira ≈ **1 000–1 500 CPU-h** → **~1–2 dias de wall-clock** com
~55 núcleos. Ajuste o smoke test acima se a medição real do servidor destoar
muito disso antes de lançar.

---

## 5. Gerar a lista de 320 combinações

De `~/STN_MoWFLOP/source_code`:

```bash
cd ~/STN_MoWFLOP/source_code
WMAP=meta_heuristics/wind_corrected/sparse_wind_map_506-513.csv

tail -n +2 "$WMAP" | awk -F',' '{print $1, $2, $3, $4, $5}' > /tmp/sparse_combos.txt
wc -l /tmp/sparse_combos.txt        # espera 320
head -3 /tmp/sparse_combos.txt      # ex: 506_e-02 moead 0 210.000000000000 11.000000000000
```

Cada linha vira 5 campos: `<instance> <algo> <run_id> <angle> <wind>`.
`stop_criteria`, `stn_p`, `stn_interval` (1000000 / 100 / 50) são fixos e
entram no comando do `xargs`, não no arquivo.

---

## 6. Lançar a campanha

Dentro de um container que fica vivo até acabar. `-P 55` deixa folga sob o
limite de cgroup do usuário (62 núcleos); cada processo é single-core e usa
<20 MB, então RAM não é gargalo.

```bash
cd ~/STN_MoWFLOP

setsid nohup podman run --rm -v ~/STN_MoWFLOP:/work:Z -w /work/source_code \
  sparse-campaign \
  xargs -a /tmp/sparse_combos.txt -n 5 -P 55 \
    bash -c './meta_heuristics/scripts/run_one_windcorrected.sh "$0" "$1" "$2" "$3" "$4" 1000000 100 50' \
  > source_code/logs/sparse_campaign_launcher.log 2>&1 < /dev/null &
disown
```

Notas:
- `/tmp/sparse_combos.txt` está no host; o container monta só `~/STN_MoWFLOP`.
  Ou copie o arquivo pra dentro do repo (`cp /tmp/sparse_combos.txt
  ~/STN_MoWFLOP/source_code/` e aponte pra `sparse_combos.txt`), ou monte
  `/tmp` também (`-v /tmp/sparse_combos.txt:/tmp/sparse_combos.txt:ro`). A
  segunda opção é mais simples.
- `run_one_windcorrected.sh` é **idempotente**: se `<...>_stn.csv` já existe e
  não está vazio, ele pula (`[skip]`). Pode relançar o mesmo comando à
  vontade pra retomar de onde parou.
- Duas runs **não** compartilham `output_dir` (cada uma tem
  `.../p100_i50/<run_id>/`), então o paralelismo é seguro.
- Se o `podman run` de longa duração for morto junto com a sessão SSH mesmo
  com `setsid nohup ... disown`, rode dentro de `tmux`/`screen`:
  `tmux new -s campanha` → o comando (sem `setsid nohup ... &`) → `Ctrl-b d`.

---

## 7. Acompanhar o progresso

```bash
cd ~/STN_MoWFLOP

# combinações completas — espera 320 no fim
find raw_results/meta_heuristics_stn_windcorrected -name '*_stn.csv' -size +0c | wc -l

# por instância/algo — espera 20 cada (16 linhas no total)
find raw_results/meta_heuristics_stn_windcorrected -name '*_stn.csv' -size +0c \
  | sed -E 's#.*/(moead|nsga2)/([0-9]+_e-0[0-9])/.*#\1 \2#' | sort | uniq -c

# erros no launcher
grep -niE 'error|erro|terminate|what\(\)|segmentation' \
  source_code/logs/sparse_campaign_launcher.log

# progresso de uma run em andamento
tail -1 raw_results/meta_heuristics_stn_windcorrected/nsga2/513_e-05/p100_i50/7/infoRun.txt

# quantos processos do algoritmo estão vivos
podman ps          # o container da campanha
pgrep -af 'meta_heuristics/(moead|nsga2)' | wc -l   # de dentro do container
```

Cada `<run_id>/` completo deve conter: `infoRun.txt`, `*_stn.csv`,
`*_<n>.txt` (n = 100000…1000000), `*_layout.txt`, e um symlink
`*_candidates.csv` → `../candidates/<instance>_candidates.csv`.

---

## 8. Trazer os resultados de volta

Quando `... | wc -l` bater **320** e o launcher log não tiver erro:

```bash
# no servidor: empacota só o que interessa
cd ~/STN_MoWFLOP
tar czf ~/sparse_campaign_results.tar.gz \
  raw_results/meta_heuristics_stn_windcorrected \
  source_code/logs/sparse_campaign_launcher.log

ls -lh ~/sparse_campaign_results.tar.gz
```

```bash
# na sua máquina local
sftp -P 4522 <user>@bambu-server1.freeddns.org
get sparse_campaign_results.tar.gz
bye

cd /home/elis/Projects/TCC/STN_MoWFLOP
tar xzf ~/sparse_campaign_results.tar.gz   # cai em raw_results/meta_heuristics_stn_windcorrected/
```

**Storage do Bambu não tem backup** e a cota é 320/322 GB — baixe os
resultados e limpe o servidor (`rm -rf ~/STN_MoWFLOP ~/sparse_campaign_*.tar.gz`,
`podman rmi sparse-campaign`) assim que confirmar que o tar chegou íntegro.

---

## 9. Depois (fora do escopo desta campanha, fica pra próxima etapa)

Os `*_stn.csv` alimentam a construção/particionamento/métricas STN no
`STNs-MOCO-MoWFLOP/` (R) — Shannon entropy, κ, occupation grid — que é onde
o RQ1 de fato se responde: ver se a entropia degenera conforme τ/|P| cai
pelas 4 densidades de cada família (τ=5: 506→509; τ=15: 510→513), com
geometria constante.
