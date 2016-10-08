

	/* cs470.h: This is the header file for our graphics utility
		    functions, error trapping functions, and
		    data structures for the module cs470.c

						Marc Thomas */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if defined(_WINNT) || defined(_WIN95) || defined(WIN32)

#include <io.h>		/* for low-level read() write() */
#else

#include <unistd.h>	/* for low-level read() write() */

#endif
#include <time.h>
			/* text bounding box parameters (0.0-1.0)
			which will be used for "selectively" erasing the 
			designated proportion of the current user 2D (z=0) 
			screen */
struct	_text_box {
	double	x_ll;		/* lower left x  0.0-1.0 */
	double	y_ll;		/* lower left y  0.0-1.0 */
	double	x_ur;		/* upper right x 0.0-1.0 */
	double	y_ur;		/* upper right y 0.0-1.0 */
};

				/* bit fields for debugging actions */
#define	DEBUG_ACTION_BOX	0x01		/* post in text box */
#define	DEBUG_ACTION_LOGFILE	0x02		/* write to logfile */
#define	DEBUG_ACTION_STDERR	0x04		/* write to stderr  */

				/* structure for 2D (z=0) buttons with
			stroke text which is also used by get_pick() */
struct	_button_box {		
	char	*label;
	double	x_ll;		/* lower left x in user coords */
	double	y_ll;		/* lower left y in user coords */
	double	x_ur;		/* upper right x in user coords */
	double	y_ur;		/* upper right y in user coords */
};

				/* global event parameter structure */
struct	_last_events {
	int	reshape_width;		/* current window width */
	int	reshape_height;		/* current window height */
	unsigned char	keyboard_khar;	/* last keyboard character */
	int	keyboard_x;
	int	keyboard_y;
	char	keyboard_buffer[256];
	int	enter_done;		/* enter_done is 0 and keystrokes
				accumulate in the keyboard_buffer[] until
				[Enter] is pressed -- then enter_done is
				set -- additional input clears enter_done */
	int	speckey_button;		/* last special key */
	int	speckey_x;
	int	speckey_y;
	int	mouse_button;		/* last mouse button */
	int	mouse_state;		/* last action (press or release) */
	int	mouse_x;		/* set by mouse event and mouse */
	int	mouse_y;		/*   motion event */
					/* global event time information */
	time_t	display_time;		/* time of last display event */
	time_t	keyboard_time;		/* time of last keyb/speckey event */
	time_t	mouse_time;		/* time of last mouse event */
	time_t	mouse_motion_time;	/* time of last mouse motion event */
	time_t	reshape_time;		/* time of last reshape event */
};

					/* use this macro for correct
				sphere vertex array size with n=latitudes
				and m=longitudes */
#define	NUM_SPHERE_VERTICES(n,m)	\
		2 + (((n) - 1) * (m))
					/* use this macro for correct
				cylinder vertex array size with m=longitudes */
#define	NUM_CYLINDER_VERTICES(m)	\
		1 + (2 * (m))

					/* object drawing modes */
#define	WIRE_FRAME_SPHERE	0	/* draw wireframe only */
#define	SOLID_SURFACE_SPHERE	1	/* draw color interpolated surface */

#define	SPHERE_COLOR_POLAR	0	/* shade with north pole one color
				interpolated to south pole another (or the
				same) color) */
#define	SPHERE_COLOR_HUE	1	/* shade with hue on longitudes
				and brightness increasing from north pole
				to south pole */
#define SPHERE_PI	3.1415926

#define	WIRE_FRAME_CYLINDER	0	/* draw wireframe only */
#define	SOLID_SURFACE_CYLINDER	1	/* draw color interpolated surface */
#define	CYLINDER_COLOR_POLAR	0	/* shade with north pole one color
				interpolated to south pole another (or the
				same) color) */
#define	CYLINDER_COLOR_HUE	1	/* shade with hue on longitudes
				and brightness increasing from north pole
				to south pole */

#define	MORTAR_ONLY_PAVEMENT	0
#define	BRICKS_ONLY_PAVEMENT	1
#define	BOTH_PAVEMENT		2

#define	WIRE_FRAME_PILLAR	0	
#define	SOLID_SURFACE_PILLAR	1

					/* lighting parameters */
static	GLfloat	dull_red_ambient[4] = { 0.8F, 0.0F, 0.0F, 1.0F };
static	GLfloat	dull_red_diffuse[4] = { 1.0F, 0.0F, 0.0F, 1.0F };
static	GLfloat	dull_red_specular[4] = { 0.2F, 0.0F, 0.0F, 1.0F };

static	GLfloat	dull_green_ambient[4] = { 0.0F, 0.8F, 0.0F, 1.0F };
static	GLfloat	dull_green_diffuse[4] = { 0.0F, 1.0F, 0.0F, 1.0F };
static	GLfloat	dull_green_specular[4] = { 0.0F, 0.2F, 0.0F, 1.0F };

static	GLfloat	dull_blue_ambient[4] = { 0.0F, 0.0F, 0.8F, 1.0F };
static	GLfloat	dull_blue_diffuse[4] = { 0.0F, 0.0F, 1.0F, 1.0F };
static	GLfloat	dull_blue_specular[4] = { 0.0F, 0.0F, 0.2F, 1.0F };

static	GLfloat non_ambient[4] = { 0.0F, 0.0F, 0.0F, 1.0F };
static	GLfloat non_diffuse[4] = { 0.0F, 0.0F, 0.0F, 1.0F };
static	GLfloat non_specular[4] = { 0.0F, 0.0F, 0.0F, 1.0F };
static	GLfloat non_emissive[4] = { 0.0F, 0.0F, 0.0F, 1.0F };
static	GLfloat non_shininess = 0.0F;

					/* *** ERROR TRAPPING *** */
