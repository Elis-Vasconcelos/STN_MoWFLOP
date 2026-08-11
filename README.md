

# Multi-Objective Offshore Wind Farm Layout Optimization (Mo-WFLOP)  

This repository contains the implementation, benchmark instances, and experimental data for the paper:  

> **"On the use of Pareto-based Features in Meta-Learning for Multi-Objective Offshore Wind Farm Layout Optimization"**  

## 📄 Abstract  

The Multi-objective Wind Farm Layout Optimization Problem (MoWFLOP) seeks the optimal turbine placement in offshore wind farms, where construct cost minimization and power generation maximization are simultaneously optimized. As MoWFLOP belongs to NP-hard, previous work has focused on metaheuristic approaches to compute high-quality solutions. However, metaheuristics' results can vary across wind farm instances, making manual algorithm selection impractical. This paper investigates the use of Pareto-based meta-features to guide a landscape-aware method based on meta-learning for automated metaheuristic selection applied to MoWFLOP. To our knowledge, no such investigation exists for this problem. We explore 30 different sampling configurations to extract Pareto-based meta-features from 300 wind farms, balancing computational cost with feature informativeness. We build a dataset for each configuration and train Random Forest models to predict the best-performing algorithm for unseen instances. The analyses comprised the metaheuristics' results regarding Pareto-compliant indicators, the impact of sampling configurations on feature values, and the meta-learning performance for automated metaheuristic selection.

## 📂 Repository Structure  
```
├── /final_dataset/                 # Refined epsilon dataset for each parameter and for all instances
├── /Instances/                 
│   ├── /instance_generator/        # Files for creating additional wind farm sites
│   ├── /site/                      # All Wind farm sites used in the study
│   ├── /wind/                      # Wind files and their probabilities
│   └── /wtg/                       # Power generated per turbine type for specific wind velocities
├── /raw_results/               
│   ├── /meta_features/             # Raw features extracted
│   ├── /meta_heuristics/           # Raw execution data of multi-objective algorithms
│   └── /stat_analysis/             # Raw statistical data for result comparison
├── /refined_results/           
│   ├── /statistical_tables/        # Refined statistical tables for each instance and each statistical measure 
│   └── /meta_learning/             # Final results from the meta-learning stage:
│       ├── features_importance/    # Contains the feature importance of each ML model
│       ├── figures/                # Visualizations showing feature importance
│       ├── logs/                   # Logs of each model run
│       └── /models/                # Trained models (theoretical models + executable formats)
└── /source_code/
    ├── /meta_features_extraction/  # Code used for features extraction
    ├── /meta_heuristics/           # Code of multi-objective algorithms
    └── /metalearning/              # Code that executes the full meta-learning process, including:
                                        # - Construction of ML models (theoretical and executable)
                                        # - Calculation of performance metrics (merit and regression-based)
                                        # - Dataset construction for model training
                                        # - Generation of figures
```
## ▶️ Running the metaheuristics (MOEA/D, NSGA-II)

`source_code/meta_heuristics/` contains the C++ MOEA/D and NSGA-II
implementations used by the paper (population size 100, 10 MOEA/D
neighbors, both hardcoded in `headers/globals.h` / `moead.cpp`). There is
no CoMOLS/D implementation in this repository.

### Build

```bash
cd source_code/meta_heuristics
make            # builds both ./nsga2 and ./moead
```

`make nsga2` / `make moead` build just one of the two; `make clean` /
`make rebuild` act on both. (Originally the `Makefile` only had a single
`nsga2.cpp`-hardcoded target — it's been extended with a proper `moead`
target, sharing the same `SRC_FILES`/flags, since both algorithm
implementations already live in the same source tree.)

### Run

**Run from `source_code/`, not from `meta_heuristics/`.** Instance loading
is hardcoded to look for `../instances/site/<id>/` relative to the current
working directory (`get_instance_info` in `instance_info.cpp` ignores the
`root_folder` CLI arg for this) — `..` only resolves to `instances/`
correctly when the process is launched from `source_code/`:

```bash
cd source_code   # NOT source_code/meta_heuristics
./meta_heuristics/moead <instance_id> [output_dir] [angle] [wind] [run_id] [stop_criteria] [stn_p] [stn_interval]
./meta_heuristics/nsga2 <instance_id> [output_dir] [angle] [wind] [run_id] [stop_criteria] [stn_p] [stn_interval]
```

Arguments are strictly positional — to reach `run_id` you must also pass
`angle` and `wind` (their defaults are `30` and `10`).

- `<instance_id>` — a directory name under `instances/site/` (e.g. `1`,
  `172`, `A`; the repo ships 300 numeric instances, no lettered A–J ones).
