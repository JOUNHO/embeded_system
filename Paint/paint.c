#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <math.h>

#define _USE_MATH_DEFINES // to use PI
#define FBDEVFILE "/dev/fb2"

//@ Data type redefinition & Structure declaration
typedef unsigned char ubyte;

typedef unsigned short Color;

typedef struct rgb_color {
	ubyte b; // blue
	ubyte g; // green
	ubyte r; // red
} RGB;

typedef struct point {
	int x;
	int y;
} Point;

typedef struct nodeData {
	Point* coord; // Pointer to save all coordinates of the shape
	int type; // Shape type 1:Line, 2: pen Rectangle, 3:fill Rectangle, 4:pen Oval, 5:fill Oval, 6:Free draw
	Color shape_color; // Shape color
} NodeData;

typedef struct ListNode {
	NodeData data; // data
	struct ListNode *link; // next node
} ListNode;


int fd;
int fbfd;
int ret;
struct fb_var_screeninfo fbvar;
struct input_event ie;
unsigned short *pfbdata;
unsigned short pixel;
int offset;
ubyte *UI;
int img_size = 320 * 240 * 3;
int i, j, x, y; // variables to use for loop
Point cur; // current TouchScreen coordinate value
Point first; // when you first touched it
Point end; // when you touch off
Point unmap_first;
Point mcur;
Point pre;
int is_touched = 0;
int is_first_touched = 1;
Color color;
ListNode *shapeList = NULL;
int length = 0; // list length
int is_moved = 0; // drag
Point lineStart = { .x = 10,.y = 10 }; // Coordinate values of the beginning of each button
Point rectStart = { .x = 10,.y = 42 };
Point ovalStart = { .x = 10,.y = 74 };
Point frdrawStart = { .x = 10,.y = 106 };
Point selStart = { .x = 10,.y = 138 };
Point eraseStart = { .x = 10,.y = 170 };
Point clearStart = { .x = 10,.y = 202 };
Point whiteStart = { .x = 256,.y = 10 };
Point redStart = { .x = 256,.y = 39 };
Point yellowStart = { .x = 256,.y = 68 };
Point lightblueStart = { .x = 256,.y = 97 };
Point orangeStart = { .x = 285,.y = 10 };
Point greenStart = { .x = 285,.y = 39 };
Point darkblueStart = { .x = 285,.y = 68 };
Point blackStart = { .x = 285,.y = 97 };
Point penStart = { .x = 257,.y = 170 };
Point fillStart = { .x = 257,.y = 202 };
int lineFlag = 0; // Flag of each button
int rectFlag = 0;
int ovalFlag = 0;
int frdrawFlag = 0;
int selFlag = 0;
int penFlag = 0;
int fillFlag = 0;
int eraseFlag = 0;
int paintFlag = 0;
float a, b, c, d, e, f, k; // car value

//@ Function declaration
unsigned short makepixel(ubyte r, ubyte g, ubyte b); // color pixel making
void make_UI_img(); // make UI img
void print_UI(); // print UI
void initialize(); // initialization function
void calibration(); // get a, b, c, d, e, f, k value
Point mapping(Point tp); // Mapping function : ts coordinate -> LCD coordinate
void loop();
void touch_split(); // 
void touch_off_split(); // touch branch function
void print_func_btn_pushed(Point func_start); // Functioning button border painting
void print_func_btn_pulled(Point func_start);
void print_color_btn_pushed(Point color_start); // Color button border painting
void print_color_btn_pulled(Point color_start);
void set_left_func_flag(int line, int rect, int oval, int frdraw, int sel, int erase); // set flags
void set_pen_mode_flag(int pen, int fill);
void clear(); // paint board clear
void check_flag(); // flag check
void drawLine();
void drawRect(); 
void drawRectFill(); 
void drawOval(); 
void drawOvalFill();
void drawFrdraw();
void erase();
void interpolation(int x0, int x1, int y0, int y1); // interpolation

