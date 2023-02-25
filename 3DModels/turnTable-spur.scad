use <gears.scad>
use <stepper.scad>

THICK = 2;
M5=5.5;
M4=4.5;
M3=3.5;
M2=2.5;
M3INSERT=3.7;
M2INSERT=3.4;


Module = 1;
width = 5.0; // 0.01
teeth = 30;
bore = 3; // 0.01
straight = false;

/* [Advanced Parameters] */
hub = true;
hub_diameter = 6; // 0.01
hub_thickness = 5; // 0.01
// (Weight optimization if applicable)
optimized = false;
pressure_angle = 20; // 0.01
helix_angle = 20; // 0.01
clearance = 0.05; // 0.01

/* [Multi-gear configurations] */
idler_teeth = 100;//36;
idler_bore = 3; // 0.01
assembled = true;

/* [Rack parameters] */
rack_length = 50; // 0.01
rack_height = 4; // 0.01

/* [Worm parameters] */
// Worm lead angle                                                                (For final worm diameter, see also number of starts.)
lead_angle = 4; // 0.01
// Number of starts                                                                (For final worm diameter, see also lead angle.)
worm_starts = 1;
// Worm bore                                             (Please note: The bore in Basic Gear Parameters is used for the bore of the worm gear, not the worm.)
worm_bore = 3; // 0.01
worm_length = 6; // 0.01

/* [Bevel gear parameters] */
bevel_angle = 45; // 0.01
bevel_width = 5; // 0.01
shaft_angle = 90; // 0.01

/* [Annular/internal gear parameters] */
rim_width = 3; // 0.01

/* [Planetary gear parameters] */
solar_teeth = 20;
planet_teeth = 10;
number_of_planets = 3;


module customizer_separator() {}

finalHelixAngle = straight ? 0 : helix_angle;
final_hub_diameter = hub ? hub_diameter : 0;
final_hub_thickness = hub ? hub_thickness : 0;

$fn = 96;

/*
 stirnrad (modul=Module, zahnzahl=teeth, breite=width, bohrung=bore, nabendurchmesser=final_hub_diameter, nabendicke=final_hub_thickness, eingriffswinkel=pressure_angle, schraegungswinkel=finalHelixAngle, optimiert=optimized);

*/


/*
kegelrad(modul=Module, zahnzahl=teeth,  teilkegelwinkel=bevel_angle, zahnbreite=bevel_width, bohrung=bore, eingriffswinkel=pressure_angle, schraegungswinkel=finalHelixAngle);

*/

/*
kegelradpaar(modul=Module, zahnzahl_rad=idler_teeth, zahnzahl_ritzel=teeth, achsenwinkel=shaft_angle, zahnbreite=bevel_width, bohrung_rad=idler_bore, bohrung_ritzel=bore, eingriffswinkel=pressure_angle, schraegungswinkel=finalHelixAngle, zusammen_gebaut=assembled);
*/

//cylinder(h=1, d=100, center=true);



mainBearing=[30,47,9];
baseH=40;




driveCog();

color("#A0FFA0")motorMount();
table();
color("#A0A0FF")base();

translate([0,0, mainBearing[2]/2])bearing30x47x9();
motor();


sensor();




//Test outer bearing
/*
intersection(){
    table();
    cylinder(d=60, h= 30, center=true);
}
*/


//Test inner bearing

/*
intersection(){
    base();
    cylinder(d=100, h= 30, center=true);
}
*/



module bearing30x47x9(){
    color("#505050"){
        difference(){
            cylinder(d=mainBearing[1], h = mainBearing[2], center=true);
            cylinder(d=mainBearing[0], h = mainBearing[2]*2, center=true);
        }
    }
}


module driveCog(){
    driveTeeth = 20;
    translate([-39,0,1]){
        stirnrad (modul=Module, zahnzahl=driveTeeth, breite=width, bohrung=bore, nabendurchmesser=final_hub_diameter, nabendicke=0, eingriffswinkel=pressure_angle, schraegungswinkel=finalHelixAngle, optimiert=optimized);
    }
    
    translate([-39,-8,18]){
        rotate([0,180,0])
            hub_28byj48();
    }
}




module table(){
    bearingShrink=0.5;
    translate([0,0,0])
        hohlrad (modul=Module, zahnzahl=idler_teeth, breite=width, randbreite=rim_width, eingriffswinkel=pressure_angle, schraegungswinkel=finalHelixAngle);
    
    translate([0,0,THICK/-2-2])
            cylinder(h=THICK, d=145, center=true);
    
    translate([0,0,THICK/-2])
        difference(){
            cylinder(h=THICK, d=108, center=true);
            cylinder(h=THICK*3, d=106, center=true);
        }
    
    
    difference(){
        translate([0,0,mainBearing[2]/2-2])
            cylinder(h=mainBearing[2], d=mainBearing[1]+THICK*3, center=true);
        cylinder(d=mainBearing[1]+ bearingShrink, h=mainBearing[2]*4, center=true);
    }
         
}




module motor(){
    
    //translate([-39+8,0,18]){
    translate([-39-8,0,18]){

        rotate([0, 0,0]) rotate([0,0,-90]) stepper_28byj48();
    }
}


module cutout_28byj48(){
    //Side M3 Bolts
    for (i=[-1, 1]){
        translate([17.5 * i,0,0]){
            cylinder(d=M3, h=THICK*3, center=true);
        }
    }
    
