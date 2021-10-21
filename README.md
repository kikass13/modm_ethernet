# modm_ethernet

## Requirements
 * ...

## Build

create and bind modm (see project.xml)
```bash
lbuild build ### do only once
```

compile code for target
```bash
scons
```

## Run

flash target

```bash
scons program
```

you can now connect the uart via usb and watch the log and visit 192.168.1.1 in your browser (after connecting the rj45) to see the webserver

