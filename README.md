

# Local_EACSF

LocalEACSF is a stand-alone tool which locally quantifies the extra-axial
cerbrospinal fluid (EA-CSF).

<h2>Build instructions</h2>

Requirements : Qt and python

1. Get files from Github:  
 ```
 $ git clone https://github.com/NIRALUser/Local_EACSF
 ```
 
  
2. Make an out-of-source build directory: 
```
$ mkdir /Local_EACSF-build
$ cd Local_EACSF-build
```

3. Configure the build:
```
 $ cmake ../Local_EACSF  
 $ make 
```
4. Open the application by typing the line below :  
```
 $ cd /Local_EACSF-inner-build/bin 
 $ ./Local_EACSF  
```
 
 
