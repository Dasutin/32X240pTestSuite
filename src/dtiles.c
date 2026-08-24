#include "32x.h"
#include "types.h"
#include "draw.h"
#include "hw_32x.h"
#include "perf.h"

typedef struct
{
    tilemap_t *tm;
    fixed_t fpcamera_x, fpcamera_y;
    void (*drawspr)(int l, void *p);
    void *sprp;
    int sprites_drawn;
    int parallax;
} drawtilecontext_t;

drawtileslavecmd_t slave_drawtilecmd ATTR_CACHE_ALIGNED;

const uint8_t yatssd_empty_tile[32 * 32] ATTR_CACHE_ALIGNED = {0};

static uint16_t global_tilemap_id = 1;

static fixed_t old_camera_x, old_camera_y;
static fixed_t main_camera_x, main_camera_y;

static fixed_t camera_x, camera_y;

static int draw_tile_layer(drawtilecontext_t *dc, int layer, int *pclipped)
ATTR_DATA_ALIGNED;

static int draw_drawtile(int x, int y, const uint8_t* data, int flags,
    void* fb, draw_spritefn_t fn, drawsprcmd_t *tilecmd)
ATTR_DATA_ALIGNED;

static int draw_parallax_tile(tilemap_t *tm, int x, int y,
    const uint8_t *data, int flags, void *fb, draw_spritefn_t fn,
    drawsprcmd_t *tilecmd)
ATTR_DATA_ALIGNED;

extern void sh2_blit_tile8(uint8_t *dst, const uint8_t *src,
    unsigned pitch);
extern void sh2_blit_tile16(uint8_t *dst, const uint8_t *src,
    unsigned pitch);
extern void sh2_blit_tile32(uint8_t *dst, const uint8_t *src,
    unsigned pitch);
extern void sh2_fill16(uint16_t *destination, unsigned count,
    uint16_t value);

static void snapshot_render_state(drawrenderstate_t *state)
{
    state->canvas_width = canvas_width;
    state->canvas_height = canvas_height;
    state->canvas_pitch = canvas_pitch;
    state->canvas_yaw = canvas_yaw;
    state->window_canvas_x = window_canvas_x;
    state->window_canvas_y = window_canvas_y;
    state->flags = nodraw ? DRAW_RENDER_NODRAW : 0;
    state->reserved = 0;
}

void init_tilemap(tilemap_t *tm, const dtilemap_t *dtm,
    const uint8_t * const *reslist)
{
    int i;
    int has_md_planes;
    int tw = dtm->tilew;
    int th = dtm->tileh;

    tm->id = global_tilemap_id++;

    tm->tw = tw;
    tm->th = th;

    tm->layers = dtm->layers;
    tm->numlayers = dtm->numlayers;
    tm->layers = dtm->layers;
    tm->mdPlane[0] = (dtilelayer_t *)&dtm->mdPlaneA;
    tm->mdPlane[1] = (dtilelayer_t *)&dtm->mdPlaneB;
    tm->reslist = reslist;

    tm->tiles_hor = dtm->numtw;
    tm->tiles_ver = dtm->numth;

    tm->scroll_tiles_hor = (canvas_pitch - canvas_width) / tw;
    tm->scroll_interval_hor = tm->scroll_tiles_hor * tw;

    tm->scroll_tiles_ver = (canvas_yaw - canvas_height) / th;
    tm->scroll_interval_ver = tm->scroll_tiles_ver * th;

    tm->canvas_tiles_hor = canvas_pitch / tw;
    tm->canvas_tiles_ver = canvas_yaw / th;

    tm->numtiles = tm->tiles_hor * tm->tiles_ver;

    set_tilemap_wrap(tm, dtm->wrapX, dtm->wrapY);

    has_md_planes = tm->mdPlane[0]->bitmap || tm->mdPlane[1]->bitmap;
    if (has_md_planes) {
        HwMdClearPlanes();

        for (i = 0; i < 2; i++) {
            const dtilelayer_t *mdpl = tm->mdPlane[i];
            if (mdpl->bitmap) {
                HwMdSetPlaneBitmap(i, mdpl->bitmap);
            }
        }

        HwMdSetPlaneScrolls(
            tm->mdPlane[0]->bitmap ? tm->mdPlane[0]->offset[0] : 0,
            tm->mdPlane[0]->bitmap ? -tm->mdPlane[0]->offset[1] : 0,
            tm->mdPlane[1]->bitmap ? tm->mdPlane[1]->offset[0] : 0,
            tm->mdPlane[1]->bitmap ? -tm->mdPlane[1]->offset[1] : 0);

        Hw32xSetBGOverlayPriorityBit(1);
        Hw32xSetFGOverlayPriorityBit(dtm->mdPriority ^ 1);
    }

    Hw32xUpdateLineTable(0, 0, 0);
}

