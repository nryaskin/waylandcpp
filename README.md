# WAYLANDCPP
Provide cpp only implemenation for wayland protocol without libwayland c bindings.

## Build

To build with sample applications:
```
cmake -S . -B <BUILD_FOLDER> -DWAYLANDCPP_GENERATOR_PATH=<PATH_TO_CUSTOM_WAYLANDCPP_GENERATOR_FOLDER> -DWAYLANDCPP_APPS=ON
cmake --build <BUILD_FOLDER>

```
To build only static waylandcpp library.

```
cmake -S . -B <BUILD_FOLDER> -DWAYLANDCPP_GENERATOR_PATH=<PATH_TO_CUSTOM_WAYLANDCPP_GENERATOR_FOLDER>
cmake --build <BUILD_FOLDER>
```
## Samples

### query-display
For now only query display sample is supported. It uses _wl_output_ interface to query display information and display.

Example:
```
$ <BUILD_FOLDER>/apps/query_display/query_display 
Manufacturer : Najing CEC Panda FPD Technology CO. ltd
Model: eDP-1-unknown
Name: eDP-1
Description: Najing CEC Panda FPD Technology CO. ltd eDP-1-unknown
Position within the global compositor space: [0,0]
Physical size: 340 mm x 190 mm
Subpixel: Unknown
Transform: no transform
Resolution: 1920 x 1080, refresh rate: 120035 mHZ, mode: current
Scale: 1

```

