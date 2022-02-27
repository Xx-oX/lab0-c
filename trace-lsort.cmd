# Test performance of and lsort
option fail 0
option malloc 0
new
ih dolphin 500000
lsort
ih RAND 100000
lsort