

OUTPUT_KEYS = 0;
OUTPUT_BED = 1;
OUTPUT_BRACE = 0;
OUTPUT_TOOL = 0;

key_spacing = 3.2;
base_width = 164;

black_key_width = 14;
white_key_length = 216;
black_key_length = white_key_length - 2 * 25.4;
white_key_height = 0.6 * 25.4;
black_key_height = white_key_height + 0.4 * 25.4;
bed_height = 6;
bed_length = white_key_length;
joint_offset = 5.5 * 25.4;
joint_radius = 1.5;
mount_width = 0.75 * 25.4;
back_height = black_key_height + 0.25 * 25.4;
screw_head_radius = 2.8;
screw_head_height = 3;
screw_hole_radius = 1.4;
pcb_length = 40;
pcb_thickness = 1.6;
pcb_mount_height = 3;
pcb_standoff_height = pcb_mount_height - pcb_thickness;

white_key_widths = [23, 24, 23, 24, 23, 23, 24];
black_key_offsets = [14, 42, 83, 110, 137];

back_widths = [14, 14, 14, 14, 14, 13, 14, 13, 14, 13, 14, 13];

function white_key_offset(i) = i == 0 ? 0 : white_key_offset(i - 1) + white_key_widths[i - 1];

function back_offset(i) = i == 0 ? 0 : back_offset(i - 1) + back_widths[i - 1];

module key_mount(j) {
    x = back_offset(j) + 3;
    w = back_widths[j] - 6;
    translate([x, joint_offset, -mount_width]) {
        difference() {
            union() {
                rotate([45, 0, 0]) {
                    cube([w, mount_width, mount_width]);
                }
                translate([0, 0, mount_width + 5.5]) rotate([0, 90, 0]) cylinder(w, 2.1, 2.1, $fn=64);
            }
            translate([0, 2.2, mount_width]) cube([w, mount_width, mount_width]);
        }
    }
}

module mag_mount(i) {
    x = back_offset(i);
    w = back_widths[i];
    radius = 4.1 / 2;
    translate([x + w / 2, white_key_length - radius * 2, -1]) {
        cylinder(white_key_height + 2, radius, radius, $fn = 32);
    }
}

module stabilizer(i) {
    x = back_offset(i);
    w = back_widths[i];
    translate([x + w / 2 - black_key_width / 6, white_key_length - 60, -1]) {
        cube([black_key_width / 3, 30, black_key_height + 2]);
    }
    
    // Cutout for rubber band
    translate([x + w / 2 - black_key_width / 4, white_key_length - 1.25 * 25.4 - 0.1, -1]) {
        cube([black_key_width / 2, 17, black_key_height + 2]);
    }
}

module band_holder(j) {
    x = back_offset(j);
    w = back_widths[j];
    y = white_key_length;
    translate([x + w / 2, y - 0.9 * 25.4 + 9, white_key_height - 4 + 3]) {
        rotate([-45, 0, 0]) cylinder(8, 1.8, 1.8, $fn=32);
    }
}

module white_key_cutout(i) {
    if(i >= 0) {
        x1 = black_key_offsets[i];
        x2 = x1 + black_key_width;
        translate([x1 - 0.01, 2 * 25.4 - key_spacing, -0.1]) {
            cube([x2 - x1 + 0.02, black_key_length * 2, black_key_height]); 
        }
    }
}

module white_key(i, j, cutout_left, cutout_right) {
    x = white_key_offset(i) + key_spacing / 2;
    w = white_key_widths[i] - key_spacing;
    difference() {
        union() {
            translate([x, 0, 0]) {
                cube([w, white_key_length, white_key_height]);
            }
            band_holder(j);
        }
        white_key_cutout(cutout_left);
        white_key_cutout(cutout_right);
        key_mount(j);
        mag_mount(j);
        stabilizer(j);
    }
}

module black_key(i, j) {
    difference() {
        union() {
            translate([black_key_offsets[i] + key_spacing / 2, 2 * 25.4, 0]) {
                difference() {
                    cube([black_key_width - key_spacing, black_key_length, black_key_height]);
                    translate([-0.1, 4 * 25.4, white_key_height]) {
                        cube([black_key_width + 0.2, black_key_length, black_key_height]);
                    }
                } 
            }
            band_holder(j);
        }
        key_mount(j);
        mag_mount(j);
        stabilizer(j);
    }
}

