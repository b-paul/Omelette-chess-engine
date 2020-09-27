#!/bin/sh

[ -e ./cutechess-cli ] || $(echo "Please put cutechess-cli in this directory for this script to work" && exit)

#https://bitbucket.org/zurichess/zurichess/wiki/Choosing%20positions%20for%20Texel's%20Tuning%20Method
./cutechess-cli \
    -srand $RANDOM \
    -pgnout games.pgn \
    -repeat \
    -recover \
    -tournament gauntlet \
    -rounds 20 \
    -concurrency 10 \
    -ratinginterval 50 \
    -draw movenumber=50 movecount=5 score=20 \
    -openings file=2moves_v2.pgn format=pgn order=random \
    -engine cmd=./omelette dir=../src/ name=omelette tc=0/300+5.00 \
    -engine cmd=./raven dir=. name=raven tc=0/300+5.00 \
    -each timemargin=60000 option.Hash=512 proto=uci
