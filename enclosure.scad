$fn = 64;

box_length = 80;
box_width = 100;
box_height = 30;
box_thickness = 1;
delta = 0.001;

lid_thickness_center = 2;
lid_thickness_edge = 1;
lid_clearance = 0.2;
lid_inner_plug_depth = lid_thickness_center - lid_thickness_edge;
lid_post_clearance = 0.2;

box_screw_post_diameter = 7;
box_screw_post_hole_diameter = 2;
box_screw_post_hole_depth = 5;
box_screw_post_inset = 5;
box_screw_post_height = box_height - lid_inner_plug_depth - lid_post_clearance;
lid_screw_hole_diameter = 2.4;

module rect_box(length, width, height, thickness) {
    difference() {
        cube([width, length, height]);

        translate([thickness, thickness, thickness])
            cube([
                width - thickness * 2,
                length - thickness * 2,
                height + thickness + delta
            ]);
    }
}

module cyl_post(diameter, height, hole_diameter, hole_depth, fillet_radius = 1.5) {
    difference() {
        union() {
            // main vertical post
            cylinder(d = diameter, h = height);

            // simple base fillet / flare
            hull() {
                cylinder(d = diameter + 2 * fillet_radius, h = delta);

                translate([0, 0, fillet_radius])
                    cylinder(d = diameter, h = delta);
            }
        }

        // blind screw hole
        translate([0, 0, height - hole_depth + delta])
            cylinder(d = hole_diameter, h = hole_depth + delta);
    }
}

module screw_post_positions(length, width, wall_thickness, inset) {
    x0 = wall_thickness + inset;
    x1 = width - wall_thickness - inset;
    y0 = wall_thickness + inset;
    y1 = length - wall_thickness - inset;

    translate([x0, y0, 0])
        children();

    translate([x1, y0, 0])
        children();

    translate([x0, y1, 0])
        children();

    translate([x1, y1, 0])
        children();
}

module box_screw_posts() {
    screw_post_positions(
        box_length,
        box_width,
        box_thickness,
        box_screw_post_inset
    ) {
        cyl_post(
            box_screw_post_diameter,
            box_screw_post_height,
            box_screw_post_hole_diameter,
            box_screw_post_hole_depth,
            fillet_radius = 1.5
        );
    }
}

nano_post_corner_offset = 1;
nano_width = 18;
nano_length = 44;
nano_board_thickness = 1.6;
nano_corner_radius = 3;
nano_post_hole_depth = 4;
nano_post_hole_diameter = 2;
nano_post_diameter = 7;
nano_post_height = 10 + box_thickness;
nano_post_fillet_radius = 1.5;

module nano_post() {
    difference() {
        cyl_post(
            nano_post_diameter,
            nano_post_height,
            nano_post_hole_diameter,
            nano_post_hole_depth,
            nano_post_fillet_radius
        );

        // pocket out circular recess for corner of board
        translate([
            nano_post_corner_offset,
            nano_post_corner_offset,
            nano_post_height - 1
        ])
            cube([4, 4, nano_board_thickness + delta]);
    }
}

module nano_supports() {
    // make center of board the reference point, board oriented long axis || y
    translate([
        -(nano_width + 2 * nano_post_corner_offset) / 2,
        -(nano_length + 2 * nano_post_corner_offset) / 2,
        0
    ]) {
        nano_post();

        translate([0, nano_length + 2 * nano_post_corner_offset, 0])
            rotate([0, 0, -90])
                nano_post();

        translate([nano_width + 2 * nano_post_corner_offset, 0, 0])
            rotate([0, 0, 90])
                nano_post();

        translate([
            nano_width + 2 * nano_post_corner_offset,
            nano_length + 2 * nano_post_corner_offset,
            0
        ])
            rotate([0, 0, 180])
                nano_post();
    }
}

usb_hole_width = 15;
usb_hole_height = 10;

module enclosure() {
    difference() {
        union() {
            rect_box(
                box_length,
                box_width,
                box_height,
                box_thickness
            );

            box_screw_posts();

            translate([box_width / 3, box_length / 2, 0])
                nano_supports();
        }

        // USB hole
        translate([
            box_width / 3 - usb_hole_width / 2,
            box_length + delta,
            nano_post_height + 1 - usb_hole_height / 2
        ])
            rotate([90, 0, 0])
                cube([usb_hole_width, usb_hole_height, 10]);
    }
}

module lid_screw_holes(length, width, wall_thickness, inset, hole_diameter, height) {
    screw_post_positions(length, width, wall_thickness, inset) {
        translate([0, 0, -delta])
            cylinder(d = hole_diameter, h = height + 2 * delta);
    }
}

module enclosure_lid(
    length,
    width,
    wall_thickness,
    center_thickness = 2,
    edge_thickness = 1,
    clearance = 0.2
) {
    plug_height = center_thickness - edge_thickness;

    difference() {
        union() {
            // lower inner plug, fits inside the box
            translate([
                wall_thickness + clearance,
                wall_thickness + clearance,
                0
            ])
                cube([
                    width - 2 * (wall_thickness + clearance),
                    length - 2 * (wall_thickness + clearance),
                    plug_height
                ]);

            // upper flange / rim, same outer dimensions as enclosure
            translate([0, 0, plug_height])
                cube([width, length, edge_thickness]);
        }

        // matching lid screw holes
        lid_screw_holes(
            length,
            width,
            wall_thickness,
            box_screw_post_inset,
            lid_screw_hole_diameter,
            center_thickness
        );
    }
}

// Main enclosure
enclosure();

// Separate lid, placed beside enclosure for printing/modeling
translate([box_width + 20, 0, 0])
    enclosure_lid(
        box_length,
        box_width,
        box_thickness,
        lid_thickness_center,
        lid_thickness_edge,
        lid_clearance
    );

// Diagnostic
echo("USB hole center Z = ", nano_post_height + 1);
echo("Box screw post height = ", box_screw_post_height);