- `[output_dir]` — where output files are written; defaults to the current
  directory (`./`). Pass a trailing slash, e.g. `results/1/`. **Two runs
  sharing an output dir at the same time will corrupt each other's
  `infoRun.txt`** (both processes truncate-and-append the same path) —
  give concurrent runs distinct output dirs.
- `[angle]` / `[wind]` — wind angle in degrees and speed in m/s; default
  30° / 10 m/s.
- `[run_id]` — integer stamped on every row of the STN trajectory CSV, so
  independent repetitions stay distinguishable once their CSVs are
  concatenated. Defaults to `0`.
- `[stop_criteria]` — number of solution evaluations to stop after;
  defaults to 1,000,000. Lower it for a quick smoke test.
- `[stn_p]` — number of STN weight vectors (`STN_LOGGER_NUM_VECTORS`);
  defaults to `10`. Runtime arg, not a compile-time constant, so sweeping
  P (e.g. 10/50/100) doesn't require rebuilding.
- `[stn_interval]` — STN sampling interval in generations
  (`STN_LOGGER_INTERVAL`); defaults to `50`.

On a modern desktop core, a full 1,000,000-evaluation run of a
75-mobile-turbine instance (e.g. instance `1`) took **~25 minutes**
end-to-end in testing; larger instances take longer. The `Run time:` line
printed to stdout is just a static header (printed *before* the run
starts, not an actual timer) — prefix the command with `time` yourself if
you want wall-clock time.

### Output

All output files land in `[output_dir]` (default cwd):

| File | Written | Contents |
|---|---|---|
| `infoRun.txt` | continuously, one line per generation | `Generation <g> \| Revalues: <n> \| GridSize: <archive size>` — progress log |
| `<instance>_<algo>_<n>.txt` | every 100,000 evaluations (`n` = 100000, 200000, …, 1000000) | current non-dominated archive snapshot, one solution per line: `<cost> <power>` |
| `<instance>_<algo>_layout.txt` | once, at the final checkpoint (`n` = 1000000) | turbine coordinates of every solution in the final non-dominated archive: one `<x> <y>` line per turbine, solutions separated by a blank line |
| `<instance>_<algo>_stn.csv` | continuously, per generation | Search Trajectory Network raw data — see below |
| `<instance>_<algo>_candidates.csv` | once, at start | `global_index,zone,zone_index,x,y` — decodes the `occupied` column of the STN CSV into coordinates |

`<algo>` is `moead` or `nsga2` depending on which binary you ran. `cost` is
minimized, `power` is maximized — both printed as positive numbers in the
snapshot files.

### Search Trajectory Network (STN) logging

`<instance>_<algo>_stn.csv` records, every `STN_LOGGER_INTERVAL`
generations, the representative solution of each of `STN_LOGGER_NUM_VECTORS`
weight vectors — the raw data an STN is built from later. Both are
**runtime CLI args** (`stn_p`/`stn_interval`, see "Run" above; defaults
`10`/`50`) — `headers/globals.h` only declares them (`extern`), the actual
values are set per-run in `main()`, alongside `SIZE_OF_POPULATION` (which
*is* still a compile-time constant).

**Columns**: `algorithm,instance,run_id,vector_id,generation,iteration,
f_cost,f_power,weight1,weight2,occupied` — matches `STN_MoWFLOP.pdf`
§10.2's minimum log fields. Notes:
- `algorithm`/`instance` are literal columns, not just encoded in the filename.
- `iteration` is a sequential recording index (0,1,2,...); `generation` is
  the raw generation number (0, `STN_LOGGER_INTERVAL`, 2×`STN_LOGGER_INTERVAL`, ...)
  — kept alongside `iteration` beyond the spec's minimum, useful for debugging.
- `weight1`/`weight2` are that row's vector's literal weights — redundant
  with `vector_id` (`build_weight_vector` is deterministic) but avoids a
  join step before feeding this into `create.R`.
- `occupied` is the space-separated list of global candidate indices
  holding a turbine (sorted ascending), decodable through
  `<instance>_<algo>_candidates.csv`.
- `f_cost` is positive and minimized; `f_power` is positive and maximized.

**Design notes:**
- The occupation-grid signature that eventually partitions the search
  space into STN nodes is deliberately *not* computed here — cell size is
  a post-processing choice, so logging raw occupied positions lets any
  cell size be recomputed later without rerunning the algorithm.
