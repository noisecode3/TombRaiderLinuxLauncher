# This is a test database

I list some levels I have tested.

You can access the game files here ~/.local/share/TombRaiderLinuxLauncher/ID.TRLE
where ID is the number listed when runngin tombll_manage_data.py -l

3573. The Infada Cult
   * <https://www.trle.net/sc/levelfeatures.php?lid=3573>
   * <https://trcustoms.org/levels/3574>
   * have played through
   * no problems

3082. Templars Secret
   * <https://www.trle.net/sc/levelfeatures.php?lid=3082>
   * <https://trcustoms.org/levels/3025>
   * have played through
   * no problems

3377. Secret of the Skull Temple
   * <https://www.trle.net/sc/levelfeatures.php?lid=3377>
   * <https://trcustoms.org/levels/3310>
   * have played through
   * lags sometimes, plying with dgvoodoo can help

318. The Rescue 1-4
   * <https://www.trle.net/sc/levelfeatures.php?lid=318>
   * <https://trcustoms.org/levels/317>
   * have not played through all of the game
   * no problems

3430. War of the Worlds
   * <https://www.trle.net/sc/levelfeatures.php?lid=3430>
   * <https://trcustoms.org/levels/3363>
   * have not played through all of the game
   * plying with lutris and dgvoodoo, fix flashlight bug

3621. The Hidden Dagger II - Dragon's Legacy
   * <https://www.trle.net/sc/levelfeatures.php?lid=3621>
   * <https://trcustoms.org/levels/3642>
   * Starts the game and first map, backgrund audio is missing
   * Note: I got the build fix here <https://github.com/noisecode3/ogg-winmm>
   * but you need to replace winmm.dll remove ffplay.dll and convert the files
   * I left a script called convert.sh you have to copy it to
   * ~/.local/share/TombRaiderLinuxLauncher/6.TRLE/music
   * and run it from there right now
   * use dll ovride winmm=n,b

3417. Create a Classic 2021 - The City of Petra
   * <https://www.trle.net/sc/levelfeatures.php?lid=3417>
   * <https://trcustoms.org/levels/3350>
   * have not played through all of the game
   * no problems

3557. Into the Realm of Eternal Darkness
   * <https://www.trle.net/sc/levelfeatures.php?lid=3557>
   * <https://trcustoms.org/levels/3541>
   * have not played through all of the game
   * no problems
   * I manage to port this one to Linux native thru TR1X
   * the patch for TR1X is in fixes/8/IntoTheRealmOfEternalDarkness.patch
   * then you need to add all data from the zip, that is
   * (data fmv music and pictures) should be put next to TR1X
   * It is the same for the most part but with 60 fps
   * The second level had broken timer, but works now
   * Some issues can be addressed next time.

   ```shell
   git clone --recurse-submodules https://github.com/LostArtefacts/TR1X
   cd TR1X
   git checkout 871259d
   cat ../TombRaiderLinuxLauncher/fixes/8/IntoTheRealmOfEternalDarkness.patch | patch -p1
   just package-linux
   ```

1388. The Experiment 3/1 - Under the Moonlight
   * <https://www.trle.net/sc/levelfeatures.php?lid=1388>
   * <https://trcustoms.org/levels/1360>
   * have not played through all of the game
   * no problems

1964. The Ancient Knowledge
    * <https://www.trle.net/sc/levelfeatures.php?lid=1964>
    * <https://trcustoms.org/levels/1947>
    * have not played through all of the game
    * no problems

3372. Tomb Raider - Biohazard
    * <https://www.trle.net/sc/levelfeatures.php?lid=3372>
    * <https://trcustoms.org/levels/3305>
    * have not played through all of the game
    * background audio seem to have broke

![screenshot1](https://raw.githubusercontent.com/noisecode3/TombRaiderLinuxLauncher/main/database/screenshot1.jpg)
