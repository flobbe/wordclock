//
//  Model to hold WS2812 LED strips so that each LED illuminates only one letter of the word clock.
//

//               [mm]
// teal box
box_width      = 16.6+0.04;
box_height     = 19.4+0.04;
box_depth      = 12.5;
wall_thickness = 0.8;
// yellow base plate / led holder
base_thickness = 0.7;
// corner pod (maroon)
pod_corner     = 3;
pod_z          = box_depth - 3.4;
// spaces
led_size       = 5+1;
resistor_size  = 4;
// gap for solder between leds
solder_width   = 9;    // y
solder_length  = wall_thickness+6;  // x
solder_height  = 1.7;  // z
// cutter thickness
cutter         = 0.15;

$fs = 0.1;
$fa = 1;

module corner(trans, rot) {
    translate(trans)
        rotate(rot)
            polyhedron(
                points=[[0,0,0], [pod_corner,0,0], [0,pod_corner,0], [0,0,-pod_corner]],
                faces=[[0,1,2], [1,3,2], [0,2,3], [0,3,1]]
            );
}

module box(top=true, right=true, bottom=true, left=true) {
    wt = wall_thickness;
    bt = base_thickness;
    o = 0.1; // erode box when building difference
    translate([(box_width+wt)/2, (box_height+wt)/2, 0]) {
        difference() {
            union() {
                difference() {      
                    union() {
                        color("Teal") {
                            translate([0, 0, box_depth/2])
                                difference() {      
                                    cube([box_width+wt, box_height+wt, box_depth],   center=true);
                                    cube([box_width-wt, box_height-wt, box_depth+1], center=true);
                                } // difference
                        } // color: Teal
                        color("Yellow") {
                            translate([0, 0, bt/2]) {
                                cube([box_width-wt, box_height-wt, bt], center=true);
                            }
                        } // color: Yellow
                        color("Blue") {
                            w = 1.9;  // width of the "solder roof"
                            s = wt+1.4; // size of the bridge pier
                            translate([(box_width-wt-w)/2, 0, solder_height/2+bt])
                                cube([w, solder_width+2*s, solder_height], center=true);
                            translate([-(box_width-wt-w)/2, 0, solder_height/2+bt])
                                cube([w, solder_width+2*s, solder_height], center=true);
                        } // color: Blue
                    } // union
                    if (! top)
                        translate([0,  (box_height-cutter+o)/2, box_depth/2])
                            cube([box_width+wt+o,  wt+cutter+o, box_depth+o], center=true);
                    if (! bottom)
                        translate([0, -(box_height-cutter+o)/2, box_depth/2])
                            cube([box_width+wt+o,  wt+cutter+o, box_depth+o], center=true);
                    if (! left)
                        translate([-(box_width-cutter+o)/2, 0,  box_depth/2])
                            cube([wt+cutter+o, box_height+wt+o, box_depth+o], center=true);
                    if (! right)
                        translate([ (box_width-cutter+o)/2, 0,  box_depth/2])
                            cube([wt+cutter+o, box_height+wt+o, box_depth+o], center=true);
                } // difference
                color("Maroon") {
                    t = top    ? 0 : cutter;
                    r = right  ? 0 : cutter;
                    b = bottom ? 0 : cutter;
                    l = left   ? 0 : cutter;
                    corner([ (wt-box_width)/2+l,  (wt-box_height)/2+b, pod_z], [0, 0,   0]);
                    corner([-(wt-box_width)/2-r,  (wt-box_height)/2+b, pod_z], [0, 0,  90]);
                    corner([-(wt-box_width)/2-r, -(wt-box_height)/2-t, pod_z], [0, 0, 180]);
                    corner([ (wt-box_width)/2+l, -(wt-box_height)/2-t, pod_z], [0, 0, 270]);
                } // color: Maroon
            } // union
            color("Gray") {
                translate([0, 0, bt/2]) {
                    cube([led_size, led_size, bt+o], center=true);
                    cube([box_width, resistor_size, bt+o], center=true);
                }
                gap_x = solder_length;
                translate([ (box_width)/2, 0, (solder_height-o/2)/2])
                    cube([gap_x, solder_width, solder_height+o/2], center=true);
                translate([-(box_width)/2, 0, (solder_height-o/2)/2])
                    cube([gap_x, solder_width, solder_height+o/2], center=true);
            } // color: Gray
        } // difference
    } // translate
}


// ===== create all boxes =====
for (y = [1:11], x = [1:13])
    translate([(x-1) * box_width, (y-1) * box_height, 0])
        box();