module white_keys() {
    difference() {
        union() {
            white_key(0, 0, -1, 0);
            white_key(1, 2, 0, 1);
            white_key(2, 4, 1, -1);
            white_key(3, 5, -1, 2);
            white_key(4, 7, 2, 3);
            white_key(5, 9, 3, 4);
            white_key(6, 11, 4, -1);
        }
    }
}

module black_keys() {
    black_key(0, 1);
    black_key(1, 3);
    black_key(2, 6);
    black_key(3, 8);
    black_key(4, 10);
}

module keys() {
    white_keys();
    black_keys();
}

module key_stand(j) {
    x = back_offset(j);
    w = back_widths[j];
    key_offset = 0.65 * 25.4;
    key_elevation = 0.4 * 25.4;
    translate([x + 3.2, joint_offset, -2]) {
        translate([0, 0, key_offset + 2]) {
            rotate([0, 90, 0]) cylinder(w - 6.4, 2, 2, $fn=32);
        }
        translate([0, -2, 2]) cube([w - 6.4, 2 * 2, key_offset]);
    }
    translate([x, joint_offset - 2, 0]) cube([w, 8, key_elevation]);
    
    // Stabilizer.
    translate([x, white_key_length - 54, 0]) {
        translate([w / 2 - black_key_width / 6 + 0.25, 0, 0]) {
            cube([black_key_width / 3 - 0.5, 16, back_height]);
        }
        cube([w, 16, key_elevation]);
        translate([w / 2 - black_key_width / 6 + 0.25, 0, back_height - 3]) {
            rotate([0, 90, 0]) cylinder(black_key_width / 3 - 0.5, 6, 6, $fn=3);
        }
        translate([w / 2 - black_key_width / 6 + 0.25, 16, back_height - 20]) {
            rotate([0, 90, 0]) cylinder(black_key_width / 3 - 0.5, 6, 6, $fn=3);
        }
        translate([w / 2 - black_key_width / 6 + 0.25, 0, 14.7]) {
            cube([black_key_width / 3 - 0.5, 21.15, back_height - 14.7]);
        }
    }

    // Rubber band holder
    translate([x + w / 2, white_key_length - 1.25 * 25.4 + 3, -1.1]) {
        rotate([45, 0, 0]) cylinder(9, 1.8, 1.8, $fn=32);
    }
}

module pcb_clip(x, y) {
    base_radius = 4;
    standoff_radius = screw_head_radius * 1.25;
    translate([x, y, pcb_standoff_height]) {
        difference() {
            union() {
                translate([0, 0, -pcb_standoff_height]) {
                    cylinder(pcb_standoff_height, standoff_radius, standoff_radius, $fn=64);
                    translate([0, 0, pcb_standoff_height]) cylinder(d=screw_hole_radius * 2, h=pcb_thickness + 1, $fn=64);
                }
                translate([0, 0, pcb_thickness]) {
                    intersection() {
                        union() {
                            translate([0, 0, 0.5]) {
                                cylinder(d1 = screw_hole_radius * 2.5, d2 = screw_hole_radius * 1.5, h = 2, $fn=64);
                            }
                            cylinder(d1 = screw_hole_radius * 2, d2 = screw_hole_radius * 2.5, h = 0.5, $fn=64);
                        }
                        cube([screw_hole_radius * 2, base_radius * 2, base_radius * 2], center=true);
                    }
                }
            }
            translate([0, 0, (pcb_thickness + 20) / 2])cube([20, 0.4, 20], center=true);
        }
    }
}

pcb_offset = 70;
module pcb_mount() {
    translate([0, 0, -pcb_standoff_height - pcb_thickness]) {
        pcb_clip(3.5 + 1, pcb_offset + 3.5);
        pcb_clip(base_width - 3.5 - 1, pcb_offset + 3.5);
        pcb_clip(base_width / 2, pcb_offset + pcb_length - 3.5);
        translate([base_width / 2 - 6, pcb_offset - 1]) cube([12, 6, pcb_standoff_height]);
        translate([0, pcb_offset + pcb_length - 3]) cube([base_width, 4, pcb_standoff_height]);
    }
}

