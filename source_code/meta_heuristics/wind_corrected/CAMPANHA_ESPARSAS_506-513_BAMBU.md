# Campanha esparsa 506–513 no servidor Bambu — instruções de execução

**RQ1 / sweep de densidade single-site.** Rodar MOEA/D e NSGA-II nas 8
instâncias `506_e-02 … 513_e-05` (geometria única compartilhada, só τ e
densidade τ/|P| variam), com `(angle, wind)` sorteado da rosa dos ventos
por execução, e coletar as trajetórias STN.

Tudo aqui é pra **você** rodar no servidor — o Claude não acessa o Bambu.
Onde tiver `<...>` troque pelo valor real. As seções abaixo estão na ordem
em que foram executadas de fato; os números de tempo em §4 são medições
reais no bambu1, não estimativas.

---

## Estado da execução

| | |
|---|---|
| 1º lançamento (20 runs) | **2026-09-01 ~13:57 UTC** (bambu1, usuário `elis`) |
| Ampliação p/ 30 runs | **2026-09-01** — Prof. Islame pediu 30 execuções por (instância, algo), não 20 (ver §0). `run_id` 20–29 adicionados ao wind map; relançar o `podman run` preenche só os que faltam (idempotente). |
| Container | `campanha` (`podman ps` / `podman logs campanha`) |
| Persistência | `loginctl enable-linger elis` habilitado → sobrevive a logout |
| Total de runs | **480** (8 instâncias × 2 algos × 30) |
| Fim esperado | ~12–18 h a partir do relançamento com 30 runs (cauda = nsga2 em `509_e-05`/`513_e-05`) |
| Retomada | idempotente — basta relançar o mesmo `podman run` (§6); runs com `_stn.csv` não-vazio são puladas |

Quando `find ... -name '*_stn.csv' -size +0c | wc -l` bater **480** e o grep
de erro (§7) não achar nada → empacotar e trazer de volta (§8).

---

## 0. Parâmetros da campanha (fixados)

| | |
|---|---|
| Instâncias | `506_e-02 507_e-03 508_e-04 509_e-05 510_e-02 511_e-03 512_e-04 513_e-05` |
| Algoritmos | `moead`, `nsga2` |
| Execuções por (instância, algo) | **30** (`run_id` 0–29) |
| `stop_criteria` | **1 000 000** avaliações |
| STN `stn_p` (nº de vetores observadores) | **100** |
| STN `stn_interval` (generations entre amostras) | **50** |
| `(angle, wind)` | pré-sorteado, uma tupla por (instância, algo, run_id) — `sparse_wind_map_506-513.csv` |
| Total de combinações | 8 × 2 × 30 = **480** |
| Servidor | **bambu1** — `bambu-server1.freeddns.org`, SSH porta **4522**, auth por senha |
| Saída | `raw_results/meta_heuristics_stn_windcorrected/<algo>/<instance>/p100_i50/<run_id>/` |

### Por que 30 execuções (10 + 20)

Instrução do Prof. Islame (2026-09-01): **30 = 10 + 20**, servindo a dois
propósitos distintos:

- **10 runs → plotar as STN.** É o número fixo do protocolo da tese
  (`STN_MoWFLOP.pdf` §10.2/§10.5, "10 runs", *"Não altere"*) e da
  metodologia STNs-MOCO da Ochoa. A STN funde as trajetórias dessas runs
  num único grafo por algoritmo — com muitas runs vira um emaranhado
  ilegível. Então a visualização fica em 10.
- **20 runs → conjunto de referência do Pareto.** Pra calcular indicadores
  de qualidade (hypervolume, IGD, ε) é preciso uma aproximação da frente
  de Pareto verdadeira. As famílias `ns178`/`ns101` podem reusar a frente
  de referência **publicada** do CEC2026; as `506–513` são sítios
  sintéticos novos, **sem frente de referência externa** — a nossa tem que
  ser construída juntando as soluções não-dominadas de muitas runs dos
  dois algoritmos. 20 é o número que casa com o protocolo do próprio
  CEC2026.

Como todas as 30 rodam com **config idêntica** (STN logging ligado —
instrumentação externa, não afeta a busca), na prática não há diferença de
execução. A divisão 10/20 é só uma partição em tempo de análise (p.ex.
`run_id` 0–9 → STN, 10–29 → frente de referência), decidida depois no
`STNs-MOCO-MoWFLOP/`.

