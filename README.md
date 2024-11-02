# Privacy Preserving Linear Regression
This is the implementation of privacy preserving linear regression.
# File structure
1. data : This folder has the data we are working with.
2. include: This folder has all the libraries we are using and the hpp header files we need to run the code.

3. src: has the source code.
    1. multiple linear regression: This folder has implementation of multiple linear regression without privacy preserving.
    2. privacy preserving linear regression (horizontally partitioned dataset): It folder has implementation of privacy preserving linear regression where data is horizontally partitioned between the parties.
    3.  privacy preserving linear regression (vertically partitioned dataset): It folder has implementation of privacy preserving linear regression where data is vertically partitioned between the parties.
# Package Requirements
## Eigen
It is a high-performing library for linear algebra, offering a wide range of functions for matrix and vector operations.
`https://gitlab.com/libeigen/eigen`
## Asio
Used for communication between parties and clients.
`https://github.com/chriskohlhoff/asio`
# Run the code
All folders has respective makefile. Run the make file to compile the code and get the executable file. Then run the executable file.
1. install eigen into your system.
2. compile the code using the Makefile. Use `make` command
3. run the code using `make run` command in the terminal.
4. first run the parties then run the client.