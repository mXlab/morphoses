import argparse

parser = argparse.ArgumentParser()
parser.add_argument("data", type=str, help="The CSV file containing the dataset")
parser.add_argument("-n", "--n-hidden", type=int, default=10, help="Number of hidden units per layer")
#parser.add_argument("-m", "--model-file", type=str, default="", help="Model file to load (in order to restart from a certain point)")
#parser.add_argument("-i", "--initial-epoch", type=int, default=0, help="Epoch at which to start training (useful for resuming previous training)")
parser.add_argument("-e", "--n-epochs", type=int, default=100, help="Number of epochs to train (total)")
parser.add_argument("-v", "--validation-split", type=float, default=0.2, help="Percentage of the training set used for validation")

parser.add_argument("-D", "--model-directory", type=str, default=".", help="The directory where to save models")
parser.add_argument("-P", "--prefix", type=str, default="lstm-weights-", help="Prefix to use for saving files")
parser.add_argument("-b", "--batch-size", type=int, default=128, help="The batch size")
#parser.add_argument("-p", "--batch-save-period", type=int, default=0, help="Period at which to save weights (ie. after every X batch, 0 = no batch save)")
#parser.add_argument("-fp", "--first-epoch-params", type=str, default=None, help="A formatted string describing the evolution of batch size and save period during the first epoch")

args = parser.parse_args()

import numpy as np
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

n_hidden = args.n_hidden
min_speed = -15.0
max_speed = 15.0
min_steering = -45.0
max_steering = 45.0

def quaternion_to_euler(x, y, z, w):
    import math
    t0 = +2.0 * (w * x + y * z)
    t1 = +1.0 - 2.0 * (x * x + y * y)
    X = math.degrees(math.atan2(t0, t1))

    t2 = +2.0 * (w * y - z * x)
    t2 = +1.0 if t2 > +1.0 else t2
    t2 = -1.0 if t2 < -1.0 else t2
    Y = math.degrees(math.asin(t2))

    t3 = +2.0 * (w * z + x * y)
    t4 = +1.0 - 2.0 * (y * y + z * z)
    Z = math.degrees(math.atan2(t3, t4))

    return X, Y, Z

# Create input matrix X.
pos_X = np.array(dataset[:,2:4],dtype='float64')
quat_X = np.array(dataset[:,4:8],dtype='float64')
euler_X = np.matrix([ quaternion_to_euler(q[0], q[1], q[2], q[3]) for q in quat_X ])
euler_X /= 180

# Join blocks.
X = np.concatenate((pos_X, quat_X, euler_X), axis=1)

# Normalize X.
scalerX = MinMaxScaler()
scalerX.fit(X)
X=scalerX.transform(X)

# Create target matrix Y.
speed_y = np.array(dataset[:,8],dtype='float64')
steering_y = np.array(dataset[:,9],dtype='float64')

speed_y = np.interp(speed_y, (min_speed, max_speed), (0, 1))
steering_y = np.interp(steering_y, (min_steering, min_steering), (0, 1))

# Join blocks.
Y = np.column_stack((speed_y, steering_y))

print(X.shape)
print(Y.shape)

# Split into different sets.
X_train, X_test, Y_train, Y_test = train_test_split(X, Y)

# Create model
model = Sequential()
model.add(Dense(n_hidden, input_dim=9, kernel_initializer='normal', activation='relu'))
model.add(Dense(2, kernel_initializer='normal', activation='tanh'))

# Compile model
model.compile(loss='mean_squared_error', optimizer='adam', metrics=["mse", "mae"])

filepath_prefix="{dir}/{prefix}-nhu{n_hidden}-".format(dir=args.model_directory,prefix=args.prefix,n_hidden=args.n_hidden)
filepath_epoch=filepath_prefix+"e{epoch:03d}.hdf5"

callbacks = [EarlyStopping(monitor='val_loss', patience=2),
             ModelCheckpoint(filepath=filepath_epoch, monitor='val_loss', save_best_only=False)]

history = model.fit(X_train, # Features
                    Y_train, # Target vector
                    epochs=args.n_epochs, # Number of epochs
                    callbacks=callbacks, # Early stopping
#                      verbose=0, # Print description after each epoch
                    batch_size=args.batch_size, # Number of observations per batch
                    verbose=1,
                    validation_split=args.validation_split)
model.save("final_model.hdf5")

scores = model.evaluate(X_test, Y_test, verbose=1)
print(scores)
print("%s: %.2f%%" % (model.metrics_names[1], scores[1]*100))

# seed = 7
# np.random.seed(seed)
# # evaluate model with standardized dataset
# estimator = KerasRegressor(build_fn=baseline_model, epochs=100, batch_size=5, verbose=0)
#
#
# kfold = KFold(n_splits=10, random_state=seed)
# results = cross_val_score(estimator, X, Y, cv=kfold)
# print("Results: %.2f (%.2f) MSE" % (results.mean(), results.std()))
