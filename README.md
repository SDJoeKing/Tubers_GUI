### An Upper Computer Software for A Customised Ultrasonic Testing Embedded Module 

## Introduction
This is an upper computer software that communicates with a customised ultrasonic testing (UT) embedded system. Using TCP Socket, this software connects to the embedded system which hosts a server, sends settings to the embedded system to configure the hardware, receives ultrasonic signals (A-scan), processes the ultrasonic signals for various NDT inspection requirements (B-scan, C-scan, TOFD etc), and visualises the scans. This software at its current state is for research purpose only, not to be compared with commercial software orsystems. 

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
## License
[GPL license](License/gpl-3.0.txt)
## Functions
  - MainWindow, resides in main thread, handles signals connection from other classes and plotting of time-series UT A-scan, and C-scan/TFM/TOFD if these are enabled.
  - mtcpClient, resides in a separate thread, which handles the communication to the hardware server, parses information, and notifies processor class for processing.
  - processor, resides in a separate thread, handles data processing including filtering, rectifying, enveloping, golay coding (if enabled), data preparating for plotting and TFM related tasks (if enabled).
  - tchartviewform, works in main thread for A-scan plotting, achieves functions such as zoom in/out, focus in area, data point gauges, gating (tgate class) etc.
  - tlogging, handles saving A-scan data into binary files locally. Can save single acquisition or continously.
  - tsettings, packs settings for Client module to send to hardware for configuration. Also contains triggering switches for various functions such as golay, B/C-scans, TFM or TOFD.
  - Other notes:
     - Release branch is currently configured for Time-of-flight Diffraction (TOFD) application, whereas other branches are targeted for other applications. C_scan branch is specialised to work with Arduino controlled automated system, which will exit running if not present.
     - When Color Map style plot is enabled, unless specifically programmed, the software and hardware turn into encoder-trigger based acquisition mode.
    
## Performance
  - A-scans: 16000, 16 bits data points are received and processed at 400 Hz.
  - Color Map plots (B/C-scan, TOFD): all received A-scans are used for processing to generate the color map plots, but the plot are limited to be updated at 30 Hz to avoid main thread jamming.
  - TFM: best performance 2 Hz TFM for 16 x 16 x 32kB FMC @125MHz sampling rate in real time acquisition.
    
## Demonstration
 [Release Branch] A TOFD trial test scan. A-Scan plot is hidden to give larger space for TOFD plot. Igore the "BScan Length" axis name.
 
  <img width="600" height="400" alt="TOFD demonstration" src="https://github.com/user-attachments/assets/ed4108ad-82b9-4a44-934c-14f105a29b3f" />
  
 [C-Scan demo, image only] A C-scan demo scan within a plastic pipe which have a loss of wall thickness and a small hole on the wall:
 
 <img width="600" height="400" alt="lom6" src="https://github.com/user-attachments/assets/a47061b7-e8ff-4477-8c12-02cd26dbb3cf" />
 
 [B-SCan] Demonstration of encoder-triggered B-scan on a calibration block with multiple Side-drilled holes (SDHs), with gates to set starting point and noise level for filtering:
 
 <img width="600" height="400" alt="SCREEN1" src="https://github.com/user-attachments/assets/de2aa244-a7c8-4eee-97aa-b84bb5f5bf6b" />

 [TFM Demo] Please see the GitHub Repo [TFM_Visualisation_Qt
](https://github.com/SDJoeKing/TFM_Visualisation_Qt). 
