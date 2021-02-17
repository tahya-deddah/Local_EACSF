

# Local_EACSF
<h1>About Local_EACSF</h1>

Local_EACSF is a Qt interface tool which computes the extra-axial
cerbrospinal fluid density

<h2>Build instructions</h2>

Requirements: Qt5, Python3 (with packages: itk, numpy)

Build:

    Get files from Github

$ git clone https://github.com/tahya-deddah/Local_EACSF

    Make an out-of-source build directory

$ mkdir ./Local_EACSF-build
$ cd Local_EACSF-build

    Configure the build

$ cmake ../Local_EACSF
$ make

Open the application by typing the line below 

$ ./Local_EACSF
