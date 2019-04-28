# FPGA-Object-detection
Code from final year project - Design of an FPGA based image processing system.

Based on Zedboard - with Zynq 7020 device.

Processing is split between Programable Logic - Object detection - and Processing System - Object tracking and decision making. 

PL object detection pipeline : MOG Foreground Detection --> Morphological operations --> BLOB analysis.

Matlab was used to model the system to confirm algorithm choices and to test HLS implementations of algorithms against bench mark.

Tests compare MOG outputs and bounding boxes of identified objects

Code is structured as below:\
  •	MATLAB code\ 
    o	Modelling\
    o	Testing\
  •	HLS code\
    o	MOG core\
    o	Metamorphic operations core\
    o	BLOB analysis core\
  •	PS SDK Code\
    o	IP Core setup\
    o	Tracking and decision making\
