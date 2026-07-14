import cadquery as cq
# ----------------------------
# Parameters
# ----------------------------

box_length = 100
box_width = 100
box_height = 40
box_wall = 1.5

lid_thickness_center = 2
lid_thickness_edge = 1
lid_clearance = 0.1
lid_plug_depth = lid_thickness_center - lid_thickness_edge
lid_post_clearance = 0.1
lid_plug_ring_width = 6

box_screw_post_diameter = 7
box_screw_post_hole_diameter = 1.9
box_screw_post_hole_depth = 6
box_screw_post_inset = 5
box_screw_post_height = box_height - lid_plug_depth - lid_post_clearance
lid_screw_hole_diameter = 2.4

post_base_fillet_radius = 1.5

nano_post_corner_offset = 1
nano_width = 18
nano_length = 44
nano_board_thickness = 1.6
nano_post_hole_depth = 4
nano_post_hole_diameter = 2
nano_post_diameter = 7
nano_post_height = 10 + box_wall
nano_post_fillet_radius = 1.5

usb_hole_width = 15
usb_hole_height = 10

conn_hole_width = 19.5
conn_hole_height = 9

conn_ledge_depth = 10
conn_ledge_thickness = 3
conn_ledge_extra_width = conn_ledge_thickness

conn_positions = [
    (-20, 15),
    ( 20, 15),
]


# ----------------------------
# Helper geometry
# ----------------------------

def screw_post_points():
    x = box_width / 2 - box_wall - box_screw_post_inset
    y = box_length / 2 - box_wall - box_screw_post_inset

    return [
        (-x, -y),
        ( x, -y),
        (-x,  y),
        ( x,  y),
    ]

def base_flare(diameter, fillet_radius, base_z=0):
    """
    Printable root chamfer/gusset.
    This is a shallow conical loft, not a true circular fillet.
    It starts at base_z so it stays visible above the enclosure floor.
    """
    return (
        cq.Workplane("XY")
        .circle(diameter / 2 + fillet_radius)
        .workplane(offset=fillet_radius)
        .circle(diameter / 2)
        .loft(combine=True)
        .translate((0, 0, base_z))
    )

def cyl_post(diameter, height, hole_diameter, hole_depth, fillet_radius=1.5, base_z=box_wall):
    post = (
        cq.Workplane("XY")
        .circle(diameter / 2)
        .extrude(height - base_z)
        .translate((0, 0, base_z))
        .union(base_flare(diameter, fillet_radius, base_z))
        .faces(">Z")
        .workplane()
        .hole(hole_diameter, hole_depth)
    )

    return post

def post_array(points, diameter, height, hole_diameter, hole_depth, fillet_radius):
    posts = cq.Workplane("XY")

    for x, y in points:
        posts = posts.union(
            cyl_post(
                diameter,
                height,
                hole_diameter,
                hole_depth,
                fillet_radius,
            ).translate((x, y, 0))
        )

    return posts

# ----------------------------
# Nano support posts
# ----------------------------

def nano_post():
    post = cyl_post(
        nano_post_diameter,
        nano_post_height,
        nano_post_hole_diameter,
        nano_post_hole_depth,
        nano_post_fillet_radius,
    )

    pocket = (
        cq.Workplane("XY")
        .box(
            4,
            4,
            nano_board_thickness,
            centered=(False, False, False),
        )
        .translate((
            nano_post_corner_offset,
            nano_post_corner_offset,
            nano_post_height - 1,
        ))
    )

    return post.cut(pocket)


def nano_supports():
    dx = nano_width + 2 * nano_post_corner_offset
    dy = nano_length + 2 * nano_post_corner_offset

    placements = [
        (-dx / 2, -dy / 2,   0),
        (-dx / 2,  dy / 2, -90),
        ( dx / 2, -dy / 2,  90),
        ( dx / 2,  dy / 2, 180),
    ]

    supports = cq.Workplane("XY")

    for x, y, angle in placements:
        p = (
            nano_post()
            .rotate((0, 0, 0), (0, 0, 1), angle)
            .translate((x, y, 0))
        )
        supports = supports.union(p)

    return supports

# ----------------------------
# Enclosure
# ----------------------------

