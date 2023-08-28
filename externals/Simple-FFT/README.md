Simple-FFT
==========

**Header-only C++ library implementing fast Fourier transform of 1D, 2D and 3D data.**

### What's this

Simple FFT is a C++ library implementing fast Fourier transform. The implemented FFT is a radix-2 Cooley-Turkey algorithm. This algorithm can't handle transform of data which size is not a power of 2. It is not the most optimal known FFT algorithm.

The library is header-only, you don't need to build anything, just include the files in your project. The user-level interface is simple and reminds the one typically found in mathematical software.

The library is distributed under MIT license

#### Why one more FFT library? Does it have any benefits compared to existing libraries?

Why do humans create things even though they already have something similar? Because they are not satisfied with what they have. So was my impression with existing libraries implementing fast Fourier transforms. My desires were:
 * Free & open-source library
 * License allowing to use the library in both open and closed-source software.
 * Convenient API somewhat similar to the one found in typical mathematical software.
 * Limited size and dependencies, ideally no dependencies (handy for multiple supported platforms and compilers).
 * As long as my arrays are not going to be huge, I don't need the fastest FFT in the galaxy, I'd be quite happy with some algorithm getting the job done.
 * Having said that my arrays are not too large, I still want minimal or no overhead for copying the data to objects of types used in the library.

The last statement deserves some explanation: many popular libraries performing FFT use their own data types, sometimes even their own containers. If you already have the data of your own type/container, you are going to either manually transform (e.g. copy) the data or try to cast pointers. Both ways are not elegant.

I didn't find the solution corresponding to the whole wishlist of mine. So I decided to create my own simple library. I've already mentioned some of disadvantages of such approach:
 * Not the fastest algorithm known nowadays.
 * Can't handle data which size is not a power of 2.

The advantages of Simple FFT behind some well-known and widely used libraries are:
 * Tiny size - just some header files.
 * No need to build it and link a library to your project.
 * Can handle 1D, 2D and 3D arrays, extendable for larger dimensions.
 * Designed to support any _reasonable_ multidimensional array/container type one can imagine.

Again, the last statement needs some details to be revealed: C++ natively supports multidimensional arrays via pointers and also via "std vector of std vectors" approach. However, there are a lot of libraries with their own implementations of multidimensional arrays. I wanted to create a library which in theory can use _any_ type of multidimensional array without data copying or pointer casting. It is not possible to guarantee that my library will work with every multidimensional array type you can imagine but there is only a limited number of restrictions for used types.

#### How the API is convenient? How to actually use the library?

By convenience of API I mean the interface somewhat similar to that found in mathematical software like Mathcad, Matlab, Octave, Scilab etc. The simplest API you can think of is something like `A = FFT(B)`. It is not very easy to efficiently implement in C++ (well, without move semantic of C++11 at least) because with such interface you are going to return the result of B transform by value which means data copying i.e. overhead (unless return by value optimization is employed). So in C++ it is better to return the result by reference. The function can also return boolean which would indicate whether the transform was successful or not. So the simplest interface would look like

```c++
 b = FFT(A,B); // FFT from A to B
```

But FFT algorithm requires the knowledge of shape and dimensionality of used arrays. Most multidimensional array implementations can provide this information but they do it in different ways and I wanted something very generic. So I decided to create functions with the same name but different signature depending on the number of dimensions:

```c++
 b = FFT(A,B,n); // FFT from A to B where A and B are 1D arrays with n elements
 b = FFT(A,B,n,m); // FFT from A to B where A and B are matrices (2D arrays) with n rows and m columns
 b = FFT(A,B,n,m,l); // FFT from A to B where A and B are 3D arrays with n rows, m columns and l depth number;
```

