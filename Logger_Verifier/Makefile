CXX = g++

LFLAGS := -g -Wall
CFLAGS = -Wall -g -I ./include
SIGN_VER_FLAGS = -I $(HOME)/cryptopp -Wall -g3 -O2
SOURCES :=  Logger.cpp
LOGGER := Logger

OPENCV = `pkg-config opencv4 --cflags --libs`
LIBS = $(OPENCV)

SDIR = src

sign_verify: sign_verify.o
	$(CXX) -o sign_verify $(SIGN_VER_FLAGS) sign_verify.o -lcryptopp

sign_verify.o: src/sign_verify.cpp
	$(CXX) $(SIGN_VER_FLAGS) -c $< -o $@

Logger: Logger.o 
	$(CXX) $(CFLAGS) -o Logger Logger.o  $(LIBS) -lpthread

Verifier: Verifier.o merkle_tree.o node.o
	$(CXX) $(CFLAGS) -o Verifier Verifier.o merkle_tree.o node.o $(LIBS) -lpthread

merkle_tree.o: src/merkle_tree.cpp
	${CXX} ${CFLAGS} -c $< -o $@
	
node.o: src/node.cpp
	${CXX} ${CFLAGS} -c $< -o $@	

Verifier.o: ./Verifier.cpp
	${CXX} ${CFLAGS} -c $< -o $@ $(LIBS)

Logger.o: ./Logger.cpp
	${CXX} ${CFLAGS} -c $< -o $@ $(LIBS) -lpthread

clean:
	$(RM) *.o Verifier Logger sign_verify
	