//@ Function definition
unsigned short makepixel(ubyte r, ubyte g, ubyte b) {
	return (unsigned short)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

void make_UI_img() {
	FILE *img = fopen("UI.bmp", "rb");
	if (img == NULL) {
		perror("img open");
		exit(1);
	}
	UI = (ubyte *)malloc(img_size);
	fseek(img, 54, SEEK_SET); // Move the file pointer to the starting position of the pixel data
	fread(UI, img_size, 1, img); // Read by pixel data size from file
}

void print_UI() {
	int line = 0;
	for (j = 239; j >= 0; j--) {
		for (i = 0; i < 320; i++) {
			int index = (i * 3) + (j * (320 * 3));
			RGB *upixel = (RGB *)&UI[index];
			pixel = makepixel(upixel->r, upixel->g, upixel->b);
			*(pfbdata + (line*fbvar.xres + i)) = pixel;
		}
		line++;
	}
}

void initialize() {
	// setting
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
	fd = open("/dev/input/event4", O_RDONLY);
	if (fd < 0) {
		perror("fd open");
		exit(1);
	}
	pfbdata = (unsigned short *)mmap(0, fbvar.xres*fbvar.yres * 16 / 8, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if ((unsigned)pfbdata == (unsigned)-1) {
		perror("fbdev mmap");
		exit(1);
	}
	// initialize lcd screen
	pixel = makepixel(0, 0, 0); // black
	for (i = 0; i < 320 * 240; i++) {
		*(pfbdata + i) = pixel;
	}
	// calibration
	calibration();
	// make UI
	make_UI_img();
	// print UI
	print_UI();
}

void calibration() {
	int xd0, yd0, xd1, yd1, xd2, yd2; // LCD coordinates
	int x0, y0, x1, y1, x2, y2; // TouchScreen coordinates
	pixel = makepixel(255, 255, 255); // white
	int cnt = 0;

	while (1) {
		/* (50,50) */
		xd0 = 50;
		yd0 = 50;
		offset = 50 * fbvar.xres + 50;
		for (i = -5; i < 5; i++) { // draw cross
			*(pfbdata + offset + i) = pixel;
			*(pfbdata + offset + i * fbvar.xres) = pixel;
		}

		if (cnt == 1) {
			/* (250, 100) */
			xd1 = 250;
			yd1 = 100;
			offset = 100 * fbvar.xres + 250;
			for (i = -5; i < 5; i++) {
				*(pfbdata + offset + i) = pixel;
				*(pfbdata + offset + i * fbvar.xres) = pixel;
			}
		}
		else if (cnt == 2) {
			/* (150, 200) */
			xd2 = 150;
			yd2 = 200;
			offset = 200 * fbvar.xres + 150;
			for (i = -5; i < 5; i++) {
				*(pfbdata + offset + i) = pixel;
				*(pfbdata + offset + i * fbvar.xres) = pixel;
			}
		}
		else if (cnt > 2) {
			k = (x0 - x2)*(y1 - y2) - (x1 - x2)*(y0 - y2);
			a = ((xd0 - xd2)*(y1 - y2) - (xd1 - xd2)*(y0 - y2)) / k;
			b = ((x0 - x2)*(xd1 - xd2) - (xd0 - xd2)*(x1 - x2)) / k;
			c = (y0*(x2*xd1 - x1 * xd2) + y1 * (x0*xd2 - x2 * xd0) + y2 * (x1*xd0 - x0 * xd1)) / k;
			d = ((yd0 - yd2)*(y1 - y2) - (yd1 - yd2)*(y0 - y2)) / k;
			e = ((x0 - x2)*(yd1 - yd2) - (yd0 - yd2)*(x1 - x2)) / k;
			f = (y0*(x2*yd1 - x1 * yd2) + y1 * (x0*yd2 - x2 * yd0) + y2 * (x1*yd0 - x0 * yd1)) / k;

			break;
		}

		read(fd, &ie, sizeof(struct input_event));
		if (ie.type == EV_KEY) {
			if (ie.code == BTN_TOUCH) {
				if (ie.value == 1) { // touched
					if (cnt == 0) {
						read(fd, &ie, sizeof(struct input_event));
						if (ie.type == EV_ABS) {
							if (ie.code == ABS_X) {
								x0 = ie.value;
							}
						}
						read(fd, &ie, sizeof(struct input_event));
						if (ie.type == EV_ABS) {
							if (ie.code == ABS_Y) {
								y0 = ie.value;
							}
						}
					}
					else if (cnt == 1) {
						read(fd, &ie, sizeof(struct input_event));
						if (ie.type == EV_ABS) {
							if (ie.code == ABS_X) {
								x1 = ie.value;
							}
						}
						read(fd, &ie, sizeof(struct input_event));
						if (ie.type == EV_ABS) {
							if (ie.code == ABS_Y) {
								y1 = ie.value;
							}
						}
					}
					else if (cnt == 2) {
						read(fd, &ie, sizeof(struct input_event));
						if (ie.type == EV_ABS) {
							if (ie.code == ABS_X) {
								x2 = ie.value;
							}
						}
						read(fd, &ie, sizeof(struct input_event));
						if (ie.type == EV_ABS) {
							if (ie.code == ABS_Y) {
								y2 = ie.value;
							}
						}
					}

					cnt++;
				}
			}
		}
	}
}

Point mapping(Point tp) {
	Point lp;
	lp.x = (int)(a*tp.x + b * tp.y + c);
	lp.y = (int)(d*tp.x + e * tp.y + f);
	return lp;
}

void loop() {
	while (1) {
		while (is_touched != 1) { // wait for input
			read(fd, &ie, sizeof(struct input_event));
			if (ie.type == EV_KEY) {
				if (ie.code == BTN_TOUCH) {
					is_touched = ie.value;
				}
			}
		}

		if (is_touched == 1 && is_first_touched == 1) { // save fisrt point on first touch
			read(fd, &ie, sizeof(struct input_event));
			if (ie.type == EV_ABS) {
				if (ie.code == ABS_X) {
					unmap_first.x = ie.value;
				}
			}
			read(fd, &ie, sizeof(struct input_event));
			if (ie.type == EV_ABS) {
				if (ie.code == ABS_Y) {
					unmap_first.y = ie.value;
				}
			}
			first = mapping(unmap_first);
			pre = first;
			is_first_touched = 0;
		}

		touch_split();

		while (is_moved != 1) {
			read(fd, &ie, sizeof(struct input_event));
			if (ie.type == EV_ABS) {
				if (ie.code == ABS_X) {
					cur.x = ie.value;
				}
			}
			read(fd, &ie, sizeof(struct input_event));
			if (ie.type == EV_ABS) {
				if (ie.code == ABS_Y) {
					cur.y = ie.value;
				}
			}
			if ((unmap_first.x != cur.x) && (unmap_first.y != cur.y)) {
				is_moved = 1;
			}
		}

		while (is_touched != 0) { // repeat until touch is removed and save cur coordinates
			read(fd, &ie, sizeof(struct input_event));
			if (ie.type == EV_ABS) {
				if (ie.code == ABS_X) cur.x = ie.value;
			}
			if (ie.type == EV_KEY) {
				if (ie.code == BTN_TOUCH) is_touched = ie.value;
			}
			read(fd, &ie, sizeof(struct input_event));
			if (ie.type == EV_ABS) {
				if (ie.code == ABS_Y) cur.y = ie.value;
			}
			if (ie.type == EV_KEY) {
				if (ie.code == BTN_TOUCH) is_touched = ie.value;
			}
			mcur = mapping(cur);
			drawFrdraw();
			pre = mcur;
			erase();
			// printf("CUR -> x : %d, y : %d\n", mcur.x, mcur.y);
		}

		if (is_touched == 0) { // save instantaneous coordinates when touched off
			end = mcur;
			is_first_touched = 1;
			is_moved = 0;
			// printf("END -> x : %d, y : %d\n", end.x, end.y);
		}

		touch_off_split();
	}
}

void touch_split() {
	if (first.x > 10 && first.x < 62) {
		if (first.y > 10 && first.y < 37) {
			print_func_btn_pushed(lineStart); // Line
		}
		else if (first.y > 42 && first.y < 69) {
			print_func_btn_pushed(rectStart); // Rectangle
		}
		else if (first.y > 74 && first.y < 101) {
			print_func_btn_pushed(ovalStart); // Oval
		}
		else if (first.y > 106 && first.y < 133) {
			print_func_btn_pushed(frdrawStart); // Free draw
		}
		else if (first.y > 138 && first.y < 165) {
			print_func_btn_pushed(selStart); // Select
		}
		else if (first.y > 170 && first.y < 197) {
			print_func_btn_pushed(eraseStart); // Erase
		}
		else if (first.y > 202 && first.y < 229) {
			print_func_btn_pushed(clearStart); // Clear
		}
	}
	// paint board
	if (first.x > 70 && first.x < 248) {
		if (first.y > 11 && first.y < 228) {
			paintFlag = 1;
		}
	}
	if (256 < first.x && first.x < 280) {
		if (10 < first.y && first.y < 40) {
			print_color_btn_pushed(whiteStart); // white
		}
		else if (42 < first.y && first.y < 66) {
			print_color_btn_pushed(redStart); // red
		}
		else if (68 < first.y && first.y < 92) {
			print_color_btn_pushed(yellowStart); // yellow
		}
		else if (97 < first.y && first.y < 121) {
			print_color_btn_pushed(lightblueStart); // lightblue
		}
	}
	if (285 < first.x && first.x < 309) {
		if (10 < first.y && first.y < 40) {
			print_color_btn_pushed(orangeStart); // orange
		}
		else if (42 < first.y && first.y < 66) {
			print_color_btn_pushed(greenStart); // green
		}
		else if (68 < first.y && first.y < 92) {
			print_color_btn_pushed(darkblueStart); // darkblue
		}
		else if (97 < first.y && first.y < 121) {
			print_color_btn_pushed(blackStart); // black
		}
	}
	if (257 < first.x && first.x < 309) {
		if (170 < first.y && first.y < 197) {
			print_func_btn_pushed(penStart); // Pen
		}
		else if (202 < first.y && first.y < 229) {
			print_func_btn_pushed(fillStart); // Fill
		}
	}
}

void touch_off_split() {
	if (first.x > 10 && first.x < 62) {
		if (first.y > 10 && first.y < 37) { // Line
			if ((10 < end.x && end.x < 62) && (10 < end.y && end.y < 37)) { // If the beginning and the end are the same
				set_left_func_flag(1, 0, 0, 0, 0, 0);
				print_func_btn_pulled(rectStart);
				print_func_btn_pulled(ovalStart);
				print_func_btn_pulled(frdrawStart);
				print_func_btn_pulled(selStart);
				print_func_btn_pulled(eraseStart);
			}
			else {
				print_func_btn_pulled(lineStart);
			}
		}
		else if (first.y > 42 && first.y < 69) { // Rectangle
			if ((10 < end.x && end.x < 62) && (42 < end.y && end.y < 69)) {
				set_left_func_flag(0, 1, 0, 0, 0, 0);
				print_func_btn_pulled(lineStart);
				print_func_btn_pulled(ovalStart);
				print_func_btn_pulled(frdrawStart);
				print_func_btn_pulled(selStart);
				print_func_btn_pulled(eraseStart);
			}
			else {
				print_func_btn_pulled(rectStart);
			}
		}
		else if (first.y > 74 && first.y < 101) { // Oval
			if ((10 < end.x && end.x < 62) && (74 < end.y && end.y < 101)) {
				set_left_func_flag(0, 0, 1, 0, 0, 0);
				print_func_btn_pulled(rectStart);
				print_func_btn_pulled(lineStart);
				print_func_btn_pulled(frdrawStart);
				print_func_btn_pulled(selStart);
				print_func_btn_pulled(eraseStart);
			}
			else {
				print_func_btn_pulled(ovalStart);
			}
		}
		else if (first.y > 106 && first.y < 133) { // Free draw
			if ((10 < end.x && end.x < 62) && (106 < end.y && end.y < 133)) {
				set_left_func_flag(0, 0, 0, 1, 0, 0);
				print_func_btn_pulled(rectStart);
				print_func_btn_pulled(ovalStart);
				print_func_btn_pulled(lineStart);
				print_func_btn_pulled(selStart);
				print_func_btn_pulled(eraseStart);
			}
			else {
				print_func_btn_pulled(frdrawStart);
			}
		}
		else if (first.y > 138 && first.y < 165) { // Select
			if ((10 < end.x && end.x < 62) && (138 < end.y && end.y < 165)) {
				set_left_func_flag(0, 0, 0, 0, 1, 0);
				print_func_btn_pulled(rectStart);
				print_func_btn_pulled(ovalStart);
				print_func_btn_pulled(frdrawStart);
				print_func_btn_pulled(lineStart);
				print_func_btn_pulled(eraseStart);
			}
			else {
				print_func_btn_pulled(selStart);
			}
		}
		else if (first.y > 170 && first.y < 197) { // Erase
			if ((10 < end.x && end.x < 62) && (170 < end.y && end.y < 197)) {
				set_left_func_flag(0, 0, 0, 0, 0, 1);
				print_func_btn_pulled(rectStart);
				print_func_btn_pulled(ovalStart);
				print_func_btn_pulled(frdrawStart);
				print_func_btn_pulled(lineStart);
				print_func_btn_pulled(selStart);
			}
			else {
				print_func_btn_pulled(eraseStart);
			}
		}
		else if (first.y > 202 && first.y < 229) { // Clear
			if ((10 < end.x && end.x < 62) && (202 < end.y && end.y < 229)) {
				clear(); // clear
				print_func_btn_pulled(clearStart);
			}
			else {
				print_func_btn_pulled(clearStart);
			}
		}
	}
	if (first.x > 70 && first.x < 248) {
		if (first.y > 11 && first.y < 228) {
			if ((70 < end.x && end.x < 248) && (11 < end.y && end.y < 228)) {
				check_flag(); // Check Flag for Branch
			}
		}
	}
	if (256 < first.x && first.x < 280) {
		if (10 < first.y && first.y < 40) { 
			if ((256 < end.x && end.x < 280) && (10 < end.y && end.y < 40)) {
				color = makepixel(255, 255, 255); 
				print_color_btn_pulled(redStart);
				print_color_btn_pulled(yellowStart);
				print_color_btn_pulled(lightblueStart);
				print_color_btn_pulled(orangeStart);
				print_color_btn_pulled(greenStart);
				print_color_btn_pulled(darkblueStart);
				print_color_btn_pulled(blackStart);
				delay(100);
			}
			else {
				print_color_btn_pulled(whiteStart);
				delay(100);
			}
		}
		else if (42 < first.y && first.y < 66) {
			if ((256 < end.x && end.x < 280) && (42 < end.y && end.y < 66)) {
				color = makepixel(255, 0, 0);
				print_color_btn_pulled(whiteStart);
				print_color_btn_pulled(yellowStart);
				print_color_btn_pulled(lightblueStart);
				print_color_btn_pulled(orangeStart);
				print_color_btn_pulled(greenStart);
				print_color_btn_pulled(darkblueStart);
				print_color_btn_pulled(blackStart);
				delay(100);
			}
			else {
				print_color_btn_pulled(redStart);
				delay(100);
			}
		}
		else if (68 < first.y && first.y < 92) { 
			if ((256 < end.x && end.x < 280) && (68 < end.y && end.y < 92)) {
				color = makepixel(255, 255, 0);
				print_color_btn_pulled(redStart);
				print_color_btn_pulled(whiteStart);
				print_color_btn_pulled(lightblueStart);
				print_color_btn_pulled(orangeStart);
				print_color_btn_pulled(greenStart);
				print_color_btn_pulled(darkblueStart);
				print_color_btn_pulled(blackStart);
				delay(100);
			}
			else {
				print_color_btn_pulled(yellowStart);
				delay(100);
			}
		}
		else if (97 < first.y && first.y < 121) {
			if ((256 < end.x && end.x < 280) && (97 < end.y && end.y < 121)) {
				color = makepixel(60, 100, 205);
				print_color_btn_pulled(redStart);
				print_color_btn_pulled(yellowStart);
				print_color_btn_pulled(whiteStart);
				print_color_btn_pulled(orangeStart);
				print_color_btn_pulled(greenStart);
				print_color_btn_pulled(darkblueStart);
				print_color_btn_pulled(blackStart);
				delay(100);
			}
			else {
				print_color_btn_pulled(lightblueStart);
				delay(100);
			}
		}
	}
	if (285 < first.x && first.x < 309) {
		if (10 < first.y && first.y < 40) {
			if ((285 < end.x && end.x < 309) && (10 < end.y && end.y < 40)) {
				color = makepixel(255, 170, 40);
				print_color_btn_pulled(redStart);
				print_color_btn_pulled(yellowStart);
				print_color_btn_pulled(lightblueStart);
				print_color_btn_pulled(whiteStart);
				print_color_btn_pulled(greenStart);
				print_color_btn_pulled(darkblueStart);
				print_color_btn_pulled(blackStart);
				delay(100);
			}
			else {
				print_color_btn_pulled(orangeStart);
				delay(100);
			}
		}
		else if (42 < first.y && first.y < 66) {
			if ((285 < end.x && end.x < 309) && (42 < end.y && end.y < 66)) {
				color = makepixel(150, 190, 35);
				print_color_btn_pulled(redStart);
				print_color_btn_pulled(yellowStart);
				print_color_btn_pulled(lightblueStart);
				print_color_btn_pulled(orangeStart);
				print_color_btn_pulled(whiteStart);
				print_color_btn_pulled(darkblueStart);
				print_color_btn_pulled(blackStart);
				delay(100);
			}
			else {
				print_color_btn_pulled(greenStart);
				delay(100);
			}
		}
		else if (68 < first.y && first.y < 92) {
			if ((285 < end.x && end.x < 309) && (68 < end.y && end.y < 92)) {
				color = makepixel(5, 20, 120);
				print_color_btn_pulled(redStart);
				print_color_btn_pulled(yellowStart);
				print_color_btn_pulled(lightblueStart);
				print_color_btn_pulled(orangeStart);
				print_color_btn_pulled(greenStart);
				print_color_btn_pulled(whiteStart);
				print_color_btn_pulled(blackStart);
				delay(100);
			}
			else {
				print_color_btn_pulled(darkblueStart);
				delay(100);
			}
		}
		else if (97 < first.y && first.y < 121) {
			if ((285 < end.x && end.x < 309) && (97 < end.y && end.y < 121)) {
				color = makepixel(0, 0, 0);
				print_color_btn_pulled(redStart);
				print_color_btn_pulled(yellowStart);
				print_color_btn_pulled(lightblueStart);
				print_color_btn_pulled(orangeStart);
				print_color_btn_pulled(greenStart);
				print_color_btn_pulled(darkblueStart);
				print_color_btn_pulled(whiteStart);
				delay(100);
			}
			else {
				print_color_btn_pulled(blackStart);
				delay(100);
			}
		}
	}
	if (257 < first.x && first.x < 309) {
		if (170 < first.y && first.y < 197) { // Pen
			if ((257 < end.x && end.x < 309) && (170 < end.y && end.y < 197)) {
				set_pen_mode_flag(1, 0);
				print_func_btn_pulled(fillStart);
			}
			else {
				print_func_btn_pulled(penStart);
			}
		}

		else if (202 < first.y && first.y < 229) { // Fill
			if ((257 < end.x && end.x < 309) && (202 < end.y && end.y < 229)) {
				set_pen_mode_flag(0, 1);
				print_func_btn_pulled(penStart);
			}
			else {
				print_func_btn_pulled(fillStart);
			}
		}
	}
}

void print_func_btn_pushed(Point func_start) {
	pixel = makepixel(255, 0, 255); // pink pixel
	for (i = func_start.y; i <= (func_start.y + 27); i++) {
		for (j = func_start.x; j <= (func_start.x + 52); j++) {
			if (i == func_start.y) {
				*(pfbdata + j + i * fbvar.xres) = pixel;
			}
			if (i == (func_start.y + 27)) {
				*(pfbdata + j + i * fbvar.xres) = pixel;
			}
			*(pfbdata + func_start.x + i * fbvar.xres) = pixel;
			*(pfbdata + (func_start.x + 52) + i * fbvar.xres) = pixel;
		}
	}
}

void print_func_btn_pulled(Point func_start) {
	pixel = makepixel(0, 0, 0); // black pixel
	for (i = func_start.y; i <= (func_start.y + 27); i++) {
		for (j = func_start.x; j <= (func_start.x + 52); j++) {
			if (i == func_start.y) {
				*(pfbdata + j + i * fbvar.xres) = pixel;
			}
			if (i == (func_start.y + 27)) {
				*(pfbdata + j + i * fbvar.xres) = pixel;
			}
			*(pfbdata + func_start.x + i * fbvar.xres) = pixel;
			*(pfbdata + (func_start.x + 52) + i * fbvar.xres) = pixel;
		}
	}
}

void print_color_btn_pushed(Point color_start) {
	pixel = makepixel(255, 0, 255); //pink
	for (i = color_start.y; i <= (color_start.y + 24); i++) {
		for (j = color_start.x; j <= (color_start.x + 24); j++) {
			if (i == color_start.y) {
				*(pfbdata + j + i * fbvar.xres) = pixel;
			}
			if (i == (color_start.y + 24)) {
				*(pfbdata + j + i * fbvar.xres) = pixel;
			}
			*(pfbdata + color_start.x + i * fbvar.xres) = pixel;
			*(pfbdata + (color_start.x + 24) + i * fbvar.xres) = pixel;
		}
	}
}

void print_color_btn_pulled(Point color_start) {
	pixel = makepixel(0, 0, 0); // black pixel
	for (i = color_start.y; i <= (color_start.y + 24); i++) {
		for (j = color_start.x; j <= (color_start.x + 24); j++) {
			if (i == color_start.y) {
				*(pfbdata + j + i * fbvar.xres) = pixel;
			}
			if (i == (color_start.y + 24)) {
				*(pfbdata + j + i * fbvar.xres) = pixel;
			}
			*(pfbdata + color_start.x + i * fbvar.xres) = pixel;
			*(pfbdata + (color_start.x + 24) + i * fbvar.xres) = pixel;
		}
	}
}

void set_left_func_flag(int line, int rect, int oval, int frdraw, int sel, int erase) {
	lineFlag = line;
	rectFlag = rect;
	ovalFlag = oval;
	frdrawFlag = frdraw;
	selFlag = sel;
	eraseFlag = erase;
}

void set_pen_mode_flag(int pen, int fill) {
	penFlag = pen;
	fillFlag = fill;
}

void clear() {
	pixel = makepixel(255, 255, 255);
	for (i = 12; i < 228; i++) {
		for (j = 71; j < 248; j++) {
			*(pfbdata + j + i * fbvar.xres) = pixel;
		}
	}
}

void check_flag() {
	if (paintFlag && penFlag && lineFlag) {
		drawLine();
	}
	if (paintFlag && penFlag && rectFlag) {
		drawRect();
	}
	if (paintFlag && fillFlag && rectFlag) {
		drawRectFill();
	}
	if (paintFlag && penFlag && ovalFlag) {
		drawOval();
	}
	if (paintFlag && fillFlag && ovalFlag) {
		drawOvalFill();
	}

	paintFlag = 0;
}

void drawLine() {
	int xpos1 = first.x;
	int ypos1 = first.y;
	int xpos2 = end.x;
	int ypos2 = end.y;

	interpolation(xpos1, xpos2, ypos1, ypos2);
}

void drawRect() {
	int xpos1 = first.x;
	int ypos1 = first.y;
	int xpos2 = end.x;
	int ypos2 = end.y;
	int t; // temp value

	if (xpos1 > xpos2) {
		t = xpos1;
		xpos1 = xpos2;
		xpos2 = t;
	}
	if (ypos1 > ypos2) {
		t = ypos1;
		ypos1 = ypos2;
		ypos2 = t;
	}

	for (i = ypos1; i <= ypos2; i++) {
		for (j = xpos1; j <= xpos2; j++) {
			if (i == ypos1) {
				*(pfbdata + j + i * fbvar.xres) = color;
			}
			if (i == ypos2) {
				*(pfbdata + j + i * fbvar.xres) = color;
			}
			*(pfbdata + xpos1 + i * fbvar.xres) = color;
			*(pfbdata + xpos2 + i * fbvar.xres) = color;
		}
	}
}

void drawRectFill() {
	int xpos1 = first.x;
	int ypos1 = first.y;
	int xpos2 = end.x;
	int ypos2 = end.y;
	int t; // temp value

	if (xpos1 > xpos2) {
		t = xpos1;
		xpos1 = xpos2;
		xpos2 = t;
	}
	if (ypos1 > ypos2) {
		t = ypos1;
		ypos1 = ypos2;
		ypos2 = t;
	}

	for (y = ypos1; y <= ypos2; y++) {
		for (x = xpos1;x <= xpos2;x++) {
			offset = y * fbvar.xres + x;
			*(pfbdata + offset) = color;
		}
	}
}

void drawOval() {
	int x0 = first.x;
	int x1 = end.x;
	int y0 = first.y;
	int y1 = end.y;
	double px, py;
	double Axis1;
	double Axis2;
	Point center;

	if (((x1 - x0) < 0) && ((y1 - y0) < 0)) {
		Axis1 = x0 - x1;
		Axis2 = y0 - y1;
		center.x = first.x - Axis1 / 2;
		center.y = first.y - Axis2 / 2;
	}
	else if (((x1 - x0) < 0) && ((y1 - y0) > 0)) {
		Axis1 = x0 - x1;
		Axis2 = y1 - y0;
		center.x = first.x - Axis1 / 2;
		center.y = first.y + Axis2 / 2;
	}
	else if (((x1 - x0) > 0) && ((y1 - y0) < 0)) {
		Axis1 = x1 - x0;
		Axis2 = y0 - y1;
		center.x = first.x + Axis1 / 2;
		center.y = first.y - Axis2 / 2;
	}
	else {
		Axis1 = x1 - x0;
		Axis2 = y1 - y0;
		center.x = first.x + Axis1 / 2;
		center.y = first.y + Axis2 / 2;
	}

	for (i = 0; i <= 360; i++) {
		double x, y;
		x = (Axis1 / 2) * cos(i * (M_PI / 180));
		y = (Axis2 / 2) * sin(i * (M_PI / 180));
		if (i != 0) {
			interpolation((int)px + center.x, (int)x + center.x, (int)py + center.y, (int)y + center.y);
		}
		px = x;
		py = y;
	}	 
}

void drawOvalFill() {
	int x0 = first.x;
	int x1 = end.x;
	int y0 = first.y;
	int y1 = end.y;
	double px, py;
	double Axis1;
	double Axis2;
	Point center;

	if (((x1 - x0) < 0) && ((y1 - y0) < 0)) {
		Axis1 = x0 - x1;
		Axis2 = y0 - y1;
		center.x = first.x - Axis1 / 2;
		center.y = first.y - Axis2 / 2;
	}
	else if (((x1 - x0) < 0) && ((y1 - y0) > 0)) {
		Axis1 = x0 - x1;
		Axis2 = y1 - y0;
		center.x = first.x - Axis1 / 2;
		center.y = first.y + Axis2 / 2;
	}
	else if (((x1 - x0) > 0) && ((y1 - y0) < 0)) {
		Axis1 = x1 - x0;
		Axis2 = y0 - y1;
		center.x = first.x + Axis1 / 2;
		center.y = first.y - Axis2 / 2;
	}
	else {
		Axis1 = x1 - x0;
		Axis2 = y1 - y0;
		center.x = first.x + Axis1 / 2;
		center.y = first.y + Axis2 / 2;
	}

	for (i = 0; i <= 360; i++) {
		double x, y;
		x = (Axis1 / 2) * cos(i * (M_PI / 180));
		y = (Axis2 / 2) * sin(i * (M_PI / 180));
		if (i != 0) {
			interpolation((int)px + center.x, (int)x + center.x, (int)py + center.y, (int)y + center.y);
			interpolation(center.x, (int)x + center.x, center.y, (int)y + center.y);
		}
		px = x;
		py = y;
	}
}

void drawFrdraw() {
	if (paintFlag && (penFlag || fillFlag) && frdrawFlag) {
		if ((70 < mcur.x && mcur.x < 248) && (11 < mcur.y && mcur.y < 229)) {
			interpolation(pre.x, mcur.x, pre.y, mcur.y);
		}
	}
}

void erase() {
	pixel = makepixel(255, 255, 255);
	if (paintFlag && (penFlag || fillFlag) && eraseFlag) {
		if ((70 < mcur.x && mcur.x < 248) && (11 < mcur.y && mcur.y < 228)) {
			for (i = mcur.x - 7; i < mcur.x + 7; i++) {
				for (j = mcur.y - 7; j < mcur.y + 7; j++) {
					if ((70 < i && i < 248) && (11 < j && j < 228)) {
						*(pfbdata + i + j * fbvar.xres) = pixel;
					}
				}
			}
		}
	}
}

void interpolation(int x0, int x1, int y0, int y1) {
	// x decrease, y decrease
	if (((x1 - x0) <= 0) && ((y1 - y0) <= 0)) {
		for (x = x1; x < x0; x++) {
			y = y0 + ((x - x0) * (y1 - y0)) / (x1 - x0);
			if ((70 < x && x < 248) && (11 < y && y < 228)) {
				*(pfbdata + x + y * fbvar.xres) = color;
			}
		}
		for (y = y1; y < y0; y++) {
			x = x0 + ((y - y0) * (x1 - x0)) / (y1 - y0);
			if ((70 < x && x < 248) && (11 < y && y < 228)) {
				*(pfbdata + x + y * fbvar.xres) = color;
			}
		}
	}
	// x decrease, y increase
	else if (((x1 - x0) <= 0) && ((y1 - y0) >= 0)) {
		for (x = x1; x < x0; x++) {
			y = y0 + ((x - x0) * (y1 - y0)) / (x1 - x0);
			if ((70 < x && x < 248) && (11 < y && y < 228)) {
				*(pfbdata + x + y * fbvar.xres) = color;
			}
		}
		for (y = y0; y < y1;y++) {
			x = x0 + ((y - y0) * (x1 - x0)) / (y1 - y0);
			if ((70 < x && x < 248) && (11 < y && y < 228)) {
				*(pfbdata + x + y * fbvar.xres) = color;
			}
		}
	}
	// x increase, y decrease
	else if (((x1 - x0) >= 0) && ((y1 - y0) <= 0)) {
		for (x = x0; x < x1;x++) {
			y = y0 + ((x - x0) * (y1 - y0)) / (x1 - x0);
			if ((70 < x && x < 248) && (11 < y && y < 228)) {
				*(pfbdata + x + y * fbvar.xres) = color;
			}
		}
		for (y = y1; y < y0;y++) {
			x = x0 + ((y - y0) * (x1 - x0)) / (y1 - y0);
			if ((70 < x && x < 248) && (11 < y && y < 228)) {
				*(pfbdata + x + y * fbvar.xres) = color;
			}
		}
	}
	// x increase, y increase
	else {
		for (x = x0; x < x1;x++) {
			y = y0 + ((x - x0) * (y1 - y0)) / (x1 - x0);
			if ((70 < x && x < 248) && (11 < y && y < 228)) {
				*(pfbdata + x + y * fbvar.xres) = color;
			}
		}
		for (y = y0; y < y1;y++) {
			x = x0 + ((y - y0) * (x1 - x0)) / (y1 - y0);
			if ((70 < x && x < 248) && (11 < y && y < 228)) {
				*(pfbdata + x + y * fbvar.xres) = color;
			}
		}
	}
}

int main(int argc, char **argv) {
	initialize();

	loop();

	// free
	free(UI);
	munmap(pfbdata, fbvar.xres*fbvar.yres * 16 / 8);
	close(fd);
	close(fbfd);

	return 0;
}