void set_tilemap_wrap(tilemap_t *tm, fixed_t wrapX, fixed_t wrapY)
{
    tm->wrapX = wrapX * (1<<16);
    tm->wrapY = wrapY * (1<<16);
}

// in window coordinates
void draw_dirtyrect(tilemap_t* tm, int x, int y, int w, int h)
{
    unsigned start_tile_hor, start_tile_ver;
    unsigned end_tile_hor, end_tile_ver;
    int num_tiles_x, num_tiles_y;
    int16_t* extrafb = (int16_t*)&MARS_FRAMEBUFFER + 0x100 + ((canvas_pitch * canvas_yaw) >> 1);

    if (x >= canvas_pitch) return;
    if (y >= canvas_yaw) return;

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    start_tile_hor = (unsigned)x;
    end_tile_hor = (unsigned)(x + w - 1);

    start_tile_ver = (unsigned)y;
    end_tile_ver = (unsigned)(y + h - 1);

    switch (tm->tw) {
    case 8:
        start_tile_hor >>= 3;
        end_tile_hor >>= 3;
        break;
    case 16:
        start_tile_hor >>= 4;
        end_tile_hor >>= 4;
        break;
    default:
        start_tile_hor /= tm->tw;
        end_tile_hor /= tm->tw;
        break;
    }

    switch (tm->th) {
    case 8:
        start_tile_ver >>= 3;
        end_tile_ver >>= 3;
        break;
    case 16:
        start_tile_ver >>= 4;
        end_tile_ver >>= 4;
        break;
    default:
        start_tile_ver /= tm->th;
        end_tile_ver /= tm->th;
        break;
    }

    num_tiles_x = end_tile_hor - start_tile_hor + 1;
    num_tiles_y = end_tile_ver - start_tile_ver + 1;

    int canvas_tiles_hor = tm->canvas_tiles_hor;
    int canvas_tiles_ver = tm->canvas_tiles_ver;

    if (start_tile_hor + num_tiles_x > canvas_tiles_hor) num_tiles_x = canvas_tiles_hor - start_tile_hor;
    if (start_tile_ver + num_tiles_y > canvas_tiles_ver) num_tiles_y = canvas_tiles_ver - start_tile_ver;

    int16_t* dirty = extrafb + 3;
    dirty += start_tile_ver * canvas_tiles_hor + start_tile_hor;

    for (y = 0; y < num_tiles_y; y++) {
        sh2_fill16((uint16_t *)dirty, num_tiles_x, 0xFFFF);
        dirty += canvas_tiles_hor;
    }
}

