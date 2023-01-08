
key_spacing = 4;
base_width = 6.5 * 25.4 / 7;
inner_width = base_width / 2;
white_key_width = base_width - key_spacing / 2;
black_key_width = base_width / 2 - key_spacing / 2;
white_key_length = (6 + 2.5) * 25.4;
black_key_length = white_key_length - 2 * 25.4;
white_key_height = 0.6 * 25.4;
black_key_height = white_key_height + 0.4 * 25.4;
bed_height = 6;
bed_length = white_key_length + 0.5 * 25.4;
hole_offset = 5.5 * 25.4;
hole_radius = 1.5;
mount_width = 0.75 * 25.4;
back_height = black_key_height + 0.25 * 25.4;
screw_head_radius = 2.8;
screw_head_height = 3;
pcb_standoff_height = 2;
pcb_length = 35;

module key_mount(w) {
    translate([w / 2 - black_key_width / 2 + 2, hole_offset, -mount_width]) {
        difference() {
            union() {
                rotate([45, 0, 0]) {
                    cube([black_key_width - 4, mount_width, mount_width]);
                }
                translate([0, 0, mount_width + 5.5]) rotate([0, 90, 0]) cylinder(black_key_width - 4, 2.1, 2.1, $fn=64);
            }
            translate([0, 2.2, mount_width]) cube([black_key_width, mount_width, mount_width]);
        }
    }
}

module mag_mount(w) {
    radius = 4.1 / 2;
    translate([w / 2, white_key_length - radius * 2, -1]) {
        cylinder(white_key_height + 2, radius, radius, $fn = 32);
    }
}

module stabilizer(w) {
    translate([w / 2 - black_key_width / 6 - 0.1, white_key_length - 2.25 * 25.4, -1]) {
        cube([black_key_width / 3 + 0.2, 25.4, black_key_height + 2]);
    }
    
    // Cutout for rubber band
    translate([w / 2 - black_key_width / 4, white_key_length - 1.25 * 25.4 - 0.1, -1]) {
        cube([black_key_width / 2, 17, black_key_height + 2]);
    }
}

module band_holder(y, w) {
    translate([w / 2, y - 0.9 * 25.4 + 9, white_key_height - 4 + 3]) {
        rotate([-45, 0, 0]) cylinder(8, 1.5, 1.5, $fn=32);
    }
}

module white_key_cutout(x) {
    translate([x, 2 * 25.4 - key_spacing, -0.1]) {
        cube([inner_width / 2, black_key_length * 2, black_key_height]); 
    }
}

module white_key(cutout_left, cutout_right) {
    difference() {
        union() {
            cube([white_key_width, white_key_length, white_key_height]);
            band_holder(white_key_length, white_key_width);
        }
        if(cutout_left) {
            white_key_cutout(-0.01);
        }
        if(cutout_right) {
            white_key_cutout(white_key_width - inner_width / 2 + 0.01);
        }
        key_mount(white_key_width);
        mag_mount(white_key_width);
        stabilizer(white_key_width);
    }
}

module black_key() {
    difference() {
        union() {
            difference() {
                cube([black_key_width, black_key_length, black_key_height]);
                translate([-0.1, 4 * 25.4, white_key_height]) {
                    cube([black_key_width + 0.2, black_key_length, black_key_height]);
                }
            }
            band_holder(black_key_length, black_key_width);
        }
        translate([0, -2 * 25.4, 0]) {
            key_mount(black_key_width);
            mag_mount(black_key_width);
            stabilizer(black_key_width);
        }
    }
}

module white_keys() {
    difference() {
        union() {
            translate([0 * base_width, 0, 0]) white_key(0, 1);
            translate([1 * base_width, 0, 0]) white_key(1, 1);
            translate([2 * base_width, 0, 0]) white_key(1, 0);
            translate([3 * base_width, 0, 0]) white_key(0, 1);
            translate([4 * base_width, 0, 0]) white_key(1, 1);
            translate([5 * base_width, 0, 0]) white_key(1, 1);
            translate([6 * base_width, 0, 0]) white_key(1, 0);
        }
    }
}

module black_keys() {
    difference() {
        union() {
            translate([0.5 * base_width + inner_width / 2, 2 * 25.4, 0]) {
                translate([0 * base_width, 0, 0]) black_key();
                translate([1 * base_width, 0, 0]) black_key();
                translate([3 * base_width, 0, 0]) black_key();
                translate([4 * base_width, 0, 0]) black_key();
                translate([5 * base_width, 0, 0]) black_key();
            }
        }
    }
}

module keys() {
    white_keys();
    black_keys();
}

module key_stand(x, w) {
    key_offset = 0.65 * 25.4;
    key_elevation = 0.4 * 25.4;
    translate([x + w / 2 - black_key_width / 2 + 2.2, hole_offset, -2]) {
        translate([0, 0, key_offset + 2]) {
            rotate([0, 90, 0]) cylinder(black_key_width - 4.4, 2, 2, $fn=32);
        }
        translate([0, -2, 2]) cube([black_key_width - 4.4, 2 * 2, key_offset]);
    }
    translate([x, hole_offset - 2, 0]) cube([white_key_width + key_spacing / 2, 8, key_elevation]);
    
