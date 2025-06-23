# install python + matplotlib
if [[ "$OSTYPE" == "darwin"* ]]; then # macos
    brew install python3
    pip3 install matplotlib
else # ubuntu + other linux
    # python3 installed by default
    pip3 install matplotlib
fi


# ensure make + cmake are present
if [[ "$OSTYPE" == "darwin"* ]]; then # macos
    brew install make cmake
else # ubuntu + other linux
    sudo apt update
    sudo apt install -y make cmake
fi


# install cpr
git clone https://github.com/libcpr/cpr.git

cd cpr
mkdir build && cd build
cmake .. && sudo make -j install
cd ../.. && rm -rf cpr

echo "CPR has been installed successfully!"


# install nlohmann-json
git clone https://github.com/nlohmann/json.git

cd json
mkdir build && cd build
cmake .. && sudo make -j install
cd ../.. && rm -rf json

echo "nlohmann-json has been installed successfully!"