static int draw_drawtile(int x, int y, const uint8_t* data, int flags,
    void *fb, draw_spritefn_t fn, drawsprcmd_t *tilecmd)
{
    const drawrenderstate_t *render = &tilecmd->render;
    int w = tilecmd->w;
    int h = tilecmd->h;
    uint8_t *dst;

    x += render->window_canvas_x;
    y += render->window_canvas_y;

    /* Full, aligned, opaque square tiles use fixed-size assembly kernels. */
    if (!(render->flags & DRAW_RENDER_NODRAW) &&
        !(flags & (DRAWSPR_HFLIP | DRAWSPR_VFLIP |
            DRAWSPR_OVERWRITE | DRAWSPR_SCALE)) &&
        x >= 0 && y >= 0 && x + w <= render->canvas_pitch &&
        y + h <= render->canvas_yaw && w == h)
    {
        dst = (uint8_t *)fb + y * render->canvas_pitch + x;
        if (!(((uintptr_t)dst | (uintptr_t)data) & 3)) {
            switch (w) {
            case 8:
                sh2_blit_tile8(dst, data, render->canvas_pitch);
                return 1;
            case 16:
                sh2_blit_tile16(dst, data, render->canvas_pitch);
                return 1;
            case 32:
                sh2_blit_tile32(dst, data, render->canvas_pitch);
                return 1;
            }
        }
    }

    tilecmd->flags = flags;
    tilecmd->x = x;
    tilecmd->y = y;
    tilecmd->sdata = (void*)data;
    fn(fb, tilecmd);

    return 1;
}

static int draw_parallax_tile(tilemap_t *tm, int x, int y,
    const uint8_t *data, int flags, void *fb, draw_spritefn_t fn,
    drawsprcmd_t *tilecmd)
{
    const drawrenderstate_t *render = &tilecmd->render;
    int physical_x = x + render->window_canvas_x;
    int display_left = render->window_canvas_x;
    int display_right = display_left + render->canvas_width;
    int clip_left = 0;
    int clip_right = 0;

    if (physical_x < display_left)
        clip_left = display_left - physical_x;
    if (physical_x + tilecmd->w > display_right)
        clip_right = physical_x + tilecmd->w - display_right;
    if (clip_left + clip_right >= tilecmd->w)
        return 0;
    if (!clip_left && !clip_right)
    {
        int drawn = draw_drawtile(x, y, data, flags, fb, fn, tilecmd);

        /* The base layer is cached per canvas tile.  Parallax tiles move
         * relative to that canvas, so dirty their footprint after drawing;
         * the next frame will restore pixels that the tile vacates. */
        draw_dirtyrect(tm, physical_x, y + render->window_canvas_y,
            tilecmd->w, tilecmd->h);
        return drawn;
    }

    tilecmd->flags = flags;
    tilecmd->x = physical_x + clip_left;
    tilecmd->y = y + render->window_canvas_y;
    tilecmd->w -= clip_left + clip_right;
    tilecmd->sx = flags & DRAWSPR_HFLIP ?
        tilecmd->sw - (tilecmd->w + clip_left) : clip_left;
    tilecmd->sdata = (void *)data;
    fn(fb, tilecmd);

    /* Preserve the same invalidation for a horizontally clipped tile. */
    draw_dirtyrect(tm, tilecmd->x, tilecmd->y, tilecmd->w, tilecmd->h);

    tilecmd->sx = 0;
    tilecmd->w = tilecmd->sw;
    return 1;
}

