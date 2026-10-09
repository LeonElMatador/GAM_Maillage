# GAM_Maillage

# Générer une triangulation aléatoire
Le programme peut ajouter un nombre choisi de points aléatoires à un triangle initial puis exporter la triangulation au format OFF dans le dossier OFF/. Par exemple, pour ajouter 50 points :
```
make
./tp1 --random 50
```
Le maillage obtenu est écrit dans OFF/random_50_points.off. Le générateur utilise une graine fixe, ce qui rend les résultats reproductibles.

# Simulation de chaleur
La courbure et la simulation thermique se lancent séparément. Pour calculer la courbure :
```
make
./tp1 --curvature ./OFF/queen.off
```
Le résultat est écrit dans curvature.off. Pour lancer la simulation thermique, indiquez le nombre d'itérations :

```
./tp1 --heat ./OFF/queen.off 10000
```
La simulation écrit des maillages colorés toutes les 2000 itérations, puis à la dernière itération. Par exemple, pour effectuer 500 itérations :
```
./tp1 --heat ./OFF/queen.off 500
```