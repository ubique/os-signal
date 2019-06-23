CXX = clang++
CXX_STANDARD = -std=c++17

all:
  $(CXX) $(CXX_STANDARD) main.cpp signal.cpp -o sig

run: all
  ./sig

clean:
  rm -rf sig