def enclosure_shell():
    outer = (
        cq.Workplane("XY")
        .box(box_width, box_length, box_height)
        .translate((0, 0, box_height / 2))
    )
    inner = (
        cq.Workplane("XY")
        .box(
            box_width - 2 * box_wall,
            box_length - 2 * box_wall,
            box_height,
        )
        .translate((0, 0, box_wall + box_height / 2))
    )

    return outer.cut(inner)


def enclosure_screw_posts():
    return post_array(
        screw_post_points(),
        box_screw_post_diameter,
        box_screw_post_height,
        box_screw_post_hole_diameter,
        box_screw_post_hole_depth,
        post_base_fillet_radius,
    )

def usb_cutout():
    return (
        cq.Workplane("XY")
        .box(
            usb_hole_width,
            box_wall + 2,
            usb_hole_height,
        )
        .translate((
            box_width / 2 - box_width / 3,
            box_length / 2,
            nano_post_height + 1,
        ))
    )

def add_conn_cutout(body, offset_y, offset_z):
    cutter = (
        cq.Workplane("XY")
        .box(
            box_wall + 2,
            conn_hole_width,
            conn_hole_height,
        )
        .translate((
            -box_width / 2,
            offset_y,
            offset_z,
        ))
    )

    ledge_width = conn_hole_width + 2 * conn_ledge_extra_width
    ledge_x = -box_width / 2 + box_wall + conn_ledge_depth / 2

    ledge = (
        cq.Workplane("XY")
        .box(
            conn_ledge_depth,
            ledge_width,
            conn_ledge_thickness,
        )
        .translate((
            ledge_x,
            offset_y,
            offset_z
                - conn_hole_height / 2
                - conn_ledge_thickness / 2,
        ))
    )

    wall_gap = conn_hole_width
    side_wall_y_offset = wall_gap / 2 + conn_ledge_thickness / 2
    side_walls = cq.Workplane("XY")

    for y_offset in (-side_wall_y_offset, side_wall_y_offset):
        side_walls = side_walls.union(
            cq.Workplane("XY")
            .box(
                conn_ledge_depth,
                conn_ledge_thickness,
                conn_hole_height,
            )
            .translate((
                ledge_x,
                offset_y + y_offset,
                offset_z,
            ))
        )

    return body.cut(cutter).union(ledge).union(side_walls)

def enclosure():
    body = enclosure_shell()
    body = body.union(enclosure_screw_posts())

    body = body.union(
        nano_supports().translate((
            box_width / 2 - box_width / 3,
            0,
            0,
        ))
    )

    body = body.cut(usb_cutout())

    for offset_y, offset_z in conn_positions:
        body = add_conn_cutout(body, offset_y, offset_z)

    return body

# ----------------------------
# Lid
# ----------------------------

def enclosure_lid():
    plug_width = box_width - 2 * (box_wall + lid_clearance)
    plug_length = box_length - 2 * (box_wall + lid_clearance)

    flange = (
        cq.Workplane("XY")
        .box(box_width, box_length, lid_thickness_edge)
        .translate((0, 0, lid_plug_depth + lid_thickness_edge / 2))
    )

    plug_inner_width = plug_width - 2 * lid_plug_ring_width
    plug_inner_length = plug_length - 2 * lid_plug_ring_width

    plug_outer = (
        cq.Workplane("XY")
        .box(plug_width, plug_length, lid_plug_depth)
        .translate((0, 0, lid_plug_depth / 2))
    )

    plug_inner = (
        cq.Workplane("XY")
        .box(plug_inner_width, plug_inner_length, lid_plug_depth + 2)
        .translate((0, 0, lid_plug_depth / 2))
    )

    plug = plug_outer.cut(plug_inner)

    lid = flange.union(plug)

    lid = (
        lid.faces(">Z")
        .workplane()
        .pushPoints(screw_post_points())
        .hole(lid_screw_hole_diameter)
    )

    return lid

# ----------------------------
# Build objects
# ----------------------------

box = enclosure()
lid = enclosure_lid().translate((box_width + 20, 0, 0))

box.export("./box.step")
lid.export("box-lid.step")

assy = cq.Assembly()
assy.add(box, name="box")
assy.add(lid, name="lid")

try:
    show_object(box, name="box")
    show_object(lid, name="lid")
except:
    pass


