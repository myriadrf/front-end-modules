Contact Details
---------------

Lime Microsystems Ltd.
Surrey Technology Centre, Occam Road,             
The Surrey Research Park,
Guilford, Surrey.
GU2 7YG 
United Kingdom

http: www.limemicro.com

contact person:
  Ignas Skudrcikas
  email: i.skudrickas@limemicro.com

Licence: 
Copyright 2016 Lime Microsystems, Ltd.
WORK IS COVERED UNDER A CREATIVE COMMONS LICENSE (CC BY 3.0)


Board Description
-----------------

board designation       : LimeSDR-Sony Board v.1 r.3
board type              : Lead Free
board size              : 51.9 mm x 80.1 mm
board thickness         : 1.6 mm +/- 10 %
board material          : IT-180A

number of layers        : 6

Top layer copper foil thickness: 0.333+plating
Dielectric thickness between Top layer and 2nd layer: 7.024 mil
Dielectric between Top layer and 2nd layer relative permittivity (Er): 3.8


minimum finished hole size  : 200 um
minimum spacing             : 100 um
minimum track width         : 100 um


drill diameters             : finished hole

plating finish (both sides) : immersion gold
                              0.08-0.20 um of gold over
                              2.50-5.00 um of nickel


Important Notes
---------------
DRCs must be run on Gerber files before building boards.

Solder mask : dark blue, both sides, halogen free, glossy finish (NOT matte).

Silkscreen : white epoxy ink, halogen free, both sides. No silkscreen on pads.

Electrical test : 100 % netlist.

Boards are to be individually bagged.

Design software used:  KiCad 4.0.2-stable release build


Controlled Impedance
--------------------

  * 50 Ohm coated single ended microstrip (Top layer)
    IT-180A, 7.024 mil, Er = 3.8, metal thickness 0.333+plating
    track width 0.31 mm



Board Stackup
------------
LimeSDR-Sony-F_Paste.gtp
LimeSDR-Sony-F_SilkS.gto
LimeSDR-Sony-F_Mask.gts
LimeSDR-Sony-Front.gtl           (TOP)
LimeSDR-Sony-Inner1_GND.gbr      (L1)
LimeSDR-Sony-Inner2_Signal.gbr   (L2)
LimeSDR-Sony-Inner3_Signal.gbr   (L3)
LimeSDR-Sony-Inner4_Power.gbr    (L4)
LimeSDR-Sony-Back.gbl            (BOT)
LimeSDR-Sony-B_Mask.gbs
LimeSDR-Sony-B_SilkS.gbo
LimeSDR-Sony-B_Paste.gbp



Reports folder
--------------           			                    
LimeSDR-Sony.cmp (component report)
LimeSDR-Sony.drc.rpt (Design Rules Check Report)
LimeSDR_Sony.erc
LimeSDR-Sony.net(netlist)
LimeSDR-Sony-drl.rpt (Drill Size Table Report)               

                  


