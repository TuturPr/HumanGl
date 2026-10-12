# HumanGL

Introduction à la modélisation hiérarchique (projet 42) : un personnage articulé,
animé avec **notre propre pile de matrices** (sans GLM ni fonctions matricielles
OpenGL dépréciées), et rendu en OpenGL 4.1 core.

## Compilation et lancement

Dépendances (Debian / Ubuntu) :

```sh
sudo apt install build-essential libglfw3-dev libgtest-dev
```

| Commande      | Effet                                   |
|---------------|-----------------------------------------|
| `make`        | compile `./humangl`                     |
| `make test`   | compile et lance les tests unitaires    |
| `make clean`  | supprime les objets (`.tmp/`)           |
| `make fclean` | supprime aussi les exécutables          |
| `make re`     | recompile tout                          |

```sh
make && ./humangl
```

## Contrôles

| Entrée               | Action                                   |
|----------------------|------------------------------------------|
| `1` à `6`            | Idle, Walk, Jump, Disco, Kung-fu, Thomas flare |
| `Espace`             | pause / reprise                          |
| Clic gauche + glisser | tourner autour du personnage            |
| Molette              | zoom                                     |
| `Échap`              | quitter                                  |

Le panneau **ImGui** permet de tout changer pendant l'exécution :
- l'animation, sa vitesse, la durée du fondu et la pause ;
- la taille (largeur, longueur, profondeur) et la couleur de chaque partie, avec
  une option pour modifier les deux côtés à la fois ;
- l'affichage des parties bonus, du fil de fer, de l'ombre et du sol.

## Points demandés en soutenance

- **Fonction de dessin** : `HumanModel::drawPart` (`src/skeleton/HumanModel.cpp`).
  C'est le seul endroit où une partie est dessinée, une fois par partie. Elle
  dessine une forme 1x1x1 à l'origine de la matrice courante, mise à l'échelle
  dans son propre `push` / `pop`.
- **Pile de matrices** : `src/math/MatrixStack.{hpp,cpp}`.
- **Appels de la fonction de dessin** : un par partie. Le torse est dessiné dans
  `HumanModel::draw`, le cou et la tête dans `drawHead`, les membres dans
  `drawArm` et `drawLeg` (via `drawSegment`).
- **Hiérarchie** : le torse est la racine ; le cou et la tête, les bras (bras,
  avant-bras, main) et les jambes (cuisse, jambe, pied) en dépendent. Voir
  l'en-tête de `src/skeleton/HumanModel.hpp`.
- **Pile de matrices en bref** : chaque membre est dessiné entre un `push` et un
  `pop`, si bien qu'un enfant hérite des transformations de son parent mais pas
  l'inverse. Les transformations sont post-multipliées, donc le code se lit du
  parent vers l'enfant. L'échelle d'une partie n'est appliquée qu'à l'intérieur
  de `drawPart`, elle ne se propage jamais aux enfants.
- **Tailles des membres** : à l'exécution via le panneau ImGui, ou dans le code
  dans `src/skeleton/BodyDimensions.cpp` (une ligne par partie).

## Organisation

```
src/math/       vecteurs, matrices, pile de matrices
src/skeleton/   pose, dimensions, modèle hiérarchique, limites articulaires, contact au sol
src/animation/  animations (fonctions pures du temps), lecteur avec fondus, clips/
src/geometry/   forme unitaire 1x1x1
src/render/     shaders, maillage GPU, caméra, ombre, rendu de la scène
src/ui/         panneau ImGui
src/platform/   fenêtre GLFW
src/app/        boucle principale
tests/          tests unitaires GoogleTest
```

Les raisons de chaque choix sont détaillées dans [`docs/DECISIONS.md`](docs/DECISIONS.md).
