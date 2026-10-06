#!/usr/bin/env python3
"""
make_pet.py - Draws the virtual pet and all of its animation frames.

The pet ("Sprout") is a round mint-colored blob with a little leaf on its head.
Every frame is drawn on a 32x32 pixel grid, then saved at 2x scale (64x64)
so it shows up big and sharp on the LCD.

Usage (from the repository root):
  python tools/make_pet.py [OUTPUT_DIR]        (default: docs/art/frames)

Customize the pet by editing the PALETTE below (colors) or the drawing
functions (eyes, mouth, leaf, extras).
"""
import os
import sys

from PIL import Image

GRID = 32          # design size
SCALE = 1          # store at 32x32; the board enlarges it while drawing

# ---- Colors (R, G, B). Avoid pure black inside the pet: the screen is black.
PALETTE = {
    "bg":        (0, 0, 0),
    "outline":   (24, 70, 78),
    "body":      (96, 214, 174),
    "light":     (178, 244, 214),
    "shade":     (58, 166, 136),
    "eye":       (30, 34, 70),
    "shine":     (255, 255, 255),
    "cheek":     (255, 140, 164),
    "mouth":     (110, 34, 60),
    "tongue":    (255, 120, 140),
    "leaf":      (124, 204, 64),
    "leaf_dark": (56, 128, 40),
    "berry":     (230, 50, 70),
    "spark":     (255, 220, 80),
    "zz":        (170, 190, 255),
    "sweat":     (120, 200, 255),
    "ghost":     (205, 215, 225),
    "ghost_out": (120, 130, 150),
    "halo":      (255, 220, 80),
    "gray":      (150, 160, 160),
    "gray_dark": (90, 100, 105),
    "gray_out":  (60, 66, 70),
}


class Canvas:
    def __init__(self):
        self.px = [["bg"] * GRID for _ in range(GRID)]

    def set(self, x, y, c):
        if 0 <= x < GRID and 0 <= y < GRID:
            self.px[y][x] = c

    def get(self, x, y):
        if 0 <= x < GRID and 0 <= y < GRID:
            return self.px[y][x]
        return "bg"

    def image(self):
        img = Image.new("RGB", (GRID, GRID))
        for y in range(GRID):
            for x in range(GRID):
                img.putpixel((x, y), PALETTE[self.px[y][x]])
        return img.resize((GRID * SCALE, GRID * SCALE), Image.NEAREST)