void gbell(void);
void gerrortrap(int silent);

					/* *** TEXT *** */
void ginit_stroke(void *font);
void genq_vectextent(char message[], void *font, double userht, double userx, 
		double usery, double *user_endx, double *user_endy, 
		double *px_ll, double *py_ll, double *px_ur, double *py_ur);
void gpost_stroke(char message[], void *font, double userht, 
		double *puser_x, double *puser_y);
void gcenter_stroke(char message[], void *font, double userht,
		double x_ll, double y_ll, double x_ur, double y_ur,
		double *user_x, double *user_y);
void genq_textextent(char message[], void *font, int rastx, int rasty, 
		int *prast_endx, int *prast_endy, 
		int *px_ll, int *py_ll, int *px_ur, int *py_ur);
void gpost_string(char message[], void *font, int *prast_x, int *prast_y);
void gcenter_string(char message[], void *font, 
		int x_ll, int y_ll, int x_ur, int y_ur,
		int *win_x, int *win_y);

					/* *** CONVERSION, ETC. *** */
void gmatrix2stderr(double matrix[16]);
void gwin2user(int winx, int winy, 
		double xleft, double xright, double ybottom, double ytop,
		double *puserx, double *pusery);
void gzx_rotate(double radians, 
		double x_old, double y_old, double z_old,
		double *px_new, double *py_new, double *pz_new);
void gazimuth_rotate(double radians, 
		double x_old, double y_old, double z_old,
		double *px_new, double *py_new, double *pz_new);
void gest_normal(double (*zfunc)(double , double ), 
	double delta_x, double delta_y, double x, double y,
	double *pnorm_x, double *pnorm_y, double *pnorm_z);

					/* *** MENU *** */
int  gget_pick(struct _button_box bboxes[], int input_x, int input_y);
void gwininput2output(int input_x, int input_y, int *poutput_x,
		int *poutput_y);
void gerase_box(struct _text_box *box, double red, double green,
		double blue, double z_near);
void gpost_buttons(struct _button_box bboxes[], double buttontextsize,
		int active, double highl_red, double highl_green,
		double highl_blue);

					/* *** EVENTS *** */
void gflush_keybbuf(struct _last_events *pev);
void gupdate_keybbuf(struct _last_events *pev, unsigned char khar);
void gupdate_specbuf(struct _last_events *pev, int button);

					/* *** DEBUG *** */
int  ginit_logfile(void);
void greshape_debug(struct _last_events *pev, int action, void *font, 
		int rast_x, int rast_y,
		double erase_red, double erase_green, double erase_blue,
		int z_near);
void gkeyboard_debug(struct _last_events *pev, int action, void *font, 
		int rast_x, int rast_y,
		double highl_red, double highl_green, double highl_blue,
		double erase_red, double erase_green, double erase_blue,
		int z_near);
void gpoint_debug(struct _last_events *pev, int action, void *font, 
		int rast_x, int rast_y,
		double erase_red, double erase_green, double erase_blue,
		int z_near);
void gpoint_motion_debug(struct _last_events *pev, int action, void *font, 
		int rast_x, int rast_y,
		double erase_red, double erase_green, double erase_blue,
		int z_near);
					/* *** TIME *** */
void gpost_time(time_t ltime, void *font, int *prast_x, int *prast_y);

					/* *** OBJECTS *** */
void gcoord_frame(double axis_width);
void ginit_sphere_vertices(double sphere_vertex[][3],
		int latitudes, int longitudes);
void ginit_sphere_vertex_normals(double sphere_vertex_normal[][3],
		int latitudes, int longitudes);
void ginit_sphere_vertex_colors(double sphere_vertex_color[][3], int mode, 
		double	north_red, double north_green, double north_blue,
		double	south_red, double south_green, double south_blue,
		int latitudes, int longitudes);
void goriented_sphere(double sphere_vertex[][3], 
		double sphere_vertex_normal[][3],
		double sphere_vertex_color[][3], int mode, 
		int latitudes, int longitudes);
void goriented_sphere_longitude(double sphere_vertex[][3], 
		double sphere_vertex_normal[][3],
		double sphere_vertex_color[][3], int k,
		double red, double green, double blue,
		int latitudes, int longitudes);
void goriented_sphere_latitude(double sphere_vertex[][3], 
		double sphere_vertex_normal[][3],
		double sphere_vertex_color[][3], int k,
		double red, double green, double blue,
		int latitudes, int longitudes);

void ginit_cylinder_vertices(double cylinder_vertex[][3], int longitudes);
void ginit_cylinder_vertex_normals(double cylinder_vertex_normal[][3], 
		int longitudes);
void ginit_cylinder_vertex_colors(double cylinder_vertex_color[][3], int mode, 
		double	north_red, double north_green, double north_blue,
		double	south_red, double south_green, double south_blue,
		int longitudes);
void goriented_cylinder(double cylinder_vertex[][3], 
		double cylinder_vertex_normal[][3],
		double cylinder_vertex_color[][3], int mode, int longitudes);

void gpavement(int rows, int columns, int mode,
		double groutwidth, double thickness, double randomheight,
		double brick_red, double brick_green, double brick_blue,
		double side_red, double side_green, double side_blue,
		double mortar_red, double mortar_green, double mortar_blue);
void grect_pillar(int mode,
		double north_red, double north_green, double north_blue,
		double south_red, double south_green, double south_blue);
