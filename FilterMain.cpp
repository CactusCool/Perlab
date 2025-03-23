#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "Filter.h"
#include <omp.h>
#include <immintrin.h> 

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
  string::size_type loc = filterOutputName.rfind(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  // Pre-allocate memory for images 
  struct cs1300bmp *input = new struct cs1300bmp;
  struct cs1300bmp *output = new struct cs1300bmp;

  double sum = 0.0;
  int samples = 0;

    // Process each input file
  for (int inNum = 2; inNum < argc; inNum++) {
      string inputFilename = argv[inNum];
      string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
      
      // Read the input file
      int ok = cs1300bmp_readfile((char *)inputFilename.c_str(), input);

      if (ok) {
          // Apply the filter
          double sample = applyFilter(filter, input, output);
          sum += sample;
          samples++;
          
          // Write the output file
          cs1300bmp_writefile((char *)outputFilename.c_str(), output);
      }
  }

    // Clean up
    delete input;
    delete output;
    delete filter;

    // Print the results
    fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);
    return 0;
}

class Filter* 
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


double applyFilter(class Filter *filter, cs1300bmp *input, cs1300bmp *output)
{

  long long cycStart, cycStop;

  cycStart = rdtscll();

  int width = input -> width;
  int height = input -> height;
  int filterDivisor = filter->getDivisor();

  output->height = height;
  output->width = width;

  //preload filter values for fast lookup
  const int f00 = filter->get(0, 0);
  const int f01 = filter->get(0, 1);
  const int f02 = filter->get(0, 2);
  const int f10 = filter->get(1, 0);
  const int f11 = filter->get(1, 1);
  const int f12 = filter->get(1, 2);
  const int f20 = filter->get(2, 0);
  const int f21 = filter->get(2, 1);
  const int f22 = filter->get(2, 2);

  // Process each color plane
        #pragma omp parallel for collapse(3)
        for (int plane = 0; plane < 3; plane++) {
            for (int row = 1; row < height - 1; row++) {
                for (int col = 1; col < width - 1; col++) {
                    // Sum the product of each 9 pixels and the filter value
                    int pixelSum = 0;
                    pixelSum += input->color[plane][row - 1][col - 1] * f00;
                    pixelSum += input->color[plane][row - 1][col    ] * f01;
                    pixelSum += input->color[plane][row - 1][col + 1] * f02;

                    pixelSum += input->color[plane][row    ][col - 1] * f10;
                    pixelSum += input->color[plane][row    ][col    ] * f11;
                    pixelSum += input->color[plane][row    ][col + 1] * f12;

                    pixelSum += input->color[plane][row + 1][col - 1] * f20;
                    pixelSum += input->color[plane][row + 1][col    ] * f21;
                    pixelSum += input->color[plane][row + 1][col + 1] * f22;

                    // Optimized division when filterDivisor is a power of 2
                    if ((filterDivisor & (filterDivisor - 1)) == 0 && filterDivisor > 0) {
                        int shift = __builtin_ctz(filterDivisor); // Count trailing zeros
                        pixelSum >>= shift;
                    } else if (filterDivisor != 0) {
                        pixelSum /= filterDivisor;
                    }

                    // Clamp result to [0, 255]
                    pixelSum = pixelSum < 0 ? 0 : (pixelSum > 255 ? 255 : pixelSum);

                    // Store the pixel value
                    output->color[plane][row][col] = static_cast<unsigned char>(pixelSum);
                }
            }
        }
    
    
    

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output -> width * output -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (output -> width * output -> height));
  return diffPerPixel;
}