int draw_handle_layercmd(drawtilelayerscmd_t *cmd)
{
    int x, y;
    tilemap_t* tm = cmd->tm;
    const drawrenderstate_t *render = &cmd->render;
    const int w = tm->tw, h = tm->th;
    int16_t* extrafb = (int16_t*)&MARS_FRAMEBUFFER + 0x100 + ((render->canvas_pitch * render->canvas_yaw) >> 1);
    int16_t* dirty = extrafb + 3;
    const int canvas_tiles_hor = tm->canvas_tiles_hor;
    const int xx = cmd->x, yy = cmd->y;
    const int start_tile = cmd->start_tile, end_tile = cmd->end_tile;
    const int scroll_tile_id = cmd->scroll_tile_id;
    const int num_tiles_x = cmd->num_tiles_x;
    const uint8_t * const *reslist = tm->reslist;
    const int tiles_hor = tm->tiles_hor;
    drawsprcmd_t tilecmd;
    int drawcnt = 0;

    tilecmd.sdata = NULL;
    tilecmd.flags = 0;
    tilecmd.sx = 0;
    tilecmd.sy = 0;
    tilecmd.sw = w;
    tilecmd.sh = h;
    tilecmd.x = 0;
    tilecmd.y = 0;
    tilecmd.w = w;
    tilecmd.h = h;
    tilecmd.scale = 0;
    tilecmd.render = *render;

    if (cmd->startlayer != 0 && cmd->parallax)
    {
        const dtilelayer_t *tl = &tm->layers[cmd->startlayer];
        const int16_t* layer = (int16_t *)tl->tiles;
        int y_tile;
        int drawmode = cmd->drawmode & ~3;
        void *fb = (void *)((uint16_t *)(drawmode & DRAWSPR_OVERWRITE ?
            &MARS_OVERWRITE_IMG : &MARS_FRAMEBUFFER) + 0x100);
        draw_spritefn_t flip_functions[4];
        unsigned flip;

        for (flip = 0; flip < 4; flip++)
            flip_functions[flip] = draw_spritefn(drawmode | flip);

        if (tl->objectLayer)
            return 0;

        y = yy;
        for (y_tile = start_tile; y_tile < end_tile; y_tile += tiles_hor)
        {
            int tile;
            int t1 = y_tile;
            int t2 = y_tile + num_tiles_x;

            x = xx;

            for (tile = t1; tile < t2; tile++)
            {
                int16_t idx = layer[tile];
                if (idx != 0)
                {
                    const uint8_t* res = reslist[(idx >> 2)];
                    //if (debug) res = reslist[1];

                    unsigned tileflip = idx & 3;
                    int tiledrawmode = drawmode | tileflip;
                    draw_parallax_tile(tm, x, y, res, tiledrawmode, fb,
                        flip_functions[tileflip], &tilecmd);
                    drawcnt++;
                }

                x += w;
            }

            y += h;
            if (y >= render->canvas_yaw)
                break;
        }

        return drawcnt;
    }
    else
    {
        int drawmode = cmd->drawmode;
        unsigned l = cmd->startlayer;
        const dtilelayer_t *tl = &tm->layers[l];
        const int16_t* layer = (int16_t *)tl->tiles;
        const dtilelayer_t *ltl = &tm->layers[tm->numlayers-1];
        int y_tile;
        int stid = scroll_tile_id;
        void* fb;
        draw_spritefn_t flip_functions[4];
        unsigned flip;

        if (tl->objectLayer)
            return 0;

        // find the last non-object layer
        while (ltl != tm->layers && ltl->objectLayer)
            ltl--;
        const int16_t* last_layer = (int16_t *)ltl->tiles;

        if (l > 0)
            drawmode |= DRAWSPR_PRECISE | DRAWSPR_OVERWRITE;
        if (!((xx + render->window_canvas_x) & 1))
            drawmode &= ~DRAWSPR_PRECISE;
        drawmode &= ~3;
        fb = (void *)((uint16_t *)(drawmode & DRAWSPR_OVERWRITE ?
            &MARS_OVERWRITE_IMG : &MARS_FRAMEBUFFER) + 0x100);
        for (flip = 0; flip < 4; flip++)
            flip_functions[flip] = draw_spritefn(drawmode | flip);

        y = yy;
        for (y_tile = start_tile; y_tile < end_tile; y_tile += tiles_hor)
        {
            int id;
            int tile;
            int t1 = y_tile;
            int t2 = y_tile + num_tiles_x;

            id = stid;
            x = xx;
            for (tile = t1; tile < t2; tile++)
            {
                int16_t idx = layer[tile];

                if (dirty[id] != idx)
                {
                    if (idx != 0 || (l == 0 && last_layer[tile] != dirty[id]))
                    {
                        if (l == 0)
                            dirty[id] = idx;
                        else
                            dirty[id] = dirty[id] == 0 ? idx : -1;

                        const uint8_t* res = reslist[(idx >> 2)];
                        unsigned tileflip = idx & 3;
                        int tiledrawmode = drawmode | tileflip;
                        /*if (debug) res = reslist[1];*/

                        draw_drawtile(x, y, res, tiledrawmode, fb,
                            flip_functions[tileflip], &tilecmd);
                        drawcnt++;
                    }
                }

                id++;
                x += w;
            }

            y += h;
            stid += canvas_tiles_hor;
            if (y >= render->canvas_yaw)
                break;
        }
    }

    return drawcnt;
}

