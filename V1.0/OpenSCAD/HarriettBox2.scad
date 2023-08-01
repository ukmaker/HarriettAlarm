/**
* Improved box for the Harriet Alarm Clock
**/
use <ukmaker_openscad_lib/Generics.scad>
use <ukmaker_openscad_lib/Standoffs.scad>
use <ukmaker_openscad_lib/Batteries.scad>
use <ukmaker_openscad_lib/Waveshare.scad>
use <ukmaker_openscad_lib/Boxes.scad>
use <ukmaker_openscad_lib/Speakers.scad>
use <ukmaker_openscad_lib/GreenProtoBoards.scad>
use <ukmaker_openscad_lib/DevBoards.scad>

include <NopSCADlib/utils/core/core.scad>
include <NopSCADlib/vitamins/buttons.scad>
include <NopSCADlib/vitamins/button.scad>

epaper_to_main_board_spacing_z = 5;

width = waveshare_2_9_epaper_pcb_width() + 25;
height = 71;
depth_top = 40;
depth_bottom = 50;
thickness = 1.6;
rounding = 2;
box_lid_lip_height = 1.5;

button_grid_spacing = 2.54 * 7;
display_left = 5.5;
display_bottom = 25;


module connector() {
    h = 18;
    s = 2.7;
    color("Black")
    cube([s,s,h]);
}

module display() 
{
    // flip over
    translate([display_left,waveshare_2_9_epaper_height(),0])
    rotate([180,0,0])
    waveshare_2_9_epaper_module();
}

module display_cutout() {
    // flip over
    translate([display_left,waveshare_2_9_epaper_height(),0])
    rotate([180,0,0])
    waveshare_2_9_epaper_cutout();
}

module main_board() 
{
    proto_board_70_x_50();
    translate([15,1,2.6]) we_act_black_pill();
    // for fitting purposes
    translate([10,40,2.6])
    connector();
}
module board_stack()
{
    dx = (waveshare_2_9_epaper_width() - 70)/2 + display_left;
    dy = (waveshare_2_9_epaper_height() - 50) / 2;
    dz = epaper_to_main_board_spacing_z;
    display();
    
    translate([dx,dy,dz])  
    main_board();
}

module epaper_mounting_clips(margin=0) 
{
    pcb_mounting_clip(38.8+margin, 1.6, 5.1+margin, 1.6, 4);

    translate([waveshare_2_9_epaper_pcb_width()-4.8,0,0])
    pcb_mounting_clip(38.8+margin, 1.6, 5.1+margin, 1.6, 4);
}

module place_epaper_mounting_clips(margin=0) 
{
    translate([display_left-.1,-2.1,-3.5])
    epaper_mounting_clips(margin);
}

module main_board_mounting_clips(margin=0) 
{
    translate([display_left,-0.1,0])
    {
        translate([10,-7,0]) cube([50,2,11]);
        rotate([0,0,-90])pcb_mounting_clip(70+margin, 1.6, 7+margin, 8.3, 4);
    }
    
    translate([display_left,50-2.8,0])
    {
        translate([10,-2,0]) cube([50,2,11]);
        rotate([0,0,-90])pcb_mounting_clip(70+margin, 1.6, 7+margin, 8.3, 4);
    }
}

module place_main_board_mounting_clips(margin=0)
{
    translate([7.75,-0.8,-3.5])
    main_board_mounting_clips(margin);
}

module sized_mounting_pillar() {
    rotate([-90,0,0])mounting_pillar(5,10,2.3);
}

module panel_mounting_pillars() {
        pillar_side = 5;
        pillar_height = 10;
        
        box_inner_height =  height - 2 * thickness;
        // mounting pillars at the corners
        x0 = thickness - 0.2 + pillar_side/2;
        z0 = thickness - 0.2 + pillar_side/2;
        x1 = width - thickness + 0.2 - pillar_side/2;
        dy = depth_bottom - thickness + 0.2 - pillar_height -box_lid_lip_height;
        z1 = thickness + box_inner_height  + 0.2 - pillar_side/2;
        translate([x0,dy,z0]) sized_mounting_pillar();
        translate([x0,dy,z1]) sized_mounting_pillar();
        translate([x1,dy,z1]) sized_mounting_pillar();
        translate([x1,dy,z0]) sized_mounting_pillar();
}

module box_body() {
    panel_mounting_pillars();
    sloping_front_console(width, height, depth_top, depth_bottom, thickness, rounding);
}

function theta() = asin(height/sqrt(height*height + (depth_bottom - depth_top) * (depth_bottom - depth_top)));

module place_on_front() {
    translate([ 9,12,50])
    rotate([-180+theta(),0,0])
    children();
}

module button_strip() {
    // buttons centred in the box
    button_left = (width - (3 * button_grid_spacing)) / 2;
    
    s = 1.05;
    translate([0,0,0])
    scale([s,s,1])
    square_button(button_12mm, "yellow");

    translate([button_grid_spacing,0,0])
    scale([s,s,1])
    square_button(button_12mm, "yellow");

    translate([2 * button_grid_spacing,0,0])
    scale([s,s,1])
    square_button(button_12mm, "yellow");

    translate([3 * button_grid_spacing,0,0])
    scale([s,s,1])
    square_button(button_12mm, "yellow");
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

module wiring_comb() {
    w = 45;
    l = 4;
    d = 3;
    color("White")
    cube([w,l,d]);
}

module button_assembly() {

    translate([(width - (3 * button_grid_spacing)) / 2 +2,8.5,4])
    {
        rotate([180,0,0])
        proto_board_mounting_strip();
        translate([0,-17,0])
        rotate([180,0,180])
        proto_board_mounting_strip();
    }
    translate([2.5,0,0])
    button_strip();
    translate([-11,-10,-1.5])
    {
        proto_board_80_x_20();
        translate([0,8,-3])
        wiring_comb();
    }
}



//board_stack();
$fn=100;
//place_epaper_mounting_clips();
//place_main_board_mounting_clips();


module button_placement_marks()
{
    translate([20,30,height])
    color("Red") cube([2,10,2]);
    translate([width - 22,30,height])
    color("Red") cube([2,10,2]);
}

difference()
{
    box_body();
    translate([display_left+24.3,30,height-9.5])
    button_assembly();
    translate([0,-0.4,1])
    place_on_front() 
    {
        display_cutout();
        place_main_board_mounting_clips(0.1);
        place_epaper_mounting_clips(0.1);
    }
}
/*
place_on_front() board_stack();

translate([3, depth_bottom-6,50])
rotate([0,90,0])
aaa_x4_flat_battery_holder();
// speaker
translate([85,depth_bottom,30])
rotate([90,0,0])
color("Silver")
speaker_round_50mm();
*/