module screw_hole(x, y) {
    translate([x, y, -bed_height - 1]) {
        cylinder(bed_height + 2, 2.1, 2.1, $fn=32);
        translate([0, 0, screw_head_height]) cylinder(bed_height, 4, 4, $fn=32);
    }
}

middle_hole_offset = 6 * 25.4;
mounting_hole_positions = [
    [base_width - 5, 5],
    [5, 5],
    [base_width - 5, middle_hole_offset],
    [5, middle_hole_offset]
];

bed_zoffset = -11;
module bed() {
    translate([0, 0, bed_zoffset]) {
        difference() {
            union() {
                translate([0, 0, -bed_height]) cube([base_width, bed_length, bed_height]);
                for(j = [0:11]) key_stand(j);
            }
            
            // Cutouts to use less filament
            translate([12, 0.5 * 25.4, -bed_height - 5]) {
                cube([base_width - 12 * 2, 1.25 * 25.4, bed_height + 10]);
            }
            
            translate([12, 0.5 * 25.4 + 2 * 25.4, -bed_height - 5]) {
                cube([(base_width - 12 * 3) / 2, 1.75 * 25.4, bed_height + 10]);
            }
            translate([base_width / 2 + 6, 0.5 * 25.4 + 2 * 25.4, -bed_height - 5]) {
                cube([(base_width - 12 * 3) / 2, 1.75 * 25.4, bed_height + 10]);
            }
            
            translate([12, pcb_offset + pcb_length + 5, -bed_height - 5]) {
                cube([base_width - 12 * 2, 18, bed_height + 10]);
            }
            
            translate([12, white_key_length - 25.4, -bed_height - 5]) {
                cube([base_width - 12 * 2, 25.4 * 0.5, bed_height + 10]);
            }
            
            // Mounting holes.
            for(pos = mounting_hole_positions) {
                screw_hole(pos[0], pos[1]);
            }
            
            // Cutout for the PCB
            translate([-0.1, pcb_offset - 0.2, -pcb_mount_height]) {
                cube([base_width + 0.2, pcb_length + 0.4, pcb_standoff_height + 2]); 
            }
            
        }
        
        pcb_mount();
        
        // Key stop
        translate([0, bed_length - 10, -1]) {
            cube([base_width, 10, 0.4 * 25.4 + 1]);
        }
        
        // Stop for black keys.
        translate([0, 1.75 * 25.4, 0]) {
            cube([base_width, 0.75 * 25.4, 1.5]);
        }
    }
}

module clip(hole_radius) {
    head_height = 3.5;
    translate([0, 0, bed_height - head_height]) difference() {
        union() {
            intersection() {
                union() {
                    translate([0, 0, 0.5]) {
                        cylinder(d1 = hole_radius * 2.5, d2 = hole_radius * 1.5, h = head_height, $fn=64);
                    }
                    cylinder(d1 = hole_radius * 2, d2 = hole_radius * 2.5, h = 0.5, $fn=64);
                }
                cube([hole_radius * 2, hole_radius * 2, head_height * 2], center=true);
            }
        }
       cube([head_height * 2, hole_radius * 0.5, head_height * 3], center=true);
    }
    cylinder(bed_height - head_height, hole_radius, hole_radius, $fn=64);
}

module brace() {
    brace_width = 25;
    brace_height = 4;
    clip_radius = 1.8;
    translate([-brace_width / 2, 0, bed_zoffset - bed_height - brace_height - 0.2]) {
        cube([brace_width, white_key_length, brace_height]);
        translate([brace_width - 7.5, 5, brace_height]) clip(clip_radius);
        translate([7.5, 5, brace_height]) clip(clip_radius);
        translate([brace_width - 7.5, middle_hole_offset, brace_height]) clip(clip_radius);
        translate([7.5, middle_hole_offset, brace_height]) clip(clip_radius);
    }
}

module tool() {
    union() {
        rotate([0, 90, 0]) rotate([0, 0, 30]) cylinder(50, 2.1, 2.1, $fn=6);
        translate([1.5, 0, 0]) rotate([0, 90, 45]) rotate([0, 0, 30]) cylinder(8, 2.0, 2.0, $fn=6);
    }
}

if(OUTPUT_TOOL) {
    translate([-20, -20, -10]) tool();
}
rotate([0, 0, 90]) {
    if(OUTPUT_KEYS) keys();
    if(OUTPUT_BED) bed();
    if(OUTPUT_BRACE) brace();
}
