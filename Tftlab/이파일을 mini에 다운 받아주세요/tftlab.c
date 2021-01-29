// TFT LCD LAB
//201501485 jounho
#include <stdio.h>
#include <stdlib.h> /* for exit */                                              
#include <wiringPi.h>
#include <unistd.h> /* for open/close .. */
#include <fcntl.h> /* for O_RDWR */                                             
#include <sys/ioctl.h> /* for ioctl */                                          
#include <sys/mman.h> /* for mmap */
#include <linux/fb.h> /* for fb_var_screeninfo, FBIOGET_VSCREENINFO */          
#define FBDEVFILE "/dev/fb2"                                                    

#define DEL 4                                                                   
#define LEFT_MOVE_CURSOR 1 
#define RIGHT_MOVE_CURSOR 5
#define KEY1 7 // prs
#define KEY2 6 // yuv
#define KEY3 11 // wxy
#define KEY4 22 // ghi
#define KEY5 21 // jkl
#define KEY6 26 // mno
#define KEY7 24 // .qz                                                          
#define KEY8 23 // abc
#define KEY9 27 // def

typedef unsigned char ubyte;

typedef struct cursor_point { // (x, y) of cursor
	int x;
	int y;
} CURSOR_POINT;

typedef struct rgb_color
{
	ubyte b; // blue
	ubyte g; // green
	ubyte r; // red
} RGB;

int fbfd;
int ret;
struct fb_var_screeninfo fbvar;
unsigned short pixel;
int offset;
unsigned short* pfbdata;
CURSOR_POINT cursor_pos;
int cursor_offset;
int i, j; // Variables to use for loop
int buttonState = 0;
int count = 0;
int key_num = 3;
int temp_key_num;
int first = 1;
int right_state = 1;
unsigned short s[80][24][24] = { 0, };
int num = 0;
int cursorlocal = 0;
int n;
int in = 0;
int insstate = 0;

unsigned short makepixel(ubyte r, ubyte g, ubyte b);
void initialize(); // initialization function
void btn_push(); // when button is pressed
void draw_font(int x, int y, ubyte* font); // font drawing
void cursor_move_left(); // move the cursor to the left
void cursor_move_right(); // move the cursor to the right
void del(); // delete function
void fr(); // free function
void ins();
int size = 24 * 3 * 24;

ubyte* font_a;
ubyte* font_b;
ubyte* font_c;
ubyte* font_d;
ubyte* font_e;
ubyte* font_f;
ubyte* font_g;
ubyte* font_h;
ubyte* font_i;
ubyte* font_j;
ubyte* font_k;
ubyte* font_l;
ubyte* font_m;
ubyte* font_n;
ubyte* font_o;
ubyte* font_p;
ubyte* font_q;
ubyte* font_r;
ubyte* font_s;
ubyte* font_t;
ubyte* font_u;
ubyte* font_v;
ubyte* font_w;
ubyte* font_x;
ubyte* font_y;
ubyte* font_z;
ubyte* font_dot;

