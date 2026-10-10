# Galaxy Fighter — PC et Android

Portage du jeu de Press Play On Tape. Les versions Windows, Linux et Android
utilisent SDL2 pour l'affichage et le son. Le jeu se commande au clavier sur
PC et avec le tactile (ou un clavier physique) sur Android ; le firmware
ESP-IDF existant reste disponible dans le dossier racine.

## Commandes

| Action | Commandes |
| --- | --- |
| Déplacer le vaisseau / choisir une entrée | Flèches, WASD ou ZQSD |
| Tirer / confirmer | Espace, J ou Entrée |
| Action secondaire | X ou Ctrl gauche |
| Action tertiaire | C ou Maj gauche |
| Quitter la version PC | Fermer la fenêtre ou Échap |

Sur Android, utilisez le pavé directionnel et les boutons A/B/C affichés à
l'écran ; un clavier physique reste également pris en charge.

## Télécharger et lancer le jeu

Les versions publiées sont disponibles dans l'onglet
[**Releases**](https://github.com/Jicehel-Aka/Galaxy-Fighter-Pokitto-ported-on-Aka/releases)
du dépôt.
Choisissez la version souhaitée, téléchargez le fichier correspondant à votre
système, puis suivez la procédure ci-dessous. Le fichier firmware `.bin` pour
la console AKA n'est pas un exécutable PC ou Android : utilisez l'archive ou
l'APK de la release.

| Système | Fichier | Procédure |
| --- | --- | --- |
| Windows 64 bits | `GalaxyFighter-windows-x64.zip` | Extraire l'archive complète, puis lancer `galaxy-fighter.exe` dans le dossier extrait. Garder le dossier `assets` à côté du programme. |
| Linux x86_64 | `GalaxyFighter-linux-x64.zip` | Installer SDL2 (`sudo apt install libsdl2-2.0-0` sur Debian/Ubuntu), extraire l'archive, puis lancer `./galaxy-fighter` depuis le dossier extrait. Si nécessaire, rendre le fichier exécutable avec `chmod +x galaxy-fighter`. |
| Android ARM64 / ARMv7 | `GalaxyFighter-android-debug.apk` | APK signé automatiquement par Gradle pour l'installation directe. Transférer le fichier sur le téléphone, autoriser l'installation depuis cette source, puis ouvrir l'APK. |

Sur PC, jouer au clavier avec les touches indiquées plus haut et quitter avec
Échap. Sur Android, jouer avec le pavé directionnel et les boutons tactiles
affichés à l'écran, ou connecter un clavier physique. La musique est intégrée
aux archives PC et à l'APK ; garder le dossier `assets` intact pour les
versions Windows/Linux. Les scores sont sauvegardés dans le dossier de
préférences de l'application.

### Installation et signature Android

L'APK `GalaxyFighter-android-debug.apk` produit par le workflow est signé avec
la clé de débogage générée automatiquement par Gradle ; aucune signature
manuelle n'est nécessaire pour l'installer. Au premier lancement de l'APK,
Android peut demander d'autoriser l'application utilisée pour ouvrir le fichier
(navigateur, gestionnaire de fichiers, etc.) à installer des applications.

Cette signature de débogage permet le test et l'installation par sideload, mais
ne constitue pas une signature de distribution. Le runner de CI peut générer
une clé différente à chaque exécution : si Android refuse une mise à jour à
cause d'une signature différente, désinstaller l'ancienne application avant
d'installer la nouvelle. La désinstallation efface les scores locaux. Une
distribution avec mises à jour transparentes nécessite une clé de signature
de production conservée de façon sécurisée et la signature de chaque version
avec cette même clé ; aucune clé privée de production n'est stockée dans ce
dépôt ou requise par le workflow.

## Porter un autre jeu destiné à l'AKA

Les archives et l'APK fournis par ce dépôt contiennent **Galaxy Fighter**,
pas un chargeur générique de jeux AKA. Le firmware `.bin` et les jeux compilés
pour les composants matériels de l'AKA ne peuvent pas être exécutés directement
sous Windows, Linux ou Android. Il faut compiler les sources du jeu avec le
backend SDL2 de ce portage.

Pour adapter un autre jeu, partir de ses sources C/C++ (et non de son binaire
AKA), intégrer les fichiers du jeu dans `main/game/`, puis adapter
`desktop/src/main.cpp` à son point d'entrée et à ses API de jeu. La couche
`desktop/src/platform.cpp` fournit actuellement la compatibilité Pokitto
utilisée par Galaxy Fighter ; un jeu qui appelle directement les API
matérielles AKA/Gamebuino devra recevoir une couche d'adaptation SDL2 pour
l'affichage, les entrées, le son et la sauvegarde. Copier aussi ses ressources
dans la configuration d'assets du jeu (`desktop/CMakeLists.txt` et
`android/app/build.gradle`), puis vérifier chaque cible avec les procédures de
compilation ci-dessous. Une fois l'adaptation testée, les releases multi-
plateformes se produisent en publiant un tag de version `v*` (voir Releases).