`stn_p=100` é **um único valor** (a campanha STN10 original varria 10/50/100;
esta não — P é instrumentação externa, não afeta a busca, e 100 é o teto já
usado). Então são 480 combinações, não 1440. Se depois quiser um check de
robustez a P, dá pra rodar P=10 e P=50 **só** nas 6 instâncias baratas
(506–508, 510–512) a custo quase zero, sem repetir 509/513.

### Origem do `(angle, wind)`

Sorteio ponderado pela rosa dos ventos `wflop_instances/wind/RVO_TNW.txt`
(mesmo arquivo do benchmark Cazzaro & Pisinger), uma tupla independente
por (instância, algo, run_id) — exatamente o caminho "fresh_geometry" do
`sample_wind_rose.py`: sítio sintético novo, sem contrapartida real no
CEC2026 pra reusar. Gerado **sem seed fixa** (`random.SystemRandom`), então
**o CSV `sparse_wind_map_506-513.csv` é o registro permanente** — não é pra
regenerar depois (cada regeração dá sorteios diferentes). Já está no bundle;
versione junto com os resultados.

**Ampliação 20 → 30 (2026-09-01):** as linhas `run_id` 0–19 (as 320
originais, que já estavam rodando no Bambu) ficaram **intactas, byte a
byte**; só foram acrescentadas 160 linhas novas (`run_id` 20–29, 10 por
instância/algo) pelo mesmo método, com:

```bash
cd source_code/meta_heuristics/wind_corrected
python3 sample_sparse_sweep_wind.py --append-from 20 >> sparse_wind_map_506-513.csv
# 321 linhas (1 header + 320)  ->  481 linhas (1 header + 480)
```

O arquivo fica agrupado 0–19 e depois 20–29 — inofensivo: o launcher lê
`(angle, wind)` direto de cada linha (§5/§6), não faz lookup por linha/chave.

Gerador: `source_code/meta_heuristics/wind_corrected/sample_sparse_sweep_wind.py`
(rastreabilidade do método + o modo `--append-from` usado acima — **não rode
sem argumento**, isso geraria um mapa 0–29 todo novo).

---

## 1. Montar o bundle de transferência (na sua máquina local)

O servidor não precisa dos 20 GB do repositório. Só de:

- `source_code/meta_heuristics/` (código C++ + `scripts/` + `wind_corrected/`) — ~3 MB
- `instances/wtg/` (curvas de potência NREL-10-179 / NREL-15-240 — `instance_info.cpp`
  abre `../instances/wtg/*.txt` sempre; sem isso o binário aborta com
  `ERROR: '/instances/wtg' not found`, mesmo o site carregando ok) — 12 KB
- as 8 pastas de instância sob `wflop_instances/sparse_instances/single_site_sweep/` — ~21 MB

`instances/wind/` **não** é necessário (a rosa dos ventos só é lida pelo bloco
comentado em `instance_info.cpp`; o `(angle, wind)` já vem pronto do wind map).
`geometry.txt` e `plot.png` das instâncias também não são lidos pelo binário.

```bash
cd /home/elis/Projects/TCC/STN_MoWFLOP

# garante que o wind map está gerado (481 linhas: 1 header + 480)
wc -l source_code/meta_heuristics/wind_corrected/sparse_wind_map_506-513.csv

tar czf /tmp/sparse_campaign_bundle.tar.gz \
  --exclude='source_code/meta_heuristics/moead' \
  --exclude='source_code/meta_heuristics/nsga2' \
  --exclude='*.o' \
  source_code/meta_heuristics \
  instances/wtg \
  wflop_instances/sparse_instances/single_site_sweep

ls -lh /tmp/sparse_campaign_bundle.tar.gz   # ~7,4 MB comprimido (medido)
```

Transferir (porta 4522, mesma senha do SSH):

```bash
scp -P 4522 /tmp/sparse_campaign_bundle.tar.gz <user>@bambu-server1.freeddns.org:~/
# ou: sftp -P 4522 <user>@bambu-server1.freeddns.org  ->  put /tmp/sparse_campaign_bundle.tar.gz  ->  bye
```

---

## 2. Preparar o layout no servidor

