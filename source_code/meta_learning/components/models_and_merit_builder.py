import argparse
import pandas as pd
import numpy as np
import os
import sys
import pickle
from sklearn.ensemble import RandomForestRegressor
from sklearn.multioutput import MultiOutputRegressor
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler

def models_and_merit_builder(file_path, arquive, label):
    output_path = f'result/{label}/features_importance/'
    models_merit_path = f'result/{label}/models/theoretical_models/{arquive}/'
    models_pickle_path = f'result/{label}/models/pickle_models/{arquive}/'
    os.makedirs(output_path, exist_ok=True)
    os.makedirs(models_merit_path, exist_ok=True)
    os.makedirs(models_pickle_path, exist_ok=True)

    df = pd.read_csv(file_path)

    indices_dict = {
        "epsilon": {
            "nsga": ['1', '101', '112', '139', '143', '157', '164', '165', '17', '176', '178', '185', '189', '192', '207', '217', '230', 
                    '235', '240', '242', '246', '262', '302', '309', '311', '314', '324', '332', '333', '341', '352', '355', '359', '361', '363', '37', '382', 
                    '393', '400', '421', '423', '430', '431', '436', '441', '451', '460', '48', '485', '494', '497', '501', '502', '503', '504', '505', '59', 
                    '87', '91', 'A', 'B', 'C', 'E', 'G'],

            "moead": ['224', '151', '401', '413', '33', '234', '110', '492', '498', '175', '486', '259', 'D', '266', '456', '12', '219', '10', '109', '353', '471', '28', '74', '213', '40', '495', '452', '32', '323', '312', '98', 
                    '482', '399', '134', '330', '337', '376', '308', '43', '409', '261', '319', '412', '303', '487', '70', '200', '244', '457', '170', '293', '467', '272', '105', '208', '179', '470', '114', '158', '57', '247', 
                    '466', '472', '153', '288', '202', '198', '496', 'J', '68', '191', '390', '69', '236', '156', '46', '271', '9', '162', '243', '469', '93', '256', 'F', '386', '276', '26', '295', '342', '348', '147', '15', '173', 
                    '124', '142', '222', '260', '103', '35', '334', '306', '375', '107', '354', '104', '440', '372', '232', '106', '255', '478', '120', '167', '435', '119', '65', '63', '113', '410', '350', '397', '346', '257', 
                    '174', '161', '152', '392', '286', '181', '473', '378', '432', '77', '83', '437', '320', '381', '290', '34', '64', '365', '403', '206', '67', '160', '414', '491', '51', '367', '190', '387', '195', '78', '483', 
                    '38', '194', '117', '248', '252', '325', '304', '61', '340', '163', '449', '488', '241', '201', '458', '339', '475', '116', '249', '299', '94', '128', '214', '80', '203', '434', '385', '277', '297', '216', '27', 
                    '479', '220', '402', '212', '368', '357', '237', '29', '417', '329', '226', '53', '445', '425', '233', '130', '81', '264', '131', '141', '199', '186', '159', '269', '298', '274', '88', '347', '96', '49', '404', 
                    '209', '450', '318', '144', '47', '405', '45', '172', '238', '84', '132', '484', '127', '281', '433', '411', '149', '477', '55', '317']
        }
    }

    if label not in indices_dict:
        raise ValueError(f"Invalid label")

    nsga_indices = indices_dict[label]["nsga"]
    moead_indices = indices_dict[label]["moead"]

    nsga_instances = df[df.iloc[:, 0].astype(str).isin(nsga_indices)]
    moead_instances = df[df.iloc[:, 0].astype(str).isin(moead_indices)]

    scaler = StandardScaler()
    n_folds = 100
    scores = []

    feature_importances = np.zeros(df.shape[1] - 3)  

    best_model = None
    worst_model = None
    best_score = float('inf')
    worst_score = float('-inf')

    as_predictions = []
    vbs_predictions = []
    sbs_predictions = []

    sbs_best_model = []
    fold_numbers = []

    if label == "epsilon":
        size_nsga_test = 50
    else:
        size_nsga_test = 6
    
    for fold_number in range(n_folds):
        fold_numbers.append(fold_number)

        selected_nsga_indices_test = np.random.choice(nsga_indices, size=size_nsga_test, replace=False)
        selected_nsga_indices_train = list(set(nsga_indices) - set(selected_nsga_indices_test))

        #---------- BUILDING THE 90/10 TRAIN/TEST SPLIT ----------
        nsga_train = nsga_instances[nsga_instances.iloc[:, 0].astype(str).isin(selected_nsga_indices_train)]
        nsga_test = nsga_instances[nsga_instances.iloc[:, 0].astype(str).isin(selected_nsga_indices_test)]
        
        moead_train, moead_test = train_test_split(moead_instances, test_size=0.1, random_state=np.random.randint(0, 1000))
        
        train_data = pd.concat([nsga_train, moead_train])
        test_data = pd.concat([nsga_test, moead_test])
        
        X_train, y_train = train_data.iloc[:, 1:-2], train_data.iloc[:, -2:]
        X_test, y_test = test_data.iloc[:, 1:-2], test_data.iloc[:, -2:]
        
        #---------- NORMALIZATION OF FEATURES ----------
        scaler.fit(X_train)
        X_train_scaled = scaler.transform(X_train)
        X_test_scaled = scaler.transform(X_test)

        #---------- MULTIOUTPUT RANDOM FOREST REGRESSOR ----------        
        model = MultiOutputRegressor(RandomForestRegressor(n_estimators=1000, max_depth=2, min_samples_split=2, min_samples_leaf=3, random_state=0)) #Model and its parameters choosen by RandomSearchCV
        model.fit(X_train_scaled, y_train) 

        #MEAN OF FEATURE IMPORTANCE OF ALL ESTIMATORS
        feature_importances += np.mean([est.feature_importances_ for est in model.estimators_], axis=0)
        
        y_pred = pd.DataFrame(model.predict(X_test_scaled), columns=y_train.columns, index=y_test.index) #PREDICTION
        
        #---------- METRIC CALCULATION ----------        
        AS = y_train.columns[np.argmin(y_pred.values, axis=1)]
        VBS = y_test.idxmin(axis=1)
        SBS = y_train.mean().idxmin()

        sbs_best_model.append(str(SBS))
        
        rhv_AS = np.array([y_test.loc[y_test.index[i], AS[i]] for i in range(len(X_test))])
        rhv_SBS = np.array([y_test.loc[y_test.index[i], SBS] for i in range(len(X_test))])
        rhv_VBS = np.array([y_test.at[y_test.index[i], VBS.iloc[i]] for i in range(len(X_test))])

        as_predictions.append(rhv_AS)
        vbs_predictions.append(rhv_VBS)
        sbs_predictions.append(rhv_SBS)
        
        rhv_AS_mean = rhv_AS.mean()
        rhv_SBS_mean = rhv_SBS.mean()
        rhv_VBS_mean = rhv_VBS.mean()
        
        if rhv_SBS_mean == rhv_VBS_mean:
            raise Exception("rhv_SBS_mean == rhv_VBS_mean")
        else:
            m = (rhv_AS_mean - rhv_VBS_mean) / (rhv_SBS_mean - rhv_VBS_mean)
        
        scores.append(m)
        
        #Finding the best and worst models
        if m < best_score:
            best_score = m
            best_model = model
        
        if m > worst_score:
            worst_score = m
            worst_model = model

    #---------- Calculates the average importance of the features from all itereations (folds) ----------    
    feature_importances /= n_folds 
    feature_importance_df = pd.DataFrame({
        'Feature': df.columns[1:-2],
        'Importance': feature_importances
    })
    feature_importance_df.sort_values(by='Importance', ascending=False, inplace=True)
    
    feature_importance_df.to_csv(f"{output_path}{arquive}_features_importance.csv", index=False)
    
    print(np.mean(scores))
    print([float(x) for x in scores])

    # final_model = Training a model with the whole data
    X, y = df.iloc[:, 1:-2], df.iloc[:, -2:]
    scaler.fit(X)
    X_scaled = scaler.transform(X)
    
    final_model = MultiOutputRegressor(RandomForestRegressor(n_estimators=1000, max_depth=2, min_samples_split=2, min_samples_leaf=3, random_state=0))
    final_model.fit(X_scaled, y)
    
    # ---------- Storing the pickle models (best_model, worst_model and final_model) ----------
    with open(f"{models_pickle_path}best_model_{arquive}.pickle", 'wb') as f:
        pickle.dump(best_model, f)
    
    with open(f"{models_pickle_path}worst_model_{arquive}.pickle", 'wb') as f:
        pickle.dump(worst_model, f)
    
    with open(f"{models_pickle_path}final_model_{arquive}.pickle", 'wb') as f:
        pickle.dump(final_model, f)

    # ---------- Calculates the average performance of AS, VBS and SBS models and store it in "models_merit_path" ----------
    instances = df.iloc[:, 0]
    as_predictions_mean = np.mean(as_predictions, axis=0)
    vbs_predictions_mean = np.mean(vbs_predictions, axis=0)
    sbs_predictions_mean = np.mean(sbs_predictions, axis=0)
    
    as_df = pd.DataFrame({'Instance': instances.iloc[:len(as_predictions_mean)], 'AS_Prediction': as_predictions_mean})
    vbs_df = pd.DataFrame({'Instance': instances.iloc[:len(vbs_predictions_mean)], 'VBS_Prediction': vbs_predictions_mean})
    sbs_df = pd.DataFrame({'Instance': instances.iloc[:len(sbs_predictions_mean)], 'SBS_Prediction': sbs_predictions_mean})
    
    os.makedirs(f"{models_merit_path}AS/", exist_ok=True)
    os.makedirs(f"{models_merit_path}VBS/", exist_ok=True)
    os.makedirs(f"{models_merit_path}SBS/", exist_ok=True)

    as_df.to_csv(f"{models_merit_path}AS/{arquive}_ASModel.csv", index=False)
    vbs_df.to_csv(f"{models_merit_path}VBS/{arquive}_VBSModel.csv", index=False)
    sbs_df.to_csv(f"{models_merit_path}SBS/{arquive}_SBSModel.csv", index=False)
    
    # Stores the chosen SBS in each fold
    sbs_df2 = pd.DataFrame({'Fold': fold_numbers, 'SBS_Best_Models': sbs_best_model})
    sbs_file_path = f"{models_merit_path}SBS/{arquive}_SBS_chosen_algorithms.csv"
    sbs_df2.to_csv(sbs_file_path, index=False)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file_path", type=str)
    parser.add_argument("arquive", type=str)
    parser.add_argument("label", type=str)
    args = parser.parse_args()
    
    models_and_merit_builder(args.file_path, args.arquive, args.label)