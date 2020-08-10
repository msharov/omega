# OMEGA

This is a roguelike game written by Laurence Brothers in 1987. It is, perhaps
the best roguelike ever written, with a huge world, a variety of quests, many
cool items and monsters, and plenty of wit, charm, and humour. The repository
contains the original 0.8 version and the latter day fixes released as 0.90.4
by various authors on Usenet. This particular project is a conversion of this
game from old school 1987 C to modern C++, with the primary goal being mostly
entertainment. Code changes so far include general cleanup, const correctness
and other things now essential that were unheard of in 1987, using stl mostly
to replace homegrown linked list containers, and few UI improvements. No, the
old saved games and omegarc are not compatible. Building omega requires zlib,
ncurses, and [cwiclo](https://github.com/msharov/cwiclo). A c++17 compiler is
also required, such as gcc 7.`./configure && make install && omega` Have fun!
