# Choix de conception

Ce document explique, pour chaque partie du projet, **ce qui a été fait et pourquoi
ce choix plutôt qu'un autre**. Le critère de départage a toujours été le même :
la solution la plus propre, la plus lisible et la plus simple à expliquer en soutenance.

---

## 1. Audit et nettoyage du dépôt

**Constat de départ.** Le dépôt d'origine utilisait GLM pour toutes ses
transformations, embarquait `stb` presque entier, un chargeur OBJ, la
documentation, les exemples et tous les backends de Dear ImGui, ainsi que des
classes VAO/VBO/EBO/Texture qui ne servaient plus.

**Ce qui a été supprimé** (commits `DEL:` directement sur `main`, comme convenu) :
GLM, `stb`, le chargeur OBJ, l'ancien code du modèle, les fichiers ImGui inutiles
(docs, exemples, backends autres que GLFW / OpenGL 3).

**Pourquoi.** Le sujet exige *« your own matrices and transformations »* : garder
GLM rendait la partie obligatoire invalide. Le reste n'était appelé par aucun
fichier ; le garder ne faisait qu'allonger la compilation et la lecture.

---

## 2. Bibliothèque mathématique maison (`src/math/`)

- `Vec3`, `Mat4` (stockage *column-major*, identique à ce qu'attend OpenGL), `MathUtils`.
- `MatrixStack` : `push`, `pop`, `translate`, `rotate`, `rotateX/Y/Z`, `scale`.

**Pourquoi column-major ?** `glUniformMatrix4fv` reçoit la matrice telle quelle,
sans transposition : aucune conversion cachée à expliquer.

**Pourquoi post-multiplier dans la pile ?** C'est le comportement de l'ancienne
pile OpenGL (`glPushMatrix`, `glTranslate`…). Le code se lit alors dans l'ordre
de la hiérarchie : d'abord le parent, puis l'enfant. C'est ce qu'un évaluateur
habitué à OpenGL s'attend à voir.

**Pourquoi `pop()` lève une exception sur la matrice de base ?** Un `pop` de
trop est toujours un bug de hiérarchie ; mieux vaut le voir immédiatement (et le
tester) que dessiner un modèle silencieusement faux.

*Alternative écartée :* une bibliothèque générique `Mat<N,M,T>`. Plus flexible,
mais beaucoup plus longue à lire pour un besoin limité aux matrices 4x4.

---

## 3. Architecture en modules (principes SOLID)

```
math/       Vec3, Mat4, MatrixStack            (aucune dépendance)
skeleton/   Pose, BodyDimensions, HumanModel,  (dépend de math/)
            JointLimits, Character, IPartDrawer
animation/  IAnimation, KeyframeTrack,         (dépend de skeleton/Pose uniquement)
            Animator, AnimationLibrary, clips/
geometry/   forme unitaire 1x1x1                (aucune dépendance OpenGL)
render/     shaders, maillage GPU, caméra,      (seul module qui parle à OpenGL)
            ombre, SceneRenderer
ui/         ImGuiLayer, ControlPanel           (interface bonus)
platform/   Window (GLFW)
app/        Application : assemble le tout
```

**Animations isolées du moteur de rendu.** Une animation est une *fonction pure
du temps* (`IAnimation::sample(t) -> Pose`). Elle ne connaît ni OpenGL, ni les
dimensions du corps. Conséquences :
- on peut tester toutes les animations sans contexte OpenGL ;
- changer de moteur de rendu ne touche à aucune animation.

**Application de SOLID :**
- **S** (une responsabilité par classe) : `HumanModel` ne fait que parcourir la
  hiérarchie, `Animator` ne fait que jouer et fondre les animations,
  `SceneRenderer` ne fait que dessiner.
- **O** (ouvert / fermé) : ajouter une danse, c'est écrire un fichier dans
  `clips/` et une ligne dans `AnimationLibrary::createDefault()`. Ni l'`Animator`,
  ni l'interface, ni les touches clavier ne changent (elles parcourent la bibliothèque).
- **L** (substitution) : toutes les animations sont interchangeables derrière `IAnimation`.
- **I** (interfaces minimales) : `IPartDrawer` n'a qu'une méthode,
  `IAnimation` deux.
- **D** (inversion des dépendances) : le squelette ne dépend pas d'OpenGL ; il
  appelle `IPartDrawer`. Le rendu OpenGL, l'ombre portée, le calcul du contact au
  sol et les tests unitaires sont autant d'implémentations de cette interface.

*Alternative écartée :* un graphe de scène générique (nœuds, composants). Plus
puissant, mais il cacherait la pile de matrices que le sujet demande justement
de montrer et d'expliquer.

---

## 4. Modèle hiérarchique (`skeleton/HumanModel`)

```
Torse ─┬─ Cou ── Tête
       ├─ Bras G/D ── Avant-bras G/D ── Main G/D
       └─ Cuisse G/D ─ Jambe G/D ───── Pied G/D
```

- Le **torse est la racine** : une rotation du torse est appliquée une seule fois,
  avant tous les enfants, donc tout le corps suit.
- Chaque membre est dessiné dans un `push` / `pop` : quand le bras bouge, seul
  l'avant-bras (et la main) suit.
- **La position de chaque articulation est calculée à partir des dimensions du
  parent** (`BodyDimensions`). Modifier la longueur d'un membre repositionne
  automatiquement les parties qui en dépendent, sans aucune valeur codée en dur.
- **L'échelle d'une partie est poussée puis retirée autour de son propre dessin
  uniquement** : elle ne se propage jamais aux enfants. C'est l'erreur classique
  d'une pile de matrices, et elle est couverte par des tests.

**Une seule fonction de dessin.** `HumanModel::drawPart` est le seul endroit où
une partie est dessinée, et elle est appelée exactement une fois par partie. Elle
dessine la forme 1x1x1 à l'origine de la matrice courante, mise à l'échelle. Un
test vérifie qu'il y a exactement un appel par partie.

**Ordre des rotations aux épaules et aux hanches.** L'abduction est appliquée
d'abord, dans le repère de l'os ; la flexion vient ensuite, autour de l'axe X du
parent, et fait pivoter vers l'avant le membre déjà écarté. Dans le code,
`rotateX(flexion)` précède donc `rotateZ(abduction)` (la pile post-multiplie).
Un bras écarté puis porté vers l'avant déplace ainsi la main sur le côté, comme
une vraie épaule ; c'était nécessaire pour le Thomas flare (main sous la
poitrine). La suite de tests `HumanModelRotationOrder` couvre cet ordre.

