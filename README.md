# SenseTOP
### Custom RealSense plugin for TouchDesigner

This plugin exposes certian device settings for the RealSense SR300 camera TouchDesigner. 

It is intended to run alongside Touch's native Realsense TOP and CHOP. Device settings set by this plugin carry over to the other Realsense instances. 

#### Features
* Depth texture: 32bit float @variable fps
* Device controls: 
   * Accuracy
   * Laser projector power
   * Filter options
   * Motion-range tradeoff

Documentation of these parameters can be found [here](https://software.intel.com/sites/landingpage/realsense/camera-sdk/v1.1/documentation/html/index.html?member_functions_f200_and_sr300_device_pxccapture.html).

Tested with TouchDesigner 099, RealSense SDK 2016 R2, SR300 camera, and Windows 10.  

#### Licensing
SenseTOP code is released under the [MIT License](https://github.com/kamindustries/SenseTOP/blob/master/LICENSE).