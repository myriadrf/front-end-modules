Board Description
-----------------

board designation           : LimeSDR_SAW
board version		        : v1.0
board type                  : Lead Free
board size                  : 20 mm x 15.88 mm
board thickness             : 1.6 mm +/- 10 %
board material              : TG170
number of layers            : 2
 
Copper foil thickness: 35 um

minimum finished hole size  :  300 um
minimum spacing             :  200 um
minimum track width         :  200 um

drill diameters             : finished hole

plating finish (both sides) : immersion gold
                              0.05-0.10 um of gold over
                              2.50-5.00 um of nickel
			  
Top silkscreen              : Required
Bottom silkscreen           : Required

Drill files
-----------------
   - LimeSDR_SAW_1v0.DRR -> Drill report detailing the tool assignments, hole sizes, hole count and tool travel. 
   
   - Throughole vias are covered in the following files:
   								File Name																Start Layer						Stop Layer
							LimeSDR_SAW_1v0.TXT															Top								Bottom

Gerber files
---------------

			File Name					Layer/Comment
		LimeSDR_SAW_1v0.GTL				Top (RF/GND)
		LimeSDR_SAW_1v0.GBL				Bottom (Signal/PWR/GND)

		 
		LimeSDR_SAW_1v0.GPB				Bottom Pad Master
		LimeSDR_SAW_1v0.GPT				Top Pad Master

		LimeSDR_SAW_1v0.GTO				Top Overlay
		LimeSDR_SAW_1v0.GTP				Top Paste
		LimeSDR_SAW_1v0.GTS				Top Solder

 
		LimeSDR_SAW_1v0.GBS				Bottom Solder
		LimeSDR_SAW_1v0.GBP				Bottom Paste
		LimeSDR_SAW_1v0.GBO				Bottom Overlay


		LimeSDR_SAW_1v0.GM1				Mechanical 1 (Board Cutout)


		LimeSDR_SAW_1v0.GM14			ASM BOT (Assembly bottom)
		LimeSDR_SAW_1v0.GM15			ASM TOP (Assembly top)
		
Important Notes
---------------
DRCs must be run on Gerber files before building boards.

All through hole vias may be plated shut.

Solder mask : dark blue, both sides, halogen free, glossy finish (NOT matte).

Silkscreen : white epoxy ink, halogen free, both sides. No silkscreen on pads.

Electrical test : 100 % netlist.

Boards are to be individually bagged.

Design software used:  Altium Designer 15.0 