---

## 5. Articulations invisibles et forme unitaire

**Problème.** Avec des cubes stricts posés bout à bout, un genou ou un coude plié
laisse apparaître un trou ou un angle vif.

**Solution retenue.**
1. Chaque segment de membre est **prolongé au-delà de ses articulations** d'une
   demi-épaisseur de la partie la plus fine. Deux segments voisins se recouvrent
   toujours, quel que soit l'angle.
2. La forme unitaire est une **boîte 1x1x1 aux arêtes légèrement arrondies**
   (`geometry/RoundedBox`). Elle reste une forme géométrique 1x1x1 dessinée par
   un seul appel, comme l'exige le sujet. Le recouvrement des extrémités
   arrondies cache la jointure.

*Alternatives écartées :* des sphères aux articulations (cela ajoute des parties
que le sujet ne demande pas et un deuxième appel de dessin par articulation) ;
des maillages déformés par *skinning* (hors sujet : le projet porte sur la pile
de matrices, pas sur le *skinning*).

---

## 6. Limites articulaires (`skeleton/JointLimits`)

Chaque articulation a une plage anatomique en degrés : un coude ou un genou va
de 0 à 150° et ne se plie jamais à l'envers, le cou et les poignets sont limités,
etc. Le **torse est libre** : le corps entier peut tourner ou basculer (salto,
flare), seules les articulations *entre deux parties* sont limitées.

**Double protection :**
- l'`Animator` borne chaque pose produite (`JointLimits::clamp`). Une animation
  mal réglée ne peut donc jamais produire un mouvement impossible à l'écran ;
- les tests vérifient que chaque animation reste *déjà* dans les limites sans
  bornage (strictement pour Disco, Kung-fu et Thomas flare ; à 2° de
  dépassement de spline près pour les autres), pour que le bornage ne soit
  qu'un filet de sécurité.

---

## 7. Animations fluides (`animation/`)

- **`KeyframeTrack`** : interpolation *Hermite cubique* avec tangentes de
  Catmull-Rom, en boucle. La position **et la vitesse** sont continues partout,
  même au raccord de la boucle : pas d'à-coups.
  *Alternative écartée :* l'interpolation linéaire, qui provoque des cassures
  de vitesse visibles à chaque image clé.
- **Une `Pose` est un simple vecteur de nombres** qui supporte `+` et `*`. Les
  splines, les fondus et les mélanges sont donc tous de simples sommes pondérées.
- **`Animator`** : changer d'animation déclenche un **fondu enchaîné** depuis la
  pose courante (0,4 s par défaut, réglable), sans saut brusque. La vitesse et la
  pause n'agissent que sur l'horloge de l'animation.