```bash
ssh <user>@bambu-server1.freeddns.org -p 4522
# senha expirada no 1º acesso: no prompt "Atual senha:" digite a senha ANTIGA
# (a do e-mail) de novo, depois a nova 2×. A conexão cai; reconecte com a nova.

mkdir -p ~/STN_MoWFLOP && cd ~/STN_MoWFLOP
tar xzf ~/sparse_campaign_bundle.tar.gz

# o tar perde o bit de execução dos scripts -> restaure
chmod +x source_code/meta_heuristics/scripts/*.sh

# dirs que os scripts esperam existir (symlinks e saída são criados on-demand)
mkdir -p instances/site raw_results source_code/logs

# confere (as 8 pastas de instância ficam em profundidade 4)
ls wflop_instances/sparse_instances/single_site_sweep/     # 506_e-02 ... 513_e-05
ls instances/wtg/                                          # NREL-10-179.txt  NREL-15-240.txt
wc -l < source_code/meta_heuristics/wind_corrected/sparse_wind_map_506-513.csv   # 481
```

`run_one_windcorrected.sh` acha cada instância com
`find ../wflop_instances -maxdepth 4 -type d -name "<instance>"` e cria o
symlink `instances/site/<instance>` na primeira vez que ela roda — por isso
`instances/site/` só precisa existir vazio.

`~/` no bambu1 fica em `/dev/md0` (1,8 TB, cota 320/322 GB) — pode trabalhar
direto em `~/STN_MoWFLOP`. `/media/nvme` (cota 1 KB) é irrelevante.

---

## 3. Container (podman) — build do C++

Host é Debian 6.1, **só podman** (docker proibido), **sem Python de sistema,
sem instalar pacote no host**. Todo o build e execução acontece dentro de um
container. Precisa só de `g++` + `make` (o `Makefile` usa
`g++ -std=c++17 -O2 -Werror`; nenhuma lib externa).

> **O podman do Bambu exige nomes de imagem totalmente qualificados** (aviso
> no MOTD): use `docker.io/library/debian:bookworm-slim`, nunca
> `debian:bookworm-slim` sozinho.

Criar `~/STN_MoWFLOP/Containerfile`:

```bash
cat > ~/STN_MoWFLOP/Containerfile <<'EOF'
FROM docker.io/library/debian:bookworm-slim
RUN apt-get update && \
    apt-get install -y --no-install-recommends build-essential ca-certificates && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /work/source_code
EOF
```

Build da imagem (precisa de saída pra internet — permitida no Bambu):

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

Esperado: `Starting compilation of nsga2...` / `Compilation completed.`, idem
`moead`, **sem warning** (`-Werror` faz qualquer warning quebrar o build).

```bash
ls -l ~/STN_MoWFLOP/source_code/meta_heuristics/{moead,nsga2}   # ~185 KB cada, executáveis
```

---

## 4. Smoke test antes da campanha inteira

Roda a instância mais pesada (`513_e-05`, 185 054 posições) com um
`stop_criteria` minúsculo, os dois algoritmos, 1 run — pra (a) confirmar o
pipeline ponta-a-ponta no container e (b) medir tempo real.

```bash
cd ~/STN_MoWFLOP
podman run --rm -v ~/STN_MoWFLOP:/work:Z -w /work/source_code sparse-campaign \
  bash -c '
    time ./meta_heuristics/scripts/run_one_windcorrected.sh 513_e-05 moead 0 210 11 2000 100 50
    time ./meta_heuristics/scripts/run_one_windcorrected.sh 513_e-05 nsga2 0 210 11 2000 100 50
  '
```

(`210 11` = `(angle, wind)` de teste — não vai pro resultado final; apaga a
pasta depois.) A linha solta `9` no stdout é um `cout` de debug do binário,
inofensiva.

Validação do CSV STN gerado (roda no host — os arquivos aparecem em
`~/STN_MoWFLOP/raw_results/...` pelo bind mount):

```bash
f=raw_results/meta_heuristics_stn_windcorrected/moead/513_e-05/p100_i50/0/513_e-05_moead_stn.csv
wc -l "$f"; head -1 "$f"

# coluna 11: cada linha deve ter exatamente τ turbinas ocupadas
#   τ = 5 pra 506–509 ;  τ = 15 pra 510–513   (aqui 513 -> 15)
awk -F',' 'NR>1{n=split($11,a," "); if(n!=15) print "linha "NR": "n}' "$f"

# coluna 5: generations amostradas 0,50,100,...   (num smoke de 2000 evals só
#   sai a generation 0 — 2000 < 1 intervalo de 50 gerações; isso é esperado,
#   não é falha. Na campanha real de 1e6 saem milhares de linhas.)
awk -F',' 'NR>1{print $5}' "$f" | sort -n -u | head

# coluna 6: iteration sequencial 0,1,2,...
awk -F',' 'NR>1{print $6}' "$f" | sort -n -u | head

# nenhum vector_id (col 4) com generation (col 5) repetida ou fora de ordem
awk -F',' 'NR>1{print $4","$5}' "$f" | sort -t, -k1,1n -k2,2n | \
  awk -F',' 'BEGIN{pv=-1} pv==$1 && $2<=pg {print "ordem quebrada no vetor "$1} {pv=$1; pg=$2}'
```

