# TTN Mapper T-beam

Energy efficient mapping software for [ttnmapper](http://ttnmapper.org/) using a T-beam.

Send your GPS-Location over [TTS](https://www.thethingsnetwork.org/). The Location can be viewed on the [advanced maps](https://ttnmapper.org/advanced-maps/) when the [integration](https://docs.ttnmapper.org/integration/tts-integration-v3.html) is activated.

## Features

* deep sleep ESP32 to save energy
* geofence for location upload
* detect no movement

## Hardware

I'm using the TTGO-T-beam v1.1(with the axp power management chip onboard)

![T-Beam v1.1](img/ttgo-t-beam-v1.1.jpg "T-Beam v1.1")

But the software can easily be modified to run on any ESP32-Board with a GPS and a LoRa module attached.

## Compiling

### Software

Clone this repository and open it with [Visual Studio Code(vscode)](https://code.visualstudio.com/). This code editor automaticly downloads the extension [platformio](https://platformio.org/) to compile and upload the program.

### Modification

You have to modify the [credentials.h](src/credentials.h) by inserting your own TTS-keys and the [locations.h](src/locations.h) to use or disable the geofencing option

### Upload

For uploading just hit the upload-button at the bottom of the vscode window