- The STN's `STN_LOGGER_NUM_VECTORS` weight vectors are a separate,
  smaller set from MOEA/D's own `SIZE_OF_POPULATION` internal
  decomposition vectors (generated the same way — `build_weight_vector`,
  uniform between `(1,0)` and `(0,1)` — but MOEA/D's own vectors are
  unaffected and still drive its mating/replacement). Because of that,
  `population[j]` is *not* aligned with the STN's vector `j`: every
  generation logged, both algorithms call `select_representatives`
  (`stn_logger.h`) to pick, for each STN vector, the population member
  minimising `calculate_gte` against it and the current `z_point`. This
  observes both algorithms the same way, at the STN's coarser resolution
  — an external instrumentation layer, not part of either algorithm, and
  should be declared as such in any methodology write-up.
- Sampling every `STN_LOGGER_INTERVAL`-th generation means a run's true
  terminal state is only captured if its last generation happens to be a
  multiple of the interval. If exact terminal nodes matter for a metric
  (e.g. `n_end`), account for this in post-processing.

**Validating a run locally**: after a short test (small `stop_criteria`,
e.g. `20000`, as the 6th CLI arg), run these manual checks from
`source_code/` (no output beyond headers on any of them means the log is
consistent):

```bash
file="<output_dir>/<instance>_<algo>_stn.csv"

# every row should have tau_total turbines occupied (column 11)
awk -F',' 'NR>1{n=split($11,a," "); if(n!=EXPECTED_TAU) print "line " NR ": " n " turbines"}' "$file"

# sampled generations should be 0, STN_LOGGER_INTERVAL, 2*STN_LOGGER_INTERVAL, ... (column 5)
awk -F',' 'NR>1{print $5}' "$file" | sort -n -u

# iteration should be 0, 1, 2, ... sequential, unlike generation above (column 6)
awk -F',' 'NR>1{print $6}' "$file" | sort -n -u

# no vector_id (col 4) should have a repeated or out-of-order generation (col 5)
# (BEGIN{prev_v=-1} avoids a false positive on vector_id 0: an uninitialized
# awk variable compares equal to "0" by default, colliding with vector 0)
awk -F',' 'NR>1{print $4","$5}' "$file" | sort -t, -k1,1n -k2,2n | \
  awk -F',' 'BEGIN{prev_v=-1} prev_v==$1 && $2<=prev_g {print "order broken in vector " $1} {prev_v=$1; prev_g=$2}'
```

### The STN instance set (Cazzaro/Pisinger "New Sites" dataset)

