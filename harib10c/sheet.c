/* マウスやウィンドウの重ね合わせ処理 */

#include "bootpack.h"

#define SHEET_USE		1

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof (struct SHTCTL));
	if (ctl == 0) {
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1; /* シートは一枚もない */
	for (i = 0; i < MAX_SHEETS; i++) {
		ctl->sheets0[i].flags = 0; /* 未使用マーク */
	}
err:
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for (i = 0; i < MAX_SHEETS; i++) {
		if (ctl->sheets0[i].flags == 0) {
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE; /* 使用中マーク */
			sht->height = -1; /* 非表示中 */
			return sht;
		}
	}
	return 0;	/* 全てのシートが使用中だった */
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}

void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
	int h, old = sht->height; /* 設定前の高さを記憶する */

	/* 指定が低すぎや高すぎだったら、修正する */
	if (height > ctl->top + 1) {
		height = ctl->top + 1;
	}
	if (height < -1) {
		height = -1;
	}
	sht->height = height; /* 高さを設定 */

	/* 以下は主にsheets[]の並べ替え */
	if (old > height) {	/* 以前よりも低くなる */
		if (height >= 0) {
			/* 間のものを引き上げる */
			for (h = old; h > height; h--) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {	/* 非表示化 */
			if (ctl->top > old) {
				/* 上になっているものをおろす */
				for (h = old; h < ctl->top; h++) {
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--; /* 表示中の下じきが一つ減るので、一番上の高さが減る */
		}
		sheet_refresh(ctl); /* 新しい下じきの情報に沿って画面を描き直す */
	} else if (old < height) {	/* 以前よりも高くなる */
		if (old >= 0) {
			/* 間のものを押し下げる */
			for (h = old; h < height; h++) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {	/* 非表示状態から表示状態へ */
			/* 上になるものを持ち上げる */
			for (h = ctl->top; h >= height; h--) {
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++; /* 表示中の下じきが一つ増えるので、一番上の高さが増える */
		}
		sheet_refresh(ctl); /* 新しい下じきの情報に沿って画面を描き直す */
	}
	return;
}

void sheet_refresh(struct SHTCTL *ctl)
{
	int h, bx, by, vx, vy;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	for (h = 0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		for (by = 0; by < sht->bysize; by++) {
			vy = sht->vy0 + by;
			for (bx = 0; bx < sht->bxsize; bx++) {
				vx = sht->vx0 + bx;
				c = buf[by * sht->bxsize + bx];
				if (c != sht->col_inv) {
					vram[vy * ctl->xsize + vx] = c;
				}
			}
		}
	}
	return;
}

int min(int x1, int x2) {
	if (x1 < x2) {
		return x1;
	}
	return x2;
}

int max(int x1, int x2) {
	if (x1 > x2) {
		return x1;
	}
	return x2;
}

struct REGION clip_region_small(struct REGION *r1, struct REGION *r2) {
	struct REGION res;
	res.x0 = max(r1->x0, r2->x0);
	res.y0 = max(r1->y0, r2->y0);
	res.x1 = min(r1->x1, r2->x1);
	res.y1 = min(r1->y1, r2->y1);
	return res;
}

struct REGION clip_region_big(struct REGION *r1, struct REGION *r2) {
	struct REGION res;
	res.x0 = min(r1->x0, r2->x0);
	res.y0 = min(r1->y0, r2->y0);
	res.x1 = max(r1->x1, r2->x1);
	res.y1 = max(r1->y1, r2->y1);
	return res;
}

int region_is_valid(struct REGION *r) {
	if (r->x0 >= r->x1 || r->y0 >= r->y1) {
		return 0;
	}
	return 1;
}

void sheet_redraw(struct SHTCTL *ctl, struct REGION *r)
{
	int h, bx, by, vx, vy;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;

	for (h = 0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;

		// 找到受影響的區域
		struct REGION sht_r = {sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize};
		struct REGION res_r = clip_region_small(&sht_r, r);
		if (!region_is_valid(&res_r))
			continue;
		
		for (vy = res_r.y0; vy < res_r.y1; vy++) {
			by = vy - sht->vy0;
			if (by < 0)
				continue;
			if (by >= sht->bysize || by >= ctl->ysize) {
				break;
			}
			for (vx = res_r.x0; vx < res_r.x1; vx++) {
				bx = vx - sht->vx0;
				if (bx < 0)
					continue;
				if (bx >= sht->bxsize || vx >= ctl->xsize)
					break;
				c = buf[by * sht->bxsize + bx];
				if (c != sht->col_inv) {
					vram[vy * ctl->xsize + vx] = c;
				}
			}
		}

	}
	return;
}

void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
	struct REGION old_r = {sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize};
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	struct REGION new_r = {sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize};

	if (sht->height >= 0) { /* もしも表示中なら */
		struct REGION res_r = clip_region_big(&old_r, &new_r);
		if (!region_is_valid(&res_r))
			return;
		sheet_redraw(ctl, &res_r); /* 新しい下じきの情報に沿って画面を描き直す */
	}
	return;
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	if (sht->height >= 0) {
		sheet_updown(ctl, sht, -1); /* 表示中ならまず非表示にする */
	}
	sht->flags = 0; /* 未使用マーク */
	return;
}
