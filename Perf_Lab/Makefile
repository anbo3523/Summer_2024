##
##

CXX	=g++
##
## Use our standard compiler flags for the course...
## You can try changing these flags to improve performance.
##
##We want to change -O0 to -0fast. This enables aggressive optimizations. This leads to faster executables but can sacrifice some compliance with standards.
##We want to change the target architecture to -march=native. This tells the compiler to optimize the code for the architecture of the machine where the compiling is being done. This can lead to better performance
##We want to enable loop unrolling which will increase the spead of the program by reducing overhead of the loop control code. -funroll-loops
##We want to enable AVX2 instructions which is for doing operations on multiple data points simulataneously. -mavx2
##We want to enable parallel programming (OpenMP). This is for the extra credit (if I can get to it)-fopenmp
CXXFLAGS= -g -march=native -Ofast -funroll-loops -mavx2 -fno-omit-frame-pointer -Wall -fopenmp

goals: filter judge
	@echo "Done"

filter: FilterMain.cpp Filter.cpp cs1300bmp.cc cs1300bmp.h Filter.h rdtsc.h
	$(CXX) $(CXXFLAGS) -o filter FilterMain.cpp Filter.cpp cs1300bmp.cc

##
## Parameters for the test run
##
FILTERS = gauss.filter vline.filter hline.filter emboss.filter
IMAGES = boats.bmp blocks-small.bmp
TRIALS = 1 2 3 4

#
# Run the Judge script to compute a score
#
judge: filter
	@echo Running run for performance / scoring information
	./Judge -p ./filter -i blocks-small.bmp
	@echo Now checking if the output is correct. Any errors indicate bugs in your code.
	@cmp filtered-avg-blocks-small.bmp tests/filtered-avg-blocks-small.bmp
	@cmp filtered-emboss-blocks-small.bmp tests/filtered-emboss-blocks-small.bmp
	@cmp filtered-gauss-blocks-small.bmp tests/filtered-gauss-blocks-small.bmp
	@cmp filtered-hline-blocks-small.bmp tests/filtered-hline-blocks-small.bmp

#
# Run the Judge tests on both images and then compare the output to the reference output.
#
# Note you shouldn't use this to compute a score -- it's just for testing
#
test:	filter
	./Judge -p ./filter -i boats.bmp
	./Judge -p ./filter -i blocks-small.bmp
	cmp filtered-avg-blocks-small.bmp tests/filtered-avg-blocks-small.bmp
	cmp filtered-avg-boats.bmp tests/filtered-avg-boats.bmp
	cmp filtered-emboss-blocks-small.bmp tests/filtered-emboss-blocks-small.bmp
	cmp filtered-emboss-boats.bmp tests/filtered-emboss-boats.bmp
	cmp filtered-gauss-blocks-small.bmp tests/filtered-gauss-blocks-small.bmp
	cmp filtered-gauss-boats.bmp tests/filtered-gauss-boats.bmp
	cmp filtered-hline-blocks-small.bmp tests/filtered-hline-blocks-small.bmp
	@echo All tests passed

clean:
	-rm -f *.o
	-rm -f filter
	-rm -f filtered-*.bmp
