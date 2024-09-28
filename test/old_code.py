import numpy as np
import pandas as pd
from sklearn import linear_model, datasets
from numpy.linalg import inv
from IPython.display import Image
from IPython.core.display import HTML 

myDataA=pd.read_csv("https://raw.githubusercontent.com/sunchang0124/PPDML/master/Privacy-preserving%20bayesians/preprocessed_dataFile_A.csv")
myDataB=pd.read_csv("https://raw.githubusercontent.com/sunchang0124/PPDML/master/Privacy-preserving%20bayesians/preprocessed_dataFile_B.csv")
colA = myDataA.columns
colB = myDataB.columns
print(colA)
print(colB)
myDataA = myDataA.drop('Unnamed: 0', axis=1)
# Y = myDataB['diag_3']
myDataB = myDataB[colB[0:6]].drop(['Unnamed: 0'], axis=1)    

# X_centralized = np.concatenate((myDataA, myDataB.drop('diag_3', axis=1)), axis=1)
X_centralized = np.concatenate((myDataA, myDataB), axis=1)
B_divide_set = 10

X_a = myDataA # df[col[0:5]] # feature0 - feature4 # myDataA

# # Add one columns with all values of 1 to dataset which uses to calculate b0
b0 = np.ones((1, len(X_a))).tolist()[0]
X_a.insert(loc=0, column='b0', value=b0)

# Calculate X_a.T * X_a locally at data site A 
XaTXa = np.matrix(X_a).T * np.matrix(X_a)
len_A = len(X_a.columns)

# Generate random numbers and add to data at Data Site A
A_randoms = []
for i in range(0, len_A):
    A_randoms.append(np.random.randint(0,5, len(X_a.iloc[:,i])))
    
C_matrix = [] # C_noises is shared between A and B 
for i in range(0, len_A):
#     np.random.seed(2)
    C_matrix.append(np.random.randint(0,5, (len(X_a.iloc[:,i]), len(X_a.iloc[:,i]))))

Sum_noises_A = [] # which will be sent to B
for i in range(0, len_A):
    Sum_noises_A.append(np.add(X_a.iloc[:,i], np.dot(C_matrix[i], A_randoms[i])))

np.save('C_matrix.npy', C_matrix)
C_matrix = np.load('C_matrix.npy')

np.save('Sum_noises_A.npy', Sum_noises_A)
Sum_noises_A = np.load('Sum_noises_A.npy')

X_b = myDataB # df[col[5:10]] # feature5 - feature8 and target feature # myDataB

XbTXb = np.matrix(X_b).T * np.matrix(X_b)
len_B = len(X_b.columns)

Sum_coef_B = []
for i in range(0, len_B):
    Sum_noises_temp = []
    for j in range(0, len_A):
        Sum_noises_temp.append(np.dot(C_matrix[j].transpose(), X_b.iloc[:,i])) 
    Sum_coef_B.append(Sum_noises_temp)

B_random_set = []
for i in range(0, len_A):
#     np.random.seed(3)
    B_random_set.append(np.random.randint(0,5, int(len(X_b.iloc[:,0])/B_divide_set))) 

Sum_noises_B = [] # which will be send to A
for n in range(0, len_B):
    B_noise = []
    for i in range(0, len_A):
        B_random_inter = []
        for j in range(0, len(B_random_set[i])): 
            for k in range(0, B_divide_set):
                B_random_inter.append(B_random_set[i][j])
        B_noise.append(Sum_coef_B[n][i] + B_random_inter)
    Sum_noises_B.append(B_noise)

# Add noises dataset A to the dataset B
Sum_noises_AB = []
for i in range(0, len_B):
    Sum_noises_temp = []
    for j in range(0, len_A):
        Sum_noises_temp.append(np.dot(Sum_noises_A[j], X_b.iloc[:,i])) # X_b[:,i]
    Sum_noises_AB.append(Sum_noises_temp)

np.save('Sum_noises_B.npy', Sum_noises_B)
Sum_noises_B = np.load('Sum_noises_B.npy')
np.save('Sum_noises_AB.npy', Sum_noises_AB)
Sum_noises_AB = np.load('Sum_noises_AB.npy')

A_randoms_Sumset = []
for i in range(0, len_A):
    sum_temp = []
    for j in range(0, int(len(X_a)/B_divide_set)):
        temp = 0
        for k in range(0, B_divide_set):
            temp = temp + A_randoms[i][B_divide_set*j + k]
        sum_temp.append(temp)
        
    A_randoms_Sumset.append(sum_temp)
 
    
Sum_noises_B_Arand = []
for n in range(0, len_B):
    temp = []
    for i in range(0, len_A):
        temp.append(np.subtract(Sum_noises_AB[n][i], np.dot(A_randoms[i],Sum_noises_B[n][i])))
    Sum_noises_B_Arand.append(temp)

np.save('A_randoms_Sumset.npy', A_randoms_Sumset)
A_randoms_Sumset = np.load('A_randoms_Sumset.npy')

np.save('Sum_noises_B_Arand.npy', Sum_noises_B_Arand)
Sum_noises_B_Arand = np.load('Sum_noises_B_Arand.npy')

rand_sums = []
for i in range(0, len_A):
    r_sum = 0
    for j in range(0, len(B_random_set[0])):
        r_sum = r_sum + A_randoms_Sumset[i][j] * B_random_set[i][j]
    rand_sums.append(r_sum)

outcomes = []
for n in range(0, len_B):
    out = []
    for i in range(0, len_A):
        out.append(Sum_noises_B_Arand[n][i] + rand_sums[i])  #Sum_noises_AB[n][i] - 
    outcomes.append(out)

XaTXb = np.matrix(outcomes)[:-1]

XbTXa = XaTXb.T

XaTY = np.matrix(outcomes)[-1]

XbTXb_exclY = XbTXb[:-1].T[:-1]

XbTY = np.delete(XbTXb[-1], -1)

pp_XTX = np.concatenate((np.concatenate((XaTXa, XbTXa), axis=1), np.concatenate((XaTXb, XbTXb_exclY), axis=1)),axis=0) 
pp_XTY = np.concatenate((XaTY, XbTY),axis=1).T

pp_out = np.linalg.inv(pp_XTX) * pp_XTY

b1 = pp_out[1:]
b0 = pp_out.item(0)
print('Coefficients: \n' ,b1)
print('Intercept: ', b0)

from sklearn.preprocessing import StandardScaler

# Standardize the features
scaler = StandardScaler()
X_scaled = scaler.fit_transform(X_centralized)

# Ensure Y_subset is 1D (in case it's a 2D column vector)
Y_subset = Y[:20].ravel()  # Adjust this depending on which rows you selected from X

# Perform linear regression
regr = linear_model.LinearRegression(fit_intercept=True)
regr.fit(X_scaled, Y_subset)

# The coefficients
print('Coefficients: \n', regr.coef_)
print('Intercept: \n', regr.intercept_)