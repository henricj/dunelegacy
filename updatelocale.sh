#!/bin/bash

find src include \( -iname "*.cpp" -or -iname "*.h" \) -type f | sort | xargs xgettext -k_ --add-comments=/ -o data/locale/dunelegacy.pot

cd data/locale

for pofile in `ls *.po`; do
 if [ "$pofile" != "English.en.po" ]
 then
 	echo -n "Updating $pofile "
 	msgmerge -U $pofile dunelegacy.pot
 fi
done