## Compiler sous Windows

Installer CMake, Visual Studio Build Tools (C++), Git et vcpkg, puis :

```powershell
vcpkg install sdl2:x64-windows-static
cmake -S desktop -B build -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build build --config Release
```

## Compiler sous Linux

Installer CMake, un compilateur C++, SDL2 et Zip (par exemple
`sudo apt install cmake g++ libsdl2-dev zip`), puis :

```sh
cmake -S desktop -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Les fichiers de musique sont copiés dans `build/assets/galaxy/music`.
Lancer le binaire depuis son répertoire de build pour conserver les chemins
relatifs. SDL2 doit également être installé sur la machine Linux cible. Les
scores sont stockés dans le répertoire de préférences SDL2 de l'utilisateur.

## Compiler pour Android

Installer JDK 17, Gradle 8.9, Android SDK/NDK et CMake 3.22.1. La compilation
utilise les plateformes Android 35, les Build Tools 35.0.0 et le NDK
27.2.12479018. Télécharger les sources SDL2 version 2.30.11 dans `android/SDL`
depuis la racine du dépôt :

```sh
mkdir -p android/SDL
curl -L --fail https://github.com/libsdl-org/SDL/releases/download/release-2.30.11/SDL2-2.30.11.tar.gz \
  | tar -xz --strip-components=1 -C android/SDL
cd android
gradle assembleDebug
```

L'APK signé pour le test est créé sous
`android/app/build/outputs/apk/debug/app-debug.apk`. La compilation vise
ARM64 et ARMv7. Le projet
Gradle récupère les sources Java et ressources
Android de SDL2 depuis `android/SDL` et intègre la musique depuis
`sdcard_files/galaxy/music` dans les assets de l'APK.

## Releases

Le workflow GitHub Actions `.github/workflows/release.yml` compile Windows,
Linux et Android. Pour lancer les builds manuellement depuis le site GitHub,
ouvrir **Actions > Build and publish releases > Run workflow**, choisir la
branche `main`, puis répondre à la question **Publier une release GitHub après
les builds ?**

- **Non** (choix par défaut) : les builds sont exécutés et les fichiers restent
  disponibles dans les artefacts du workflow ; aucune release n'est publiée.
- **Oui** : saisir un nouveau tag de version tel que `v1.1.1`. Après réussite
  des builds, le workflow crée le tag sur le commit choisi et publie la release
  avec les archives Windows/Linux et l'APK Android.

Un tag existant ne doit pas être réutilisé pour une autre version. Pour publier
une mise à jour après `v1.1.1`, choisir un nouveau tag, par exemple `v1.1.2`.
On peut aussi créer et pousser le tag annoté avec Git ; ce déclenchement publie
la release automatiquement, sans lancement manuel :

```sh
git tag -a v1.0.1 -m "Galaxy Fighter 1.0.1"
git push origin v1.0.1
```

L'APK Android attaché, `GalaxyFighter-android-debug.apk`, est signé avec la clé
de débogage Gradle pour
permettre l'installation directe ; ce n'est pas une signature de production.
L'option manuelle permet ainsi de choisir au lancement si l'exécution doit
se limiter aux artefacts ou publier une release. La signature Android de
production et la distribution via un magasin d'applications ne sont pas
configurées.

## Licences et crédits

Le jeu d'origine est sous BSD-3-Clause (voir `LICENSE.original`). Le dépôt
inclut aussi `LICENSE` pour le port et ses composants. SDL2 est fourni par les
environnements de build et reste sous sa licence zlib ; il n'est pas recopié
dans les archives desktop. Crédits du jeu : Press Play On Tape.
