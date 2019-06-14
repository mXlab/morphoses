

Expériences

# Expérience 1 : Immobilité (BUG)

* Données: delta euler
* Reward: inverse des delta euler
* Curiosité: OFF
* Learning rate: 0.2
* Gamma: 0.5
* Epsilon: 0.1
* Modèle: Q-tables 5x5x5
* Note: Il y avait un bug dans le programme donc les résultats ne sont peut-être pas fiables.

Note: Modification effectuée à rl_curiosity.py ligne 408 pour augmenter la magnitude des deltas:
```
delta_data = 100 * delta(data, prev_data) / (t - prev_time)
```

Ligne de commande:
```
python rl_curiosity.py --n-action-bins 3 --use-delta-euler --reward-inv-delta-euler --curiosity-weight 0 --model tables --n-state-tiles 5 --use-robot --time-step 1 --learning-rate 0.2 --gamma 0.5 --send-port 7765 --receive-port 7767
```

Résultats:
* Très exploratoire dans la première partie
* Après avoir rencontré des obstacles il se met à reculer
* À un moment, le robot est plus ou moins immobile mais fait bouger ses moteurs à l'intérieur
* Vers la fin: mouvements saccadés: bouge, attends, bouge, attends, ...

# Expérience 2 : Immobilité

* Données: delta euler
* Reward: inverse des delta euler
* Curiosité: OFF
* Learning rate: 0.2
* Gamma: 0.5
* Epsilon: 0.1
* Modèle: Q-tables 5x5x5

Note: Modification effectuée à rl_curiosity.py ligne 408 pour augmenter la magnitude des deltas:
```
delta_data = 10 * delta(data, prev_data) / (t - prev_time)
```

Ligne de commande:
```
python rl_curiosity.py --n-action-bins 3 --use-delta-euler --reward-inv-delta-euler --curiosity-weight 0 --model tables --n-state-tiles 5 --use-robot --time-step 1 --learning-rate 0.2 --gamma 0.5 --send-port 7765 --receive-port 7767
```

Résultats:
* Très exploratoire et "chaotique" dans la première partie
* Après avoir rencontré des obstacles il se met à reculer
* À un moment, le robot se met à se dandiner
* Après 4-5 minutes, il atteint son objectif et demeure immobile
* Hugo: Pas nécessairement perçu comme un apprentissage mais plutôt comme si le robot s'était calmé
* Nous notons également que s'il y a des obstacles le robot semble adopter un comportement différent et n'arrive pas à apprendre à se stabiliser.

# Expérience 3 : Position angulaire cible

* Données: euler
* Reward: position précise
* Curiosité: OFF
* Learning rate: 0.1
* Gamma: 0.7
* Epsilon: 0.1
* Modèle: Q-tables 5x5x5

Ligne de commande:
```
python rl_curiosity.py --n-action-bins 3 --use-euler --reward-euler-state-robot-1 --curiosity-weight 0 --model tables --n-state-tiles 5 --send-port 7765 --receive-port 7767 --use-robot --time-step 1 --learning-rate 0.1 --gamma 0.7 --epsilon 0.1
```

Résultats:
* Le robot atteint très rapidement la position souhaitée et cesse de bouger

# Expérience 3 : Position angulaire cible

* Données: euler
* Reward: position précise
* Curiosité: OFF
* Learning rate: 0.1
* Gamma: 0.7
* Epsilon: 0.1
* Modèle: Q-tables 5x5x5

Ligne de commande:
```
python rl_curiosity.py --n-action-bins 3 --use-euler --reward-euler-state-robot-1 --curiosity-weight 0 --model tables --n-state-tiles 5 --send-port 7765 --receive-port 7767 --use-robot --time-step 1 --learning-rate 0.1 --gamma 0.7 --epsilon 0.1
```

Résultats:
* Le robot atteint très rapidement la position souhaitée et cesse de bouger
* Le robot se retrouve parfois dans des positions où il est "pris" et il continue alors de prendre la même décision (eg. tourner son moteur). Ceci pourrait être dû en partie au fait que le robot n'a accès qu'à la position de la boule et non celle du moteur à l'intérieur. Ceci est vrai par ailleurs non seulement pour cette expérience mais de façon générale.
