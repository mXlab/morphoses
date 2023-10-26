# DevNotes

## Problème d'interruption du ESP

Issue liée: [#91](https://github.com/mXlab/morphoses/issues/91)

Causes possibles et solutions envisagées
- Core dump
- Problème de mémoire
- Déconnection du wifi -> reconnection / reboot; utilisation d'un routeur plus puissant; utilisation de channels prédéterminés communs
- Batterie critical (mode sleep) (possiblement dû à un spike) -> logging / affichage sans sleep pour tester
- Streaming constant overflow un buffer -> envoi des données sur demande
- Utilisation des bundles OSC -> refactoring / enlever les bundles