    // Stabilizer.
    translate([x, white_key_length - 2.0 * 25.4, 0]) {
        translate([w / 2 - black_key_width / 6 + 0.25, 0, 0]) {
            cube([black_key_width / 3 - 0.5, 0.5 * 25.4, back_height]);
        }
        cube([white_key_width + key_spacing / 2, 0.5 * 25.4, key_elevation]);
        translate([w / 2 - 1.4, 0, back_height - 4]) {
            rotate([0, 90, 0]) cylinder(black_key_width / 3 - 0.5, 8, 8, $fn=3);
        }
    }

    // Rubber band holder
    translate([x + w / 2, white_key_length - 1.25 * 25.4 + 3, -1.1]) {
        rotate([45, 0, 0]) cylinder(8.5, 1.5, 1.5, $fn=32);
    }
}

module clip() {
    base_radius = 4;
    board_width = 1.6;
    hole_radius = 1.4;
    difference() {
        union() {
            cylinder(d=hole_radius * 2, h=board_width, $fn=64);
            translate([0, 0, board_width]) {
                intersection() {
                    union() {
                        translate([0, 0, 0.5]) {
                            cylinder(d1 = hole_radius * 2.5, d2 = hole_radius * 1.5, h = 2, $fn=64);
                        }
                        cylinder(d1 = hole_radius * 2, d2 = hole_radius * 2.5, h = 0.5, $fn=64);
                    }
                    cube([hole_radius * 2, base_radius * 2, base_radius * 2], center=true);
                }
            }
        }
        translate([0, 0, (board_width + 20) / 2])cube([20, 0.4, 20], center=true);
    }
}

module standoff(x, y) {
    standoff_radius = screw_head_radius * 1.25;
    translate([x, y]) {
        cylinder(pcb_standoff_height, standoff_radius, standoff_radius, $fn=32);
    }
    translate([x, y, pcb_standoff_height]) {
        clip();
    }
}

module pcb_mount() {
    standoff(3.5 + 1, bed_length - pcb_length + 3.5);
    standoff(3.5 + 1, bed_length - 3.5);
    standoff(6.5 * 25.4 - 3.5 - 1, bed_length - pcb_length + 3.5);
    standoff(6.5 * 25.4 - 3.5 - 1, bed_length - 3.5);
}

module screw_hole(x, y) {
    translate([x, y, -bed_height - 1]) {
        cylinder(bed_height + 2, 2.1, 2.1, $fn=32);
        translate([0, 0, screw_head_height]) cylinder(bed_height, 4, 4, $fn=32);
    }
}

module bed() {
    translate([0, 0, -25.4 * 0.45]) {
        difference() {
            union() {
                translate([0, 0, -bed_height]) cube([7 * base_width, bed_length, bed_height]);
                pcb_mount();
                key_stand(0 * base_width, white_key_width);
                key_stand(0.5 * base_width + inner_width / 2, black_key_width);
                key_stand(1 * base_width, white_key_width);
                key_stand(1.5 * base_width + inner_width / 2, black_key_width);
                key_stand(2 * base_width, white_key_width);
                key_stand(3 * base_width, white_key_width);
                key_stand(3.5 * base_width + inner_width / 2, black_key_width);
                key_stand(4 * base_width, white_key_width);
                key_stand(4.5 * base_width + inner_width / 2, black_key_width);
                key_stand(5 * base_width, white_key_width);
                key_stand(5.5 * base_width + inner_width / 2, black_key_width);
                key_stand(6 * base_width, white_key_width);
            }
            translate([0.5 * 25.4, 0.5 * 25.4, -bed_height - 5]) {
                cube([5.5 * 25.4, 1.25 * 25.4, bed_height + 10]);
            }
            translate([0.5 * 25.4, 0.5 * 25.4 + 2.0 * 25.4, -bed_height - 5]) {
                cube([5.5 * 25.4, 2.5 * 25.4, bed_height + 10]);
            }
            translate([0.5 * 25.4, 0.5 * 25.4 + 7.25 * 25.4, -bed_height - 5]) {
                cube([5.5 * 25.4, 25.4, bed_height + 10]);
            }
            screw_hole(7 * base_width - 5, 5);
            screw_hole(5, 5);
            screw_hole(7 * base_width - 5, 6 * 25.4);
            screw_hole(5, 6 * 25.4);
        }
        
        // Support for PCB.
        translate([3 * base_width - 2, bed_length - pcb_length - 3.5, -bed_height]) {
            cube([4, pcb_length + 3.5, pcb_standoff_height + bed_height]);
        }
    }
}

rotate([0, 0, 90]) {
  //  keys();
    bed();
}
