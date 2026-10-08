# GAM_Maillage

## Tests

Lancer les tests du maillage et de triangulation avec :

```sh
make test
```

Cela compile le programme puis exécute les cas de test via `tp1 --test`.
Les cas de triangulation écrivent également leurs maillages finis au format OFF
dans le dossier `OFF/` : `triangle_plus_point.off`, `square_plus_point.off` et
`10_points.off`.