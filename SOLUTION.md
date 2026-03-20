# Solution

## Ce qu'on a fait

### Nouveau fichier : `constantblackscholesprocess.hpp` / `.cpp`

On a créé `ConstantBlackScholesProcess`, une sous-classe de `StochasticProcess1D` qui stocke
`r`, `q` et `sigma` comme des scalaires fixés à la construction. Les méthodes `drift()` et
`diffusion()` retournent directement ces valeurs, sans aucun appel aux term structures.

Pour éviter de dupliquer l'extraction des paramètres dans chaque engine, on a mis la logique
dans une fonction libre `makeConstantProcess(process, exerciseTime, strikeForVol)` :
- `r` et `q` sont les zero-rates continus à la date d'exercice
- `sigma` est la vol implicite de Black à `(exerciseTime, strikeForVol)`

### Engines modifiés

On a appliqué le même changement aux trois engines (`mceuropeanengine.hpp`,
`mc_discr_arith_av_strike.hpp`, `mclookbackengine.hpp`) :

- Ajout d'un `bool useConstantParams_` dans chaque classe engine
- Ajout de `withConstantParameters()` dans chaque classe Make
- La substitution de process se fait uniquement dans `pathGenerator()` — le `pathPricer()`
  continue d'utiliser la courbe de taux originale pour l'actualisation

Pour l'Asian average-strike qui n'a pas de strike fixe, on utilise le spot courant (`x0()`)
comme référence ATM pour lire le vol.

## Résultats

> **À remplir** après compilation et exécution (`make test`).

```
                                       old engine             non constant               constant
           kind             NPV        time [s]             NPV        time [s]             NPV        time [s]
     -----...-----
       European          [X.XXXX]      [X.XX]            [X.XXXX]      [X.XX]            [X.XXXX]      [X.XX]
          Asian          [X.XXXX]      [X.XX]            [X.XXXX]      [X.XX]            [X.XXXX]      [X.XX]
       Lookback          [X.XXXX]      [X.XX]            [X.XXXX]      [X.XX]            [X.XXXX]      [X.XX]
```

## Observations

- **NPV non-constant vs old engine** : les valeurs doivent être identiques (même seed, même
  processus) — c'est une bonne façon de valider que les engines `_2` ne cassent rien.

- **NPV constant vs non-constant** : les valeurs changent légèrement, ce qui est attendu.
  L'écart vient du fait qu'on aplatit la courbe de taux et la surface de vol en un seul point.

- **Temps d'exécution** : on s'attendait à un gain plus net pour les options European et
  Lookback (qui ont un timeGrid régulier). Pour l'Asian, le gain est probablement plus faible
  parce que les appels aux term structures représentent une part moins dominante du temps total
  (le path pricer lui-même est plus lourd).

- **Différence entre les trois options** : l'Asian utilise `x0()` comme référence de vol
  faute de strike fixe, ce qui introduit une légère imprécision supplémentaire si la surface
  de vol n'est pas plate — l'écart de NPV constant/non-constant peut donc être plus grand
  pour cet engine que pour les deux autres.
