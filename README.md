# SenseTOP
### Custom RealSense plugin for TouchDesigner

This plugin retrieves the raw depth image and other data from the RealSense SR300 camera in TouchDesigner.  

The motivation was to gain more access to device settings than what's provided by Touch's default implemention.  

Features:
* Depth texture: 32bit float @60fps
* Device controls: 
   * Accuracy
   * Laser projector power
   * Filter options
   * Motion-range tradeoff

Tested with TouchDesigner 099, RealSense SDK 2016 R2, SR300 camera, and Windows 10.  

#### Licensing
SenseTOP code is released under the [MIT License](https://github.com/kamindustries/SenseTOP/blob/master/LICENSE).