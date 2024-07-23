

Expériences

# Expérience 1 : Immobilité (BUG)

* Données: delta euler
* Reward: inverse des delta euler
* Curiosité: OFF
* Learning rate: 0.2
* Gamma: 0.5
* Epsilon: 0.1
* Modèle: Q-tables 5x5x5
* Policy: E-Greedy (avec np.argmax() ... favorise toujours la première action au début, ce qui explique que le robot roule vers l'avant-gauche)
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
* Policy: E-Greedy (avec np.argmax() ... favorise toujours la première action au début, ce qui explique que le robot roule vers l'avant-gauche)

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
* Policy: E-Greedy (avec np.argmax() ... favorise toujours la première action au début, ce qui explique que le robot roule vers l'avant-gauche)

Ligne de commande:
```
python rl_curiosity.py --n-action-bins 3 --use-euler --reward-euler-state-robot-1 --curiosity-weight 0 --model tables --n-state-tiles 5 --send-port 7765 --receive-port 7767 --use-robot --time-step 1 --learning-rate 0.1 --gamma 0.7 --epsilon 0.1
```

Résultats:
* Le robot atteint très rapidement la position souhaitée et cesse de bouger

# Expérience 4 : Curiosité + immobilisme

* Données: delta euler
* Reward: immobilisme (75%)
* Curiosité: 25%
* Learning rate: 0.1
* Gamma: 0.7
* Epsilon: 0.1
* Modèle: Q-tables 5x5x5
* Policy: E-Greedy (avec np.argmax() ... favorise toujours la première action au début, ce qui explique que le robot roule vers l'avant-gauche)

Ligne de commande:
```
python3 rl_curiosity.py --n-action-bins 3 --use-delta-euler --reward-inv-delta-euler --curiosity-weight 0.25 --model tables --n-state-tiles 5 --send-port 7765 --receive-port 7767 --use-robot --time-step 1 --learning-rate 0.1 --gamma 0.7 --epsilon 0.1 -nf 32
```

Résultats:
* Le robot bouge à travers l'espace et adopte différentes postures et mouvement. Parfois difficile à décrire.
* Par rapport au mode curiosité "pur" l'ajout de la contrainte de l'immobilisme le rend un peu moins actif.

# Expérience 5 : Apprendre à bouger

* Données: delta euler
* Reward: mouvement
* Learning rate: 0.1
* Gamma: 0.7
* Epsilon: 0.1
* Modèle: Q-tables 5x5x5
* Policy: E-Greedy avec argmax corrigé

Ligne de commande:
```
python3 rl_curiosity.py --n-action-bins 3 --use-delta-euler --reward-delta-euler --curiosity-weight 0.0 --model tables --n-state-tiles 5 --send-port 7765 --receive-port 7767 --use-robot --time-step 1 --learning-rate 0.1 --gamma 0.7 --epsilon 0.1 -nf 32
```

Résultats:
* TODO

# Expérience 6 : Rouler drette


* Données: delta euler
* Reward: + mouvement pitch - mouvement roll
* Learning rate: 0.1
* Gamma: 0.7
* Epsilon: 0.1
* Modèle: Q-tables 5x5x5
* Policy: E-Greedy avec argmax corrigé

Ligne de commande:

```
python3 rl_curiosity.py --n-action-bins 3 --use-delta-euler --reward-delta-pitch --reward-inv-delta-roll --curiosity-weight 0.0 --model tables --n-state-tiles 5 --send-port 7765 --receive-port 7767 --use-robot --time-step 1 --learning-rate 0.1 --gamma 0.7 --epsilon 0.1 -nf 32
```


# Expérience 7 : Tanguer

* Données: delta euler
* Reward: - mouvement pitch + mouvement roll
* Learning rate: 0.1
* Gamma: 0.7
* Epsilon: 0.1
* Modèle: Q-tables 5x5x5
* Policy: E-Greedy avec argmax corrigé

```
python3 rl_curiosity.py --n-action-bins 3 --use-delta-euler --reward-inv-delta-pitch --reward-delta-roll --curiosity-weight 0.0 --model tables --n-state-tiles 5 --send-port 7765 --receive-port 7767 --use-robot --time-step 1 --learning-rate 0.1 --gamma 0.7 --epsilon 0.1 -nf 32
```

Résultats:
* Ça semble fonctionner au sens où (1) ça donne un comportement plus de "tangage" que l'expérience 6 et (2) les actions les plus fréquemment sélectionnées sont "gauche" et "droite".
* Par contre le robot ne connaît pas la position actuelle (gauche-droite) de son moteur donc il est incapable d'interpréter que le passage de gauche à droite le fera tanguer. Il faudrait peut-être lui ajouter cette information.

# Expérience 8 : Curiosité (quaternion) + handicap

* Données: quaternion
* Reward: n/a
* Curiosité: 100%
* Learning rate: 0.1
* Gamma: 0.7
* Epsilon: 0.1
* Modèle: Q-tables 5x5x5x5
* Policy: E-Greedy avec argmax corrigé

Ligne de commande:
```
python3 rl_curiosity.py --n-action-bins 3 --use-quaternion --curiosity-weight 1.0 --model tables --n-state-tiles 5 --send-port 7765 --receive-port 7767 --use-robot --time-step 1 --learning-rate 0.1 --gamma 0.7 --epsilon 0.1 -nf 32 
```

Résultats:
* Testé avec une boule avec un handicap (excroissance). Au départ le robot reposait sur son excroissance. Pendant les premières 2-3 minutes il fait des micro-mouvements mais n'arrive jamais à se "libérer".
* Une fois finalement "libéré" le robot bouge à travers l'espace et adopte différentes postures et mouvement. Parfois difficile à décrire.
