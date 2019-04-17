import argparse

parser = argparse.ArgumentParser()
parser.add_argument("data", type=str, help="The CSV file containing the dataset")
parser.add_argument("-n", "--n-hidden", type=int, default=10, help="Number of hidden units per layer")
#parser.add_argument("-m", "--model-file", type=str, default="", help="Model file to load (in order to restart from a certain point)")
#parser.add_argument("-i", "--initial-epoch", type=int, default=0, help="Epoch at which to start training (useful for resuming previous training)")
parser.add_argument("-e", "--n-epochs", type=int, default=100, help="Number of epochs to train (total)")
parser.add_argument("-v", "--validation-split", type=float, default=0.2, help="Percentage of the training set used for validation")

parser.add_argument("-D", "--model-directory", type=str, default=".", help="The directory where to save models")
parser.add_argument("-P", "--prefix", type=str, default="ann-regression-", help="Prefix to use for saving files")
parser.add_argument("-b", "--batch-size", type=int, default=128, help="The batch size")
#parser.add_argument("-p", "--batch-save-period", type=int, default=0, help="Period at which to save weights (ie. after every X batch, 0 = no batch save)")
#parser.add_argument("-fp", "--first-epoch-params", type=str, default=None, help="A formatted string describing the evolution of batch size and save period during the first epoch")

parser.add_argument("-c", "--classification", default=False, action='store_true', help="Treat as a classification problem instead of regression")
parser.add_argument("--n-bins", type=int, default=5, help="Number of bins to use for classification (only valid if used with --classification)")

args = parser.parse_args()

# This is to allow the inclusion of common python code in the ../mp folder
import sys
sys.path.append('../')

import numpy as np
import pandas
import os
from keras.models import Sequential
from keras.layers import Dense
from keras.utils import np_utils
from keras.callbacks import EarlyStopping, ModelCheckpoint
from sklearn.model_selection import train_test_split

import mp.preprocessing as mpp

# Load dataset.
dataframe = pandas.read_csv(args.data)
dataset = dataframe.values

# Pre-process.
if args.classification:
    bins = args.n_bins
else:
    bins = None
X, Y, _, _ = mpp.preprocess_data(dataset, prune_experiments=True, bins = bins)

if args.classification:
    Y = [ mpp.speed_steering_to_class(y, bins) for y in Y ]
    print(np.histogram(Y))
    Y = np_utils.to_categorical(Y, bins*bins)

print(X.shape)
print(Y.shape)

# Split into different sets.
X_train, X_test, Y_train, Y_test = train_test_split(X, Y)

# Create model
n_hidden = args.n_hidden

model = Sequential()
model.add(Dense(n_hidden, input_dim=9, kernel_initializer='normal', activation='relu'))

if args.classification:
    print(Y.shape)
    model.add(Dense(Y.shape[1], kernel_initializer='normal', activation='softmax'))
    model.compile(loss='categorical_crossentropy', optimizer='adam', metrics=['accuracy'])

else:
    model.add(Dense(2, kernel_initializer='normal', activation='tanh'))
    #model.add(Dense(2, kernel_initializer='normal', activation='linear'))

    # Compile model
    model.compile(loss='mean_squared_error', optimizer='adam', metrics=["mse", "mae"])


# Create callbacks.
if not os.path.exists(args.model_directory):
	os.mkdir(args.model_directory)

filepath_prefix="{dir}/{prefix}-nhu{n_hidden}-".format(dir=args.model_directory,prefix=args.prefix,n_hidden=args.n_hidden)
filepath_epoch=filepath_prefix+"e{epoch:03d}.hdf5"

callbacks = [EarlyStopping(monitor='val_loss', patience=2),
             ModelCheckpoint(filepath=filepath_epoch, monitor='val_loss', save_best_only=False)]

# Fit model
history = model.fit(X_train, # Features
                    Y_train, # Target vector
                    epochs=args.n_epochs, # Number of epochs
                    callbacks=callbacks, # Early stopping
#                      verbose=0, # Print description after each epoch
                    batch_size=args.batch_size, # Number of observations per batch
                    verbose=1,
                    validation_split=args.validation_split)
model.save(filepath_prefix+"final.hdf5")

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
