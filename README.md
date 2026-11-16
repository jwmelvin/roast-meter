<!--
repo name: roast-meter
description: DIY roast meter guide
github name:  Juztins-Lab
link: https://github.com/juztins-lab/roast-meter
logo path: assets/logo.png
screenshot: 
twitter: your_username
email: justin@skitlabs.net
-->

<!-- PROJECT SHIELDS -->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
<!-- [![LinkedIn][linkedin-shield]][linkedin-url] -->



<!-- PROJECT LOGO -->
<br />
<p align="center">
    <a href="https://github.com/juztins-lab/roast-meter">
        <img src="assets/logo.png" alt="Logo" width="80" height="80">
    </a>
<h3 align="center"><a href="https://github.com/juztins-lab/roast-meter">Roast Meter</a></h3>
    <p align="center">
        An awesome DIY projects that will help you get better consistent roasts by measuring roast color between batches
        <br />
        <br />
        <a href="https://github.com/juztins-lab/roast-meter/issues">Report Bug</a>
        •
        <a href="https://github.com/juztins-lab/roast-meter/issues">Request Feature</a>
    </p>
</p>



<!-- TABLE OF CONTENTS -->
## Table of Contents

- [About The Project](#about-the-project)
- [Assembly Instructions](#assembly-instructions)
- [User Guide](#user-guide)
- [Calibration process](#calibration-process)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)



<!-- ABOUT THE PROJECT -->
## About The Project

Starting into home roasting I've quickly realised the importance of a roast meter to evaluate the color of the coffee, this helps you better target different end temperature/time but retain the same level of color and see how it influences taste.


Here's why:
* Commercial grade roast meters are very expensive
* Chips and sensors are are readily available/cheap and easy to get going
* At the end of the guide you will have an ~100$ roast meter, that is consistent and can help you get the most out of your roast.

Check out our wiki for detailed instructions for assembly usage and calibration!

## Assembly Instructions
https://github.com/juztins-lab/roast-meter/wiki/Roast-Meter-Assembly-Instructions

## User Guide
https://github.com/juztins-lab/roast-meter/wiki/Roast-Meter-User's-Guide

## This fork: full-resolution, averaged, reference-normalised readings

Branch `feature/full-resolution` changes the plain build (`src/roast_meter.ino`);
the Bluetooth build is untouched.

* **Tenths.** Upstream divides the IR count by 1000 and drops the remainder
  before mapping it, so its display moves in steps of 1.165. The whole count is
  now kept.
* **A 2 s mean, with its scatter.** The number is the mean of the last 25
  samples. Right of it: the tenths (`..` until 2 s are in), the standard
  deviation of those samples in the same units, `ok` once that is under 0.5,
  and the reference state.
* **Baking soda re-references the meter.** A settled reading within 20 of 230
  is taken to be the soda standard (`REF`). Until power-off every later reading
  is scaled so soda is exactly 230.0 (`ref`). Before soda has been read the
  scale is `ledBrightness` alone, as upstream (`raw`). The reference is not
  stored: read soda at the start of each session.
* Serial prints `real` (mean count), `sd`, `n`, `raw` (un-normalised),
  `reference` (soda count in force) and `agtron`.

The arithmetic is in `src/roast_math.h`, tested on a desktop:

    c++ -std=c++11 -Isrc test/host/roast_math_test.cpp -o /tmp/roast_math_test && /tmp/roast_math_test

**Not yet built for or run on the hardware.**

## Calibration process
https://github.com/juztins-lab/roast-meter/wiki/Roast-Meter-User's-Guide#calibration-methods

<!-- ROADMAP -->
## Roadmap
* ~~Calibration against Agtron Gourmet Scale~~ (DONE)
* ~~Add battery, on/off button~~ (DONE)
* ~~Sensor calibration process - improved for consistency~~ (DONE)
* ~~Mobile application to store coffee roast information as well as roast level and history (bluetooth)~~ (BT version available but works with **external** web app)

See the [open issues](https://github.com/juztins-lab/roast-meter/issues) for a list of proposed features (and known issues).



<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to be learn, inspire, and create. Any contributions you make are **greatly appreciated**.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request



<!-- LICENSE -->
## License

Distributed under the GNU License. See `LICENSE` for more information.



<!-- CONTACT -->
## Contact

Juztin - roastmeter@skitlabs.net

Project Link: [https://github.com/juztins-lab/roast-meter](https://github.com/juztins-lab/roast-meter)



<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/juztins-lab/roast-meter.svg?style=flat-square
[contributors-url]: https://github.com/juztins-lab/roast-meter/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/juztins-lab/roast-meter.svg?style=flat-square
[forks-url]: https://github.com/juztins-lab/roast-meter/network/members
[stars-shield]: https://img.shields.io/github/stars/juztins-lab/roast-meter.svg?style=flat-square
[stars-url]: https://github.com/juztins-lab/roast-meter/stargazers
[issues-shield]: https://img.shields.io/github/issues/juztins-lab/roast-meter.svg?style=flat-square
[issues-url]: https://github.com/juztins-lab/roast-meter/issues
[license-shield]: https://img.shields.io/github/license/juztins-lab/roast-meter.svg?style=flat-square
[license-url]: https://github.com/juztins-lab/roast-meter/blob/master/LICENSE.txt
[product-screenshot]: images/screenshot.png