static int draw_tile_layer(drawtilecontext_t *dc, int layer, int *pclipped)
{
    tilemap_t *tm = dc->tm;
    const dtilelayer_t *tl = &tm->layers[layer];
    unsigned w = tm->tw, h = tm->th;
    const fixed_t *plx = tm->layers[layer].parallax;
    int clipped = 0;
    int drawcnt;
    drawtilelayerscmd_t master_cmd;

    if (tl->objectLayer)
    {
        *pclipped = 0;
        if (!dc->drawspr)
            return 0;
        dc->drawspr(tl->objectLayer, dc->sprp);
        dc->sprites_drawn = 1;
        return 0;
    }

    camera_x = FixedMul(dc->fpcamera_x, plx[0])>>16;
    camera_y = FixedMul(dc->fpcamera_y, plx[1])>>16;

    if (camera_x < 0)
    {
        camera_x = 0;
        clipped |= 1;
    }

    if (camera_x > tm->tiles_hor * tm->tw - canvas_width)
    {
        camera_x = tm->tiles_hor * tm->tw - canvas_width;
        clipped |= 2;
    }

    if (camera_y < 0)
    {
        camera_y = 0;
        clipped |= 4;
    }

    if (camera_y > tm->tiles_ver * tm->th - canvas_height)
    {
        camera_y = tm->tiles_ver * tm->th - canvas_height;
        clipped |= 8;
    }

    int scroll_tiles_hor = 0, scroll_interval_hor = 0;
    int scroll_tiles_ver = 0, scroll_interval_ver = 0;
    int scroll_count_hor = 0, scroll_count_ver = 0;
    int top_scroll_tile_hor = 0, top_scroll_tile_ver = 0;

    scroll_tiles_hor = tm->scroll_tiles_hor;
    scroll_interval_hor = tm->scroll_interval_hor;

    scroll_tiles_ver = tm->scroll_tiles_ver;
    scroll_interval_ver = tm->scroll_interval_ver;

    scroll_count_hor = scroll_interval_hor ? camera_x / scroll_interval_hor : 0;
    scroll_count_ver = scroll_interval_ver ? camera_y / scroll_interval_ver : 0;

    top_scroll_tile_hor = scroll_tiles_hor * scroll_count_hor;
    top_scroll_tile_ver = scroll_tiles_ver * scroll_count_ver;

    int tiles_hor = tm->tiles_hor;
    int tiles_ver = tm->tiles_ver;

    unsigned start_tile_hor, start_tile_ver;
    unsigned end_tile_hor, end_tile_ver;

    start_tile_hor = (unsigned)camera_x;
    end_tile_hor = (unsigned)camera_x + w - 1 + canvas_width;
    start_tile_ver = (unsigned)camera_y;
    end_tile_ver = (unsigned)camera_y + h - 1 + canvas_height;

    switch (w) {
    case 8:
        start_tile_hor >>= 3;
        end_tile_hor >>= 3;
        break;
    case 16:
        start_tile_hor >>= 4;
        end_tile_hor >>= 4;
        break;
    default:
        start_tile_hor /= w;
        end_tile_hor /= w;
        break;
    }

    switch (h) {
    case 8:
        start_tile_ver >>= 3;
        end_tile_ver >>= 3;
        break;
    case 16:
        start_tile_ver >>= 4;
        end_tile_ver >>= 4;
        break;
    default:
        start_tile_ver /= h;
        end_tile_ver /= h;
        break;
    }

    if (start_tile_hor >= tiles_hor) return 0;
    if (start_tile_ver >= tiles_ver) return 0;

    if (start_tile_hor < 0) start_tile_ver = 0;
    if (start_tile_ver < 0) start_tile_ver = 0;

    if (end_tile_hor > tiles_hor) end_tile_hor = tiles_hor;

    if (end_tile_ver < 1) end_tile_ver = 1;
    if (end_tile_ver > tiles_ver) end_tile_ver = tiles_ver;

    int start_tile = start_tile_ver * tiles_hor + start_tile_hor;
    if (start_tile >= tm->numtiles)
        return 0;

    int end_tile = (end_tile_ver - 1) * tiles_hor + end_tile_hor;
    if (end_tile > tm->numtiles)
        end_tile = tm->numtiles;

    int canvas_tiles_hor = tm->canvas_tiles_hor;
    int canvas_tiles_ver = tm->canvas_tiles_ver;

    unsigned scroll_x, scroll_y;

    int xx, yy;

    scroll_x = camera_x - scroll_count_hor * scroll_interval_hor;
    scroll_y = camera_y - scroll_count_ver * scroll_interval_ver;

    xx = ((start_tile_hor - top_scroll_tile_hor) * w) - scroll_x;
    yy = -(scroll_y & (h - 1));

    if (layer == 0)
    {
        main_camera_x = camera_x;
        main_camera_y = camera_y;
        window_canvas_x = scroll_x;
        window_canvas_y = scroll_y;
    }

	if (layer == 0)
	{
		int16_t* extrafb = (int16_t*)&MARS_FRAMEBUFFER + 0x100 + ((canvas_pitch * canvas_yaw) >> 1);
		int16_t *dirty = extrafb + 3;
		uint16_t *render_state = (uint16_t *)extrafb;
		drawrenderstate_t render;

		snapshot_render_state(&render);

        MARS_VDP_SHIFTREG = old_camera_x;

        int16_t scrollwords = (scroll_x + (scroll_y * canvas_pitch)) >> 1;        
        if (scrollwords != MARS_FRAMEBUFFER - 0x100)
        {
            Hw32xUpdateLineTable(scroll_x >> 1, scroll_y, 0);
        }

		if (render_state[0] != tm->id || render_state[1] != canvas_rebuild_id)
		{
			/* Full invalidation is rare; clear two entries per longword. */
			sh2_fill16((uint16_t *)dirty,
				canvas_tiles_ver * canvas_tiles_hor, 0xFFFF);
			render_state[0] = tm->id;
			render_state[1] = canvas_rebuild_id;
		}
	}

    drawtileslavecmd_t *slave_cmd = &slave_drawtilecmd;
    drawtilelayerscmd_t *scmd = &slave_cmd->draw;
    slave_cmd->tilemap = *tm;
    scmd->tm = &slave_cmd->tilemap;
    scmd->x = xx;
    scmd->y = yy;
    scmd->start_tile = start_tile;
    scmd->end_tile = end_tile;
    scmd->scroll_tile_id = (start_tile_ver - top_scroll_tile_ver) * canvas_tiles_hor + (start_tile_hor - top_scroll_tile_hor);
    scmd->num_tiles_x = end_tile_hor - start_tile_hor;
    scmd->startlayer = layer;
    scmd->camera_x = camera_x, scmd->camera_y = camera_y;
    if (layer == 0) {
        scmd->drawmode = 0;
    } else{
        scmd->drawmode = DRAWSPR_PRECISE|DRAWSPR_OVERWRITE;
    }
    scmd->parallax = dc->parallax;
    snapshot_render_state(&scmd->render);

    /* Split visible rows into two non-overlapping contiguous stripes. */
    master_cmd = *scmd;
    master_cmd.tm = tm;
    {
        int row_count = (end_tile - start_tile + tm->tiles_hor - 1) /
            tm->tiles_hor;
        int master_rows = (row_count + 1) >> 1;
        int split_tile;

        split_tile = start_tile + master_rows * tm->tiles_hor;

        if (split_tile > end_tile)
            split_tile = end_tile;
        master_cmd.end_tile = split_tile;
        scmd->start_tile = split_tile;
        scmd->y = yy + master_rows * h;
        scmd->scroll_tile_id += master_rows * canvas_tiles_hor;
    }

    while (MARS_SYS_COMM4 != 0);
    MARS_SYS_COMM4 = 3;

    drawcnt = draw_handle_layercmd(&master_cmd);
    while (MARS_SYS_COMM4 == 3);
    drawcnt += ((MARS_SYS_COMM4 >> 2) - 1);
    MARS_SYS_COMM4 = 4;

    *pclipped = clipped;
    return drawcnt;
}

