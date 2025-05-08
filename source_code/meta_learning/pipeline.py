import subprocess

merit_table_builder_path = 'src/components/merit_table_builder.py'
regression_metrics_path = 'src/components/regression_metrics.py'
figure_builder_metrics_path = 'src/components/figure_builder.py'

label= "epsilon"

print("===================== Running merit_table_builder_.py =====================")
print(f"""
DESCRIPTION:
This file gets the merit value for each combination of 'l' and 'r' (by calling 'models_and_merit_builder.py'). Each of these merits is stored in a table called 'merit_epsilon.csv'.
""")
subprocess.run(["python", merit_table_builder_path, label], check=True)
print(f"""
OUTPUT FILES:
1. refined_results/meta_learning/epsilon/merit_epsilon.csv = Contains the merit values for each model.
2. refined_results/meta_learning/epsilon/models/theoretical_models/ = Contains all the theoretical models (SBS, VBS and AS) for each combination of 'r' and 'l'.
3. refined_results/meta_learning/epsilon/models/pickle_models/ = Contains the models of each configuration of 'l' and 'r' in a pickle format. For each configuration, we have the best and worst models of all iterations (folds). We also have the final model, that is, a model trained with all 300 instances. To test these models with an unseen instance, just run 'src/utils/test_model'.
4. refined_results/meta_learning/features_importance/ = Contains the feature importance of each model.
5. refined_results/meta_learning/logs/ = Contains logs of each run for each model.
""")
print("==========================================================================")

print("\n\n")

print("===================== Running regression_metrics.py =====================")
print(f"""
DESCRIPTION:
This file gets regression metrics (MSE, RMSE, MAE, MAPE and R²) for each model based in the theorical models (AS, VBS and SBS).
""")

subprocess.run(["python", regression_metrics_path, label], check=True)

print("\n")

print(f"""
OUTPUT FILES:
1. MSE, RMSE, MAE, MAPE and R² metrics saved in 'refined_results/meta_learning/epsilon/regression_metrics/'
""")
print("==========================================================================")

print("\n")

print("===================== Running figure_builder.py =====================")
print(f"""
DESCRIPTION:
This file generates the figures about the importance of features for each model.
""")

subprocess.run(["python", figure_builder_metrics_path, label], check=True)

print("==========================================================================")

print("\n")

print("Pipeline finished.")