// ===== OR, if your printer is not large enough, print it in parts =====
/*
parts = [1,2,3,4];  // select one of the four parts here to render and export separately

module __() {                    }  // empty element, to make spreadElems() over all childrens work
module b_() { box();             }  // normal box
module bt() { box(top=false);    }  // box without top wall
module bb() { box(bottom=false); }  // box without bottom wall
module bl() { box(left=false);   }  // box without left wall
module br() { box(right=false);  }  // box without right wall
module spreadElems(dx=0, dy=0) {
    for (i = [0:1:$children-1])   // step=1 needed in case $children < 2
        translate([i * dx, i * dy, 0])
            children(i);
}

for (p = parts) {
    if (p == 1) {
        translate([0, -box_height, 0])
            spreadElems(0, -box_height) {
                spreadElems(box_width, 0) { b_();b_();b_();b_();b_();br();      }
                spreadElems(box_width, 0) { b_();b_();b_();b_();b_();b_();br(); }
                spreadElems(box_width, 0) { b_();b_();b_();b_();b_();b_();br(); }
                spreadElems(box_width, 0) { b_();b_();b_();b_();b_();br();      }
                spreadElems(box_width, 0) { b_();b_();bb();bb();b_();br();      }
                spreadElems(box_width, 0) { bb();bb();__();__();bb();bb();br(); }
            }
    }
    if (p == 2) {
        translate([6*box_width, -box_height, 0])
            spreadElems(0, -box_height) {
                spreadElems(box_width, 0) { bb();b_();b_();b_();b_();b_();b_(); }
                spreadElems(box_width, 0) { __();b_();b_();b_();b_();b_();b_(); }
                spreadElems(box_width, 0) { __();b_();b_();b_();b_();b_();b_(); }
                spreadElems(box_width, 0) { bt();b_();b_();b_();b_();b_();b_(); }
                spreadElems(box_width, 0) { bb();bb();bb();b_();b_();bb();bb(); }
                spreadElems(box_width, 0) { __();__();__();bb();bb();           }
            }
    }
    if (p == 3) {
        translate([0, 6*-box_height, 0])
            spreadElems(0, -box_height) {
                spreadElems(box_width, 0) { __();__();bl();br();                }
                spreadElems(box_width, 0) { b_();b_();b_();b_();b_();br();      }
                spreadElems(box_width, 0) { b_();b_();b_();b_();b_();br();      }
                spreadElems(box_width, 0) { b_();b_();b_();b_();b_();b_();br(); }
                spreadElems(box_width, 0) { b_();b_();b_();b_();b_();b_();br(); }
                spreadElems(box_width, 0) { b_();b_();b_();b_();b_();br();      }
                }
    }
    if (p == 4) {
        translate([6*box_width, 6*-box_height, 0])
            spreadElems(0, -box_height) {
                spreadElems(box_width, 0) { __();b_();br();__();__();bl();b_(); }
                spreadElems(box_width, 0) { bt();b_();b_();b_();b_();b_();b_(); }
                spreadElems(box_width, 0) { bb();b_();b_();b_();b_();b_();b_(); }
                spreadElems(box_width, 0) { __();b_();b_();b_();b_();b_();b_(); }
                spreadElems(box_width, 0) { __();b_();b_();b_();b_();b_();b_(); }
                spreadElems(box_width, 0) { bt();b_();b_();b_();b_();b_();b_(); }
                }
    }
}
//*/

/*
// ===== OR, if your printer is still not large enough, you can also split it into 5 parts =====
translate([0, -box_height, 0])
spreadElems(0, -box_height) {
    spreadElems(box_width, 0) { b_();b_();b_();b_();b_();bb();bb();bl();b_();b_();b_();b_();b_(); }
    spreadElems(box_width, 0) { b_();b_();b_();b_();bb();bl();br();bb();b_();b_();b_();b_();b_(); }
    spreadElems(box_width, 0) { b_();b_();b_();bb();bl();b_();b_();br();bb();b_();b_();b_();b_(); }
    spreadElems(box_width, 0) { b_();b_();bb();bl();b_();b_();b_();b_();br();bb();b_();b_();b_(); }
    spreadElems(box_width, 0) { bb();bb();bl();b_();b_();b_();b_();b_();b_();br();bb();bb();b_(); }
    spreadElems(box_width, 0) { b_();bl();b_();b_();b_();b_();b_();b_();b_();b_();b_();br();b_(); }
    spreadElems(box_width, 0) { b_();bt();bt();bl();b_();b_();b_();b_();b_();b_();br();bt();bt(); }
    spreadElems(box_width, 0) { b_();b_();b_();bt();bl();b_();b_();b_();b_();br();bt();b_();b_(); }
    spreadElems(box_width, 0) { b_();b_();b_();b_();bt();bl();b_();b_();br();bt();b_();b_();b_(); }
    spreadElems(box_width, 0) { b_();b_();b_();b_();b_();bt();bl();br();bt();b_();b_();b_();b_(); }
    spreadElems(box_width, 0) { b_();b_();b_();b_();b_();br();bt();bt();b_();b_();b_();b_();b_(); }
}
//*/
