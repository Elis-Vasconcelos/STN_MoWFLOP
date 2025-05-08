import os
import sys
import time
import pandas as pd
import subprocess

def run_merit_script(script_path, dataset_path, arquive, label, log_path):
    os.makedirs(log_path, exist_ok=True)
    try:
        start_time = time.time()
        result = subprocess.run(
            ["python", script_path, dataset_path, arquive, label],
            capture_output=True, text=True, check=True
        )
        end_time = time.time()
        run_time = end_time - start_time
        
        output_lines = result.stdout.strip().split("\n")
        metric_value = float(output_lines[0])  
        merit_list = output_lines[1:]  
        
        log_file_path = os.path.join(log_path, f"{arquive}_log.txt")
        with open(log_file_path, 'w') as log_file:
            log_file.write("================== LOG ==================\n")
            log_file.write(f"Parameters: {arquive}\n")
            log_file.write(f"Run time: {run_time:.4f} seconds\n")
            log_file.write(f"Merit Average: {metric_value:.4f}\n")
            log_file.write(f"Merit List: {', '.join(merit_list)}\n")
        
        return round(metric_value, 3)
    except Exception as e:
        print(f"Error processing {arquive}: {e}")
        return "Erro"

def process_random_walk(label, script_path, base_path, l_values, r_values, results_table):
    print("-> Building the dataset for random walk")
    log_path = f"result/{label}/logs/"
    
    for l in l_values:
        for r in r_values:
            print(f"--> Building for r = {r} and l = {l}")
            dataset_path = os.path.join(base_path, f"l{l}_r{r}.csv")
            results_table.at[r, l] = run_merit_script(script_path, dataset_path, f"l{l}_r{r}", label, log_path)

def process_adaptive_walk(label, script_path, base_path, r_values):
    print("\n-> Building the dataset for adaptive walk")
    log_path = f"result/{label}/logs/"
    adaptive_walk_values = []
    
    for r in r_values:
        print(f"--> Building for r = {r}")
        dataset_path = os.path.join(base_path, f"r{r}.csv")
        adaptive_walk_values.append(run_merit_script(script_path, dataset_path, f"r{r}", label, log_path))
    
    return adaptive_walk_values

def main():
    if len(sys.argv) != 2:
        print("Uso: python builder_merit_table.py <label>")
        sys.exit(1)
    
    label = sys.argv[1]

    base_path_random = f'final_dataset_{label}/pareto_based/random_walk/'
    base_path_adaptive = f'final_dataset_{label}/pareto_based/adaptive_walk/'
    output_path = f'result/{label}/'
    script_path = 'src/components/models_and_merit_builder.py'
    
    l_values = [100, 50, 25, 10, 5]
    r_values = ["1.0", "0.5", "0.25", "0.1", "0.05"]

    results_table = pd.DataFrame(index=r_values, columns=l_values)
    
    os.makedirs(output_path, exist_ok=True)

    process_random_walk(label, script_path, base_path_random, l_values, r_values, results_table) 
    adaptive_walk_values = process_adaptive_walk(label, script_path, base_path_adaptive, r_values)
    
    results_table["Adaptive Walk"] = adaptive_walk_values
    results_table.index = [f"{int(float(r) * 100)}%" for r in r_values]
    results_table.columns = [f"\u2113 = {l}" for l in l_values] + ["Adaptive Walk"]
    results_table.index.name = "r"
    
    output_csv = os.path.join(output_path, f"merit_{label}.csv")
    results_table.to_csv(output_csv)    
    
    print("\nDONE! Table with merit values generated and saved")

if __name__ == "__main__":
    main()
