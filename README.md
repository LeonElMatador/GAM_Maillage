# GAM_Maillage

## Générer une triangulation aléatoire

Le programme peut ajouter un nombre choisi de points aléatoires à un triangle
initial puis exporter la triangulation au format OFF dans le dossier `OFF/`.
Par exemple, pour ajouter 50 points :

```sh
make
./tp1 --random 50
```

Le maillage obtenu est écrit dans `OFF/random_50_points.off`. Le générateur
utilise une graine fixe, ce qui rend les résultats reproductibles.

## Test du laplacien 

Pour générer les mesh colorés à partir de la courbure et de la diffusion thermique lancez :

```sh
make
./tp1 ./OFF/queen.off
```

Un fichier `OFF/curvature.off` et des fichier `OFF/queen_xxxx.off` seront crées.