# pacdrive_rp2040
quick and dirty pacdrive compatible firmware for the rp2040

# usage
...don't 

the pacdrive isn't perfectly HID compliant (the hid report is invalid) and often requires special vendor libraries or shims to talk to properly. Most HID applications will rely on the report to be valid and thus the PacDrive itself won't work with them.

This project was an experiment to see how quickly I could remake it and to get some debug output for stepmania based games.

but if you really want to use this...then setup your pin definitions in `main`, and compile.

```
mkdir build
cd build
cmake ../
make all -j$(nproc)
```

then flash on your rp2040/pi pico

if you need debug/printf to work, change the `pico_enable_stdio_uart` in the cmake file to 1.