call ../misc/nightly/base_32.bat

nmake /f makefile_vc build USEDEPS=1 NO_RELEASE_PDB=1