Silêncio nos três `awk` = log consistente. Depois:

```bash
rm -rf raw_results/meta_heuristics_stn_windcorrected/moead/513_e-05 \
       raw_results/meta_heuristics_stn_windcorrected/nsga2/513_e-05
```

### Tempo — MEDIDO no bambu1 (não estimativa)

Smoke em `513_e-05` @ 2000 evals: **MOEA/D 3,3 s**, **NSGA-II 55,6 s**.
Escala ~linear → por run completo de 1e6 evals:

| Instância | posições | ~MOEA/D | ~NSGA-II |
|---|---|---|---|
| 506/507/510/511 (e-02, e-03) | 60–2 548 | segundos | segundos–minutos |
| 508_e-04 | 6 000 | ~1–3 min | ~15–25 min |
| 512_e-04 | 19 505 | ~5–10 min | ~45–60 min |
| 509_e-05 | 60 000 | ~10 min | ~2,5–3 h |
| 513_e-05 | 185 054 | ~15–25 min | **~7–8 h** |

NSGA-II é o gargalo (non-dominated sorting + crowding sobre um espaço de
185k posições por geração). Tabela acima é por instância/algoritmo,
independente de quantas runs — pra **30** runs (em vez de 20) o custo
agregado escala ≈ 1,5×: **~390–450 CPU-h**; wall-clock esperado **~12–18 h**
com `-P 55` (o piso continua sendo uma run nsga2 de `513_e-05` ≈ 8 h, só que
agora são 30 dessas na fila em vez de 20). Ainda bem abaixo do 1–3 dias que
a estimativa a priori supunha.

Medição real do 1º lançamento (20 runs): 506–509 (τ=5) terminaram em ~20 min
— **bem mais rápido** que a extrapolação acima previa. Ao ampliar pra 30,
confirme um run pesado (`509`/`513` nsga2, `run_id` ≥ 20) de fato bateu 1e6
avaliações (tem `*_1000000.txt`, generation alta em `infoRun.txt`) em vez de
assumir pela contagem de arquivos.

---

## 5. Gerar a lista de 480 combinações

Colocada em `source_code/` (dentro do bind mount do container), **não** em
`/tmp` (que o container não monta):

```bash
cd ~/STN_MoWFLOP/source_code
WMAP=meta_heuristics/wind_corrected/sparse_wind_map_506-513.csv

tail -n +2 "$WMAP" | awk -F',' '{print $1, $2, $3, $4, $5}' > sparse_combos.txt
wc -l sparse_combos.txt        # espera 480
head -3 sparse_combos.txt      # ex: 506_e-02 moead 0 210.000000000000 11.000000000000
grep -c ' 2[0-9] ' sparse_combos.txt  # espera 160 (run_id 20-29, os novos)
```

Relançar esta lista contra um `~/STN_MoWFLOP` que já tem os `run_id` 0–19
prontos é seguro: `run_one_windcorrected.sh` pula (`[skip]`) tudo que já tem
`_stn.csv` não-vazio (§6) — só `run_id` 20–29 de fato executam.

Cada linha vira 5 campos: `<instance> <algo> <run_id> <angle> <wind>`.
`stop_criteria`, `stn_p`, `stn_interval` (1000000 / 100 / 50) são fixos e
entram no comando do `xargs`, não no arquivo.

---

## 6. Lançar a campanha

`tmux`/`screen` **não** existem no bambu1 e não dá pra instalar. Solução:
container **em modo detached** (`-d`) — roda independente do shell — mais
`loginctl enable-linger` pra sobreviver a logout. `-P 55` deixa folga sob o
limite de cgroup (62 núcleos); cada processo é single-core, <20 MB de RAM.

```bash
cd ~/STN_MoWFLOP

podman run -d --name campanha \
  -v ~/STN_MoWFLOP:/work:Z -w /work/source_code \
  sparse-campaign \
  xargs -a sparse_combos.txt -n 5 -P 55 \
    bash -c './meta_heuristics/scripts/run_one_windcorrected.sh "$0" "$1" "$2" "$3" "$4" 1000000 100 50'

loginctl enable-linger $USER   # mantém o container vivo após logout
```

