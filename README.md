# Intro to Computer Communication EX01  
Iris Taubkin 208410969
Omri Elad 204620702

## General Information  
In order to vuild the solution correctly we used a sub-solution for each module (Sender, Receiver, Noisy-Channel)  
some of the In order to share features and functions between the 3 modules, we added a 4th sub-solution called utils (a static library).  
In this way each soultion ban be built and debugged independently but code can still be shared. 

## Initializion Process on Visual Studio Code  
Make sure to aplly all these changes for each sub-solution (in project properties):  
* add `Ws2_32.lib` to `Linker->Input->Additional Dependencies`  
* add `_CRT_SECURE_NO_WARNINGS;_WINSOCK_DEPRECATED_NO_WARNINGS ` to `C/C++->Preprocessor->Preprocessor Definitions`  
* add `../../utils/utils` to `C/C++->Additional Include Directories` (**no need on utils sub-solution**)  
<br>In addition make sure to modify utils to be `.lib`: `General->Configuration Type->Static Library`  

## Task Managment:
Sender: Iris
Receiver: Iris
Utils: Omri
Noisy Channel: Omri
