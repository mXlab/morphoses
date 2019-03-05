import numpy
import pandas
from keras.models import Sequential
from keras.layers import Dense
from keras.callbacks import EarlyStopping, ModelCheckpoint
from keras.wrappers.scikit_learn import KerasRegressor
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import MinMaxScaler
from sklearn.model_selection import cross_val_score
from sklearn.model_selection import KFold
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import Pipeline

# load dataset
dataframe = pandas.read_csv("unity_interop/seed_data_with_steering_medium.csv")
dataset = dataframe.values

n_hidden = 10
max_speed = 15.0
max_steering = 45.0

# split into input (X) and output (Y) variables
X = dataset[:,2:8]
Y = dataset[:,8:10]

# Normalize X.
print(Y.shape)
scalerX = MinMaxScaler()
print(scalerX.fit(X))
X=scalerX.transform(X)

# Normalize Y.
Y[:,0] /= max_speed
Y[:,1] /= max_steering

print(X.shape)
print(Y.shape)

# Split into different sets.
X_train, X_test, y_train, y_test = train_test_split(X, Y)

# Create model
model = Sequential()
model.add(Dense(n_hidden, input_dim=6, kernel_initializer='normal', activation='relu'))
model.add(Dense(2, kernel_initializer='normal'))

# Compile model
model.compile(loss='mean_squared_error', optimizer='adam', metrics=["mse", "mae"])

callbacks = [EarlyStopping(monitor='val_loss', patience=2),
             ModelCheckpoint(filepath='best_model.hdf5', monitor='val_loss', save_best_only=True)]

history = model.fit(X_train, # Features
                    y_train, # Target vector
                    epochs=20, # Number of epochs
                    callbacks=callbacks, # Early stopping
#                      verbose=0, # Print description after each epoch
                    batch_size=256, # Number of observations per batch
                    verbose=1,
                    validation_split=0.2)
model.save("final_model.hdf5")

# seed = 7
# numpy.random.seed(seed)
# # evaluate model with standardized dataset
# estimator = KerasRegressor(build_fn=baseline_model, epochs=100, batch_size=5, verbose=0)
#
#
# kfold = KFold(n_splits=10, random_state=seed)
# results = cross_val_score(estimator, X, Y, cv=kfold)
# print("Results: %.2f (%.2f) MSE" % (results.mean(), results.std()))