# ---------------------------------------------------------------- body parts
def draw_body(cv, cx, cy, rx, ry, colors=("body", "light", "shade", "outline")):
    body, light, shade, outline = colors
    inside = set()
    for y in range(GRID):
        for x in range(GRID):
            dx = (x + 0.5 - cx) / rx
            dy = (y + 0.5 - cy) / ry
            if dx * dx + dy * dy <= 1.0:
                inside.add((x, y))
    for (x, y) in inside:
        edge = any((x + ox, y + oy) not in inside
                   for ox, oy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
        if edge:
            cv.set(x, y, outline)
        elif (y + 0.5) > cy + ry * 0.45 or (x + 0.5) > cx + rx * 0.7:
            cv.set(x, y, shade)
        else:
            cv.set(x, y, body)
    # small highlight, top-left
    hx, hy = int(cx - rx * 0.55), int(cy - ry * 0.55)
    for (x, y) in ((hx, hy), (hx + 1, hy), (hx, hy + 1)):
        if (x, y) in inside and cv.get(x, y) != outline:
            cv.set(x, y, light)


def draw_feet(cv, cx, cy, ry, color="outline"):
    fy = int(cy + ry) - 1
    for fx in (int(cx) - 6, int(cx) + 3):
        for x in range(fx, fx + 3):
            cv.set(x, fy, color)
            cv.set(x, fy + 1, color)


def draw_leaf(cv, cx, top, droop=False):
    x = int(cx)
    cv.set(x, top - 1, "leaf_dark")
    cv.set(x, top - 2, "leaf_dark")
    if droop:   # wilted leaf hanging to the side
        for p in ((x + 1, top - 2), (x + 2, top - 2), (x + 2, top - 1),
                  (x + 3, top - 1), (x + 3, top)):
            cv.set(*p, "leaf_dark")
        return
    for p in ((x + 1, top - 3), (x + 2, top - 3), (x + 1, top - 4),
              (x + 2, top - 4), (x + 3, top - 4), (x + 3, top - 5),
              (x - 1, top - 3), (x - 2, top - 3), (x - 2, top - 4)):
        cv.set(*p, "leaf")
    cv.set(x + 1, top - 2, "leaf_dark")


def draw_eyes(cv, cx, cy, style="open"):
    for ex in (int(cx) - 5, int(cx) + 3):
        ey = int(cy) - 2
        if style == "open":
            for y in range(ey, ey + 3):
                cv.set(ex, y, "eye")
                cv.set(ex + 1, y, "eye")
            cv.set(ex, ey, "shine")
        elif style == "closed":
            cv.set(ex, ey + 2, "eye")
            cv.set(ex + 1, ey + 2, "eye")
        elif style == "happy":      # upside-down V
            cv.set(ex, ey + 2, "eye")
            cv.set(ex + 1, ey + 1, "eye")
            cv.set(ex + 2, ey + 2, "eye")
        elif style == "x":
            for d in range(3):
                cv.set(ex + d, ey + d, "eye")
                cv.set(ex + 2 - d, ey + d, "eye")
        elif style == "worried":
            for y in range(ey + 1, ey + 3):
                cv.set(ex, y, "eye")
                cv.set(ex + 1, y, "eye")
            cv.set(ex, ey + 1, "shine")
            # slanted brow
            if ex < cx:
                cv.set(ex, ey - 1, "eye")
                cv.set(ex + 1, ey - 2, "eye")
            else:
                cv.set(ex + 1, ey - 1, "eye")
                cv.set(ex, ey - 2, "eye")


def draw_cheeks(cv, cx, cy):
    cv.set(int(cx) - 8, int(cy) + 1, "cheek")
    cv.set(int(cx) - 7, int(cy) + 1, "cheek")
    cv.set(int(cx) + 6, int(cy) + 1, "cheek")
    cv.set(int(cx) + 7, int(cy) + 1, "cheek")


def draw_mouth(cv, cx, cy, style="smile"):
    mx, my = int(cx) - 1, int(cy) + 2
    if style == "smile":
        cv.set(mx - 1, my, "mouth")
        cv.set(mx, my + 1, "mouth")
        cv.set(mx + 1, my + 1, "mouth")
        cv.set(mx + 2, my, "mouth")
    elif style == "open":
        for x in range(mx - 1, mx + 3):
            cv.set(x, my, "mouth")
            cv.set(x, my + 2, "mouth")
        cv.set(mx - 1, my + 1, "mouth")
        cv.set(mx + 2, my + 1, "mouth")
        cv.set(mx, my + 1, "tongue")
        cv.set(mx + 1, my + 1, "tongue")
    elif style == "flat":
        for x in range(mx, mx + 2):
            cv.set(x, my + 1, "mouth")
    elif style == "chew":
        for x in range(mx - 1, mx + 3):
            cv.set(x, my + 1, "mouth")
        cv.set(mx, my, "mouth")
    elif style == "sad":
        cv.set(mx - 1, my + 1, "mouth")
        cv.set(mx, my, "mouth")
        cv.set(mx + 1, my, "mouth")
        cv.set(mx + 2, my + 1, "mouth")


# ---------------------------------------------------------------- extras
def draw_berry(cv, x, y):
    for p in ((x, y), (x + 1, y), (x, y + 1), (x + 1, y + 1), (x - 1, y + 1),
              (x + 2, y + 1), (x, y + 2), (x + 1, y + 2)):
        cv.set(*p, "berry")
    cv.set(x, y, "shine")
    cv.set(x + 1, y - 1, "leaf")


def draw_heart(cv, x, y):
    for p in ((x, y), (x + 2, y), (x - 1, y + 1), (x, y + 1), (x + 1, y + 1),
              (x + 2, y + 1), (x + 3, y + 1), (x, y + 2), (x + 1, y + 2),
              (x + 2, y + 2), (x + 1, y + 3)):
        cv.set(*p, "cheek")


def draw_spark(cv, x, y):
    for p in ((x, y), (x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
        cv.set(*p, "spark")


def draw_z(cv, x, y, big=False):
    n = 5 if big else 4
    for i in range(n):
        cv.set(x + i, y, "zz")
        cv.set(x + i, y + n - 1, "zz")
        cv.set(x + n - 1 - i, y + i, "zz")


def draw_sweat(cv, x, y):
    cv.set(x, y, "sweat")
    cv.set(x, y + 1, "sweat")
    cv.set(x - 1, y + 1, "sweat")
    cv.set(x, y + 2, "sweat")


def draw_halo(cv, cx, y):
    for x in range(int(cx) - 3, int(cx) + 4):
        cv.set(x, y, "halo")
    cv.set(int(cx) - 4, y + 1, "halo")
    cv.set(int(cx) + 4, y + 1, "halo")


# ---------------------------------------------------------------- one pet pose
CX, CY, RX, RY = 16.0, 19.0, 10.0, 8.5


def pet(dx=0, dy=0, squash=0, eyes="open", mouth="smile", cheeks=True,
        leaf=True, droop=False, feet=True, colors=None):
    cv = Canvas()
    cx, cy = CX + dx, CY + dy + squash * 0.5
    rx, ry = RX + squash, RY - squash
    cols = colors or ("body", "light", "shade", "outline")
    if feet:
        draw_feet(cv, cx, cy, ry, cols[3])
    draw_body(cv, cx, cy, rx, ry, cols)
    if leaf:
        draw_leaf(cv, cx, int(cy - ry) + 1, droop)
    draw_eyes(cv, cx, cy, eyes)
    if cheeks:
        draw_cheeks(cv, cx, cy)
    draw_mouth(cv, cx, cy, mouth)
    return cv, cx, cy


GRAY = ("gray", "gray", "gray_dark", "gray_out")
GHOST = ("ghost", "shine", "ghost", "ghost_out")


def frames():
    out = {}

    # Idle: gentle breathing bounce and a blink
    out["idle"] = [pet()[0], pet(squash=1)[0], pet()[0], pet(eyes="closed")[0]]

    # Feed: berry flies in, chomp, chew, happy heart
    f = []
    cv, cx, cy = pet(eyes="open", mouth="open"); draw_berry(cv, 26, 22); f.append(cv)
    cv, cx, cy = pet(eyes="open", mouth="open"); draw_berry(cv, 20, 21); f.append(cv)
    cv, cx, cy = pet(squash=1, eyes="happy", mouth="chew"); f.append(cv)
    cv, cx, cy = pet(eyes="happy", mouth="smile"); draw_heart(cv, 24, 6); f.append(cv)
    out["feed"] = f

    # Play: crouch, jump high with sparkles, land
    f = []
    cv, *_ = pet(squash=2, eyes="happy", mouth="smile"); f.append(cv)
    cv, *_ = pet(dy=-5, eyes="happy", mouth="open", feet=False)
    draw_spark(cv, 4, 6); draw_spark(cv, 27, 9); f.append(cv)
    cv, *_ = pet(dy=-3, eyes="happy", mouth="open"); draw_spark(cv, 5, 12); f.append(cv)
    cv, *_ = pet(squash=1, eyes="happy", mouth="smile"); f.append(cv)
    out["play"] = f

    # Third action: sleep, slow breathing with floating Zz
    f = []
    cv, *_ = pet(eyes="closed", mouth="flat"); draw_z(cv, 23, 5); f.append(cv)
    cv, *_ = pet(squash=1, eyes="closed", mouth="flat"); draw_z(cv, 23, 2, True); f.append(cv)
    cv, *_ = pet(eyes="closed", mouth="flat"); draw_z(cv, 26, 1); draw_z(cv, 21, 7); f.append(cv)
    cv, *_ = pet(squash=1, eyes="closed", mouth="flat"); draw_z(cv, 23, 3, True); f.append(cv)
    out["sleep"] = f

    # Death: sad, wilts and turns gray, X eyes, ghost floats up with halo
    f = []
    cv, *_ = pet(eyes="closed", mouth="sad", droop=True); f.append(cv)
    cv, *_ = pet(squash=2, eyes="closed", mouth="sad", cheeks=False, droop=True, colors=GRAY); f.append(cv)
    cv, *_ = pet(squash=3, eyes="x", mouth="flat", cheeks=False, droop=True, colors=GRAY); f.append(cv)
    cv, cx, cy = pet(dy=-3, eyes="closed", mouth="flat", cheeks=False, leaf=False, feet=False, colors=GHOST)
    draw_halo(cv, cx, int(cy - RY) - 2); f.append(cv)
    out["death"] = f

    # Runaway: worried with a sweat drop, then runs off to the right
    f = []
    cv, cx, cy = pet(eyes="worried", mouth="sad"); draw_sweat(cv, 26, 9); f.append(cv)
    cv, *_ = pet(dx=5, dy=-1, eyes="worried", mouth="sad"); f.append(cv)
    cv, *_ = pet(dx=12, eyes="worried", mouth="sad"); f.append(cv)
    cv, *_ = pet(dx=21, dy=-1, eyes="worried", mouth="sad")
    for x in (3, 6, 9):            # little dust cloud left behind
        cv.set(x, 27, "gray"); cv.set(x + 1, 26, "gray")
    f.append(cv)
    out["runaway"] = f

    return out


def main():
    out_dir = sys.argv[1] if len(sys.argv) > 1 else os.path.join("docs", "art", "frames")
    os.makedirs(out_dir, exist_ok=True)
    count = 0
    for anim, fl in frames().items():
        for i, cv in enumerate(fl):
            cv.image().save(os.path.join(out_dir, f"pet_{anim}_{i}.png"))
            count += 1
    print(f"Saved {count} frames ({GRID * SCALE}x{GRID * SCALE}) to {out_dir}")


if __name__ == "__main__":
    main()
