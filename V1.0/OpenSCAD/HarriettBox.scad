use <ukmaker_openscad_lib/Generics.scad>
use <ukmaker_openscad_lib/Standoffs.scad>
use <ukmaker_openscad_lib/Batteries.scad>
use <ukmaker_openscad_lib/Waveshare.scad>
use <ukmaker_openscad_lib/Boxes.scad>
use <ukmaker_openscad_lib/Speakers.scad>
use <ukmaker_openscad_lib/GreenProtoBoards.scad>

include <NopSCADlib/utils/core/core.scad>


include <NopSCADlib/vitamins/buttons.scad>
include <NopSCADlib/vitamins/button.scad>

if($preview) {
    $fn=10;
    } else {
    $fn = 100;
    }





box_w = 110;

// box needs too be tall enough to hold a 4-cell aa battery box
box_h = 64;
box_d1 = 60;

// depth to allow for batteries
box_d2 = 80;

box_thickness = 1.4;
radius = 4;
base = box_d2 - box_d1;
theta = atan(box_h/base);

button_grid_spacing = 2.54 * 7;
// eink_panel_height = 37
// eink_panel_width = 79
display_offset_y = (box_h - 37) / 2;
display_offset_x = (box_w - 79) / 2 - 2.4;
eink_panel_width = 79;

module box_front() {
    difference() 
    {

        sloping_front_console(box_w,box_h,box_d1,box_d2,box_thickness,radius);

        // buttons centred in the box
        button_left = (box_w - (3 * button_grid_spacing)) / 2;

        translate([button_left,10,box_h-10]){
            translate([0,30,0])
            square_button(button_12mm, "yellow");

            translate([button_grid_spacing,30,0])
            square_button(button_12mm, "yellow");

            translate([2 * button_grid_spacing,30,0])
            square_button(button_12mm, "yellow");

            translate([3 * button_grid_spacing,30,0])
            square_button(button_12mm, "yellow");

        }

    
        rotate([theta,0,0])
        translate([display_offset_x,display_offset_y,-4])
        waveshare_2_9_epaper_cutout();
    }

    //rotate([theta,0,0])
    //    translate([display_offset_x,display_offset_y,-(1+2)])
    //    waveshare_2_9_epaper_standoffs();



    // add screw mountings for m3 screws
    translate([0,box_d2-2.5,0])
    rotate([90,0,0])
    op_4_grid(box_w, box_h, 4,4,4,4) {
        difference() {
            translate([-3,-3,0])
            cube([6,6,20]);
            translate([0,0,-.5])
            cylinder(h=21,d=2.5);
        }
    }
}

/*
// batteries
translate([70,45,3])
rotate([0,0,180])
aa_4_flat_battery_holder();

// speaker
translate([3,30,30])
rotate([0,90,0])
color("Silver")
speaker();
*/

module back_panel() {
    // back panel
    b_w = box_w - 2*box_thickness - 0.15;
    b_h = box_h - 2*box_thickness - 0.15;
    
    bh_left = (box_w - aa_4_flat_battery_holder_width()) / 2;
    bh_top = (box_h - aa_4_flat_battery_holder_height()) / 2;
    
    
    cube([b_w, b_h, box_thickness]);
    
    // battery pack mounting points
    translate([bh_left,bh_top,box_thickness - 0.15])
    aa_4_flat_battery_holder_mounting_holes(depth = 1.6, dia = 4.5);
}


module back_panel_piercings() {
    // back panel
    b_w = box_w - 2*box_thickness - 0.15;
    b_h = box_h - 2*box_thickness - 0.15;
    b_o = 4 - box_thickness;

    delta = box_thickness + 0.15/2;
    // screw holes
    op_4_grid(b_w, b_h, b_o, b_o, b_o, b_o)
    translate([0,0,-0.1])
    cylinder(h=2 * box_thickness, d = 2.5);
}

module place_back_panel() {
    // back panel
    b_w = box_w - 2*box_thickness - 0.15;
    b_h = box_h - 2*box_thickness - 0.15;
    b_o = 4 - box_thickness;

    translate([delta,box_d2,0])
   // rotate([90,0,0])
    color("grey")
    difference() {
        back_panel();
        back_panel_piercings();
    }
}

// mounting strips for the button proto-board
// this is a green proto 2cmx8cm
module proto_board_mounting_strip() {

    spacing_x = 73.9 + 2;
    spacing_y = 18.2 - 2;
    dy = 1.8;
    term_c = 5.5;
    term_s = 2.5;

    difference() {
        cube([80,4,8.2], center=true);
        
        translate([-36.1,1.3,-1])
        cube([8, 1.5,8.3], center = true);
        translate([36.1,1.3,-1])
        cube([8, 1.5,8.3], center = true);
        
        translate([-41.35 + term_c, 0.55, 2])
        cube([term_s*1.5, term_s, term_s]);
        translate([37.6 - term_c, 0.55, 2])
        cube([term_s*1.5, term_s, term_s]);
        
        translate([ - 1.5 * button_grid_spacing, 7,-4.2]) {
            
            translate([0,0,0])
            cylinder(h=9, d=12.8);
            
            translate([button_grid_spacing,0,0])
            cylinder(h=9, d=12.8);
            
            translate([2* button_grid_spacing,0,0])
            cylinder(h=9, d=12.8);
            
            translate([3 * button_grid_spacing,0,0])
            cylinder(h=9, d=12.8);
        }
    }
    
    translate([-spacing_x/2,.6,4-0.1])
    cylinder(h=1.6,d=2);
    translate([spacing_x/2,.6,4-0.1])
    cylinder(h=1.6,d=2);
}




module probe() {
    translate([0,6,0])
    cube([4,4,1.6]);
}



place_back_panel();
//proto_board_mounting_strip();
//translate([0,8,0])
//proto_board_mounting_strip();

module front() {
    difference() {

        box_front();
        eink_pcb_width = 89.6;
        rotate([theta,0,0])
        translate([display_offset_x,display_offset_y,-1]) {
            translate([eink_pcb_width-4.8,0,0])
            
            translate([0,41,0])
            rotate([180,0,0])
            pcb_mounting_clip(38.8, 1.6, 5.1, 1.6, 4);
            
            translate([0,41,0])
            rotate([180,0,0])    
            pcb_mounting_clip(38.8, 1.6, 5.1, 1.6, 4);

        }
    }
}

//probe();
front();
// for the display
//pcb_mounting_clip(38.5, 1.6, 5, 1.6, 4);
// for the main board
board_w = 70;
board_d = 50;
board_t = 1.6;
board_c = 7;

//pcb_mounting_clip(board_d, board_t+0.2, 5, board_c, 4);
