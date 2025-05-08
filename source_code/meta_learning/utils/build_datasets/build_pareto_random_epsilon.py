import os
import pandas as pd

base_path = 'meta_features/dataset/'
analysis_path = 'Analises/'
output_path = 'final_dataset_eps/pareto_based/random_walk/'

os.makedirs(output_path, exist_ok=True)

l_values = [5, 10, 25, 50, 100]
r_values = ["0.05", "0.1", "0.25", "0.5", "1.0"]

combined_data = {f"l{l}_r{r}": [] for l in l_values for r in r_values}

for instance in os.listdir(base_path):
    pareto_random_walk_path = os.path.join(base_path, instance, 'pareto_based', 'random_walk')
    analysis_instance_path = os.path.join(analysis_path, instance)

    if not os.path.isdir(pareto_random_walk_path) or not os.path.isdir(analysis_instance_path):
        continue

    moead_file = os.path.join(analysis_instance_path, 'esp_ad_moead.out')
    nsga2_file = os.path.join(analysis_instance_path, 'esp_ad_nsga2.out')

    def calcular_media_eps(filepath):
        if os.path.exists(filepath):
            with open(filepath, 'r') as f:
                valores = [float(line.strip()) for line in f if line.strip()]
            return sum(valores) / len(valores) if valores else None
        return None

    moead_mean = calcular_media_eps(moead_file)
    nsga2_mean = calcular_media_eps(nsga2_file)

    for l in l_values:
        for r in r_values:
            dir_name = f"l{l}_r{r}"
            dir_path = os.path.join(pareto_random_walk_path, dir_name)

            if not os.path.isdir(dir_path):
                continue

            csv_file = next((f for f in os.listdir(dir_path) if f.startswith('ofe_') and f.endswith('.csv')), None)

            if csv_file:
                csv_path = os.path.join(dir_path, csv_file)
                df = pd.read_csv(csv_path)

                df['NSGAII'] = nsga2_mean
                df['MOEAD'] = moead_mean

                combined_data[dir_name].append(df)

for config, dataframes in combined_data.items():
    if dataframes:  
        final_df = pd.concat(dataframes, ignore_index=True)
        final_path = os.path.join(output_path, f"{config}.csv")
        final_df.to_csv(final_path, index=False)
        print(f"Generated archive: {final_path}")

print("Process completed")
