/**
* Improved box for the Harriet Alarm Clock
**/
use <ukmaker_openscad_lib/Generics.scad>
use <ukmaker_openscad_lib/Standoffs.scad>
use <ukmaker_openscad_lib/Batteries.scad>
use <ukmaker_openscad_lib/Waveshare.scad>
use <ukmaker_openscad_lib/Boxes.scad>
use <ukmaker_openscad_lib/Panels.scad>
use <ukmaker_openscad_lib/Screws.scad>
use <ukmaker_openscad_lib/Speakers.scad>
use <ukmaker_openscad_lib/GreenProtoBoards.scad>
use <ukmaker_openscad_lib/DevBoards.scad>

include <NopSCADlib/utils/core/core.scad>
include <NopSCADlib/vitamins/buttons.scad>
include <NopSCADlib/vitamins/button.scad>

$fn = 200;

epaper_to_main_board_spacing_z = 5;

width = waveshare_2_9_epaper_pcb_width() + 25;
height = 71;
depth_top = 50;
depth_bottom = 60;
thickness = 1.6;
rounding = 2;
box_lid_lip_height = 1.5;

button_grid_spacing = 2.54 * 7;
display_left = 5.5;
display_bottom = 25;

box_inner_width =  width - 2 * thickness;
box_inner_height =  height - 2 * thickness;

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

module epaper_mounting_clip(margin=0) 
{
    pcb_mounting_clip(38.8+margin, 1.6, 5.1+margin, 1.6, 4);
}

module epaper_mounting_clips(margin=0) 
{
    epaper_mounting_clip(margin);

    translate([waveshare_2_9_epaper_pcb_width()-4.8,0,0])
    epaper_mounting_clip(margin);
}

module place_epaper_mounting_clips(margin=0) 
{
    translate([display_left-.1,-2.1,-3.5])
    epaper_mounting_clips(margin);
}

module main_board_mounting_clip(margin=0,left=true) 
{
    if(left) {
        translate([10,-2,0]) cube([50,2,11]);
    } else {
        translate([10,-7,0]) cube([50,2,11]);
    }
    rotate([0,0,-90])pcb_mounting_clip(70+margin, 1.6, 7+margin, 8.3, 4);
}

module main_board_mounting_clips(margin=0) 
{
    translate([display_left,-0.1,0])
    {
        main_board_mounting_clip(margin, false);
    }
    
    translate([display_left,50-2.8,0])
    {
        main_board_mounting_clip(margin);
    }
}

module place_main_board_mounting_clips(margin=0)
{
    translate([7.75,-0.8,-3.5])
    main_board_mounting_clips(margin);
}

module sized_mounting_pillar() {
    rotate([-90,0,0])
    blended_mounting_pillar(5,10,2.3,10);
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
        translate([x0,dy,z0]) rotate([0,-90,0]) sized_mounting_pillar();
        translate([x0,dy,z1]) sized_mounting_pillar();
        translate([x1,dy,z1]) rotate([0,90,0]) sized_mounting_pillar();
        translate([x1,dy,z0]) rotate([0,180,0]) sized_mounting_pillar();
}

module mounting_hole_piercing() {
    translate([0,0,-8.99])
    countersunk_screw_M2(10);
}

module panel_mounting_holes_piercings() {
        pillar_side = 5;
        pillar_height = 10;
        
        // mounting pillars at the corners
        delta = 0.1;
        x0 = - delta + pillar_side/2;
        y0 = - delta + pillar_side/2;
        x1 = box_inner_width  - x0;
        y1 = box_inner_height - y0;
        dz = 1.2;
        translate([x0,y0,dz]) mounting_hole_piercing();
        translate([x0,y1,dz]) mounting_hole_piercing();
        translate([x1,y1,dz]) mounting_hole_piercing();
        translate([x1,y0,dz]) mounting_hole_piercing();
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
module proto_board_mounting_strip(for_difference = false) {

    module button_mounting_strip() {
        spacing_x = 73.9 + 2;
        spacing_y = 18.2 - 2;
        dy = 1.8;
        term_c = 5.5;
        term_s = 2.5;

        difference() 
        {
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
    
    if(for_difference) {
        scale([1.007, 1.007, 1.005])
        button_mounting_strip();
    } else {
        button_mounting_strip();
    }
}

module wiring_comb() {
    w = 45;
    l = 4;
    d = 3;
    color("White")
    cube([w,l,d]);
}

module button_assembly(for_difference=false) {

    translate([(width - (3 * button_grid_spacing)) / 2 - 1.7,8.5,4])
    {
        rotate([180,0,0])
        proto_board_mounting_strip(for_difference);
        translate([0,-17,0])
        rotate([180,0,180])
        proto_board_mounting_strip(for_difference);
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

module back_panel(component)
{
    holder_w = 50;
    holder_d = 53;
    holder_h = 13;
    
   difference() 
    {
        panel_with_recess_and_screw_cover_components(
            component, 
            box_inner_width-0.1,box_inner_height-0.1,3,0.1,  
            holder_w,holder_d,holder_h,2,1.5,  
            1.5, 
            5,8, 
            5, 
            2);
        if(component == Panel_Component_Recess()) {
            // drill a hole for the battery wires
            translate([holder_w - 5, holder_d + 6, -holder_h-3])
            cylinder(h=4, d=4);
        }
        
        if(component == Panel_Component_Panel()) {        
            panel_mounting_holes_piercings();
            translate([85.5,35,0.2])
            color("Silver")
            rotate([180,00])
            {
                scale([1.005,1.005,1])
                speaker_round_50mm();
                translate([0,0,-4])
                grille_speaker_round_50mm(2,5);
            }
        }
    }

}

module button_placement_marks()
{
    translate([20,30,height])
    color("Red") cube([2,10,2]);
    translate([width - 22,30,height])
    color("Red") cube([2,10,2]);
}

module place_button_assembly(for_difference = false) {
    translate([display_left+24.3,30,height-9.2])
    button_assembly(for_difference);
}

module main_body() 
{
    difference()
    {
        box_body();
        place_button_assembly(true);
        translate([0,-0.4,1])
        place_on_front() 
        {
            display_cutout();
            place_main_board_mounting_clips(0.1);
            place_epaper_mounting_clips(0.1);
        }

    }
}

module back_panel_assembly() {
    back_panel(0);
    back_panel(1);
    back_panel(2);
}

module place_back_panel_assembly() {
    translate([1.5, depth_bottom-3, height-1.5])
    rotate([-90,0,0])
    back_panel_assembly();
}

module back_panel_exploded() {
    back_panel(0);
    translate([80,0,0]) back_panel(1);
    translate([200,0,0]) back_panel(2);
}

module place_board_stack() {
    place_on_front() 
    board_stack();
}

module internals_exploded() {
    translate([0,0,0])
    proto_board_mounting_strip();
    translate([0,10,0])
    proto_board_mounting_strip();
    translate([0,20,0])
    rotate([0,0,90])
    epaper_mounting_clip();
    translate([0,30,0])
    rotate([0,0,90])
    epaper_mounting_clip();
    translate([10,30,0])
    //scale([0.99,0.99,1])
    main_board_mounting_clip(0, false);
    translate([10,40,0])
    //scale([0.99,0.99,1])
    main_board_mounting_clip(0, true);
}

translate([100,0,0]) main_body();
internals_exploded();
//place_board_stack();
//place_button_assembly();
//place_back_panel_assembly();


