#!/bin/bash
gcc cliente.c -o cliente -lm
sleep 0.5

for ((c =0; c < $1 ; c++))
do
    ./cliente&
done    
 