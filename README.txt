WIP

# 1. Clone Repository
    git clone https://github.com/nlowe2019/nes-emulator.git
# 2. Install dependancies
    apt-get install libsdl2-dev libsdl2-ttf-dev
# 3. Navigate to build folder
    cd nes-emulator/src/build
# 4. Run CMake commands
    cmake .. .
# 5.
    cmake --build .
# 6. Compile Program
    make
# 7. Run Program
    ./NES <ROM_FILE>    (e.g. ./NES "Super Mario Bros.nes")

Controls:

WASD = D_PAD
K = A
L = B 
N = START
M = SELECT

Debug:

Pause = 'P'
Show Memory = '='
Show Nametable Overlay = '-'