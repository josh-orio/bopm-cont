git clone https://github.com/OpenMathLib/OpenBLAS.git
cd OpenBLAS
mkdir build && cd build
cmake .. && sudo make -j install
cd ../.. && rm -rf OpenBLAS
