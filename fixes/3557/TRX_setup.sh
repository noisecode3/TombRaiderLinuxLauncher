#!/bin/bash
cd "$(dirname "$0")" || exit 1
rm -fr TRX
md5sum="475b292dee44e978bda1567459bfaabd"
md5sum_file=$(md5sum BlackWolfTR-IntoTheRealmOfEternalDarkness.zip | cut -d' ' -f1)

if [[ $md5sum == "$md5sum_file" ]]
    then
        echo "sum OK"
    else
        rm -f BlackWolfTR-IntoTheRealmOfEternalDarkness.zip
        wget --no-check-certificate https://www.trle.net/levels/levels/2023/0723/BlackWolfTR-IntoTheRealmOfEternalDarkness.zip
        md5sum_file=$(md5sum BlackWolfTR-IntoTheRealmOfEternalDarkness.zip | cut -d' ' -f1)
        if [[ $md5sum == "$md5sum_file" ]]
            then
                echo "sum OK"
            else
                rm -f BlackWolfTR-IntoTheRealmOfEternalDarkness.zip
                exit 1
        fi
fi
   git clone --recurse-submodules https://github.com/LostArtefacts/TRX
   cd TRX || exit 1
   git checkout c1a9933
   patch -p1 < ../IntoTheRealmOfEternalDarkness.patch
   just tr1-package-linux release
   cd ..
   mkdir IntoTheRealmOfEternalDarkness
   cd IntoTheRealmOfEternalDarkness || exit 1
   unzip ../BlackWolfTR-IntoTheRealmOfEternalDarkness.zip
   unzip ../TRX/TR1X-4.11.2-dirty-Linux.zip
