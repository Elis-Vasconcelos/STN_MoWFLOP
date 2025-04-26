#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>

#include "../headers/utils/features_csv.h"

using namespace std;

void build_csv(const vector<double> &mo_features, const vector<string> &column_names, 
               const string &rootfolder, const string &instance, const string &folder, 
               const string &subfolder, const string &subsubfolder, const string &filename) {
                
    string separator = "/"; 
    string aux = ".." + separator; 
    string full_path = rootfolder + separator + instance + separator + folder + separator + subfolder + separator + subsubfolder;
    string file_path = aux + full_path + separator + filename;

    
    ofstream file(file_path);

    if (file.is_open()) {
        
        vector<string> updated_column_names = column_names;
        updated_column_names.insert(updated_column_names.begin(), "Instance");

        for (size_t i = 0; i < updated_column_names.size(); ++i) {
            file << updated_column_names[i];
            if (i != updated_column_names.size() - 1) {
                file << ",";
            }
        }
        file << "\n";

        
        file << fixed << setprecision(10);
        file << instance;  
        for (size_t i = 0; i < mo_features.size(); ++i) {
            
            double value = isnan(mo_features[i]) ? 0.0 : mo_features[i];
            file << "," << value;
        }
        file << "\n";

        file.close();
        
    } else {
        cerr << "File path Error: " << file_path << endl;
    }
}