- **`Character`** : avant le dessin, le modèle est parcouru une fois pour trouver
  son point le plus bas, qu'on pose sur le sol. Les genoux pliés, les jambes
  redimensionnées ou les mains au sol restent ainsi en contact avec le sol sans
  que les animations ne connaissent aucune dimension. Le *support pinning*
  garde un appui (la main du flare) fixe au sol au lieu de le faire glisser.

### Animations obligatoires
- **Idle** : respiration et léger balancement.
- **Walk** : cycle de marche décrit pour une seule jambe et un seul bras ; l'autre
  côté joue la même courbe décalée d'une demi-période (une seule source de vérité).
  Les bras se balancent à l'opposé des jambes.
- **Jump** : flexion, impulsion, vol avec un vrai temps de vol, réception amortie.

### Animations bonus
- **Disco** : le célèbre « doigt vers le ciel puis en travers du corps », avec un
  balancement des hanches. Il est suivi de moulinets d'avant-bras et de rebonds
  des genoux sur un tempo de 120 BPM.
- **Kung-fu** : un kata. Position du cavalier, coup de poing droit, coup de poing
  gauche (le torse tourne avec le coup, la tête reste face à l'adversaire), puis
  coup de pied avant (armé, frappe, réarmé) et retour en garde. Le coude du
  coup de poing garde 5° de flexion : la spline ne peut ainsi jamais passer
  sous la limite de 0°, et un poing réel ne se verrouille pas non plus.
- **Thomas flare** (breakdance) : animation *procédurale*. Chaque canal est une
  fonction de l'angle de rotation, ce qui garantit une boucle parfaite. Le corps
  est horizontal, face au sol, et porté par un bras tendu à la fois, la main
  d'appui fixée au sol. Les jambes, en grand écart, balaient des cercles. Le
  changement de main est réglé pour qu'aucune articulation ne bouge de plus de
  6° par image à 120 Hz.
  *Pourquoi procédural plutôt que des images clés ?* Le flare est un mouvement
  circulaire continu. Le décrire par l'angle de rotation est plus court, plus
  lisible et ne peut pas « décrocher » au raccord de la boucle.

---

## 8. Moteur de rendu (`render/`)

- **OpenGL 4.1 core profile** (le sujet demande au moins la 4.0 ; 4.1 est aussi la
  version maximale disponible sur macOS, donc le projet tourne sur les postes 42).
- **RAII** pour les shaders et les maillages GPU : aucune fuite de ressource,
  aucun `glDelete*` oublié.
- **Éclairage simple, sol quadrillé et ombre portée plane** (stencil) : ils
  donnent la profondeur nécessaire pour juger visuellement un saut ou un contact
  au sol, sans complexité inutile.
- **Caméra orbitale** à la souris : clic gauche pour tourner, molette pour zoomer.

*Alternative écartée :* le *shadow mapping*. Il est plus réaliste, mais aussi
beaucoup plus de code, pour un sol plat où une ombre plane est exacte.

---

## 9. Interface graphique bonus (`ui/`)

Dear ImGui, pour tout modifier **à la volée** :
- choix de l'animation, vitesse, durée du fondu, pause ;
- largeur, longueur et épaisseur de chaque partie. Les enfants se repositionnent
  automatiquement. Une option applique la modification aux deux côtés à la fois
  (`mirrorPart`) ;
- couleur de chaque partie, remise à zéro des tailles et des couleurs ;
- affichage des parties bonus (cou, mains, pieds), fil de fer, ombre, sol ;
  réinitialisation de la caméra.

`ImGuiLayer` possède le contexte ImGui (RAII) et `ControlPanel` ne fait que
l'interface. Le clavier et la souris ne vont à la scène que lorsque ImGui ne les
utilise pas.

*Pourquoi Dear ImGui ?* Le code reste immédiat (une fonction par section), il n'y
a pas d'état d'interface à synchroniser, et c'est la référence des outils de
réglage temps réel.

---

## 10. Tests et processus

- **GoogleTest**, lancé par `make test`. Seuls les modules sans OpenGL sont liés
  aux tests : maths, squelette, animations, géométrie, caméra.
- Les tests vérifient des **propriétés physiques**, pas des valeurs recopiées :
  - un appel de dessin par partie ;
  - l'échelle ne fuit pas vers les enfants ;
  - les limites articulaires sont respectées sans bornage ;
  - les animations bouclent et restent fluides d'une image à l'autre ;
  - les pieds restent au sol ;
  - la main d'appui du flare ne glisse pas.
- Les tests ont été écrits par un sous-agent sur un **autre modèle** (Sonnet).
  Chaque PR a été relue par un sous-agent avant le merge, et un dernier
  sous-agent a vérifié la conformité au sujet en fin de projet.
- Commits au format `ADD:` / `CHANGE:` / `DEL:` / `REFACTOR:`, une branche et une
  PR par fonctionnalité.
