# Changes Implemented
##Change to data structure in 'cs1300bmp.h'

### Data Structure change
* I changed the 3D array into a 2D array of structs shorts(R, G, B)
Old code: 
```
int color[MAX_COLORS][MAX_DIM][MAX_DIM];
```
New code: 
```
struct Pixel{
  short R;
  short G;
  short B;
};
```
```
pixel color[MAX_DIM][MAX_DIM];
```
Reason: This reduces the number of passes through the array to visit every value for every pixel. It reduces the number of passes because instead of accessing another layer of the array, we are accessing a variable in the struct. Additionally, the 3D array involves three indeces which can lead to more cache misses. Modern processors work more efficiently with data that is laid out sequetially.

* Due to the data structure change above, I needed to change 'cs1300bmp.cc' to reflect this new change
Old code cs1300bmp.cc:
```
for (int row = 0; row < height; row++) {
  for (unsigned int col = 0; col < width; col++) {
    image->color[COLOR_RED][row][col] = rarray[row * width + col];
    image->color[COLOR_GREEN][row][col] = garray[row * width + col];
    image->color[COLOR_BLUE][row][col] = barray[row * width + col];
  }
}

...

for (int row = 0; row < height; row++) {
  for (int col = 0; col < width; col++) {
    rarray[row * width + col] = image->color[COLOR_RED][row][col];
    garray[row * width + col] = image->color[COLOR_GREEN][row][col];
    barray[row * width + col] = image->color[COLOR_BLUE][row][col];
  }
}
```
New code cs1300bmp.cc:
```
for (int row = 0; row < height; row++) {
  for (unsigned int col = 0; col < width; col++) {
    image->color[row][col].R = rarray[row * width + col];
    image->color[row][col].G = garray[row * width + col];
    image->color[row][col].B = barray[row * width + col];
  }
}

...

for (int row = 0; row < height; row++) {
  for (int col = 0; col < width; col++) {
    rarray[row * width + col] = image->color[row][col].R;
    garray[row * width + col] = image->color[row][col].G;
    barray[row * width + col] = image->color[row][col].B;
  }
}
```
### FilterMain.cpp
* Change filters in 'FilterMain.cpp'
I started by moving all variable initialization outsize the loops
```
int filterSize = filter->getSize();
int divisor = filter->getDivisor();
int height = (input->height) - 1;
int width = (input->width) - 1;
int *data = filter->getData();
```
Reason: We want to move invarient values outside the loop to prevent the same calculations being calculated over and over again. Putting them outside the loop and provide performance improvements.
* Flipped loop order
Old code:
```
for (int col = 1; col < (input->width) - 1; col++) {
  for (int row = 1; row < (input->height) - 1; row++) { ... }
}
```
New code:
```
for (int row = 1; row < (input->height) - 1; row++) {  
  for (int col = 1; col < (input->width) - 1; col++) { ... }
}
```
Reason: Flipping the loop order to iterate over rows first and then columns allow for better use of cashing and memory access. Memory heiarchies are designed to take advantage of spatial locality (whih is the tendency to access data that is close to one another). This is important because rows are subsequential and starting with rows prevents jumping through the array. This reduces cache misses and improves cache efficiency.

* Split cases since not all cases have a divisor that is > 1
```
if (divisor == 1) { ... } else { ... }
```
Reason: Seperating these cases presents unnesessary division operations when the divisor is 1, which is very common.

* Pre-Compute offset
In the original loop, the offset `row + j -1` and `col + i - 1` are computed in every iteration. Instead of having to compute these offsets in every single iteration, we can pre-compute these values outside the innermost loop which will prevent redundent calculations:
```
int rowOffset = row + j - 1;
int colOffset = col + i - 1;
```
Reason: This prevents redundent calculations by pre-calculating the offset outside the innerloop. We won't have to run the calculation after every single iteration of the for loop. 

* Loop Unrolling
For each filter application, the inner loop is unrolled to process two elements at a time. This means we process two columns for each row in a single iteration instead of just one column.
```
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
```
Reason: By unrolling the inner loop, we reduce the overhead of loop control and increae instruction-level parallelism. This also improves cache performace since more data is processed in fewer iterations, making more efficient use of cache.

* Final transfer of data to output at end of loop
Instead of accessing the array multiple times, we transfer all the data to local variables and then at the end, we transfer to the array.
```
output->color[row][col].R = rval;
```
Reason: Reducing the number of times we access the array by performing this once at the end can improve performance. We instead store the information in a local variable and save the transfer to the end. 

### Makefile
* Changes to the compiler flag
I made some changes to the compiler flags to improve efficiency. These changes include:
```
##We want to change -O0 to -0fast. This enables aggressive optimizations. This leads to faster executables but can sacrifice some compliance with standards.
##We want to change the target architecture to -march=native. This tells the compiler to optimize the code for the architecture of the machine where the compiling is being done. This can lead to better performance
##We want to enable loop unrolling which will increase the spead of the program by reducing overhead of the loop control code. -funroll-loops
##We want to enable AVX2 instructions which is for doing operations on multiple data points simulataneously. -mavx2
##We want to enable parallel programming (OpenMP). This is for the extra credit (if I can get to it)-fopenmp
```
```
CXXFLAGS= -g -march=native -Ofast -funroll-loops -mavx2 -fno-omit-frame-pointer -Wall -fopenmp
```

### Loop Order
* I switched the order of for loops to start with rows then columns to have more effecient access to memory and cache performance

### Strength Reduction
* I used simpler arithmetic operations to receive the same results.
* I also pre-computed offsets to prevent repeating calculations

### Code Motion
* I moved invarient values outside loops to reduce repeating calculations

### Loop Unrolling
* Included loop unrolling to be more efficient with how the program ran. With loop unrolling, we are able to run two columns in one iteration.

### GCC optimization level
* Used '0fast' along with other compiler flags that enable aggressive optimization features.