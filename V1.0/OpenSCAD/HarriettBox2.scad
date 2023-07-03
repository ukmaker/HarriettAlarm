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

epaper_to_main_board_spacing_z = 5;

width = waveshare_2_9_epaper_pcb_width() + 12;
height = 62;
depth_top = 31;
depth_bottom = 41;
thickness = 1.6;
rounding = 2;

module display() 
{
    // flip over
    translate([0,waveshare_2_9_epaper_height(),0])
    rotate([180,0,0])
    waveshare_2_9_epaper_module();
}

module main_board() 
{
    proto_board_70_x_50();
    translate([15,1,2.6]) we_act_black_pill();
}
module board_stack()
{
    dx = (waveshare_2_9_epaper_width() - 70)/2;
    dy = (waveshare_2_9_epaper_height() - 50) / 2;
    dz = epaper_to_main_board_spacing_z;
    display();
    
    translate([dx,dy,dz])  
    main_board();
}

module epaper_mounting_clips() 
{
    pcb_mounting_clip(38.8, 1.6, 5.1, 1.6, 4);

    translate([waveshare_2_9_epaper_pcb_width()-4.8,0,0])
    pcb_mounting_clip(38.8, 1.6, 5.1, 1.6, 4);
}

module place_epaper_mounting_clips() 
{
    translate([-.1,-2.1,-3.5])
    epaper_mounting_clips();
}

module main_board_mounting_clips() 
{
    translate([0,-0.1,0])
    {
        translate([10,-7,0]) cube([50,2,11]);
        rotate([0,0,-90])pcb_mounting_clip(70, 1.6, 7, 8.3, 4);
    }
    
    translate([0,50-2.8,0])
    {
        translate([10,-2,0]) cube([50,2,11]);
        rotate([0,0,-90])pcb_mounting_clip(70, 1.6, 7, 8.3, 4);
    }
}

module place_main_board_mounting_clips()
{
    translate([7.75,-0.8,-3.5])
    main_board_mounting_clips();
}

module box_body() {

    sloping_front_console(width, height, depth_top, depth_bottom, thickness, rounding);
}

function theta() = asin(height/sqrt(height*height + (depth_bottom - depth_top) * (depth_bottom - depth_top)));

module box_front_opening() {
    rotate([theta(),0,0])
    translate([9,13,-6])
    waveshare_2_9_epaper_cutout();
}

module place_on_front() {
    translate([waveshare_2_9_epaper_pcb_width() + 3,6.8,12])
    rotate([-180+theta(),0,0])
    rotate([0,0,180]) children();
}


//board_stack();

//place_epaper_mounting_clips();
//place_main_board_mounting_clips();
//difference()
{
    box_body();
    box_front_opening();

    place_on_front() 
    {
        place_main_board_mounting_clips();
        place_epaper_mounting_clips();
    }
}

place_on_front() board_stack();

translate([25, depth_bottom-6,46])
rotate([0,90,0])
aaa_x4_flat_battery_holder();