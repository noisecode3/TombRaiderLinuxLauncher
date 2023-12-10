#!/bin/bash
cd $(dirname $0)
rm -fr ./WINSDEC.DLL \
./ExtraOptions \
./tomb3.exe \
./ReadmeHost.txt \
./WINSTR.DLL \
./tomb3_ConfigTool.exe \
./pix \
./audio \
./data \
./DEC130.DLL \
./tomb3_ConfigTool.json \
./WINPLAY.DLL \
./ReadMe.pdf \
./EDEC.DLL \
./Uninst.isu
rm saves/DoNotDeleteMe.txt
rm -fr 3Dfx/ Cpl/ Doc/ MS/ QuickGuide.url dgVoodoo.conf dgVoodooCpl.exe
wget -c https://www.trle.net/scadm/trle_dl.php?lid=3573
if echo "152d33e5c28d7db6458975a5e53a3122  trle_dl.php?lid=3573" | md5sum -c; then
        unzip trle_dl.php?lid=3573
else
  echo "download problem"
  exit 1
fi
#wget -c https://github.com/dege-diosg/dgVoodoo2/releases/download/v2.82.1/dgVoodoo2_82_1.zip
#if echo "bf169cdda3bf1925850ff1a25cdb7d48  dgVoodoo2_82_1.zip" | md5sum -c; then
#        unzip dgVoodoo2_82_1.zip
#else
#  echo "download problem"
#  exit 1
#fi
#mv MS/x86/DDraw.dll .
#mv MS/x86/D3DImm.dll .
#sed -i 's|dgVoodooWatermark: true|dgVoodooWatermark: false|g' dgVoodoo.conf
