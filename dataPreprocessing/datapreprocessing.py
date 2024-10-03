import pandas as pd
import numpy as np

# Load the dataset
diabetes_data = pd.read_csv('diabetic_data.csv', on_bad_lines='skip')

# Replace '?' with NaN for easier filtering
diabetes_data.replace('?', np.nan, inplace=True)

# Drop rows with any NaN values in the selected columns
filtered_data = diabetes_data[['race', 'diabetesMed', 'medical_specialty', 'gender', 'age', 'time_in_hospital']].dropna()

# Create File 1 (race, diabetesMed, medical_specialty)
file1_data = filtered_data[['race', 'diabetesMed', 'medical_specialty']]

# Create File 2 (gender, age, time_in_hospital)
file2_data = filtered_data[['gender', 'age', 'time_in_hospital']]

# Save the partitions as separate CSV files
file1_data.to_csv('file1.csv', index=False)
file2_data.to_csv('file2.csv', index=False)

print("Files have been successfully split and saved as 'file1.csv' and 'file2.csv'.")
