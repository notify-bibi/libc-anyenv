#!/bin/bash


LibcSearcher="$HOME/.local/LibcSearcher"
libcdatabase="$HOME/libc-database"
tmp=".libc-database-tmp-git"
git clone https://github.com/lieanu/LibcSearcher.git $LibcSearcher


git clone --no-checkout https://github.com/niklasb/libc-database.git $tmp/libc-database
rm -rf $LibcSearcher/libc-database/.git
mv $tmp/libc-database/.git $LibcSearcher/libc-database/ && rm -rf $tmp


mkdir $LibcSearcher/libc-database/db
ln -sf --relative $LibcSearcher/libc-database/db db


cd $LibcSearcher/libc-database
git reset --hard HEAD


echo "you can download the librepo"
echo "cd $LibcSearcher/libc-database && ./get ubuntu debian"