The instance set selected for the STN experiments (41, 48, 101, 178, 192,
202, 203, 440, 465, 488) refers to the **Cazzaro/Pisinger real-world
instance set** ("New Sites"), **not** the 300 synthetic instances already
bundled under `instances/site/`. The two sets reuse the same numeric IDs
for completely different wind farms (e.g. bundled `instances/site/41` has
1 zone/48 turbines; New Sites' instance `41` has 3 zones/123 turbines) —
using the wrong one silently runs the wrong experiment.

The full New Sites collection (501 instances, ~700MB, `wflop_instances/`
at the repo root) is committed **in full**, not just this instance set's
10 — `git push`/`git pull` alone gets everything, no separate
data-transfer step, and running any of the other 490+ instances later
needs no re-setup.

`instance_info.cpp` resolves instances through a fixed path
(`../instances/site/<instance>/...`), so pointing binaries at
`wflop_instances/` directly isn't possible without a C++ change. Instead,
referencing an instance as **`ns<id>`** anywhere (an instances file,
directly on the CLI) makes `run_one.sh` auto-create the symlink
`instances/site/ns<id>` → `wflop_instances/New Sites/<id>` the first time
that instance is used — no separate setup step. The `ns` prefix just
avoids colliding with the bundled instance of the same number; a plain
numeric ID still means the bundled one, unaffected. These symlinks aren't
committed (machine-specific, regenerated automatically wherever
`wflop_instances/` exists — see `.gitignore`).

An instances file (what `batch.sh` reads) can freely mix plain and
`ns`-prefixed IDs on separate lines, in any combination — nothing
validates the format, each line is just passed straight through.
`instances_stn10.txt` lists the STN instance set as `ns`-prefixed IDs.

To run every instance of one dataset instead (e.g. if more of these 500+
real-world instances end up needed later):

```bash
seq 1 300 > instances_all_bundled.txt              # all 300 bundled synthetic instances
seq 0 500 | sed 's/^/ns/' > instances_all_new.txt  # all 501 Cazzaro/Pisinger instances
```
**Concurrency warning**: `batch.sh` (below) launches one process per line,
all at once, with no cap — fine for 10 instances, but 300 or 501
concurrent processes will likely exceed available cores. Check core
availability and chunk into smaller batches first.

### Running a campaign

Three scripts in `meta_heuristics/scripts/`, layered:

**`run_one.sh`** — runs exactly one `(instance, algo, stn_p, stn_interval,
run_id)` combination, output under
`raw_results/meta_heuristics_stn/<algo>/<instance>/p<stn_p>_i<stn_interval>/<run_id>/`:

```bash
cd source_code
./meta_heuristics/scripts/run_one.sh 41 moead 0 1000000 30 10 10 50
```
- **Idempotent**: skips a combination whose `_stn.csv` already exists —
  safe to interrupt and resume. `stn_p`/`stn_interval` are part of the
  output path specifically so sweeping P actually works: re-running the
  same `(instance, algo, run_id)` with a *different* P lands in a
  different directory, instead of finding the previous P's `_stn.csv`
  already there and silently skipping.
- `_candidates.csv` is instance-only, so `run_one.sh` keeps one canonical
  copy per instance under `raw_results/meta_heuristics_stn/candidates/`
  and symlinks (relative, portable) it into every run's directory,
  instead of letting the ~290KB table regenerate in every output
  directory a campaign creates.
- Also the unit to call directly from a real HPC job scheduler, one
  job/task per combination — no scheduler-specific logic baked in.

**`run_instance.sh`** — for one instance, loops over `{algos} ×
run_id 0..num_runs-1` calling `run_one.sh` for each, sequentially.

**`batch.sh`** — the entry point you actually call. Reads an instances
file (default `instances_stn10.txt`) and launches, via `nohup`, one
`run_instance.sh <instance> ...` process per instance, all in parallel:

```bash
cd source_code
./meta_heuristics/scripts/batch.sh                      # defaults: instances_stn10.txt, moead+nsga2, 10 runs
./meta_heuristics/scripts/batch.sh my_instances.txt "moead nsga2" 10 1000000 30 10 100 50
```
- Logs land in `source_code/logs/<instance>_p<stn_p>_i<stn_interval>.log`;
  check progress with `tail -f logs/*.log` or `ps -p <pid>` (PIDs are
  printed when `batch.sh` launches).
- Composes with `run_one.sh`'s idempotency — safe to re-launch to resume
  after an interruption.

**Sweeping P** (e.g. 10/50/100) with the interval held fixed: call
`batch.sh` directly, once per P value. Since `batch.sh` only launches
detached `nohup` jobs and returns immediately, three calls back to back
run the P values **concurrently**, not sequentially:

```bash
./meta_heuristics/scripts/batch.sh instances_stn10.txt "moead nsga2" 10 1000000 30 10 10  50
./meta_heuristics/scripts/batch.sh instances_stn10.txt "moead nsga2" 10 1000000 30 10 50  50
./meta_heuristics/scripts/batch.sh instances_stn10.txt "moead nsga2" 10 1000000 30 10 100 50
```
Each P lands in its own output directory/log — nothing gets skipped or
overwritten between them. This needs enough free cores to run all of it
at once (10 instances × 2 algorithms × 3 P values = 60 concurrent
single-threaded processes) — check availability first (`nproc`, `uptime`,
`top`) if the machine is shared. Fewer P values per invocation, or waiting
for one to finish before the next, trades wall-clock time for a smaller
concurrent footprint.

**Provenance**: `run_instance.sh`/`batch.sh` adapt Gustavo/João's
`MO_WFLOP-experiment-runner` two-file split (`scripts/main.sh` +
`scripts/batch.sh`) directly — same idiom (`nohup "$script" ... &>
logfile &`, no scheduler, no concurrency cap, no final `wait`), same
layering. Real differences: the instance list comes from a file instead
of hardcoded literals, and it's one process per instance rather than per
~10-instance chunk (their original use case had 300+ instances to
distribute; this set only has 10). `run_one.sh`'s idempotency and
`_candidates.csv` dedup have no equivalent in their scripts — their
`comolsd` binary doesn't write a shared per-instance file the way
`STNLogger` does, so there was nothing to adapt there; both are
necessary additions, not deviations from their pattern.

## 👥 Authors  
| Name | Affiliation | Contact |  
|------|-------------|---------|  
| **Gustavo J. N. Silva** | Federal University of Bahia, Institute of Computing | [gustavojorge080@gmail.com](mailto:gustavojorge080@gmail.com) |  
| **João G. Lofiego** | Federal University of Bahia, Institute of Computing | [joao.lofiego@ufba.br](mailto:joao.lofiego@ufba.br) |
| **Islame F. C. Fernandes** | Federal University of Bahia, Institute of Computing | [islame.felipe@ufba.br](mailto:islame.felipe@ufba.br) |

## 📧 Contact  
For questions, contact the lead authors: [gustavojorge080@gmail.com](mailto:gustavojorge080@gmail.com) and [joao.lofiego@ufba.br](mailto:joao.lofiego@ufba.br).  
