#make clean
#rm -rf CMakeCache.txt CMakeFiles/
cmake -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles"
make -j 8
