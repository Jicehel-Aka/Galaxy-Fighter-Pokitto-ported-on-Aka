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
| Android ARM64 / ARMv7 | `GalaxyFighter-android-unsigned.apk` | L'APK de release est non signé et ne s'installe pas tel quel. Il faut le signer (voir ci-dessous), transférer l'APK signé sur le téléphone, autoriser l'installation depuis cette source, puis ouvrir l'APK pour l'installer. |

Sur PC, jouer au clavier avec les touches indiquées plus haut et quitter avec
Échap. Sur Android, jouer avec le pavé directionnel et les boutons tactiles
affichés à l'écran, ou connecter un clavier physique. La musique est intégrée
aux archives PC et à l'APK ; garder le dossier `assets` intact pour les
versions Windows/Linux. Les scores sont sauvegardés dans le dossier de
préférences de l'application.

### Signer l'APK Android pour une installation locale

Installer les outils Android SDK Build Tools et utiliser `keytool` pour créer
une clé personnelle, puis `apksigner` pour signer l'APK téléchargé. Ne pas
publier ni partager la clé privée ou son mot de passe :

```sh
keytool -genkeypair -keystore galaxy-release.jks -alias galaxy \
  -keyalg RSA -keysize 2048 -validity 10000
apksigner sign --ks galaxy-release.jks --out GalaxyFighter-android.apk \
  GalaxyFighter-android-unsigned.apk
apksigner verify GalaxyFighter-android.apk
```

Les commandes demanderont le mot de passe de manière interactive. Pour mettre
à jour une installation existante, signer chaque nouvelle version avec la même
clé ; Android refusera une mise à jour signée avec une autre clé.

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

Installer JDK 17, Android SDK/NDK et CMake 3.22.1. Placer les sources SDL2
(version 2.30.11) dans `android/SDL`, puis lancer :

```sh
cd android
gradle assembleRelease
```

L'APK non signé est créé sous
`android/app/build/outputs/apk/release/`. Android exige qu'un APK soit signé
avant installation : signez-le avec votre propre clé de distribution avant de
l'installer ou de le distribuer. La compilation vise ARM64 et ARMv7. Le projet
Gradle récupère les sources Java et ressources
Android de SDL2 depuis `android/SDL` et intègre la musique depuis
`sdcard_files/galaxy/music` dans les assets de l'APK.

## Releases

Le workflow GitHub Actions `.github/workflows/release.yml` compile Windows,
Linux et Android. Pousser un tag `v*` déclenche les builds puis crée une
release GitHub avec les archives Windows/Linux et l'APK Android **non signé**.
Par exemple, après avoir poussé les changements à publier, créer et envoyer un
tag annoté :

```sh
git tag -a v0.1.0 -m "Galaxy Fighter 0.1.0"
git push origin v0.1.0
```

Le workflow compile chaque système puis attache les fichiers à la release du
tag. Le déclenchement manuel (`workflow_dispatch`) ne publie pas de release.
La signature Android automatique et la distribution via un magasin
d'applications ne sont pas configurées.

## Licences et crédits

Le jeu d'origine est sous BSD-3-Clause (voir `LICENSE.original`). Le dépôt
inclut aussi `LICENSE` pour le port et ses composants. SDL2 est fourni par les
environnements de build et reste sous sa licence zlib ; il n'est pas recopié
dans les archives desktop. Crédits du jeu : Press Play On Tape.
