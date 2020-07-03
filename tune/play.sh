#!/bin/sh

[ -e ./cutechess-cli ] || $(echo "Please put cutechess-cli in this directory for this script to work" && exit)

#https://bitbucket.org/zurichess/zurichess/wiki/Choosing%20positions%20for%20Texel's%20Tuning%20Method
./cutechess-cli \
    -srand $RANDOM \
    -pgnout games.pgn \
    -repeat \
    -recover \
    -tournament gauntlet \
    -rounds 64 \
    -concurrency 2 \
    -ratinginterval 50 \
    -draw movenumber=50 movecount=5 score=20 \
    -openings file=2moves_v2.pgn format=pgn order=random \
    -engine cmd=omelette name=omelette1 tc=40/1+0.05 \
    -engine cmd=omelette name=omelette2 tc=40/1+0.05 \
    -each timemargin=60000 option.Hash=512 proto=uci dir=../src/