void make_font() {
	// a
	FILE* a = fopen("a.bmp", "rb");
	font_a = malloc(size);
	fseek(a, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_a, size, 1, a); // Read by pixel data size from file
		// b
	FILE* b = fopen("b.bmp", "rb");
	font_b = malloc(size);
	fseek(b, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_b, size, 1, b); // Read by pixel data size from file
		// c
	FILE* c = fopen("c.bmp", "rb");
	font_c = malloc(size);
	fseek(c, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_c, size, 1, c); // Read by pixel data size from file
		// d
	FILE* d = fopen("d.bmp", "rb");
	font_d = malloc(size);
	fseek(d, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_d, size, 1, d); // Read by pixel data size from file
		// e
	FILE* e = fopen("e.bmp", "rb");
	font_e = malloc(size);
	fseek(e, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_e, size, 1, e); // Read by pixel data size from file
		// f
	FILE* f = fopen("f.bmp", "rb");
	font_f = malloc(size);
	fseek(f, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_f, size, 1, f); // Read by pixel data size from file
		// g
	FILE* g = fopen("g.bmp", "rb");
	font_g = malloc(size);
	fseek(g, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_g, size, 1, g); // Read by pixel data size from file
		// h
	FILE* h = fopen("h.bmp", "rb");
	font_h = malloc(size);
	fseek(h, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_h, size, 1, h); // Read by pixel data size from file
		// i
	FILE* i = fopen("i.bmp", "rb");
	font_i = malloc(size);
	fseek(i, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_i, size, 1, i); // Read by pixel data size from file
		// j
	FILE* j = fopen("j.bmp", "rb");
	font_j = malloc(size);
	fseek(j, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_j, size, 1, j); // Read by pixel data size from file
		// k
	FILE* k = fopen("k.bmp", "rb");
	font_k = malloc(size);
	fseek(k, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_k, size, 1, k); // Read by pixel data size from file
		// l
	FILE* l = fopen("l.bmp", "rb");
	font_l = malloc(size);
	fseek(l, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_l, size, 1, l); // Read by pixel data size from file
		// m
	FILE* m = fopen("m.bmp", "rb");
	font_m = malloc(size);
	fseek(m, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_m, size, 1, m); // Read by pixel data size from file
		// n
	FILE* n = fopen("n.bmp", "rb");
	font_n = malloc(size);
	fseek(n, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_n, size, 1, n); // Read by pixel data size from file
		// o
	FILE* o = fopen("o.bmp", "rb");
	font_o = malloc(size);
	fseek(o, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_o, size, 1, o); // Read by pixel data size from file
		// p
	FILE* p = fopen("p.bmp", "rb");
	font_p = malloc(size);
	fseek(p, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_p, size, 1, p); // Read by pixel data size from file
		// q
	FILE* q = fopen("q.bmp", "rb");
	font_q = malloc(size);
	fseek(q, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_q, size, 1, q); // Read by pixel data size from file
		// r
	FILE* r = fopen("r.bmp", "rb");
	font_r = malloc(size);
	fseek(r, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_r, size, 1, r); // Read by pixel data size from file
		// s
	FILE* s = fopen("s.bmp", "rb");
	font_s = malloc(size);
	fseek(s, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_s, size, 1, s); // Read by pixel data size from file
		// t
	FILE* t = fopen("t.bmp", "rb");
	font_t = malloc(size);
	fseek(t, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_t, size, 1, t); // Read by pixel data size from file
		// u
	FILE* u = fopen("u.bmp", "rb");
	font_u = malloc(size);
	fseek(u, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_u, size, 1, u); // Read by pixel data size from file
		// v
	FILE* v = fopen("v.bmp", "rb");
	font_v = malloc(size);
	fseek(v, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_v, size, 1, v); // Read by pixel data size from file
		// w
	FILE* w = fopen("w.bmp", "rb");
	font_w = malloc(size);
	fseek(w, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_w, size, 1, w); // Read by pixel data size from file
		 // x
	FILE* x = fopen("x.bmp", "rb");
	font_x = malloc(size);
	fseek(x, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_x, size, 1, x); // Read by pixel data size from file
		// y
	FILE* y = fopen("y.bmp", "rb");
	font_y = malloc(size);
	fseek(y, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_y, size, 1, y); // Read by pixel data size from file
		// z
	FILE* z = fopen("z.bmp", "rb");
	font_z = malloc(size);
	fseek(z, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_z, size, 1, z); // Read by pixel data size from file
		// dot
	FILE* dot = fopen("dot.bmp", "rb");
	font_dot = malloc(size);
	fseek(dot, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
		fread(font_dot, size, 1, dot); // Read by pixel data size from file
}

unsigned short makepixel(ubyte r, ubyte g, ubyte b) {
	return (unsigned short)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

void initialize() {
	// Set all buttons to input mode & Use pull-up resistance
	pinMode(KEY1, INPUT); pullUpDnControl(KEY1, PUD_DOWN);
	pinMode(KEY2, INPUT); pullUpDnControl(KEY2, PUD_DOWN);
	pinMode(KEY3, INPUT); pullUpDnControl(KEY3, PUD_DOWN);
	pinMode(KEY4, INPUT); pullUpDnControl(KEY4, PUD_DOWN);
	pinMode(KEY5, INPUT); pullUpDnControl(KEY5, PUD_DOWN);
	pinMode(KEY6, INPUT); pullUpDnControl(KEY6, PUD_DOWN);
	pinMode(KEY7, INPUT); pullUpDnControl(KEY7, PUD_DOWN);
	pinMode(KEY8, INPUT); pullUpDnControl(KEY8, PUD_DOWN);
	pinMode(KEY9, INPUT); pullUpDnControl(KEY9, PUD_DOWN);
	pinMode(DEL, INPUT); pullUpDnControl(DEL, PUD_UP);
	pinMode(LEFT_MOVE_CURSOR, INPUT); pullUpDnControl(LEFT_MOVE_CURSOR, PUD_UP);
	pinMode(RIGHT_MOVE_CURSOR, INPUT); pullUpDnControl(RIGHT_MOVE_CURSOR, PUD_UP);
	fbfd = open(FBDEVFILE, O_RDWR);
	if (fbfd < 0) {
		perror("fbdev open");
		exit(1);
	}

	ret = ioctl(fbfd, FBIOGET_VSCREENINFO, &fbvar);
	if (ret < 0) {
		perror("fbdev ioctl");
		exit(1);
	}

	if (fbvar.bits_per_pixel != 16) {
		fprintf(stderr, "bpp is not 16\n");
		exit(1);
	}
	pfbdata = (unsigned short*)mmap(0, fbvar.xres * fbvar.yres * 16 / 8, PROT_READ |PROT_WRITE, MAP_SHARED, fbfd, 0);
	if ((unsigned)pfbdata == (unsigned)-1) {
		perror("fbdev mmap");
		exit(1);
	}

	// Initialize lcd screen
	pixel = makepixel(0, 0, 0); // black
	for (i = 0; i < 320 * 240; i++) {
		*(pfbdata + i) = pixel;
	}

	// Set cursor position to first start position
	pixel = makepixel(255, 255, 255); // white
	cursor_pos.x = 22;
	cursor_pos.y = 35;
	cursor_offset = cursor_pos.y * fbvar.xres + cursor_pos.x; // (22, 35)
	for (i = 0; i < 24; i++) {
		*(pfbdata + cursor_offset + i) = pixel;
	}

	make_font();
}

void btn_push() {
	key_num = 3;
	buttonState = (digitalRead(KEY1) || digitalRead(KEY2) || digitalRead(KEY3) || digitalRead(KEY4) || digitalRead(KEY5)|| digitalRead(KEY6) || digitalRead(KEY7) || digitalRead(KEY8) || digitalRead(KEY9));
	if (digitalRead(KEY1)) key_num = 7;
	if (digitalRead(KEY2)) key_num = 6;
	if (digitalRead(KEY3)) key_num = 11;
	if (digitalRead(KEY4)) key_num = 22;
	if (digitalRead(KEY5)) key_num = 21;
	if (digitalRead(KEY6)) key_num = 26;
	if (digitalRead(KEY7)) key_num = 24;
	if (digitalRead(KEY8)) key_num = 23;
	if (digitalRead(KEY9)) key_num = 27;

	if (buttonState == 1 && cursorlocal < num) {
		in = 1;
	}
	else if (buttonState == 1) {
		if (insstate == 1) {
			cursor_move_right();
			insstate = 0;
		}
		in = 0;
		count++;
		if (key_num != temp_key_num && first != 1) {
			if (right_state == 1) {
				cursor_move_right();
				count = 1;
			}
			right_state = 1;
		}
		right_state = 1;
		first = 2;
		temp_key_num = key_num;
	}
	delay(50);

	if ((count % 3) == 1) {
		if (key_num == 7) draw_font(cursor_pos.x, cursor_pos.y - 25, font_p);
		if (key_num == 6) draw_font(cursor_pos.x, cursor_pos.y - 25, font_t);
		if (key_num == 11) draw_font(cursor_pos.x, cursor_pos.y - 25, font_w);
		if (key_num == 22) draw_font(cursor_pos.x, cursor_pos.y - 25, font_g);
		if (key_num == 21) draw_font(cursor_pos.x, cursor_pos.y - 25, font_j);
		if (key_num == 26) draw_font(cursor_pos.x, cursor_pos.y - 25, font_m);
		if (key_num == 24) draw_font(cursor_pos.x, cursor_pos.y - 25, font_dot);
		if (key_num == 23) draw_font(cursor_pos.x, cursor_pos.y - 25, font_a);
		if (key_num == 27) draw_font(cursor_pos.x, cursor_pos.y - 25, font_d);
	}

	if ((count % 3) == 2) {
		if (key_num == 7) draw_font(cursor_pos.x, cursor_pos.y - 25, font_r);
		if (key_num == 6) draw_font(cursor_pos.x, cursor_pos.y - 25, font_u);
		if (key_num == 11) draw_font(cursor_pos.x, cursor_pos.y - 25, font_x);
		if (key_num == 22) draw_font(cursor_pos.x, cursor_pos.y - 25, font_h);
		if (key_num == 21) draw_font(cursor_pos.x, cursor_pos.y - 25, font_k);
		if (key_num == 26) draw_font(cursor_pos.x, cursor_pos.y - 25, font_n);
		if (key_num == 24) draw_font(cursor_pos.x, cursor_pos.y - 25, font_q);
		if (key_num == 23) draw_font(cursor_pos.x, cursor_pos.y - 25, font_b);
		if (key_num == 27) draw_font(cursor_pos.x, cursor_pos.y - 25, font_e);
	}

	if ((count % 3) == 0) {
		if (key_num == 7) draw_font(cursor_pos.x, cursor_pos.y - 25, font_s);
		if (key_num == 6) draw_font(cursor_pos.x, cursor_pos.y - 25, font_v);
		if (key_num == 11) draw_font(cursor_pos.x, cursor_pos.y - 25, font_y);
		if (key_num == 22) draw_font(cursor_pos.x, cursor_pos.y - 25, font_i);
		if (key_num == 21) draw_font(cursor_pos.x, cursor_pos.y - 25, font_l);
		if (key_num == 26) draw_font(cursor_pos.x, cursor_pos.y - 25, font_o);
		if (key_num == 24) draw_font(cursor_pos.x, cursor_pos.y - 25, font_z);
		if (key_num == 23) draw_font(cursor_pos.x, cursor_pos.y - 25, font_c);
		if (key_num == 27) draw_font(cursor_pos.x, cursor_pos.y - 25, font_f);
	}

	delay(50);

	if (!digitalRead(LEFT_MOVE_CURSOR) || !digitalRead(RIGHT_MOVE_CURSOR) ||!digitalRead(DEL)) {
		if (!digitalRead(LEFT_MOVE_CURSOR)) {
			cursor_move_left();
		}
		if (!digitalRead(RIGHT_MOVE_CURSOR)) {
			cursor_move_right();
			key_num = 3;
			right_state = 2;
		}
		if (!digitalRead(DEL)) {
			del();
			key_num = 3;
			right_state = 2;
		}
	}
}

void draw_font(int x, int y, ubyte* font) {
	int line = 0;
	if (in == 0) {
		for (j = 23; j >= 0; j--) {
			for (i = 0; i < 24; i++) {
				int index = (i * 3) + (j * (24 * 3));
				RGB* upixel = (RGB*)& font[index];
				pixel = makepixel(abs((upixel->b) - 255), abs((upixel->g) - 255), abs((upixel->r) - 255));
				s[num][j][i] = pixel;
				offset = y * fbvar.xres + x;
				*(pfbdata + offset + (line * fbvar.xres + i)) =pixel;
			}
			line++;
		}
	}
	else {
		x = x + 28;
		ins(font);
		for (j = 23; j >= 0; j--) {
			for (i = 0; i < 24; i++) {
				int index = (i * 3) + (j * (24 * 3));
				RGB* upixel = (RGB*)& font[index];
				pixel = makepixel(abs((upixel->b) - 255), abs((upixel->g) - 255), abs((upixel->r) - 255));
				s[cursorlocal + 1][j][i] = pixel;
				offset = y * fbvar.xres + x;
				*(pfbdata + offset + (line * fbvar.xres + i)) = pixel;
			}
			line++;
		}
	}
}

void cursor_move_left() {
	count = 0;
	pixel = makepixel(0, 0, 0); // Background color
	for (i = 0; i < 24; i++) { // Clear Cursors in Current Location
		*(pfbdata + cursor_offset + i) = pixel;
	}

	cursor_pos.x -= 28; // Move the cursor to the left

	if (cursor_pos.x < 22 && cursor_pos.y == 35) { // If the cursor is out of position for the first time
			cursor_pos.x = 274;
		cursor_pos.y = 231;
	}
	else if (cursor_pos.x < 22) { // If the cursor crosses the left end of aline
			cursor_pos.x = 274;
		cursor_pos.y -= 32;
	}

	cursor_offset = cursor_pos.y * fbvar.xres + cursor_pos.x;
	pixel = makepixel(255, 255, 255); // font color
	for (i = 0; i < 24; i++) { // Rewrite Cursors
		*(pfbdata + cursor_offset + i) = pixel;
	}
	delay(30);
	key_num = 3;
	cursorlocal--;
}

void cursor_move_right() {
	count = 0;
	pixel = makepixel(0, 0, 0); // Background color
	for (i = 0; i < 24; i++) { // Clear Cursors in Current Location
		*(pfbdata + cursor_offset + i) = pixel;
	}

	cursor_pos.x += 28; // Move the cursor to the right

	if (cursor_pos.x > 298 && cursor_pos.y == 231) { // If the cursor is outof position for the last time
			cursor_pos.x = 22;
		cursor_pos.y = 35;
	}
	else if (cursor_pos.x > 298) { // If the cursor crosses the right end ofa line
			cursor_pos.x = 22;
		cursor_pos.y += 32;
	}

	cursor_offset = cursor_pos.y * fbvar.xres + cursor_pos.x; // cursor offset update
		pixel = makepixel(255, 255, 255); // font color
	for (i = 0; i < 24; i++) { // Rewrite Cursors
		*(pfbdata + cursor_offset + i) = pixel;
	}
	delay(50);
	if (cursorlocal == num)
		num++;
	cursorlocal++;
}

void ins() {
	insstate = 1;
	int line = 0;
	int linenum = 0;
	int temp_posx = cursor_pos.x;
	cursor_pos.x = cursor_pos.x + 28;
	for (n = num + 1; n > cursorlocal + 1; n--) {
		for (j = 23; j >= 0; j--) {
			for (i = 0; i < 24; i++) {
				pixel = makepixel(0, 0, 0);
				offset = (cursor_pos.y - 25) * fbvar.xres + cursor_pos.x;
				*(pfbdata + offset + (line * fbvar.xres + i)) = pixel;
				s[n][j][i] = s[n - 1][j][i];
			}
			line++;
		}
		delay(30);
		line = 0;
		cursor_pos.x += 28;
		if (cursor_pos.x > 298) {
			cursor_pos.x = 22;
			cursor_pos.y += 32;
			linenum++;
		}
	}
	cursor_pos.x = temp_posx;
	cursor_pos.y = cursor_pos.y - 32 * linenum;
	num++;
	cursor_pos.x = cursor_pos.x + 28;
	linenum = 0;
	for (n = cursorlocal + 1; n < num + 1; n++) {
		for (j = 23; j >= 0; j--) {
			for (i = 0; i < 24; i++) {
				offset = (cursor_pos.y - 25) * fbvar.xres + cursor_pos.x;

				*(pfbdata + offset + (line * fbvar.xres + i)) = s[n][j][i];
			}
			line++;
		}
		line = 0;
		cursor_pos.x += 28;
		if (cursor_pos.x > 298) {
			cursor_pos.x = 22;
			cursor_pos.y += 32;
			linenum++;
		}
	}
	cursor_pos.x = temp_posx;
	cursor_pos.y = cursor_pos.y - 32 * linenum;
}


void del() {
	int line = 0;
	int linenum = 0;
	int temp_posx = cursor_pos.x;
	if (cursorlocal == num) {
		for (j = 23; j >= 0; j--) {
			for (i = 0; i < 24; i++) {
				int index = (i * 3) + (j * (24 * 3));
				pixel = makepixel(0, 0, 0);
				offset = (cursor_pos.y - 25) * fbvar.xres + cursor_pos.x;
				*(pfbdata + offset + (line * fbvar.xres + i)) =pixel;
				s[cursorlocal][j][i] = 0;
			}
			line++;
		}
	}
	else {
		line = 0;
		for (n = cursorlocal; n < num + 1; n++) {
			for (j = 23; j >= 0; j--) {
				for (i = 0; i < 24; i++) {
					pixel = makepixel(0, 0, 0);
					offset = (cursor_pos.y - 25) * fbvar.xres + cursor_pos.x;
					*(pfbdata + offset + (line * fbvar.xres + i)) =pixel;

					s[n][j][i] = s[n + 1][j][i];
				}
				line++;
			}
			delay(3);
			line = 0;
			cursor_pos.x += 28;
			if (cursor_pos.x > 298) {
				cursor_pos.x = 22;
				cursor_pos.y += 32;
				linenum++;
			}
		}
		cursor_pos.x = temp_posx;
		cursor_pos.y = cursor_pos.y - 32 * linenum;
		num--;

		linenum = 0;
		for (n = cursorlocal; n < num + 1; n++) {
			for (j = 23; j >= 0; j--) {
				for (i = 0; i < 24; i++) {
					offset = (cursor_pos.y - 25) * fbvar.xres + cursor_pos.x;

					*(pfbdata + offset + (line * fbvar.xres + i)) =s[n][j][i];
				}
				line++;
			}
			line = 0;
			cursor_pos.x += 28;
			if (cursor_pos.x > 298) {
				cursor_pos.x = 22;
				cursor_pos.y += 32;
				linenum++;
			}
		}
		cursor_pos.x = temp_posx;
		cursor_pos.y = cursor_pos.y - 32 * linenum;
	}
	count = 0;
	temp_key_num = 3;
}
void fr() {
	munmap(pfbdata, fbvar.xres * fbvar.yres * 16 / 8);
	close(fbfd);
	free(font_a);
	free(font_b);
	free(font_c);
	free(font_d);
	free(font_e);
	free(font_f);
	free(font_g);
	free(font_h);
	free(font_i);
	free(font_j);
	free(font_k);
	free(font_l);
	free(font_m);
	free(font_n);
	free(font_o);
	free(font_p);
	free(font_q);
	free(font_r);
	free(font_s);
	free(font_t);
	free(font_u);
	free(font_v);
	free(font_w);
	free(font_x);
	free(font_y);
	free(font_z);
	free(font_dot);
}

int main(int argc, char** argv) {
	wiringPiSetup();

	initialize();
	while (1) {
		btn_push();
	}

	fr();
	return 0;
}