int draw_tilemap(tilemap_t *tm, int fpcamera_x, int fpcamera_y, int *cameraclip, void (*drawspr)(int l, void *p), void *sprp)
{
    uint32_t perf_start = perf_master_ticks();
    int i;
    int clip, drawcnt;
    int ignored_clip;
    char parallax;
    const fixed_t *bplx = tm->layers[0].parallax;
    drawtilecontext_t dc;

    if (!cameraclip)
        cameraclip = &ignored_clip;
    *cameraclip = 0;
    old_camera_x = main_camera_x;
    old_camera_y = main_camera_y;

    if (tm->wrapX) {
        while (fpcamera_x >= tm->wrapX)
            fpcamera_x -= tm->wrapX;
    }
    if (tm->wrapY) {
        while (fpcamera_y >= tm->wrapY)
            fpcamera_y -= tm->wrapY;
    }

    // test for parallax in the upper layers
    parallax = 0;
    for (i = 1; i < tm->numlayers; i++)
    {
        const dtilelayer_t *tl = &tm->layers[i];
        const fixed_t *tplx = tl->parallax;
        if (tplx[0] != bplx[0] || tplx[1] != bplx[1])
        {
            parallax = 1;
            break;
        }
    }

    if (tm->mdPlane[0]->bitmap || tm->mdPlane[1]->bitmap) {
        int md_scroll[4] = { 0, 0, 0, 0 };

        for (i = 0; i < 2; i++) {
            const dtilelayer_t *mdpl = tm->mdPlane[i];

            if (mdpl->bitmap) {
                fixed_t md_camera_x = FixedMul(fpcamera_x,
                    mdpl->parallax[0]) >> 16;
                fixed_t md_camera_y = FixedMul(fpcamera_y,
                    mdpl->parallax[1]) >> 16;

                md_scroll[i * 2] = mdpl->offset[0] + md_camera_x;
                md_scroll[i * 2 + 1] = -mdpl->offset[1] - md_camera_y;
            }
        }

        HwMdSetPlaneScrolls(md_scroll[0], md_scroll[1],
            md_scroll[2], md_scroll[3]);
    }

    dc.tm = tm;
    dc.fpcamera_x = fpcamera_x;
    dc.fpcamera_y = fpcamera_y;
    dc.drawspr = drawspr;
    dc.sprp = sprp;
    dc.sprites_drawn = 0;
    dc.parallax = parallax;

    drawcnt = draw_tile_layer(&dc, 0, cameraclip);
    for (i = 1; i < tm->numlayers; i++)
        drawcnt += draw_tile_layer(&dc, i, &clip);

    if (drawspr && !dc.sprites_drawn)
    {
        drawspr(1, sprp);
        dc.sprites_drawn = 1;
    }

    perf_record(PERF_CPU_MASTER, PERF_METRIC_TILEMAP,
        perf_master_ticks() - perf_start);
    return drawcnt;
}