    translate([0,8,0]){
        cylinder(d=12, h=THICK*3, center=true);
    }
}


module hub_28byj48(){
    spindleD = 5.2;
    spindleX = 3.2;
    hubLen = 10;
    hubDiam = 14;
    translate([0, 8, THICK*1.5 + hubLen/2]){
        difference(){
            union(){
                cylinder(d=hubDiam, h=hubLen, center=true);
                cube([hubDiam/1.5, hubDiam , hubLen], center=true);
            }
            
            //Shaft
            intersection(){
                cylinder(d=spindleD, h=hubLen+THICK, center=true);
                cube([spindleX, hubDiam,hubLen+THICK], center=true);
            }
            
        
        
            // M2 Holes
            for (i=[-1,1]){
                translate([0, i*4, -3]){
                    rotate([0,90,0]) {
                        cylinder(d=M2, h= hubDiam *2, center=true);
                        translate([0,0,hubDiam/2])
                            cylinder(d=M2INSERT, 8, center=true);
                    }
                }
                translate([0, i*4, 1]){
                    rotate([0,90,0]){
                        cylinder(d=M2, h= hubDiam *2, center=true);
                        translate([0,0,hubDiam/2])
                            cylinder(d=M2INSERT, 8, center=true);
                    }
                }
            }
            
            
            //Slot
            translate([0,0,-2]){
                cube([1, hubDiam*1.5, hubLen], center=true);
            }
            
        }
        
    }
}


module base(){
    edge = [-34.5,0,45-18];
    bearingShrink=0;
    
    
    
    //Base Plate
    translate([0,0,baseH]) {
        cylinder(h=THICK, d=100, center=true);
    }
    
    //Mainbearing Post
    translate([0,0,(baseH + mainBearing[2])/2-THICK]){
        difference(){
            //Outer Wall
            cylinder(d=mainBearing[0]+THICK*3, h = baseH - mainBearing[2] + THICK, center=true);
            //Seat
            translate([0,0,baseH/-2+THICK*2.5]) difference(){
                cylinder(d=mainBearing[1]+THICK*2, h = mainBearing[2]/2, center=true);
                cylinder(d=mainBearing[0]-bearingShrink, h = mainBearing[2]/2, center=true);
            }
            
            //Inner
            cylinder(d=mainBearing[0]-THICK*3, h = baseH *2, center=true);
        }
    }
    
    
    //Motor Plate
    translate([-47,0, baseH]) {
        translate([22,0,(10+THICK)/(-2)]){
            difference(){
                cube([THICK, 40, 10], center=true);
                for (y=[-17,17])translate([0,y,1]){
                    rotate([0,90,0])
                        cylinder(d=M3, h=THICK*4, center=true);
                }
            }
        }
    }
   
}

module motorMount(){
    
    
    //Motor Mount
    motorH=24;
    translate([-47,0, baseH]) {
        translate([0,0,(motorH+THICK)/(-2)]){
            difference(){
                cube([40,50,motorH], center=true);
                translate([THICK*-1,0,THICK]){
                    cube([40,50-THICK*2,motorH], center=true);
                }
                translate([THICK*-5,0,THICK]){
                    cube([40,55-THICK*2,motorH], center=true);
                }
                
                translate([0,0,motorH/(-2)]){
                    rotate([0,0,-90]){
                        cutout_28byj48();
                        for (i=[-10:1:10]){
                            rotate_about([0,0,i],p=[17.5,0,0]){
                                cutout_28byj48();
                            }
                        }
                    }
                }
                
                //Bolt holes
                translate([40/2,0,motorH/(2) - THICK*2]){
                    for (y=[-17,17])translate([0,y,0]){
                        rotate([0,90,0])
                            cylinder(d=M3, h=THICK*4, center=true);
                    }
                }
            }
        }
    }
}

module rotate_about(a, v, p=[0,0,0]) {
     translate(p) rotate(a,v) translate(-p) children();
}


module m4Spacer(){
    difference(){
        cylinder(d=7, h=4, center=true);          
        cylinder(d=M4, h=40, center=true);     
    }
}


module sensor(){
    
    bracketW=10;
    bracketH=35-10;
    bracketL=40;
    pcbW=10;
    pcbL=10;
     //Motor Plate
    translate([-47 + THICK,0, baseH]) {
        translate([22,0,(10+THICK)/(-2)]){
            difference(){
                cube([THICK, 40, 10], center=true);
                for (y=[-17,17])translate([0,y,1]){
                    rotate([0,90,0])
                        cylinder(d=M3, h=THICK*4, center=true);
                }
            }
        }
    }
    
    //Right Angle Bracket
    translate([-47+22 + THICK,bracketL/2+20, baseH - bracketW/2-THICK/2]) {
        //Base
        cube([THICK,bracketL,bracketW], center=true);
        
        //Upright
        translate([0, bracketL/2, bracketH/-2+bracketW/2])
            cube([THICK, bracketW, bracketH], center=true);
        
        //PCB Plate
        translate([(pcbW-THICK)/-2, bracketL/2, bracketH/-1+bracketW/2]){
            difference(){
                cube([pcbW, pcbL,THICK], center=true);
                cylinder(d=M2, h=THICK*3, center=true);
            }
        }
    }
}