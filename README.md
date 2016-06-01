# RF Front-end Modules

![Sony PA and TriQuint LNA boards](/images/PA_LNA_722w.jpg)

The [Front-end Modules project](https://myriadrf.org/projects/front-end-modules/) is concerned with creating RF front-end modules, such as PAs, LNAs and filters, that can be used together with an SDR platform in order to create a complete wireless solution.

## Contents

The directory structure is as follows:

      LimeSDR_PA_LNA/           
          firmware/              - firmware for Sony PA
          gui/                   - GUI for firmware PA
          hardware/              - design files for Sony PA and TriQuint LNA boards
          lms_vcp_drivers/       - Windows Virtual COM Port drivers for Sony PA board

      LimeSDR_SAW/<version>/
          BOM/                   - bill of materials spreadsheet
          DRC/                   - Design Rule Verification Report
          ERC/                   - Electrical Rule Check Report
          Gerber/                - Gerber CAM files
          NC Drill/              - Drill files
          Pick Place/            - Pick and place files         

## Licensing

### Hardware

The hardware designs are licensed under a Creative Commons Attribution 3.0 Unported licence.

### Firmware

All original code is provided under the Apache 2.0 License.
