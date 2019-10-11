#CPP=clang++
#CPP=icpc
CPP=g++

WFLAGS=-Wall -Wextra -pedantic

CFLAGS=-g3 -std=c++11

OFLAGS=-march=native -mavx2 -Ofast #-fopenmp #-qopenmp -DAVX2

INCLUDES=-I/usr/include/opencv4

LIBS=-lopencv_core -lopencv_videoio -lopencv_highgui -lopencv_imgproc

all:
	$(CPP) $(CFLAGS) $(OFLAGS) $(INCLUDES) filter_cv.cpp $(LIBS) -o filter_cv

s:
	$(CPP) -S $(CFLAGS) $(OFLAGS) $(INCLUDES) filter_cv.cpp $(LIBS) -o filter_cv.s

clean: 
	rm -f *~ *.s filter_cv
