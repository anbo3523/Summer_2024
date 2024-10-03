#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "Filter.h"
#include "immintrin.h"  // [AVX2] For AVX2

using namespace std;

#include "rdtsc.h"

//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);

int
main(int argc, char **argv)
{

  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }

  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];

  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  double sum = 0.0;
  int samples = 0;

  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
    struct cs1300bmp *input = new struct cs1300bmp;
    struct cs1300bmp *output = new struct cs1300bmp;
    int ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

    if ( ok ) {
      double sample = applyFilter(filter, input, output);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
    delete input;
    delete output;
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

class Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());

  if ( ! input.bad() ) {
    int size = 0;
    input >> size;
    Filter *filter = new Filter(size);
    int div;
    input >> div;
    filter -> setDivisor(div);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {
	int value;
	input >> value;
	filter -> set(i,j,value);
      }
    }
    return filter;
  } else {
    cerr << "Bad input in readFilter:" << filename << endl;
    exit(-1);
  }
}

/* A lot of changes were made to applyFilter. 
1.Since we updated the data structure to be a 2d array w/struct instead of a 3d array,
  We needed to update the code to follow that format data structure. 
2.We moved many invarients outside of the loop.
  These values do not change and we don't need our program to
  calculate them over and over again.
3.Loop unrolling was implemented. This allows for fewer increments and condition checks which reduces overhead.
  It also allows for better cache utilization by accessing multiple elements within a single iteration.
4.We included an offset to compute both the row and column index of neighboring pixels relative to the current row/column being processed.
  This allows us to access the needed pixels for the input image without needing to perform the same
  arithmetic operations inside the inner-most loop.
5.Additionally, we adjusted the loop order to first iterate through rows and then columns for better use of caching and memory access.
6.We seperated the two cases since not all cases have a divisor that is > 1. Prevents unnecessary division ops.
7.Finally, we include optimizations that reduces access to the array by using local variables (rval, gval, bval). This prevents constant access to the large array which
  reduces the overhead associated with memory access. 
8.PLEASE VIEW Notes.MD for more in depth explanation about changes!
*/
double applyFilter(class Filter *filter, cs1300bmp *input, cs1300bmp *output)
{
  long long cycStart, cycStop;

  cycStart = rdtscll();

  output->width = input->width;
  output->height = input->height;

  int filterSize = filter->getSize();  // [CODE MOTION] Move invariant outside the loops
  int divisor = filter->getDivisor();  // [CODE MOTION] Move invariant outside the loops
  int height = (input->height) - 1;  // [CODE MOTION] Move invariant outside the loops
  int width = (input->width) - 1;  // [CODE MOTION] Move invariant outside the loops
  int *data = filter->getData();  // [CODE MOTION] Use getData() to access private data

  if (divisor == 1) {
    for (int row = 1; row < height; row++) {
      for (int col = 1; col < width; col++) {
        int rval = 0, gval = 0, bval = 0;

        for (int j = 0; j < filterSize; j++) {
          int rowOffset = row + j - 1;  // [STRENGTH REDUCTION] Pre-compute offset
                for (int i = 0; i < filterSize; i += 2) {  // [LOOP UNROLLING] Unroll inner loop by 2
                  int colOffset1 = col + i - 1;  // [STRENGTH REDUCTION] Pre-compute offset
                  rval += input->color[rowOffset][colOffset1].R * data[j * filterSize + i];
                  gval += input->color[rowOffset][colOffset1].G * data[j * filterSize + i];
                  bval += input->color[rowOffset][colOffset1].B * data[j * filterSize + i];

                  if (i + 1 < filterSize) {  // Check if within bounds
                    int colOffset2 = col + i;  // [STRENGTH REDUCTION] Pre-compute offset for the unrolled iteration
                    rval += input->color[rowOffset][colOffset2].R * data[j * filterSize + (i + 1)];
                    gval += input->color[rowOffset][colOffset2].G * data[j * filterSize + (i + 1)];
                    bval += input->color[rowOffset][colOffset2].B * data[j * filterSize + (i + 1)];
                  }
                }
          }

        if (rval < 0) {
          rval = 0;
        } else if (rval > 255) {
          rval = 255;
        }

        if (gval < 0) {
          gval = 0;
        } else if (gval > 255) {
          gval = 255;
        }

        if (bval < 0) {
          bval = 0;
        } else if (bval > 255) {
          bval = 255;
        }

        output->color[row][col].R = rval;  // [FINAL TRANSFER] Do final transfer of data to output at end of the loop to reduce number of times it is accessed
        output->color[row][col].G = gval;  // [FINAL TRANSFER] Do final transfer of data to output at end of the loop to reduce number of times it is accessed
        output->color[row][col].B = bval;  // [FINAL TRANSFER] Do final transfer of data to output at end of the loop to reduce number of times it is accessed
      }
    }
  } else {
    for (int row = 1; row < height; row++) {
      for (int col = 1; col < width; col++) {
        int rval = 0, gval = 0, bval = 0;

        for (int j = 0; j < filterSize; j++) {
          int rowOffset = row + j - 1;  // [STRENGTH REDUCTION] Pre-compute offset
          for (int i = 0; i < filterSize; i += 2) {  // [LOOP UNROLLING] Unroll inner loop by 2
            int colOffset1 = col + i - 1;  // [STRENGTH REDUCTION] Pre-compute offset
            rval += input->color[rowOffset][colOffset1].R * data[j * filterSize + i];
            gval += input->color[rowOffset][colOffset1].G * data[j * filterSize + i];
            bval += input->color[rowOffset][colOffset1].B * data[j * filterSize + i];

            if (i + 1 < filterSize) {  // Check if within bounds
              int colOffset2 = col + i;  // [STRENGTH REDUCTION] Pre-compute offset for the unrolled iteration
              rval += input->color[rowOffset][colOffset2].R * data[j * filterSize + (i + 1)];
              gval += input->color[rowOffset][colOffset2].G * data[j * filterSize + (i + 1)];
              bval += input->color[rowOffset][colOffset2].B * data[j * filterSize + (i + 1)];
            }
          }
        }

        rval /= divisor;
        gval /= divisor;
        bval /= divisor;

        if (rval < 0) {
          rval = 0;
        } else if (rval > 255) {
          rval = 255;
        }

        if (gval < 0) {
          gval = 0;
        } else if (gval > 255) {
          gval = 255;
        }

        if (bval < 0) {
          bval = 0;
        } else if (bval > 255) {
          bval = 255;
        }

        output->color[row][col].R = rval;  // [FINAL TRANSFER] Do final transfer of data to output at end of the loop to reduce number of times it is accessed
        output->color[row][col].G = gval;  // [FINAL TRANSFER] Do final transfer of data to output at end of the loop to reduce number of times it is accessed
        output->color[row][col].B = bval;  // [FINAL TRANSFER] Do final transfer of data to output at end of the loop to reduce number of times it is accessed
      }
    }
  }

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output->width * output->height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n", diff, diffPerPixel);
  return diffPerPixel;
}
  /*below is my old code
  long long cycStart, cycStop;

  cycStart = rdtscll();

  output -> width = input -> width;
  output -> height = input -> height;


  for(int col = 1; col < (input -> width) - 1; col = col + 1) {
    for(int row = 1; row < (input -> height) - 1 ; row = row + 1) {
      /*for(int plane = 0; plane < 3; plane++) {

	output -> color[plane][row][col] = 0;

	for (int j = 0; j < filter -> getSize(); j++) {
	  for (int i = 0; i < filter -> getSize(); i++) {	
	    output -> color[plane][row][col]
	      = output -> color[plane][row][col]
	      + (input -> color[plane][row + i - 1][col + j - 1] 
		 * filter -> get(i, j) );
	  }
	}

	output -> color[plane][row][col] = 	
	  output -> color[plane][row][col] / filter -> getDivisor();

	if ( output -> color[plane][row][col]  < 0 ) {
	  output -> color[plane][row][col] = 0;
	}

	if ( output -> color[plane][row][col]  > 255 ) { 
	  output -> color[plane][row][col] = 255;
	}
      // Apply filter to each color plane
      output->color[row][col].R = 0;
      output->color[row][col].G = 0;
      output->color[row][col].B = 0;

      for (int j = 0; j < filter->getSize(); j++) {
        for (int i = 0; i < filter->getSize(); i++) {
          // Apply filter to R channel
          output->color[row][col].R += input->color[row + i - 1][col + j - 1].R * filter->get(i, j);
          // Apply filter to G channel
          output->color[row][col].G += input->color[row + i - 1][col + j - 1].G * filter->get(i, j);
          // Apply filter to B channel
          output->color[row][col].B += input->color[row + i - 1][col + j - 1].B * filter->get(i, j);
        }
      }

      output->color[row][col].R = output->color[row][col].R / filter->getDivisor();
      output->color[row][col].G = output->color[row][col].G / filter->getDivisor();
      output->color[row][col].B = output->color[row][col].B / filter->getDivisor();

      if (output->color[row][col].R < 0) {
        output->color[row][col].R = 0;
      }
      if (output->color[row][col].G < 0) {
        output->color[row][col].G = 0;
      }
      if (output->color[row][col].B < 0) {
        output->color[row][col].B = 0;
      }

      if (output->color[row][col].R > 255) {
        output->color[row][col].R = 255;
      }
      if (output->color[row][col].G > 255) {
        output->color[row][col].G = 255;
      }
      if (output->color[row][col].B > 255) {
        output->color[row][col].B = 255;
      }
    }
  }

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (output -> width * output -> height));
  return diffPerPixel; */
