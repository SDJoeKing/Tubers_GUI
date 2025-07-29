### An Upper Computer Software for A Customised Ultrasonic Testing Embedded Module 

## Introduction
This is an upper computer software that communicates with a customised ultrasonic testing (UT) embedded system. This software connects to the embedded system which hosts a server via TCP Socket, sends settings to the embedded system to configure the hardware, receives ultrasonic signals (A-scan), processes the ultrasonic signals for various NDT inspection requirements (B-scan, C-scan, TOFD etc), and visualises the scans.

Part of this development was funded by [TUBERS](https://tubers-project.eu/) project, which is a research project from European Union’s Horizon Europe programme.

## Dependencies

 - The code was developed using Desktop Qt 6.8.0.
 - IDE Qt Creator 17.0.0.
 - Develop OS: Ubuntu 22.04, but can be compiled and run on Windows OS thanks to Qt's cross-platform development capability. 
 - [QCustomPlot](https://www.qcustomplot.com/) library, under GPL license to handle Color Map style plots.
 - [DSPFilters](https://github.com/vinniefalco/DSPFilters?tab=readme-ov-file), a collection of digital filters written in C++.
 - [For TFM FMC only] Matrix/Array manipulation was achieved using [Eigen 3](https://eigen.tuxfamily.org/index.php?title=Main_Page), Version 3.4.0.
 - The server code, firmware and the design of the UT embedded system are not public, without which this software cannot be tested to its fullest. A simple local server is included for testing purpose only which mimics the behaviour of the actual hardware. To enable the test server, in CMakeLists.txt:
   ```
    ## testing server enabling - note the settings wont work for testing servers except prf
    # add_definitions(-DTEST_SERVER) // uncomment this line
   ```

## Functions
  - MainWindow, resides in main thread, handles signals connection from other classes and plotting of time-series UT A-scan, and C-scan/TFM/TOFD if these are enabled.
  - mtcpClient, resides in a separate thread, which handles the communication to the hardware server, parses information, and notifies processor class for processing.
  - processor, resides in a separate thread, handles data processing including filtering, rectifying, enveloping, golay coding (if enabled), data preparating for plotting and TFM related tasks (if enabled).
  - tchartviewform, works in main thread for A-scan plotting, achieves functions such as zoom in/out, focus in area, data point gauges, gating (tgate class) etc.
  - tlogging, handles saving A-scan data into binary files locally. Can save single acquisition or continously.
  - tsettings, packs settings for Client module to send to hardware for configuration. Also contains triggering switches for various functions such as golay, B/C-scans, TFM or TOFD.
  - Other notes:
     - Release branch is currently configured for Time-of-flight Diffraction (TOFD) application, whereas other branches are targeted for other applications.
     - When Color Map style plot is enabled, unless specifically programmed, the software and hardware turn into encoder-trigger based acquisition mode.
    
## Performance and Limitations
  - A-scans: 16000, 16 bits data points are received at 400 Hz.
  
## Demonstration
 
  
 
<img src=" " alt=" " width =600 height=400>. 
 
 
<img src=" "  alt=" " width =600 height=400>
