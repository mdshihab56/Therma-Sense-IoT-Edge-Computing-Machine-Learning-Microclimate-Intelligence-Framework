import os
import numpy as np
import pickle
from sklearn.neural_network import MLPClassifier
from sklearn.ensemble import RandomForestRegressor

os.makedirs('models', exist_ok=True)


np.random.seed(42)
X_train = np.random.rand(1000, 4) * 40
y_ann = np.random.randint(0, 2, size=(1000,))
y_reg = X_train[:, 0] + (X_train[:, 2] * 0.15) + np.random.normal(0, 1, 1000)

print(" Re-training Scikit-Learn ANN (MLP) with 4 Features...")
ann = MLPClassifier(hidden_layer_sizes=(32, 16), max_iter=500, random_state=42)
ann.fit(X_train, y_ann)

print(" Re-training Random Forest Regressor with 4 Features...")
regressor = RandomForestRegressor(n_estimators=50, random_state=42)
regressor.fit(X_train, y_reg)

with open('models/ann_model.pkl', 'wb') as f: pickle.dump(ann, f)
with open('models/regressor.pkl', 'wb') as f: pickle.dump(regressor, f)
print(" Success! Models rebuilt with 4 features.")