#include <stdint.h>
#include <stddef.h>

void set_planeBitmap(int plane, short* imgptr) __attribute__((section(".data")));
void set_palette(short* pal, int start, int count);
void clear_plane(int plane);
void set_vram(int offset, int val);
void next_vram(int val);

static int base_offs;

#define PLANE_WIDTH_TILES 64
#define PLANE_A_OFFSET 0xC000
#define PLANE_B_OFFSET 0xE000

void clear_planes(void)
{
	int i;

    clear_plane(0);
    clear_plane(1);

	/* Cleared name-table entries reference pattern 0, so keep that pattern
	 * transparent instead of overwriting it with the first bitmap tile. */
	set_vram(0, 0);
	for (i = 1; i < 16; i++)
		next_vram(0);
	base_offs = 32;
}

void set_planeBitmap(int plane, short* imgptr)
{
    int x, y, tx, ty;
    int w, h;
    int offs = 0;
    int vram_inc = 0;
    int start = base_offs/32;
    int tiles_wide;

    clear_plane(plane);

    if (!imgptr)
        return;

    w = imgptr[0];
    h = imgptr[1];
    if (!w || !h)
        return;

    tiles_wide = (w + 7) / 8;

    // use palette 0 for plane A, palette 1 for plane B
    set_palette(imgptr + 2, plane*16, 16);

    imgptr += 18;
    for (y = 0; y < h; y++)
    {
        /* Tile rows are packed according to this bitmap's actual width. */
        offs = base_offs + ((y >> 3) * tiles_wide * 32) + ((y & 7) * 4);
        for (x = 0; x < w; x += 8)
        {
            set_vram(offs, imgptr[0]);
            next_vram(imgptr[1]);

            vram_inc += 4;
            imgptr += 2;
            offs += 32;
        }
    }
    base_offs += vram_inc;

    /*
     * Map only the tiles belonging to this bitmap.  The old mapper assumed
     * every bitmap was a 320-pixel-wide background, which caused small MD
     * assets to reference uninitialized VRAM tiles across the plane.
     */
    {
        int plane_offset = plane ? PLANE_B_OFFSET : PLANE_A_OFFSET;
        /* Plane A is the high-priority/front MD layer.  Plane B uses palette
         * 1 and remains the low-priority/rear MD layer. */
        int tile_mask = plane ? 0x2000 : 0x8000;
        int tiles_high = (h + 7) / 8;

        for (ty = 0; ty < tiles_high; ty++)
        {
            for (tx = 0; tx < tiles_wide; tx++)
            {
                int name_offset = plane_offset + ((ty * PLANE_WIDTH_TILES + tx) * 2);
                int tile = start + ty * tiles_wide + tx;
                set_vram(name_offset, tile | tile_mask);
            }
        }
    }
}