Imprime o container ID e volta na hora. **Sem `--rm`** de propósito: assim o
container fica após terminar e dá pra ler `podman logs campanha` e o exit
code depois (limpe com `podman rm campanha` no fim, §8).

Notas:
- `run_one_windcorrected.sh` é **idempotente**: se `<...>_stn.csv` já existe
  e não está vazio, pula (`[skip]`). Pode relançar o mesmo `podman run`
  (troque o `--name`, ou `podman rm campanha` antes) pra retomar de onde
  parou.
- Duas runs **não** compartilham `output_dir` (cada uma em
  `.../p100_i50/<run_id>/`), então o paralelismo é seguro.
- `Ctrl-C` num `podman logs -f` só para de acompanhar o log — **não** mata o
  container.

---

## 7. Acompanhar o progresso

```bash
cd ~/STN_MoWFLOP

# container vivo?
podman ps                                # 'campanha' deve estar Up

# combinações completas — espera 480 no fim
find raw_results/meta_heuristics_stn_windcorrected -name '*_stn.csv' -size +0c | wc -l

# por instância/algo — espera 30 cada (16 linhas no total)
find raw_results/meta_heuristics_stn_windcorrected -name '*_stn.csv' -size +0c \
  | sed -E 's#.*/(moead|nsga2)/([0-9]+_e-0[0-9])/.*#\1 \2#' | sort | uniq -c

# atividade recente do launcher (NÃO use 'logs -f' — ele repassa o log todo)
podman logs --tail 40 campanha

# erros
podman logs campanha 2>&1 | grep -iE 'error|erro|abort|terminate|what\(\)|segmentation'

# progresso de uma run longa específica
tail -1 raw_results/meta_heuristics_stn_windcorrected/nsga2/513_e-05/p100_i50/7/infoRun.txt
```

O contador vai parecer travar: 506/507/510/511 terminam em segundos, aí
508→512→509→513 seguram cada núcleo por minutos/horas. Esperar o contador
ficar em ~370–390 por boa parte de um dia enquanto a cauda 509/513 nsga2
termina é normal.

Cada `<run_id>/` completo tem: `infoRun.txt`, `*_stn.csv`, `*_<n>.txt`
(n = 100000…1000000), `*_layout.txt`, e um symlink `*_candidates.csv` →
`../candidates/<instance>_candidates.csv`.

---

## 8. Trazer os resultados de volta

Quando `... | wc -l` bater **480** e o grep de erro não achar nada:

```bash
cd ~/STN_MoWFLOP

# salva o log do launcher a partir do container antes de removê-lo
podman logs campanha > source_code/logs/sparse_campaign_launcher.log 2>&1

tar czf ~/sparse_campaign_results.tar.gz \
  raw_results/meta_heuristics_stn_windcorrected \
  source_code/logs/sparse_campaign_launcher.log \
  source_code/meta_heuristics/wind_corrected/sparse_wind_map_506-513.csv

ls -lh ~/sparse_campaign_results.tar.gz
```

```bash
# na sua máquina local
scp -P 4522 <user>@bambu-server1.freeddns.org:~/sparse_campaign_results.tar.gz .

cd /home/elis/Projects/TCC/STN_MoWFLOP
tar xzf ~/sparse_campaign_results.tar.gz   # cai em raw_results/meta_heuristics_stn_windcorrected/
```

**Storage do Bambu não tem backup** e a cota é 320/322 GB — baixe os
resultados, confira a integridade do tar, e **só então** limpe o servidor:

```bash
podman rm campanha
podman rmi sparse-campaign
rm -rf ~/STN_MoWFLOP ~/sparse_campaign_bundle.tar.gz ~/sparse_campaign_results.tar.gz
```

---

## 9. Depois (fora do escopo desta campanha)

Os `*_stn.csv` alimentam a construção/particionamento/métricas STN no
`STNs-MOCO-MoWFLOP/` (R) — é onde o RQ1 de fato se responde: ver se a
entropia de Shannon degenera conforme τ/|P| cai pelas 4 densidades de cada
família (τ=5: 506→509; τ=15: 510→513), com geometria constante.

**Métricas a calcular (tabela da p. 11 do PDF de instruções do Prof.
Islame, versão 31/08/2026):** além das já existentes, ele acrescentou
**`Step_len`**, **`R(κ)`** e **`D(κ)`** — considerar na etapa de análise em R,
por densidade e por família.