One more thing: if the returned value is false then some error has happened. In order to protect user from debugging into 3rdparty library to figure out what happened I decided to return error description as C-style string (because some people don't use `std::string`). So the interface became looking like this:

```c++
 const char * error = NULL; // error description
 b = FFT(A,B,n,error); // FFT from A to B where A and B are 1D arrays with n elements
 b = FFT(A,B,n,m,error); // FFT from A to B where A and B are matrices (2D arrays) with n rows and m columns
 b = FFT(A,B,n,m,l,error); // FFT from A to B where A and B are 3D arrays with n rows, m columns and l depth number;
```

But how about the inverse transform? The flag can be used to tell the forward transform from inverse but I thought that different function names would be easier: FFT for forward transform and IFFT for inverse transform:

```c++
 const char * error = NULL; // error description
 b = FFT(A,B,n,error); // forward FFT from A to B where A and B are 1D arrays with n elements
 b = FFT(A,B,n,m,error); // forward FFT from A to B where A and B are matrices (2D arrays) with n rows and m columns
 b = FFT(A,B,n,m,l,error); // forward FFT from A to B where A and B are 3D arrays with n rows, m columns and l depth number;
 b = IFFT(B,A,n,error); // inverse FFT from B to A where A and B are 1D arrays with n elements
 b = IFFT(B,A,n,m,error); // inverse FFT from B to A where A and B are matrices (2D arrays) with n rows and m columns
 b = IFFT(B,A,n,m,l,error); // inverse FFT from B to A where A and B are 3D arrays with n rows, m columns and l depth number;
```

Beyond that there are only two settings:
* User needs to define two types called `real_type` and `complex_type`. These are needed by design to avoid extra problems with template instantiation
* User needs to define macro `__USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR` if the multidimensional array type you want to use accesses elements via `operator[]` (for example, if it is native C++ multidimensional array or `boost::multi_array` or something similar). Otherwise the library will attempt to use `operator()` for element access.

That's the whole explanation of API.

#### How does it work with multiple libraries implementing matrices and multidimensional arrays in C++ without being aware of those?

Well, the common generic technique of C++ was used - templates. I also added two `typedef`ed types (`real_type` and `complex_type`) to avoid overcomplication of code. But it's not all about templates - I also tried to implement the most generic element access I could imagine. After a bit of thinking I realized that in terms of interface different libraries implementing multidimensional arrays commonly differ only by their element access operator - some implementations use `operator[]` and others - `operator()`. So I splitted every code section using element access operator into two code blocks - the one with `operator[]` and the one with `operator()`. The switch between them is controlled by macro `__USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR` - if it's defined, `operator[]` is used, if not - `operator()`. User can define this macro in some translation units and not define/undef in other ones - just like I did with unit-tests.

#### Are there any examples or, even better, tests?
I wrote some tests illustrating how to use SimpleFFT library along with some well-known C++ libraries implementing multidimensional arrays. They can also serve as examples (probably, a bit overcomplicated). The tests are based on the following checks:
* after each forward of inverse FFT [Parseval's theorem](https://en.wikipedia.org/wiki/Parseval%27s_theorem) must be satisfied for input and output data
* after each sequence of FFT and IFFT the energy conservation law must be satisfied
* after each sequence of FFT and IFFT the result should be the very same as initial input; so I decided to measure the largest discrepancy between the result and input and calculate the relative error there. If it is small enough (less than 0.01%), this test is considered passed.

I also implemented a simple benchmark test comparing the execution time of multiple loops of Simple FFT and fftw3. The results for Linux with three compilers can be found in the "benchmark-tests" folder. As expected, fftw3 is much faster.

#### There are multiple files here, which I should use?
There are only two files of interest for library user:
* `/include/simple_fft/fft_settings.h`
* `/include/simple_fft/fft.h`

The first one is supposed to contain `typedef`s for `real_type` and `complex_type` and, if needed, the define of `__USE_SQUARE_BRACKETS_FOR_ELEMENT_ACCESS_OPERATOR` macro. This stuff can also be done somewhere else. The second file is what's actually needed to calculate FFT and IFFT: it contains only API declarations and includes some of other files.
