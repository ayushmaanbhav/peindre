#include <GL/glut.h>
#include <math.h>
#include "cs470.h"

char	reshape_params[256] = "";
char	keyboard_params[256] = "";
char	mouse_params[256] = "";
char	mouse_motion_params[256] = "";
char	reshape_prompt[]  = "Reshape : ";
char	keyboard_prompt[] = "Keyboard: ";
char	mouse_prompt[]    = "Mouse   : ";
char	mouse_motion_prompt[]    = "MousMove: ";

				/* next are used by debug functions */
struct 	_text_box	reshape_box, keyboard_box, mouse_box,
			mouse_motion_box;

FILE	*logfp;
int	logfile_open = 0;
char	logname[] = "cs470.log";

/* ------------------------------------------------------------------ 
Windowing-specific Error Handler:
			gbell()
			gerrortrap()
   ------------------------------------------------------------------ */

void gbell(void)
		/* This function rings the terminal bell */
	{
	char	buf[2];

	buf[0] = 0x07;		/* CONTROL-G */
	buf[1] = 0x00;
	write(2, buf, 1);	/* write to stderr */
	}  /* end of gbell */

/* ------------------------------------------------------------------ */

#if defined(_WINNT) || defined(_WIN95) || defined(WIN32)
int	silent_flag = 0;	/* normally we print errors/warnings
				to stderr, if this is set we don't */

#elif defined(__bsd)
int	silent_flag = 0;	/* normally we print errors/warnings
				to stderr, if this is set we don't */

#else
#include<GL/glx.h>		/* some GLX-specific stuff to fix
				problem with DEC ZLX-p2 boards so
				that we can trap moronic GLX-warnings
				before they kill the program */

int	silent_flag = 0;	/* normally we print errors/warnings
				to stderr, if this is set we don't */
Display	*xDisplay = NULL;
int	xScreen = 0;
Window	wRoot = 0;

static int ErrorHandler(Display *xDisplay, XErrorEvent *event)
		/* This is a typical X11 error handler. */
	{
	char buf[80];

	if (!silent_flag)
		{
		fprintf(stderr, "\nReceived X error!\n");
		fprintf(stderr, "\tError code   : %d\n", event->error_code);
		fprintf(stderr, "\tRequest code : %d\n", event->request_code);
		fprintf(stderr, "\tMinor code   : %d\n\n", event->minor_code);
		}
	XGetErrorText(xDisplay, event->error_code, buf, 80);
	if (!silent_flag)
		fprintf(stderr, "\tError text : '%s'\n\n", buf);
	return(0);
	}  /* end of ErrorHandler */

extern	Display *__glutDisplay;
#endif

/* ------------------------------------------------------------------ */

void gerrortrap(int silent)
		/* Currently, this only handles the X-Windowing
	version of OpenGL. See the glut-code in:

		/usr/examples/GL/glut/lib/glut

	especially __glutCreateWindow() in glut_win.c for implementation
	details of the GLX-initialization. Errors/warnings to stderr
	can be supressed by setting silent to 1. */
	{

#if defined(_WINNT) || defined(_WIN95) || defined(WIN32)

	silent_flag = silent;
	if (!silent_flag)
		fprintf(stderr, 
			"\nNo Win32-error handler currently installed..");
#elif defined(__linux)

	silent_flag = silent;
	if (!silent_flag)
		fprintf(stderr, 
			"\nNo Linux-error handler currently installed..");
#elif defined(__bsd)

	silent_flag = silent;
	if (!silent_flag)
		fprintf(stderr, 
			"\nNo OSX/Darwin(bsd)-error handler currently installed..");

#else
	int erb, evb;

	silent_flag = silent;		/* make copy for ErrorHandler */
	__glutOpenXConnection(NULL); 	/* should set __glutDisplay */
	xDisplay = __glutDisplay;	/* make a copy since we shouldn't
				be mucking about with glut's internal 
				variables. */
	if (!xDisplay) 
		{
		if (!silent_flag)
			fprintf(stderr, "\nCan't connect to xDisplay!");
       		exit(-1);
		}
	if (!glXQueryExtension(xDisplay, &erb, &evb)) 
		{
		if (!silent_flag)
			fprintf(stderr, "\nNo glx extension!");
		exit(-2);
		}
	if (!silent_flag)
		fprintf(stderr, "\nsetting xScreen..");
    	xScreen = DefaultScreen(xDisplay);
	if (!silent_flag)
		fprintf(stderr, "\nsetting xRoot..");
	wRoot = RootWindow(xDisplay, xScreen);
	if (!silent_flag)
		fprintf(stderr, "\nsetting ErrorHandler to trap GLX-errors..");
	XSetErrorHandler(ErrorHandler); 
#endif
	}  /* end of gerrortrap */

/* ------------------------------------------------------------------
Conversion Routines, Etc.:
				gmatrix2stderr()
				gwin2user()
				gzx_rotate()
				gazimuth_rotate()
				gest_normal()
   ------------------------------------------------------------------ */

void gmatrix2stderr(double matrix[16])
		/* This function accepts an OpenGL matrix (stored
	in column major format) and prints it out to stderr */
	{
	fprintf(stderr,
		"\nmatrix: %6.3f  %6.3f  %6.3f  %6.3f ",
			matrix[0], matrix[4], matrix[8], matrix[12]);
	fprintf(stderr,
		"\n        %6.3f  %6.3f  %6.3f  %6.3f ",
			matrix[1], matrix[5], matrix[9], matrix[13]);
	fprintf(stderr,
		"\n        %6.3f  %6.3f  %6.3f  %6.3f ",
			matrix[2], matrix[6], matrix[10], matrix[14]);
	fprintf(stderr,
		"\n        %6.3f  %6.3f  %6.3f  %6.3f ",
			matrix[3], matrix[7], matrix[11], matrix[15]); 
	}

/* ------------------------------------------------------------------ */
void gwin2user(int winx, int winy, 
		double xleft, double xright, double ybottom, double ytop,
		double *puserx, double *pusery)
		/* This function takes integer window output
	coordinates (with the origin in the lower right corner),
	the x and y extents (xleft, xright, ybottom, ytop) in user
	space, and returns the corresponding user space coordinates 
	in the plane z=0 */
	{
	double	window_width, window_height;

	window_width = (double)glutGet(GLUT_WINDOW_WIDTH);
	window_height = (double)glutGet(GLUT_WINDOW_HEIGHT);

	if (xright <= xleft)		/* insurance */
		{
		fprintf(stderr,
			"\ngwin2user: bad value xleft=%f xright=%f",
			xleft, xright);
		exit(-1);
		}
	if (ytop <= ybottom)		/* insurance */
		{
		fprintf(stderr,
			"\ngwin2user: bad value ybottom=%f ytop=%f",
			ybottom, ytop);
		exit(-1);
		}
				/* convert to user space coords (with
				z=0, of course) */
	*puserx = xleft + (((double)winx/window_width) *
			(xright - xleft));
	*pusery = ybottom + (((double)winy/window_height) *
			(ytop - ybottom));

	}  /* end of gwin2user */

/* ------------------------------------------------------------------ */

void gzx_rotate(double radians, 
		double x_old, double y_old, double z_old,
		double *px_new, double *py_new, double *pz_new)
		/* This function accepts an angle in radians. If
	the angle is greater than zero it rotates the vector
	<x_old, y_old, z_old> clockwise in the zx-plane. The
	result is placed in <*x_new, *y_new, *z_new>. If the
	angle is less than zero the rotation is counter-clockwise */
	{
	double	rads_right, rads_left, cosd, sind;

	if (radians > 0.0)
		{
						/* circle right 
		(i.e. clockwise) in the zx-plane using 
		the matrix:

		 cos(rads_right)   sin(rads_right)   z_old

		-sin(rads_right)   cos(rads_right)   x_old   */

		rads_right = radians;
		cosd = cos(rads_right);
		sind = sin(rads_right);

		*pz_new =   (z_old * cosd) + (x_old * sind);
		*px_new = - (z_old * sind) + (x_old * cosd);
		*py_new = y_old;
		}  /* end of if (dx > 0) */
	else if (radians < 0.0)
		{
						/* circle left 
		(i.e. counter-clockwise) in the zx-plane using 
		the matrix:

		cos(rads_left)  -sin(rads_left)   z_old

		sin(rads_left)   cos(rads_left)   x_old   */

		rads_left = - radians;
		cosd = cos(rads_left);
		sind = sin(rads_left);

		*pz_new = (z_old * cosd) - (x_old * sind);
		*px_new = (z_old * sind) + (x_old * cosd);
		*py_new = y_old;
		}
	else
		{
		*px_new = x_old;
		*py_new = y_old;
		*pz_new = z_old;
		}

	}  /* end of gzx_rotate */

/* ------------------------------------------------------------------ */

void gazimuth_rotate(double radians, 
		double x_old, double y_old, double z_old,
		double *px_new, double *py_new, double *pz_new)
		/* This function accepts an angle in radians. If
	the angle is less than zero it rotates the vector
	<x_old, y_old, z_old> in an azimuthal fashion by reducing
	the azimuthal spherical coordinate angle. The result is 
	placed in <*x_new, *y_new, *z_new>. If the angle is greater 
	than zero the rotation is the other way (towards the south
	pole). It will NOT rotate beyond either the north pole
	or the south pole. */
	{
	double	sinphi, rho_sinphi, rho, delta_phi, costheta, sintheta,
		cosd, sind;

							/* circle up
					by reducing phi in spherical
					coordinates, i.e. we need to map:

	z = rho * sin(phi) * cos(theta) -> rho * sin(phi - d) * cos(theta)
	x = rho * sin(phi) * sin(theta) -> rho * sin(phi - d) * sin(theta)
	y = rho * cos(phi)              -> rho * cos(phi - d) 

					but, since 

	sin(phi - d) = sin(phi) * cos(d) - cos(phi) * sin(d)
	cos(phi - d) = cos(phi) * cos(d) + sin(phi) * sin(d)

					we have

	z -> z * cos(d) - y * cos(theta) * sin(d)
	x -> x * cos(d) - y * sin(theta) * sin(d)
	y -> y * cos(d) + rho * sin(phi) * sin(d)
			= y * cos(d) + sqrt(x^2 + z^2) * sin(d)

							*/
	if (radians < 0)
		{
		delta_phi = - radians;
		cosd = cos(delta_phi);
		sind = sin(delta_phi);
		rho = 
		  sqrt((x_old * x_old) + (y_old * y_old) + (z_old * z_old));
		rho_sinphi = sqrt((x_old * x_old) + (z_old * z_old));
		if (rho_sinphi <= 0.001)
			{
			fprintf(stderr,
		"\ngazimuth_rotate: rho_sinphi=%f is too small", rho_sinphi);
			*px_new = x_old;
			*py_new = y_old;
			*pz_new = z_old;
			return;
			}
		sinphi = rho_sinphi/rho;
		if ( (y_old > 0.0 ) && (sinphi <= sind) )
			{
			*px_new = x_old;
			*py_new = y_old;
			*pz_new = z_old;
			return;		/* DON'T pass north pole */
			}

		costheta = z_old/rho_sinphi;
		sintheta = x_old/rho_sinphi;

		*pz_new = (z_old * cosd) - (y_old * costheta * sind);
		*px_new = (x_old * cosd) - (y_old * sintheta * sind);
		*py_new = (y_old * cosd) + (rho_sinphi * sind);
		}
	else if (radians > 0)
		{
							/* circle down
					by increasing phi in spherical
					coordinates, i.e. we need to map:

	z = rho * sin(phi) * cos(theta) -> rho * sin(phi + d) * cos(theta)
	x = rho * sin(phi) * sin(theta) -> rho * sin(phi + d) * sin(theta)
	y = rho * cos(phi)              -> rho * cos(phi + d) 

					but, since 

	sin(phi + d) = sin(phi) * cos(d) + cos(phi) * sin(d)
	cos(phi + d) = cos(phi) * cos(d) - sin(phi) * sin(d)

					we have

	z -> z * cos(d) + y * cos(theta) * sin(d)
	x -> x * cos(d) + y * sin(theta) * sin(d)
	y -> y * cos(d) - rho * sin(phi) * sin(d)
			= y * cos(d) - sqrt(x^2 + z^2) * sin(d)

							*/
		delta_phi = radians;
		cosd = cos(delta_phi);
		sind = sin(delta_phi);
		rho = 
		  sqrt((x_old * x_old) + (y_old * y_old) + (z_old * z_old));
		rho_sinphi = sqrt((x_old * x_old) + (z_old * z_old));
		if (rho_sinphi <= 0.001)
			{
			fprintf(stderr,
		"\ngazimuth_rotate: rho_sinphi=%f is too small", rho_sinphi);
			*px_new = x_old;
			*py_new = y_old;
			*pz_new = z_old;
			return;
			}
		sinphi = rho_sinphi/rho;
		if ( (y_old < 0.0 ) && (sinphi <= sind) )
			{
			*px_new = x_old;
			*py_new = y_old;
			*pz_new = z_old;
			return;		/* DON'T pass south pole */
			}

		costheta = z_old/rho_sinphi;
		sintheta = x_old/rho_sinphi;

		*pz_new = (z_old * cosd) + (y_old * costheta * sind);
		*px_new = (x_old * cosd) + (y_old * sintheta * sind);
		*py_new = (y_old * cosd) - (rho_sinphi * sind);
		}
	else
		{
		*px_new = x_old;
		*py_new = y_old;
		*pz_new = z_old;
		}

	}  /* end of gazimuth_rotate */

/* ------------------------------------------------------------------ */

void gest_normal(double (*zfunc)(double , double ), 
	double delta_x, double delta_y, double x, double y,
	double *pnorm_x, double *pnorm_y, double *pnorm_z)
		/* This function accepts a double valued function
	of two double variables and delta_x, and delta_y values and
	estimates the normal_vector <norm_x, norm_y, norm_z> at
	the point on the surface
					<x, y, zfunc(x,y)>

	using the well know fact from calculus that the surface
	normal is
			   < - partial_x, - partial_y, 1>
			------------------------------------
			sqrt( partial_x^2 + partial_y^2 + 1) */
	{
	double	partial_x, partial_y, norm;

	if ( (delta_x == 0.0) || (delta_y == 0.0) )
		{
		fprintf(stderr,
		  "\nest_normal: one of delta_x=%f delta_y=%f is zero.",
			delta_x, delta_y);
		return;
		}

	partial_x = (zfunc(x + delta_x,y) - zfunc(x,y))/delta_x;
	partial_y = (zfunc(x,y + delta_y) - zfunc(x,y))/delta_y;
	norm = sqrt(1.0 + 
		(partial_x * partial_x) + (partial_y * partial_y));

	*pnorm_x = - (partial_x/norm);
	*pnorm_y = - (partial_y/norm);
	*pnorm_z =   (1.0/norm);
	}  /* end of gest_normal */

/* ------------------------------------------------------------------
Drawing Utility Routines:
			gget_pick()
			gwininput2output()
			ginit_stroke()
			genq_vectextent()
			gpost_stroke()
			gcenter_stroke()
			genq_textextent()
			gpost_string()
			gcenter_string()
			gpost_time()
			gerase_box()
			gpost_buttons()
   ------------------------------------------------------------------ */

int gget_pick(struct _button_box bboxes[], int input_x, int input_y)
		/* This function accepts an array of struct _button_box
	and returns either 0 (if every button was missed) or
	the index if the pick was in that button box. The only
	complication is that the input coordinates are INPUT, i.e.
	the origin is in the UPPER LEFT CORNER of the window.
	These have to be converted to OUPUT window coordinates
	with the origin in the lower left */
	{
	int	output_x, output_y, i, ret;
	double	user_x, user_y;

	output_x = input_x;
	output_y = glutGet(GLUT_WINDOW_HEIGHT) - input_y;

				/* The next lines will have to be
			updated by the inverse of the GL_PROJECTION
			matrix -- right now we have a 1-1 mapping
			between user space (double precision reals)
			and window space (integer pixel coordinates) */

	user_x = (double)output_x;		/* tentative */
	user_y = (double)output_y;		/* tentative */

	ret = 0;
	i = 1;
	while (bboxes[i].label[0] != '_')
		{
		if ( (bboxes[i].x_ll < user_x) &&
		     (bboxes[i].x_ur > user_x) &&
		     (bboxes[i].y_ll < user_y) &&
		     (bboxes[i].y_ur > user_y) )
			{
			ret = i;
			break;
			}
		i++;
		} 
	return(ret);
	}  /* end of gget_pick */

/* ------------------------------------------------------------------ */

void gwininput2output(int input_x, int input_y, int *poutput_x,
		int *poutput_y)
		/* This function converts window input coordinates
	(e.g. from the mouse) with origin at the upper left corner
	to window output coordinates with origin at the bottom
	left corner */
	{
	*poutput_x = input_x;
	*poutput_y = glutGet(GLUT_WINDOW_HEIGHT) - input_y;
	}  /* end of gwininput2output */

/* ------------------------------------------------------------------ */

int	stroke_base;
static	int	stroke_init_done = 0;

void ginit_stroke(void *font)
		/* This procedure is usually called by myinit() and
	initializes display lists for the stroke font starting at
	the global stroke_base */
	{
	int 	i;

	if (font != GLUT_STROKE_ROMAN)
		{
		fprintf(stderr, 
		  "\nfatal error: ginit_stroke: stroke font not supported");
		fprintf(stderr, "\n");
		exit(-1);
		}
	stroke_base = glGenLists(128);
	for (i = 0 ; i < 128 ; i++)
		{
		glNewList(stroke_base + i, GL_COMPILE);
		glutStrokeCharacter(GLUT_STROKE_ROMAN, i);
		glEndList();
		}
	glListBase(stroke_base);
	stroke_init_done = 1;		/* record that stroke has been
					initialized */
	}  /* end of ginit_stroke */

/* ------------------------------------------------------------------ */

#define	GLUT_STROKE_HEIGHT	119.05		/* GLUT_STROKE_ROMAN */
#define	GLUT_STROKE_DESCEND	 34.00
#define GLUT_STROKE_SPACE	  6.00

/* ------------------------------------------------------------------ */

void genq_vectextent(char message[], void *font, double userht, double userx, 
		double usery, double *user_endx, double *user_endy, 
		double *px_ll, double *py_ll, double *px_ur, double *py_ur)
	/* This function returns more detailed information about the
	placement of a non-empty stroke string. It assumes that you want 
	to position the stroke message[] starting at (userx, usery) 
	using post_stroke() with the designated user height. It returns:

	    1. the final position in user 2D (z=0) coordinates 
	       (*user_endx, *user_endy)
	    2. the user 2D (z=0) coordinates of the bounding box, lower 
	       left corner at (*px_ll, *py_ll), upper right corner at
	       (*px_ur, *py_ur) which would be needed to completely erase 
	       the text */
	{
	int	index;
	double	endx, endy, magnif;

	if (font != GLUT_STROKE_ROMAN)
		fprintf(stderr, 
			"\nerror: genq_vectextent: stroke font not supported");

	magnif = userht/GLUT_STROKE_HEIGHT;
	endx = userx;
	endy = usery;
	index = 0;
	while(message[index] != '\0')
		{
		endx = endx + (magnif * glutStrokeWidth(GLUT_STROKE_ROMAN, 
			(int)message[index]));
		index++;
		}
	*user_endx = endx;
	*user_endy = endy;
	*px_ll = userx - (magnif * GLUT_STROKE_SPACE);
	*py_ll = usery - (magnif * GLUT_STROKE_DESCEND);
	*px_ur = endx + (magnif * GLUT_STROKE_SPACE);
	*py_ur = endy + (magnif * GLUT_STROKE_HEIGHT);
	}  /* end of genq_vectextent */

/* ------------------------------------------------------------------ */

void gpost_stroke(char message[], void *font, GLdouble userht, 
		GLdouble *puser_x, GLdouble *puser_y)
		/* This procedure accepts a character string with null 
	'\0' termination and puts it on the user 2D screen (at z==0), 
	using the GLUT_STROKE_ROMAN font but scaling its height
	so that it matches what was requested and translating it
	to the designated user position.  This function ASSUMES THAT THE 
	CALLER HAS PREVIOUSLY INITIALIZED DISPLAY LISTS FOR THIS FONT 
	STARTING at the global stroke_base by calling init_stroke() 
	(usually done in myinit()). This procedure saves and restores
	the user MODELVIEW */
	{
	if (!stroke_init_done)		/* check if init_stroke() done */
		{
		fprintf(stderr, 
	"\nfatal error: gpost_stroke: stroke has NOT been initialized");
		fprintf(stderr, "\n");
		exit(-1);
		}
	if (font != GLUT_STROKE_ROMAN)
		fprintf(stderr, 
			"\nerror: gpost_stroke: stroke font not supported");

	if (userht <= 0.0)
		{
		fprintf(stderr, 
			"\nerror: gpost_stroke: invalid user height=%f",
				userht);
		return;
		}
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();			/* save MODELVIEW matrix */
					/* note reverse order */
	glTranslated(*puser_x, *puser_y, 0.0);
	glScaled(userht/GLUT_STROKE_HEIGHT, 
		 userht/GLUT_STROKE_HEIGHT, 1.0);
	glCallLists((GLint) strlen(message), GL_BYTE, message);

	glPopMatrix(); 			/* restore MODELVIEW matrix */
	}  /* end of gpost_stroke */

/* ------------------------------------------------------------------ */

void gcenter_stroke(char message[], void *font, double userht,
		double x_ll, double y_ll, double x_ur, double y_ur,
		double *user_x, double *user_y)
		/* This function accepts a character string with
	null '\0' termination which the caller wants CENTERED on the
	2D (z=0) screen in the box with USER coordinates (x_ll, y_ll, 0.0) 
	to (x_ur, y_ur, 0.0). The user height must be specified as well.
	The function returns the correct starting point in USER 
	coordinates (*user_x, *user_y, 0.0) */
	{
	double	xcenter, ycenter, user_endx, user_endy,
		box_xll, box_yll, box_xur, box_yur, xerror, yerror;

	if (font != GLUT_STROKE_ROMAN)
		fprintf(stderr, 
			"\nerror: gcenter_stroke: stroke font not supported");

	xcenter = (x_ll + x_ur)/2.0;	/* compute the center of box */
	ycenter = (y_ll + y_ur)/2.0;

	genq_vectextent(message, GLUT_STROKE_ROMAN, userht, 
		xcenter, ycenter, &user_endx,
		&user_endy, &box_xll, &box_yll, &box_xur, &box_yur);
	xerror = (box_xur + box_xll)/2.0 - xcenter;
	yerror = (box_yur + box_yll)/2.0 - ycenter;
	*user_x = xcenter - xerror;
	*user_y = ycenter - yerror;
	}  /* end of gcenter_stroke */

/* ------------------------------------------------------------------ */

#define	DEFAULT_STRING_DESCEND	 4
#define DEFAULT_STRING_HEIGHT	13

void genq_textextent(char message[], void *font, int rastx, int rasty, 
		int *prast_endx, int *prast_endy, 
		int *px_ll, int *py_ll, int *px_ur, int *py_ur)
	/* This function returns more detailed information about the
	placement of a non-empty text string. It assumes that you want 
	to position the text string message[] starting at (rastx, rasty) 
	using post_string() with the designated font. It returns:

	    1. the final position in window coordinates 
	       (*prast_endx, *prast_endy)
	    2. the window coordinates of the bounding box, lower left 
	       corner at (*px_ll, *py_ll), upper right corner at
	       (*px_ur, *py_ur) which would be needed to completely erase 
	       the text */
	{
	int	index;
	int	endx, endy, string_height, string_descend;

	if (font == GLUT_BITMAP_8_BY_13)
		{
		string_height = 13;
		string_descend = 4;
		}
	else if (font == GLUT_BITMAP_9_BY_15)
		{
		string_height = 15;
		string_descend = 5;
		}
	else if (font == GLUT_BITMAP_TIMES_ROMAN_10)
		{
		string_height = 10;
		string_descend = 3;
		}
	else if (font == GLUT_BITMAP_TIMES_ROMAN_24)
		{
		string_height = 24;
		string_descend = 8;
		}
	else
		{
		string_height = DEFAULT_STRING_HEIGHT;
		string_descend = DEFAULT_STRING_DESCEND;
		fprintf(stderr,
		  "\nerror: genq_textextent: bitmap font not supported");
		}	
	endx = rastx;
	endy = rasty;
	index = 0;
	while(message[index] != '\0')
		{
		endx = endx + glutBitmapWidth(font, (int)message[index]);
		index++;
		}
	*prast_endx = endx;
	*prast_endy = endy;
	*px_ll = rastx - 1;
	*py_ll = rasty - string_descend;
	*px_ur = endx + 1;
	*py_ur = endy + string_height + 1;
	}  /* end of genq_textextent */

/* ------------------------------------------------------------------ */

void gpost_string(char message[], void *font, int *prast_x, int *prast_y)
		/* This procedure accepts a character string with 
	null '\0' termination and puts it directly on the window 
	(in raster coordinates, (0,0) = origin = the lower left corner) 
	starting at the position (*prast_x, *prast_y) and using the 
	specified bitmap font (e.g. GLUT_BITMAP_8_BY_13). It UPDATES the 
	position so that the caller can check how the raster units 
	(*prast_x, *prast_y) have changed. 

		WARNING: you will have to do something like the
	following (setting a 1-1 modelview and projection temporarily)
	in order to mix 2D string labels with 3D graphics (because
	vertex transformations are applied to glRasterPos):

	glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();	
	glMatrixMode(GL_PROJECTION);	save previous setting
	glPushMatrix();
		glLoadIdentity();
		gluOrtho2D((double)0.0, glutGet(GLUT_WINDOW_WIDTH),
			(double)0.0, glutGet(GLUT_WINDOW_HEIGHT));
		...
		post_string() and other string and raster operations
		...

	glPopMatrix();			restore previous setting */
	{
	int	index;
				/* move to starting position */
	glRasterPos2i(*prast_x, *prast_y);
	index = 0;
	while(message[index] != '\0')
		{
		glutBitmapCharacter(font, (int)message[index]);
		*prast_x = *prast_x + glutBitmapWidth(font,
					(int)message[index]);
		glRasterPos2i(*prast_x, *prast_y);
		index++;
		} 
	}  /* end of gpost_string */

/* ------------------------------------------------------------------ */

void gcenter_string(char message[], void *font, 
		int x_ll, int y_ll, int x_ur, int y_ur,
		int *win_x, int *win_y)
		/* This function accepts a character string with
	null '\0' termination which the caller wants CENTERED on the
	window in the box with window PIXEL coordinates (x_ll, y_ll)
	to (x_ur, y_ur).  The function returns the correct starting point 
	in window PIXEL coordinates (*user_x, *user_y). */
	{
	int	xcenter, ycenter, win_endx, win_endy,
		box_xll, box_yll, box_xur, box_yur, xerror, yerror;

	xcenter = (x_ll + x_ur)/2;	/* compute the center of box */
	ycenter = (y_ll + y_ur)/2;

	genq_textextent(message, font, xcenter, ycenter, 
		&win_endx, &win_endy, 
		&box_xll, &box_yll, &box_xur, &box_yur);
	xerror = (box_xur + box_xll)/2 - xcenter;
	yerror = (box_yur + box_yll)/2 - ycenter;
	*win_x = xcenter - xerror;
	*win_y = ycenter - yerror;
	}  /* end of gcenter_string */

/* ------------------------------------------------------------------ */

void gpost_time(time_t ltime, void *font, int *prast_x, int *prast_y)
		/* This procedure accepts the raw time value (in
	seconds since the Epoch), converts it to the standard "ctime" 
	format, and puts it directly on the window (in raster coordinates,
	origin = the lower left corner) starting at the position 
	(*prast_x, *prast_y) using the specified bitmap font. It
	UPDATES the position (*prast_x, *prast_y) so that the caller can 
	check how the raster units have changed. */
	{
	int	index;
	char	ctimebuf[64];	/* format of ctime() is as follows:
	Wed Jan 02 02:03:04 2001\n\0 */

				/* convert the raw time */
	strncpy(ctimebuf, ctime((time_t *)&ltime), 26);
				/* move to starting position */
	glRasterPos2i(*prast_x, *prast_y);
				/* loop through the 24 characters */
	for (index = 0 ; index < 24 ; index++)
		{
		glutBitmapCharacter(font, (int)ctimebuf[index]);
		*prast_x = *prast_x + glutBitmapWidth(font,
					(int)ctimebuf[index]);
		glRasterPos2i(*prast_x, *prast_y);
		} 
	}  /* end of gpost_time */

/* ------------------------------------------------------------------ */

void gerase_box(struct _text_box *box, double red, double green,
		double blue, double z_near)
		/* This function accepts a pointer to a
	_text_box structure and erases the box using the
	designated GLfloats in red, green, blue - it pushes
	the attributes and restores them afterwards. */
	{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glColor3d(red, green, blue);
	glBegin(GL_POLYGON);
		glVertex3f((*box).x_ll * glutGet(GLUT_WINDOW_WIDTH), 
			   (*box).y_ll * glutGet(GLUT_WINDOW_HEIGHT), 
				z_near);
		glVertex3f((*box).x_ll * glutGet(GLUT_WINDOW_WIDTH), 
			   (*box).y_ur * glutGet(GLUT_WINDOW_HEIGHT), 
				z_near);
		glVertex3f((*box).x_ur * glutGet(GLUT_WINDOW_WIDTH), 
			   (*box).y_ur * glutGet(GLUT_WINDOW_HEIGHT), 
				z_near);
		glVertex3f((*box).x_ur * glutGet(GLUT_WINDOW_WIDTH), 
			   (*box).y_ll * glutGet(GLUT_WINDOW_HEIGHT), 
				z_near);
	glEnd();
	glPopAttrib();
	}  /* end of gerase_box */

/* ------------------------------------------------------------------ */

void gpost_buttons(struct _button_box bboxes[], double buttontextsize,
		int active, double highl_red, double highl_green,
		double highl_blue)
		/* This function accepts an array of struct _button_box
	and puts them on the screen in the current drawing color
	using the highlight color for the active button (if 
	there is one (set this to zero if not) */
	{
	int	i;
	double	user_x, user_y, user_endx, user_endy, x_ll, y_ll,
		x_ur, y_ur;
					
	i= 1;				/* skip the 0th button */
	while(bboxes[i].label[0] != '_')
		{
		if (active == i)	/* not current operation */
			{
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glColor3d(highl_red, highl_green, highl_blue);
			}
			
		glBegin(GL_LINE_LOOP);		/* draw the button box */
			glVertex3f(bboxes[i].x_ll, 
				bboxes[i].y_ll, 0.0);
			glVertex3f(bboxes[i].x_ll, 
				bboxes[i].y_ur, 0.0);
			glVertex3f(bboxes[i].x_ur, 
				bboxes[i].y_ur, 0.0);
			glVertex3f(bboxes[i].x_ur, 
				bboxes[i].y_ll, 0.0);
		glEnd();
					/* find out where to start
					the text so that it will be
					centered in the box */
		gcenter_stroke(bboxes[i].label, GLUT_STROKE_ROMAN,
			buttontextsize, 
			bboxes[i].x_ll, bboxes[i].y_ll,
			bboxes[i].x_ur, bboxes[i].y_ur,
			&user_x, &user_y);
					/* find the erasing box */
		genq_vectextent(bboxes[i].label, GLUT_STROKE_ROMAN,
			buttontextsize,
			user_x, user_y, &user_endx, &user_endy, 
			&x_ll, &y_ll, &x_ur, &y_ur);
		gpost_stroke(bboxes[i].label, GLUT_STROKE_ROMAN,
			buttontextsize, &user_x, &user_y);
		if (active == i)
			{
			glPopAttrib();
			}
		i++;
		}
	}  /* end of gpost_buttons */

/* ------------------------------------------------------------------
Event Handling Utility Routines:
				gflush_keybbuf()
				gupdate_keybbuf()
				gupdate_specbuf()
   ------------------------------------------------------------------ */

void gflush_keybbuf(struct _last_events *pev)
		/* This function accepts a pointer to an event
	struture and clears the keyboard_buffer[], also resetting
	the enter_done flag. */
	{
	(*pev).keyboard_buffer[0] = '\0';
	(*pev).enter_done = 0;

	}  /* end of gflush_keybbuf */

/* ------------------------------------------------------------------ */

void gupdate_keybbuf(struct _last_events *pev, unsigned char khar)
		/* This function accepts a pointer to an event
	struture and an unsigned character. If the character is
	a [CR] (from [ENTER]) it sets the enter_done flag */
	{
	int i;

	if ( ((int)khar >= 32) && ((int)khar <= 126) )
		{
		if ((*pev).enter_done == 1)		/* new line */
			{
			(*pev).keyboard_buffer[0] = '\0';
			(*pev).enter_done = 0;
			}
		i = strlen((*pev).keyboard_buffer);
		if (i < (sizeof((*pev).keyboard_buffer) - 1))
			{
			(*pev).keyboard_buffer[i] = khar;
			i++;
			(*pev).keyboard_buffer[i] = '\0'; /* term string */
			}
		}
	else			/* non-printable ascii character */
		{
		if (khar == 27)		/* ESCAPE KEY */
			{
			i = strlen((*pev).keyboard_buffer);
			if (i < (sizeof((*pev).keyboard_buffer) - 5))
				{
				sprintf(&(*pev).keyboard_buffer[i], "[ESC]");
				i = i + 5;
				(*pev).keyboard_buffer[i] = '\0'; 
				}
			}
		else if (khar == 13) 	/* CARRIAGE RETURN */
			{
			(*pev).enter_done = 1;
			i = strlen((*pev).keyboard_buffer);
/* *** DEBUG ***
			if (i < (sizeof((*pev).keyboard_buffer) - 4))
				{
				sprintf(&(*pev).keyboard_buffer[i], "[CR]");
				i = i + 4; 
				} */

			(*pev).keyboard_buffer[i] = '\0'; 
			}
		else if (khar == 8) 	/* BACKSPACE */
			{
			i = strlen((*pev).keyboard_buffer);
			if (i > 0)
				{
				i--;
				(*pev).keyboard_buffer[i] = '\0';
				}
			}
		else
			{
			i = strlen((*pev).keyboard_buffer);
			if (i < (sizeof((*pev).keyboard_buffer) - 5))
				{
				sprintf(&(*pev).keyboard_buffer[i], "[%3d]", 
					(int)(*pev).keyboard_khar);
				i = i + 5;
				(*pev).keyboard_buffer[i] = '\0'; 
				}
			}
		}  /* end of else .. (non-printable char) */

	}  /* end of gupdate_keybbuf */

/* ------------------------------------------------------------------ */

void gupdate_specbuf(struct _last_events *pev, int button)
		/* This function accepts a pointer to an event
	struture and an integer representing a special key
	(e.g. function key, arrow key, etc.). It does the
	same thing that gupdate_keybbuf() does for ordinary
	keystrokes, but puts a mnemonic in the keyboard_buffer[] */
	{
	int	i;

	if ((*pev).enter_done == 1)		/* new line */
		{
		(*pev).keyboard_buffer[0] = '\0';
		(*pev).enter_done = 0;
		}
	if ( 
		(button != GLUT_KEY_F1) && (button != GLUT_KEY_F2) &&
		(button != GLUT_KEY_F3) && (button != GLUT_KEY_F4) &&
		(button != GLUT_KEY_F5) && (button != GLUT_KEY_F6) &&
		(button != GLUT_KEY_F7) && (button != GLUT_KEY_F8) &&
		(button != GLUT_KEY_LEFT) && (button != GLUT_KEY_UP) &&
		(button != GLUT_KEY_RIGHT) && (button != GLUT_KEY_DOWN) )
		{
		switch(button) {
		  case 127:		/* DELETE */
				i = strlen((*pev).keyboard_buffer);
				if (i > 0)
					{
					i--;
					(*pev).keyboard_buffer[i] = '\0';
					}
				break;
		  default:		/* OTHER */
				i = strlen((*pev).keyboard_buffer);
				if (i < (sizeof((*pev).keyboard_buffer) - 5))
					{
					sprintf(&(*pev).keyboard_buffer[i], 
						"[%3d]", button);
					i = i + 5;
					(*pev).keyboard_buffer[i] = '\0'; 
					}
		  }  /* end of switch */
		}  /* end of if ( (button >= 32) && (button <= 127) ) */
	else
		{
		if ( (button >= GLUT_KEY_F1) && (button <= GLUT_KEY_F8) )
			{
			i = strlen((*pev).keyboard_buffer);
			if (i < (sizeof((*pev).keyboard_buffer) - 4))
				{
				sprintf(&(*pev).keyboard_buffer[i], "[F%d]",
					button - GLUT_KEY_F1 + 1);
				i = i + 4;
				(*pev).keyboard_buffer[i] = '\0'; 
				}
			}
		else if ( button == GLUT_KEY_LEFT)
			{
			i = strlen((*pev).keyboard_buffer);
			if (i < (sizeof((*pev).keyboard_buffer) - 4))
				{
				sprintf(&(*pev).keyboard_buffer[i], "[LF]");
				i = i + 4;
				(*pev).keyboard_buffer[i] = '\0'; 
				}
			}
		else if ( button == GLUT_KEY_UP)
			{
			i = strlen((*pev).keyboard_buffer);
			if (i < (sizeof((*pev).keyboard_buffer) - 4))
				{
				sprintf(&(*pev).keyboard_buffer[i], "[UP]");
				i = i + 4;
				(*pev).keyboard_buffer[i] = '\0'; 
				}
			}
		else if ( button == GLUT_KEY_RIGHT)
			{
			i = strlen((*pev).keyboard_buffer);
			if (i < (sizeof((*pev).keyboard_buffer) - 4))
				{
				sprintf(&(*pev).keyboard_buffer[i], "[RT]");
				i = i + 4;
				(*pev).keyboard_buffer[i] = '\0'; 
				}
			}
		else if ( button == GLUT_KEY_DOWN)
			{
			i = strlen((*pev).keyboard_buffer);
			if (i < (sizeof((*pev).keyboard_buffer) - 4))
				{
				sprintf(&(*pev).keyboard_buffer[i], "[DN]");
				i = i + 4;
				(*pev).keyboard_buffer[i] = '\0'; 
				}
			}
		else
			{
			i = strlen((*pev).keyboard_buffer);
			if (i < (sizeof((*pev).keyboard_buffer) - 5))
				{
				sprintf(&(*pev).keyboard_buffer[i], 
					"[%3d]", button);
				i = i + 5;
				(*pev).keyboard_buffer[i] = '\0'; 
				}
			}
		}  /* end of else */

	}  /* end of gupdate_specbuf */

/* ------------------------------------------------------------------
Debug Handling Utility Routines:
				ginit_logfile()
				greshape_debug()
				gkeyboard_debug()
				gpoint_debug()
				gpoint_motion_debug()
   ------------------------------------------------------------------ */

int ginit_logfile(void)
		/* this function tries to open and reset the logfile.
	It returns 0 if successful, 1 if failure */
	{
	if (logfile_open)
		return(0);		/* already opened */
	logfp = fopen(logname, "w");
	if (logfp == (FILE *)NULL)
		{
		fprintf(stderr, 
			"\nginit_logfile: fopen(\"%s\",\"w\") failed.",
				logname);
		return(1);
		}
	else
		{
		logfile_open = 1;
		return(0);
		}
	}  /* end of ginit_logfile */

/* ------------------------------------------------------------------ */

void greshape_debug(struct _last_events *pev, int action, void *font, 
		int rast_x, int rast_y,
		double erase_red, double erase_green, double erase_blue,
		int z_near)
		/* this function is used primarily for debugging.
	It takes the global event parameters from *pev and composes 
	a one-line message (which is saved in the global string: 
	reshape_params[]). The message, depending on the bitfields
 	set in action post the information to one or more of the
	following:  { text_box, logfile, standard error } */
	{
	int	rast_endx, rast_endy,
		ix_ll, iy_ll, ix_ur, iy_ur, ret;
	char	tmpparams[64];

	if ( (action & DEBUG_ACTION_LOGFILE) && (!logfile_open) )
		{
		ret = ginit_logfile();
		if (ret) return;		/* no logfile */
		}
				/* build parameter information */
	strncpy(reshape_params, reshape_prompt, sizeof(reshape_prompt));
	strncat(reshape_params, ctime((time_t *)
		&(*pev).reshape_time) + 11, 8);
	sprintf(tmpparams, " dim= %4d x %4d", glutGet(GLUT_WINDOW_WIDTH),
		glutGet(GLUT_WINDOW_HEIGHT));
	strncat(reshape_params, tmpparams, 19);

	if (action & DEBUG_ACTION_BOX)
	    {
	    genq_textextent(reshape_params, font, rast_x, rast_y,
		&rast_endx, &rast_endy, &ix_ll, &iy_ll, &ix_ur, &iy_ur);
	    reshape_box.x_ll = (double)ix_ll/(double)
		glutGet(GLUT_WINDOW_WIDTH);
	    reshape_box.y_ll = (double)iy_ll/(double)
		glutGet(GLUT_WINDOW_HEIGHT);
	    reshape_box.x_ur = (double)ix_ur/(double)
		glutGet(GLUT_WINDOW_WIDTH);
	    reshape_box.y_ur = (double)iy_ur/(double)
		glutGet(GLUT_WINDOW_HEIGHT);

	    gerase_box(&reshape_box, erase_red, erase_green, erase_blue,
		z_near);
	    gpost_string(reshape_params, font, &rast_x, &rast_y);  
	    glFlush();			/* output the display buffer */
	    }
	if (action & DEBUG_ACTION_STDERR)
	    {
	    fprintf(stderr, "\n%s", reshape_params);
	    }
	if (action & DEBUG_ACTION_LOGFILE)
	    {
	    fprintf(logfp, "\n%s", reshape_params);
	    fflush(logfp);
	    }
	}  /* end of greshape_debug */

/* ------------------------------------------------------------------ */

void gkeyboard_debug(struct _last_events *pev, int action, void *font, 
		int rast_x, int rast_y,
		double highl_red, double highl_green, double highl_blue,
		double erase_red, double erase_green, double erase_blue,
		int z_near)
		/* this function is used primarily for debugging.
	It takes the global event parameters from *pev and composes 
	a one-line message (which is saved in the global string: 
	keyboard_params[]). The message, depending on the bitfields
 	set in action post the information to one or more of the
	following:  { text_box, logfile, standard error } */
	{
	int	rast_endx, rast_endy,
		ix_ll, iy_ll, ix_ur, iy_ur, ret;
	char	tmpkeys[64];

	if ( (action & DEBUG_ACTION_LOGFILE) && (!logfile_open) )
		{
		ret = ginit_logfile();
		if (ret) return;		/* no logfile */
		}
				/* build parameter information */
	strncpy(keyboard_params, keyboard_prompt, sizeof(keyboard_prompt));
	strncat(keyboard_params, ctime((time_t *)
		&(*pev).keyboard_time) + 11, 8);
	if (strlen((*pev).keyboard_buffer) <= 30)
		sprintf(tmpkeys, " buf=%-30.30s", (*pev).keyboard_buffer);
	else
		sprintf(tmpkeys, " buf=%-30.30s", 
		&(*pev).keyboard_buffer[strlen((*pev).keyboard_buffer) - 30]);
	strncat(keyboard_params, tmpkeys, 35);

	if (action & DEBUG_ACTION_BOX)
	    {
	    genq_textextent(keyboard_params, font, rast_x, rast_y,
		&rast_endx, &rast_endy, &ix_ll, &iy_ll, &ix_ur, &iy_ur);
	    keyboard_box.x_ll = (double)ix_ll/(double)
		glutGet(GLUT_WINDOW_WIDTH);
	    keyboard_box.y_ll = (double)iy_ll/(double)
		glutGet(GLUT_WINDOW_HEIGHT);
	    keyboard_box.x_ur = (double)ix_ur/(double)
		glutGet(GLUT_WINDOW_WIDTH);
	    keyboard_box.y_ur = (double)iy_ur/(double)
		glutGet(GLUT_WINDOW_HEIGHT);

	    gerase_box(&keyboard_box, erase_red, erase_green, erase_blue,
		z_near);
	    glPushAttrib(GL_ALL_ATTRIB_BITS);
	    if ((*pev).enter_done) 
		glColor3f(highl_red, highl_green, highl_blue);
	    gpost_string(keyboard_params, font, &rast_x, &rast_y);
	    glPopAttrib();
	    glFlush();			/* output the display buffer */
	    }
	if (action & DEBUG_ACTION_STDERR)
	    {
	    fprintf(stderr, "\n%s", keyboard_params);
	    }
	if (action & DEBUG_ACTION_LOGFILE)
	    {
	    fprintf(logfp, "\n%s", keyboard_params);
	    fflush(logfp);
	    }
	}  /* end of gkeyboard_debug */

/* ------------------------------------------------------------------ */

void gpoint_debug(struct _last_events *pev, int action, void *font, 
		int rast_x, int rast_y,
		double erase_red, double erase_green, double erase_blue,
		int z_near)
		/* this function is used primarily for debugging.
	It takes the global event parameters from *pev and composes 
	a one-line message (which is saved in the global string: 
	mouse_params[]). The message, depending on the bitfields
 	set in action post the information to one or more of the
	following:  { text_box, logfile, standard error } */
	{
	int	rast_endx, rast_endy, 
		ix_ll, iy_ll, ix_ur, iy_ur, ret;
	char	tmpcoords[64];

	if ( (action & DEBUG_ACTION_LOGFILE) && (!logfile_open) )
		{
		ret = ginit_logfile();
		if (ret) return;		/* no logfile */
		}
				/* build parameter information */
	strncpy(mouse_params, mouse_prompt, sizeof(mouse_prompt));
	strncat(mouse_params, ctime((time_t *)
		&(*pev).mouse_time) + 11, 8);
	strncat(mouse_params, " mou=", 5);
	switch((*pev).mouse_button)
		{
		case GLUT_LEFT_BUTTON:	
			strncat(mouse_params, "L ", 2);
			break;
		case GLUT_MIDDLE_BUTTON:	
			strncat(mouse_params, "M ", 2);
			break;
		case GLUT_RIGHT_BUTTON:	
			strncat(mouse_params, "R ", 2);
			break;

		default:
			strncat(mouse_params, "? ", 2);
		}  /* end of switch */
	switch((*pev).mouse_state)
		{
		case GLUT_DOWN:
			strncat(mouse_params, "DN ", 3);
			break;
		case GLUT_UP:
			strncat(mouse_params, "UP ", 3);
			break;

		default:
			strncat(mouse_params, "?? ", 3);
		}
	sprintf(tmpcoords, "(%4d,%4d)", (*pev).mouse_x, (*pev).mouse_y);
	strncat(mouse_params, tmpcoords, 11);

	if (action & DEBUG_ACTION_BOX)
	    {
	    genq_textextent(mouse_params, font, rast_x, rast_y,
		&rast_endx, &rast_endy, &ix_ll, &iy_ll, &ix_ur, &iy_ur);
	    mouse_box.x_ll = (double)ix_ll/(double)
		glutGet(GLUT_WINDOW_WIDTH);
	    mouse_box.y_ll = (double)iy_ll/(double)
		glutGet(GLUT_WINDOW_HEIGHT);
	    mouse_box.x_ur = (double)ix_ur/
		(double)glutGet(GLUT_WINDOW_WIDTH);
	    mouse_box.y_ur = (double)iy_ur/(double)
		glutGet(GLUT_WINDOW_HEIGHT);

	    gerase_box(&mouse_box, erase_red, erase_green, erase_blue,
		z_near);
	    gpost_string(mouse_params, font, &rast_x, &rast_y);
	    glFlush();			/* output the display buffer */
	    }
	if (action & DEBUG_ACTION_STDERR)
	    {
	    fprintf(stderr, "\n%s", mouse_params);
	    }
	if (action & DEBUG_ACTION_LOGFILE)
	    {
	    fprintf(logfp, "\n%s", mouse_params);
	    fflush(logfp);
	    }
	}  /* end of gpoint_debug */

/* ------------------------------------------------------------------ */

void gpoint_motion_debug(struct _last_events *pev, int action, void *font, 
		int rast_x, int rast_y,
		double erase_red, double erase_green, double erase_blue,
		int z_near)
		/* this function is used primarily for debugging.
	It takes the global event parameters from *pev and composes 
	a one-line message (which is saved in the global string: 
	mouse_motion_params[]). The message, depending on the bitfields
 	set in action post the information to one or more of the
	following:  { text_box, logfile, standard error } */
	{
	int	rast_endx, rast_endy, 
		ix_ll, iy_ll, ix_ur, iy_ur, ret;
	char	tmpcoords[64];

	if ( (action & DEBUG_ACTION_LOGFILE) && (!logfile_open) )
		{
		ret = ginit_logfile();
		if (ret) return;		/* no logfile */
		}
				/* build parameter information */
	strncpy(mouse_motion_params, mouse_motion_prompt, 
		sizeof(mouse_motion_prompt));
	strncat(mouse_motion_params, ctime((time_t *)
		&(*pev).mouse_motion_time) + 11, 8);
	strncat(mouse_motion_params, " mou_motion=", 12);
	sprintf(tmpcoords, "(%4d,%4d)", (*pev).mouse_x, (*pev).mouse_y);
	strncat(mouse_motion_params, tmpcoords, 11);

	if (action & DEBUG_ACTION_BOX)
	    {
	    genq_textextent(mouse_motion_params, font, rast_x, rast_y,
		&rast_endx, &rast_endy, &ix_ll, &iy_ll, &ix_ur, &iy_ur);
	    mouse_motion_box.x_ll = (double)ix_ll/(double)
		glutGet(GLUT_WINDOW_WIDTH);
	    mouse_motion_box.y_ll = (double)iy_ll/(double)
		glutGet(GLUT_WINDOW_HEIGHT);
	    mouse_motion_box.x_ur = (double)ix_ur/
		(double)glutGet(GLUT_WINDOW_WIDTH);
	    mouse_motion_box.y_ur = (double)iy_ur/(double)
		glutGet(GLUT_WINDOW_HEIGHT);

	    gerase_box(&mouse_motion_box, erase_red, erase_green, erase_blue,
		z_near);
	    gpost_string(mouse_motion_params, font, &rast_x, &rast_y);
	    glFlush();			/* output the display buffer */
	    }
	if (action & DEBUG_ACTION_STDERR)
	    {
	    fprintf(stderr, "\n%s", mouse_motion_params);
	    }
	if (action & DEBUG_ACTION_LOGFILE)
	    {
	    fprintf(logfp, "\n%s", mouse_motion_params);
	    fflush(logfp);
	    }
	}  /* end of point_motion_debug */

/* ------------------------------------------------------------------
Object Drawing Routines:
			gcoord_frame()
			ginit_sphere_vertices()
			ginit_sphere_vertex_normals()
			ginit_sphere_vertex_colors()
			goriented_sphere()
			goriented_sphere_longitude()
			goriented_sphere_latitude()
			gpavement()
			grect_pillar()
   ------------------------------------------------------------------ */

void gcoord_frame(double axis_width)
		/* this function draws a right hand 3D coordinate
	frame from the origin to +1 on all three axes. It temporarily
	saves the current drawing attributes and uses RED for the x-axis, 
	GREEN for the y-axis, and BLUE for the z-axis. The default
	value in OpenGL for LINE_WIDTH is 1.0 - the caller should
	set axis_width to be at least 1.0 . Note that we use both
	glColor and glMaterial parameters so that the coordinate
	frame will work in both GL_COLOR_MATERIAL mode as well as
	full color precision mode */
	{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glLineWidth(axis_width);
	glBegin(GL_LINES);
		glColor3d(1.0, 0.0, 0.0);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, 
			dull_red_ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, 
			dull_red_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, 
			dull_red_specular);

		glVertex3d( 0.0, 0.0, 0.0);
		glVertex3d( 1.0, 0.0, 0.0);

		glColor3d(0.0, 1.0, 0.0);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, 
			dull_green_ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, 
			dull_green_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, 
			dull_green_specular);

		glVertex3d( 0.0, 0.0, 0.0);
		glVertex3d( 0.0, 1.0, 0.0);

		glColor3d(0.0, 0.0, 1.0);

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, 
			dull_blue_ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, 
			dull_blue_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, 
			dull_blue_specular);

		glVertex3d( 0.0, 0.0, 0.0);
		glVertex3d( 0.0, 0.0, 1.0);
	glEnd();
	glPopAttrib();
	}  /* end of gcoord_frame */

/* ------------------------------------------------------------------ */

void ginit_sphere_vertices(double sphere_vertex[][3],
		int latitudes, int longitudes)
		/* fill the sphere_vertex[][3] 2D array with the 
	vertices of a sphere of radius 1.0 starting at the north pole 
	using latitudes and longitudes given in the following
	pattern:

			  north pole = 0

   	     first latitude /  /  /  |  \  \  \

	     0 +           1  2  3 ...          m

	    second latitude
			 /   /   /   |    \   \   \

	    m +         1   2   3 ...               m



	    (n-1)-st latitude
			    \  \  \  |  /  /  /

	    (n-2)m +         1  2  3 ...     m

			  south pole = (n-1)m + 1 

	the caller MUST provide storage of AT LEAST

	double	sphere_vertex[2 + (latitudes - 1)*longitudes][3]; */
	{
	int	n, m;
	double	theta, phi;
					/* north pole */
	sphere_vertex[0][0] =  0.0;
	sphere_vertex[0][1] =  1.0;
	sphere_vertex[0][2] =  0.0;

					/* use spherical coordinates:
				phi, and theta in RADIAN measure */
	for (n = 0 ; n <= (latitudes - 2) ; n++)
		{
		phi = SPHERE_PI * ((double)(n+1)/(double)latitudes);
		for (m = 1 ; m <= longitudes ; m++)
			{
			theta = 2.0 * SPHERE_PI *
				((double)(m-1)/(double)longitudes);
			sphere_vertex[ n*longitudes + m ][0] =
				sin(phi) * cos(theta);
			sphere_vertex[ n*longitudes + m ][1] =
				cos(phi);
			sphere_vertex[ n*longitudes + m ][2] =
				sin(phi) * sin(theta);
/* *** DEBUG ***
	fprintf(stderr,
	"\nginit_sphere_vertices: n=%d m=%d (%f,%f,%f) ", n, m,
		sphere_vertex[n*longitudes + m][0],
		sphere_vertex[n*longitudes + m][1],
		sphere_vertex[n*longitudes + m][2]); */

			}  /* end of m */
		}  /* end of n */
					/* south pole */
	sphere_vertex[(latitudes - 1)*longitudes + 1][0] =  0.0;
	sphere_vertex[(latitudes - 1)*longitudes + 1][1] = -1.0;
	sphere_vertex[(latitudes - 1)*longitudes + 1][2] =  0.0;

	}  /* end of ginit_sphere_vertices */

/* ------------------------------------------------------------------ */

void ginit_sphere_vertex_normals(double sphere_vertex_normal[][3],
		int latitudes, int longitudes)
		/* fill the sphere_vertex_normal[][3] 2D array with the 
	vertex normals (same as the vertex on a sphere with radius 1.0).

	the caller MUST provide storage of AT LEAST

	double	sphere_vertex_normal[2 + (latitudes - 1)*longitudes][3]; */
	{
	int	n, m;
	double	theta, phi;
					/* north pole */
	sphere_vertex_normal[0][0] =  0.0;
	sphere_vertex_normal[0][1] =  1.0;
	sphere_vertex_normal[0][2] =  0.0;

					/* use spherical coordinates:
				phi, and theta in RADIAN measure */
	for (n = 0 ; n <= (latitudes - 2) ; n++)
		{
		phi = SPHERE_PI * ((double)(n+1)/(double)latitudes);
		for (m = 1 ; m <= longitudes ; m++)
			{
			theta = 2.0 * SPHERE_PI *
				((double)(m-1)/(double)longitudes);
			sphere_vertex_normal[ n*longitudes + m ][0] =
				sin(phi) * cos(theta);
			sphere_vertex_normal[ n*longitudes + m ][1] =
				cos(phi);
			sphere_vertex_normal[ n*longitudes + m ][2] =
				sin(phi) * sin(theta);
/* *** DEBUG ***
	fprintf(stderr,
	"\nginit_sphere_vertex_normals: n=%d m=%d (%f,%f,%f) ", n, m,
		sphere_vertex_normal[n*longitudes + m][0],
		sphere_vertex_normal[n*longitudes + m][1],
		sphere_vertex_normal[n*longitudes + m][2]); */

			}  /* end of m */
		}  /* end of n */
					/* south pole */
	sphere_vertex_normal[(latitudes - 1)*longitudes + 1][0] =  0.0;
	sphere_vertex_normal[(latitudes - 1)*longitudes + 1][1] = -1.0;
	sphere_vertex_normal[(latitudes - 1)*longitudes + 1][2] =  0.0;

	}  /* end of ginit_sphere_vertex_normals */

/* ------------------------------------------------------------------ */

void ginit_sphere_vertex_colors(double sphere_vertex_color[][3], int mode, 
		double	north_red, double north_green, double north_blue,
		double	south_red, double south_green, double south_blue,
		int latitudes, int longitudes)
		/* fill the sphere_vertex_color[][3] 2D array with the 
	vertex colors according to the mode.  permissible 
	modes are:
			SPHERE_COLOR_POLAR
			SPHERE_COLOR_HUE

	only the caller MUST provide storage of AT LEAST

	double	sphere_vertex_color[2 + (latitudes - 1)*longitudes][3]; */
	{
	int	n, m;
	double	phi, red, green, blue, diff_red, diff_green, diff_blue;

					/* north pole */
	if (mode == SPHERE_COLOR_POLAR)
		{
		sphere_vertex_color[0][0] =  north_red;
		sphere_vertex_color[0][1] =  north_green;
		sphere_vertex_color[0][2] =  north_blue;
		diff_red = south_red - north_red;
		diff_green = south_green - north_green;
		diff_blue = south_blue - north_blue;
		}
	else if (mode == SPHERE_COLOR_HUE)
		{
		sphere_vertex_color[0][0] =  0.1;
		sphere_vertex_color[0][1] =  0.3;
		sphere_vertex_color[0][2] =  0.2;
		}
					/* use spherical coordinates:
				phi, and theta in RADIAN measure */
	for (n = 0 ; n <= (latitudes - 2) ; n++)
		{
		phi = SPHERE_PI * ((double)(n+1)/(double)latitudes);
		for (m = 1 ; m <= longitudes ; m++)
			{
			if (mode == SPHERE_COLOR_POLAR)
				{
				red = north_red + 
					((phi/SPHERE_PI) * diff_red);
				green = north_green + 
					((phi/SPHERE_PI) * diff_green);
				blue = north_blue +
					((phi/SPHERE_PI) * diff_blue);
				}
			else if (mode == SPHERE_COLOR_HUE)
				{
				red = 0.2 + 
					( (phi/SPHERE_PI) * 0.8 *
					  ((double)((longitudes + 1) - m)/
					   (double)longitudes));
				green = 0.4 + 
					( (phi/SPHERE_PI) * 0.6 *
	( (1.0 - 2.0*fabs( ((double)m - (double)longitudes/2.0)/
					   (double)longitudes ) ) ) );
				blue = 0.3 + 
					( (phi/SPHERE_PI) * 0.7 *
					  ((double)m/
					   (double)longitudes));
				}

			sphere_vertex_color[ n*longitudes + m ][0] =
				red;
			sphere_vertex_color[ n*longitudes + m ][1] =
				green;
			sphere_vertex_color[ n*longitudes + m ][2] =
				blue;
/* *** DEBUG *** 
	fprintf(stderr,
	"\nginit_sphere_vertex_color: n=%d m=%d rgb=(%f,%f,%f) ", n, m,
		sphere_vertex_color[n*longitudes + m][0],
		sphere_vertex_color[n*longitudes + m][1],
		sphere_vertex_color[n*longitudes + m][2]); */

			}  /* end of m */
		}  /* end of n */
					/* south pole */
	if (mode == SPHERE_COLOR_POLAR)
		{
		sphere_vertex_color[(latitudes - 1)*longitudes + 1][0] =  
			south_red;
		sphere_vertex_color[(latitudes - 1)*longitudes + 1][1] =  
			south_green;
		sphere_vertex_color[(latitudes - 1)*longitudes + 1][2] =  
			south_blue;
		}
	else if (mode == SPHERE_COLOR_HUE)
		{
		sphere_vertex_color[(latitudes - 1)*longitudes + 1][0] =  1.0;
		sphere_vertex_color[(latitudes - 1)*longitudes + 1][1] =  1.0;
		sphere_vertex_color[(latitudes - 1)*longitudes + 1][2] =  1.0;
		}
	}  /* end of ginit_sphere_vertex_colors */

/* ------------------------------------------------------------------ */

void goriented_sphere(double sphere_vertex[][3], 
		double sphere_vertex_normal[][3],
		double sphere_vertex_color[][3], int mode, 
		int latitudes, int longitudes)
		/* Draw the oriented sphere whose vertices have previously
	been put in the 2D array sphere_vertex[][3] by ginit_sphere_vertices()
	by drawing the faces in OUTWARD NORMAL orientation and the
	normals and colors indicated for the vertices. permissible 
	modes are:
			WIRE_FRAME_SPHERE	
			SOLID_SURFACE_SPHERE
	only */
	{
	int	n, m;

	if ( (mode != WIRE_FRAME_SPHERE) && (mode != SOLID_SURFACE_SPHERE) )
		{
		fprintf(stderr,
	"\ngoriented_sphere: mode=%d is not currently supported", mode);
		exit(-1);
		}
					/* north pole triangles */
	for (m = 1 ; m <= longitudes ; m++)
		{
		if (mode == WIRE_FRAME_SPHERE)
			glBegin(GL_LINE_LOOP);
		else if (mode == SOLID_SURFACE_SPHERE)
			glBegin(GL_POLYGON);
		glColor3d(sphere_vertex_color[0][0], 
				sphere_vertex_color[0][1],
				sphere_vertex_color[0][2]);
		glNormal3d(sphere_vertex_normal[0][0], 
				sphere_vertex_normal[0][1],
				sphere_vertex_normal[0][2]);
		glVertex3d(sphere_vertex[0][0], 
				sphere_vertex[0][1],
				sphere_vertex[0][2]);

		if (m < longitudes)
			{
			glColor3d(sphere_vertex_color[m + 1][0], 
				sphere_vertex_color[m + 1][1],
				sphere_vertex_color[m + 1][2]);
			glNormal3d(sphere_vertex_normal[m + 1][0], 
				sphere_vertex_normal[m + 1][1],
				sphere_vertex_normal[m + 1][2]);
			glVertex3d(sphere_vertex[m + 1][0], 
				sphere_vertex[m + 1][1],
				sphere_vertex[m + 1][2]);
			}
		else
			{
			glColor3d(sphere_vertex_color[1][0], 
				sphere_vertex_color[1][1],
				sphere_vertex_color[1][2]);
			glNormal3d(sphere_vertex_normal[1][0], 
				sphere_vertex_normal[1][1],
				sphere_vertex_normal[1][2]);
			glVertex3d(sphere_vertex[1][0], 
				sphere_vertex[1][1],
				sphere_vertex[1][2]);
			}

		glColor3d(sphere_vertex_color[m][0], 
				sphere_vertex_color[m][1],
				sphere_vertex_color[m][2]);
		glNormal3d(sphere_vertex_normal[m][0], 
				sphere_vertex_normal[m][1],
				sphere_vertex_normal[m][2]);
		glVertex3d(sphere_vertex[m][0], 
				sphere_vertex[m][1],
				sphere_vertex[m][2]);
		glEnd();
		}  /* end of m */
					/* middle quads */
	for (n = 0 ; n <= (latitudes - 3) ; n++)
		{
		for (m = 1 ; m <= longitudes ; m++)
			{
			if (mode == WIRE_FRAME_SPHERE)
				glBegin(GL_LINE_LOOP);
			else if (mode == SOLID_SURFACE_SPHERE)
				glBegin(GL_POLYGON);
			glColor3d(
				sphere_vertex_color[n*longitudes + m][0], 
				sphere_vertex_color[n*longitudes + m][1],
				sphere_vertex_color[n*longitudes + m][2]);
			glNormal3d(
				sphere_vertex_normal[n*longitudes + m][0], 
				sphere_vertex_normal[n*longitudes + m][1],
				sphere_vertex_normal[n*longitudes + m][2]);
			glVertex3d(
				sphere_vertex[n*longitudes + m][0], 
				sphere_vertex[n*longitudes + m][1],
				sphere_vertex[n*longitudes + m][2]);
			if (m < longitudes)
				{
				glColor3d(
				sphere_vertex_color[n*longitudes + m + 1][0], 
				sphere_vertex_color[n*longitudes + m + 1][1],
				sphere_vertex_color[n*longitudes + m + 1][2]);
				glNormal3d(
				sphere_vertex_normal[n*longitudes + m + 1][0], 
				sphere_vertex_normal[n*longitudes + m + 1][1],
				sphere_vertex_normal[n*longitudes + m + 1][2]);
				glVertex3d(
				sphere_vertex[n*longitudes + m + 1][0], 
				sphere_vertex[n*longitudes + m + 1][1],
				sphere_vertex[n*longitudes + m + 1][2]);
				}
			else
				{
				glColor3d(
				sphere_vertex_color[n*longitudes + 1][0], 
				sphere_vertex_color[n*longitudes + 1][1],
				sphere_vertex_color[n*longitudes + 1][2]);
				glNormal3d(
				sphere_vertex_normal[n*longitudes + 1][0], 
				sphere_vertex_normal[n*longitudes + 1][1],
				sphere_vertex_normal[n*longitudes + 1][2]);
				glVertex3d(
				sphere_vertex[n*longitudes + 1][0], 
				sphere_vertex[n*longitudes + 1][1],
				sphere_vertex[n*longitudes + 1][2]);
				}
			if (m < longitudes)
				{
				glColor3d(
			  sphere_vertex_color[(n+1)*longitudes + m + 1][0], 
			  sphere_vertex_color[(n+1)*longitudes + m + 1][1],
			  sphere_vertex_color[(n+1)*longitudes + m + 1][2]);
				glNormal3d(
			  sphere_vertex_normal[(n+1)*longitudes + m + 1][0], 
			  sphere_vertex_normal[(n+1)*longitudes + m + 1][1],
			  sphere_vertex_normal[(n+1)*longitudes + m + 1][2]);
				glVertex3d(
				sphere_vertex[(n+1)*longitudes + m + 1][0], 
				sphere_vertex[(n+1)*longitudes + m + 1][1],
				sphere_vertex[(n+1)*longitudes + m + 1][2]);
				}
			else
				{
				glColor3d(
			  sphere_vertex_color[(n+1)*longitudes + 1][0], 
			  sphere_vertex_color[(n+1)*longitudes + 1][1],
			  sphere_vertex_color[(n+1)*longitudes + 1][2]);
				glNormal3d(
			  sphere_vertex_normal[(n+1)*longitudes + 1][0], 
			  sphere_vertex_normal[(n+1)*longitudes + 1][1],
			  sphere_vertex_normal[(n+1)*longitudes + 1][2]);
				glVertex3d(
				sphere_vertex[(n+1)*longitudes + 1][0], 
				sphere_vertex[(n+1)*longitudes + 1][1],
				sphere_vertex[(n+1)*longitudes + 1][2]);
				}
			glColor3d(
				sphere_vertex_color[(n+1)*longitudes + m][0], 
				sphere_vertex_color[(n+1)*longitudes + m][1],
				sphere_vertex_color[(n+1)*longitudes + m][2]);
			glNormal3d(
				sphere_vertex_normal[(n+1)*longitudes + m][0], 
				sphere_vertex_normal[(n+1)*longitudes + m][1],
				sphere_vertex_normal[(n+1)*longitudes + m][2]);
			glVertex3d(
				sphere_vertex[(n+1)*longitudes + m][0], 
				sphere_vertex[(n+1)*longitudes + m][1],
				sphere_vertex[(n+1)*longitudes + m][2]);
			glEnd();

			}  /* end of m */
		}  /* end of n */
					/* south pole triangles */
	for (m = 1 ; m <= longitudes ; m++)
		{
		if (mode == WIRE_FRAME_SPHERE)
			glBegin(GL_LINE_LOOP);
		else if (mode == SOLID_SURFACE_SPHERE)
			glBegin(GL_POLYGON);
		glColor3d(
		  sphere_vertex_color[(latitudes - 1)*longitudes + 1][0],
		  sphere_vertex_color[(latitudes - 1)*longitudes + 1][1],
		  sphere_vertex_color[(latitudes - 1)*longitudes + 1][2]);
		glNormal3d(
		  sphere_vertex_normal[(latitudes - 1)*longitudes + 1][0],
		  sphere_vertex_normal[(latitudes - 1)*longitudes + 1][1],
		  sphere_vertex_normal[(latitudes - 1)*longitudes + 1][2]);
		glVertex3d(
		  sphere_vertex[(latitudes - 1)*longitudes + 1][0],
		  sphere_vertex[(latitudes - 1)*longitudes + 1][1],
		  sphere_vertex[(latitudes - 1)*longitudes + 1][2]);
		glColor3d(
		  sphere_vertex_color[(latitudes - 2)*longitudes + m][0], 
		  sphere_vertex_color[(latitudes - 2)*longitudes + m][1],
		  sphere_vertex_color[(latitudes - 2)*longitudes + m][2]);
		glNormal3d(
		  sphere_vertex_normal[(latitudes - 2)*longitudes + m][0], 
		  sphere_vertex_normal[(latitudes - 2)*longitudes + m][1],
		  sphere_vertex_normal[(latitudes - 2)*longitudes + m][2]);
		glVertex3d(
		  sphere_vertex[(latitudes - 2)*longitudes + m][0], 
		  sphere_vertex[(latitudes - 2)*longitudes + m][1],
		  sphere_vertex[(latitudes - 2)*longitudes + m][2]);
		if (m < longitudes)
			{
			glColor3d(
		  sphere_vertex_color[(latitudes - 2)*longitudes + m + 1][0], 
		  sphere_vertex_color[(latitudes - 2)*longitudes + m + 1][1],
		  sphere_vertex_color[(latitudes - 2)*longitudes + m + 1][2]);
			glNormal3d(
		  sphere_vertex_normal[(latitudes - 2)*longitudes + m + 1][0], 
		  sphere_vertex_normal[(latitudes - 2)*longitudes + m + 1][1],
		  sphere_vertex_normal[(latitudes - 2)*longitudes + m + 1][2]);
			glVertex3d(
			sphere_vertex[(latitudes - 2)*longitudes + m + 1][0], 
			sphere_vertex[(latitudes - 2)*longitudes + m + 1][1],
			sphere_vertex[(latitudes - 2)*longitudes + m + 1][2]);
			}
		else
			{
			glColor3d(
		  sphere_vertex_color[(latitudes - 2)*longitudes + 1][0], 
		  sphere_vertex_color[(latitudes - 2)*longitudes + 1][1],
		  sphere_vertex_color[(latitudes - 2)*longitudes + 1][2]);
			glNormal3d(
		  sphere_vertex_normal[(latitudes - 2)*longitudes + 1][0], 
		  sphere_vertex_normal[(latitudes - 2)*longitudes + 1][1],
		  sphere_vertex_normal[(latitudes - 2)*longitudes + 1][2]);
			glVertex3d(
			sphere_vertex[(latitudes - 2)*longitudes + 1][0], 
			sphere_vertex[(latitudes - 2)*longitudes + 1][1],
			sphere_vertex[(latitudes - 2)*longitudes + 1][2]);
			}
		glEnd();
		}  /* end of m */

	}  /* end of goriented sphere */

/* ------------------------------------------------------------------ */

void goriented_sphere_longitude(double sphere_vertex[][3], 
		double sphere_vertex_normal[][3],
		double sphere_vertex_color[][3], int k,
		double red, double green, double blue,
		int latitudes, int longitudes)
		/* Draw the k-th longitude (1 <= k <= longitudes)
	in the given color */
	{
	int	n;

	if ( (k < 1) || (k > longitudes) )	/* insurance */
		{
		fprintf(stderr,
			"\ngoriented_sphere_longitude: bad longitude=%d", k);
		return;
		}
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glLineWidth(2.0);
	glBegin(GL_LINE_STRIP);
		glColor3d(red, green, blue);
						/* north pole */
		glVertex3d(sphere_vertex[0][0], 
			sphere_vertex[0][1],
			sphere_vertex[0][2]);
		for (n = 0 ; n <= (latitudes - 2) ; n++)
			glVertex3d(
				sphere_vertex[n*longitudes + k][0], 
				sphere_vertex[n*longitudes + k][1],
				sphere_vertex[n*longitudes + k][2]);
						/* south pole */
		glVertex3d(
			sphere_vertex[(latitudes - 1)*longitudes + 1][0],
			sphere_vertex[(latitudes - 1)*longitudes + 1][1],
			sphere_vertex[(latitudes - 1)*longitudes + 1][2]);
	glEnd();
	glPopAttrib();
	}  /* end of goriented_sphere_longitude */

/* ------------------------------------------------------------------ */

void goriented_sphere_latitude(double sphere_vertex[][3], 
		double sphere_vertex_normal[][3],
		double sphere_vertex_color[][3], int k,
		double red, double green, double blue,
		int latitudes, int longitudes)
		/* Draw the k-th latitude (0 <= k <= (latitudes - 2) )
	in the given color */
	{
	int	m;

	if ( (k < 0) || (k > latitudes) )	/* insurance */
		{
		fprintf(stderr,
			"\ngoriented_sphere_latitude: bad latitude=%d", k);
		return;
		}
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glLineWidth(2.0);
	glBegin(GL_LINE_LOOP);
		glColor3d(red, green, blue);

	for (m = 1 ; m <= longitudes ; m++)
		{
		glVertex3d(
			sphere_vertex[k*longitudes + m][0], 
			sphere_vertex[k*longitudes + m][1],
			sphere_vertex[k*longitudes + m][2]);
		}  /* end of m */

	glEnd();
	glPopAttrib();
	}  /* end of goriented_sphere_latitude */

/* ------------------------------------------------------------------ */

void ginit_cylinder_vertices(double cylinder_vertex[][3], int longitudes)
		/* fill the cylinder_vertex[][3] 2D array with the 
	vertices of a cylinder of radius 1.0 and height 1.0 (above
	the xz-plane) using longitudes given in the following pattern:

	0 = unused (so we can index longitude same as sphere),

			  top circle 

	        1   2   3 ...              m

   	     	|   |   | ...  |   |   |   |

	   m +  1   2   3 ...              m

			bottom circle

	the caller MUST provide storage of AT LEAST

	double	cylinder_vertex[2*longitudes + 1][3]; */
	{
	int	m;
	double	theta;
					/* unused */
	cylinder_vertex[0][0] =  0.0;
	cylinder_vertex[0][1] =  1.0;
	cylinder_vertex[0][2] =  0.0;

					/* use cylindrical coordinates:
				theta in RADIAN measure */
	for (m = 1 ; m <= longitudes ; m++)
		{
		theta = 2.0 * SPHERE_PI *
			((double)(m-1)/(double)longitudes);
		cylinder_vertex[ m ][0] = cos(theta);
		cylinder_vertex[ m ][1] = 1.0;
		cylinder_vertex[ m ][2] = sin(theta);

		cylinder_vertex[ longitudes + m ][0] = cos(theta);
		cylinder_vertex[ longitudes + m ][1] = 0.0;
		cylinder_vertex[ longitudes + m ][2] = sin(theta);

/* *** DEBUG ***
	fprintf(stderr,
	"\nginit_cylinder_vertices: m=%d (%f,%f,%f) (%f,%f,%f)", m,
		cylinder_vertex[m][0],
		cylinder_vertex[m][1],
		cylinder_vertex[m][2],
		cylinder_vertex[longitudes + m][0],
		cylinder_vertex[longitudes + m][1],
		cylinder_vertex[longitudes + m][2]); */

		}  /* end of m */

	}  /* end of ginit_cylinder_vertices */

/* ------------------------------------------------------------------ */

void ginit_cylinder_vertex_normals(double cylinder_vertex_normal[][3], 
		int longitudes)
		/* fill the cylinder_vertex_normal[][3] 2D array with the 
	vertex normals.

	the caller MUST provide storage of AT LEAST

	double	cylinder_vertex[2*longitudes + 1][3]; */
	{
	int	m;
	double	theta;
					/* unused */
	cylinder_vertex_normal[0][0] =  0.0;
	cylinder_vertex_normal[0][1] =  1.0;
	cylinder_vertex_normal[0][2] =  0.0;

					/* use cylindrical coordinates:
				theta in RADIAN measure */
	for (m = 1 ; m <= longitudes ; m++)
		{
		theta = 2.0 * SPHERE_PI *
			((double)(m-1)/(double)longitudes);
		cylinder_vertex_normal[ m ][0] = cos(theta);
		cylinder_vertex_normal[ m ][1] = 0.0;
		cylinder_vertex_normal[ m ][2] = sin(theta);

		cylinder_vertex_normal[ longitudes + m ][0] = cos(theta);
		cylinder_vertex_normal[ longitudes + m ][1] = 0.0;
		cylinder_vertex_normal[ longitudes + m ][2] = sin(theta);
		}  /* end of m */

	}  /* end of ginit_cylinder_vertex_normals */

/* ------------------------------------------------------------------ */

void ginit_cylinder_vertex_colors(double cylinder_vertex_color[][3], int mode, 
		double	north_red, double north_green, double north_blue,
		double	south_red, double south_green, double south_blue,
		int longitudes)
		/* fill the cylinder_vertex_color[][3] 2D array with the 
	vertex colors according to the mode.  permissible 
	modes are:
			CYLINDER_COLOR_POLAR
			CYLINDER_COLOR_HUE

	only the caller MUST provide storage of AT LEAST

	double	cylinder_vertex[2*longitudes + 1][3]; */
	{
	int	m;
	double	theta, red, green, blue, diff_red, diff_green, diff_blue;

					/* unused */
	cylinder_vertex_color[0][0] =  north_red;
	cylinder_vertex_color[0][1] =  north_green;
	cylinder_vertex_color[0][2] =  north_blue;

	diff_red = south_red - north_red;
	diff_green = south_green - north_green;
	diff_blue = south_blue - north_blue;

					/* use cylindrical coordinates:
				theta in RADIAN measure */
	for (m = 1 ; m <= longitudes ; m++)
		{
		theta = 2.0 * SPHERE_PI *
			((double)(m-1)/(double)longitudes);
		if (mode == CYLINDER_COLOR_POLAR)
			{
			cylinder_vertex_color[ m ][0] = north_red;
			cylinder_vertex_color[ m ][1] = north_green;
			cylinder_vertex_color[ m ][2] = north_blue;

			cylinder_vertex_color[ longitudes + m ][0] = 
				south_red;
			cylinder_vertex_color[ longitudes + m ][1] = 
				south_green;
			cylinder_vertex_color[ longitudes + m ][2] = 
				south_blue;
			}
		else if (mode == CYLINDER_COLOR_HUE)
			{
			red = 0.2 + 
				( (0.1) * 0.8 *
				  ((double)((longitudes + 1) - m)/
				   (double)longitudes));
			green = 0.4 + 
				( (0.1) * 0.6 *
	( (1.0 - 2.0*fabs( ((double)m - (double)longitudes/2.0)/
				   (double)longitudes ) ) ) );
			blue = 0.3 + 
				( (0.1) * 0.7 *
				  ((double)m/
				   (double)longitudes));
			cylinder_vertex_color[ m ][0] = red;
			cylinder_vertex_color[ m ][1] = green;
			cylinder_vertex_color[ m ][2] = blue;

			red = 0.2 + 
				( (0.9) * 0.8 *
				  ((double)((longitudes + 1) - m)/
				   (double)longitudes));
			green = 0.4 + 
				( (0.9) * 0.6 *
	( (1.0 - 2.0*fabs( ((double)m - (double)longitudes/2.0)/
				   (double)longitudes ) ) ) );
			blue = 0.3 + 
				( (0.9) * 0.7 *
				  ((double)m/
				   (double)longitudes));
			cylinder_vertex_color[ longitudes + m ][0] = red;
			cylinder_vertex_color[ longitudes + m ][1] = green;
			cylinder_vertex_color[ longitudes + m ][2] = blue;
			}
		}  /* end of m */

	}  /* end of ginit_cylinder_vertex_colors */

/* ------------------------------------------------------------------ */

void goriented_cylinder(double cylinder_vertex[][3], 
		double cylinder_vertex_normal[][3],
		double cylinder_vertex_color[][3], int mode, int longitudes)
		/* Draw the oriented cylinder whose vertices have previously
	been put in the 2D array cylinder[][3] by ginit_cylinder_vertices()
	by drawing the faces in OUTWARD NORMAL orientation and the
	normals and colors indicated for the vertices. permissible 
	modes are:
			WIRE_FRAME_CYLINDER	
			SOLID_SURFACE_CYLINDER
	only */
	{
	int	m;
	double	theta;
					/* use cylindrical coordinates:
				theta in RADIAN measure */
	for (m = 1 ; m <= longitudes ; m++)
		{
		theta = 2.0 * SPHERE_PI *
			((double)(m-1)/(double)longitudes);
		cylinder_vertex_normal[ m ][0] = cos(theta);
		cylinder_vertex_normal[ m ][1] = 0.0;
		cylinder_vertex_normal[ m ][2] = sin(theta);

		cylinder_vertex_normal[ longitudes + m ][0] = cos(theta);
		cylinder_vertex_normal[ longitudes + m ][1] = 0.0;
		cylinder_vertex_normal[ longitudes + m ][2] = sin(theta);

		if (mode == WIRE_FRAME_CYLINDER)
			glBegin(GL_LINE_LOOP);
		else if (mode == SOLID_SURFACE_CYLINDER)
			glBegin(GL_POLYGON);
		glColor3d(
			cylinder_vertex_color[m][0], 
			cylinder_vertex_color[m][1],
			cylinder_vertex_color[m][2]);
		glNormal3d(
			cylinder_vertex_normal[m][0], 
			cylinder_vertex_normal[m][1],
			cylinder_vertex_normal[m][2]);
		glVertex3d(
			cylinder_vertex[m][0], 
			cylinder_vertex[m][1],
			cylinder_vertex[m][2]);
		if (m < longitudes)
			{
			glColor3d(
			cylinder_vertex_color[m + 1][0], 
			cylinder_vertex_color[m + 1][1],
			cylinder_vertex_color[m + 1][2]);
			glNormal3d(
			cylinder_vertex_normal[m + 1][0], 
			cylinder_vertex_normal[m + 1][1],
			cylinder_vertex_normal[m + 1][2]);
			glVertex3d(
			cylinder_vertex[m + 1][0], 
			cylinder_vertex[m + 1][1],
			cylinder_vertex[m + 1][2]);
			}
		else
			{
			glColor3d(
			cylinder_vertex_color[1][0], 
			cylinder_vertex_color[1][1],
			cylinder_vertex_color[1][2]);
			glNormal3d(
			cylinder_vertex_normal[1][0], 
			cylinder_vertex_normal[1][1],
			cylinder_vertex_normal[1][2]);
			glVertex3d(
			cylinder_vertex[1][0], 
			cylinder_vertex[1][1],
			cylinder_vertex[1][2]);
			}
		if (m < longitudes)
			{
			glColor3d(
			  cylinder_vertex_color[longitudes + m + 1][0], 
			  cylinder_vertex_color[longitudes + m + 1][1],
			  cylinder_vertex_color[longitudes + m + 1][2]);
			glNormal3d(
			  cylinder_vertex_normal[longitudes + m + 1][0], 
			  cylinder_vertex_normal[longitudes + m + 1][1],
			  cylinder_vertex_normal[longitudes + m + 1][2]);
			glVertex3d(
			  cylinder_vertex[longitudes + m + 1][0], 
			  cylinder_vertex[longitudes + m + 1][1],
			  cylinder_vertex[longitudes + m + 1][2]);
			}
		else
			{
			glColor3d(
			  cylinder_vertex_color[longitudes + 1][0], 
			  cylinder_vertex_color[longitudes + 1][1],
			  cylinder_vertex_color[longitudes + 1][2]);
			glNormal3d(
			  cylinder_vertex_normal[longitudes + 1][0], 
			  cylinder_vertex_normal[longitudes + 1][1],
			  cylinder_vertex_normal[longitudes + 1][2]);
			glVertex3d(
			  cylinder_vertex[longitudes + 1][0], 
			  cylinder_vertex[longitudes + 1][1],
			  cylinder_vertex[longitudes + 1][2]);
			}
		glColor3d(
			cylinder_vertex_color[longitudes + m][0], 
			cylinder_vertex_color[longitudes + m][1],
			cylinder_vertex_color[longitudes + m][2]);
		glNormal3d(
			cylinder_vertex_normal[longitudes + m][0], 
			cylinder_vertex_normal[longitudes + m][1],
			cylinder_vertex_normal[longitudes + m][2]);
		glVertex3d(
			cylinder_vertex[longitudes + m][0], 
			cylinder_vertex[longitudes + m][1],
			cylinder_vertex[longitudes + m][2]);
		glEnd();
		}  /* end of m */

	}  /* end of goriented_cylinder */

/* ------------------------------------------------------------------ */

void gpavement(int rows, int columns, int mode,
		double groutwidth, double thickness, double randomheight,
		double brick_red, double brick_green, double brick_blue,
		double side_red, double side_green, double side_blue,
		double mortar_red, double mortar_green, double mortar_blue)
		/* this function draws a unit square 
	( 0.0 < x < 1.0 , 0.0 < y < 1.0 , z=0 ) pavement of bricks 
	of color brick_red, brick_green, brick_blue, with sides of
	color side_red, side_green, side_blue, and mortar of 
	color mortar_red, mortar_green, mortar_blue. The mortar
	width (the distance between the bricks) is given by
	grout_width. The thickness of the bricks is given by thickness.
	If randomheight > 0.0 the level of the bricks will vary
	randomly as well.  The mode must be one of the following:

			MORTAR_ONLY_PAVEMENT
			BRICKS_ONLY_PAVEMENT
			BOTH_PAVEMENT  */
	{
	int	n,m;
	time_t	ltime;
	double	x[4], z[4], mortar_up, rand_up;
					/* pattern: 

					x3,z3 ----------- x2,z2
					  |                 |
					  |                 |
					  |                 |
					  |                 |
					  |                 |
					x0,z0 ----------- x1,z1 */


	time(&ltime);
	srand((unsigned int)ltime);
	rand_up = ((2.0*(double)rand())/(double)RAND_MAX) - 1.0;
	mortar_up = - 0.025 * ((1.0)/(double)rows);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	for (n = 0 ; n < rows ; n++)
	    {
	    z[0] = ((double)n * (1.0/(double)rows)) + (groutwidth/2.0);
	    z[1] = z[0];
	    z[2] = ((double)(n + 1) * (1.0/(double)rows)) - (groutwidth/2.0);
	    z[3] = z[2];
	    for (m = 0 ; m < columns ; m++)
		{
	        x[0] = ((double)m * (1.0/(double)columns)) + (groutwidth/2.0);
	        x[1] = ((double)(m + 1) * (1.0/(double)columns)) -
				(groutwidth/2.0);
	        x[2] = x[1];
	        x[3] = x[0];

					/* do bricks */
		if ( (mode == BRICKS_ONLY_PAVEMENT) ||	
			(mode == BOTH_PAVEMENT) )
			{
			if (randomheight > 0.0)	/* -1.0 < rand_up < 1.0 */
				rand_up = 
				((2.0*(double)rand())/(double)RAND_MAX) - 1.0;
			else
				rand_up = 0.0;		/* even height */
			rand_up = rand_up * randomheight; /* scale */

							/* top surface */	
			glColor3d(brick_red, brick_green, brick_blue);
			glBegin(GL_POLYGON);
				glNormal3d(0.0, 1.0, 0.0);
				glVertex3d(x[0], rand_up, z[0]);
				glNormal3d(0.0, 1.0, 0.0);
				glVertex3d(x[1], rand_up, z[1]);
				glNormal3d(0.0, 1.0, 0.0);
				glVertex3d(x[2], rand_up, z[2]);
				glNormal3d(0.0, 1.0, 0.0);
				glVertex3d(x[3], rand_up, z[3]);
			glEnd();
							/* bottom surface */	
			glColor3d(brick_red, brick_green, brick_blue);
			glBegin(GL_POLYGON);
				glNormal3d(0.0, -1.0, 0.0);
				glVertex3d(x[0], rand_up - thickness, z[0]);
				glNormal3d(0.0, -1.0, 0.0);
				glVertex3d(x[3], rand_up - thickness, z[3]);
				glNormal3d(0.0, -1.0, 0.0);
				glVertex3d(x[2], rand_up - thickness, z[2]);
				glNormal3d(0.0, -1.0, 0.0);
				glVertex3d(x[1], rand_up - thickness, z[1]);
			glEnd();
							/* bottom side */
			glColor3d(side_red, side_green, side_blue);
			glBegin(GL_POLYGON);
				glNormal3d(0.0, 0.0, -1.0);
				glVertex3d(x[0], rand_up, z[0]);
				glNormal3d(0.0, 0.0, -1.0);
				glVertex3d(x[0], rand_up - thickness, z[0]);
				glNormal3d(0.0, 0.0, -1.0);
				glVertex3d(x[1], rand_up - thickness, z[1]);
				glNormal3d(0.0, 0.0, -1.0);
				glVertex3d(x[1], rand_up, z[1]);
			glEnd();
							/* right side */
			glColor3d(side_red, side_green, side_blue);
			glBegin(GL_POLYGON);
				glNormal3d(1.0, 0.0, 0.0);
				glVertex3d(x[1], rand_up, z[1]);
				glNormal3d(1.0, 0.0, 0.0);
				glVertex3d(x[1], rand_up - thickness, z[1]);
				glNormal3d(1.0, 0.0, 0.0);
				glVertex3d(x[2], rand_up - thickness, z[2]);
				glNormal3d(1.0, 0.0, 0.0);
				glVertex3d(x[2], rand_up, z[2]);
			glEnd();
							/* top side */
			glColor3d(side_red, side_green, side_blue);
			glBegin(GL_POLYGON);
				glNormal3d(0.0, 0.0, 1.0);
				glVertex3d(x[2], rand_up, z[2]);
				glNormal3d(0.0, 0.0, 1.0);
				glVertex3d(x[2], rand_up - thickness, z[2]);
				glNormal3d(0.0, 0.0, 1.0);
				glVertex3d(x[3], rand_up - thickness, z[3]);
				glNormal3d(0.0, 0.0, 1.0);
				glVertex3d(x[3], rand_up, z[3]);
			glEnd();
							/* left side */
			glColor3d(side_red, side_green, side_blue);
			glBegin(GL_POLYGON);
				glNormal3d(-1.0, 0.0, 0.0);
				glVertex3d(x[3], rand_up, z[3]);
				glNormal3d(-1.0, 0.0, 0.0);
				glVertex3d(x[3], rand_up - thickness, z[3]);
				glNormal3d(-1.0, 0.0, 0.0);
				glVertex3d(x[0], rand_up - thickness, z[0]);
				glNormal3d(-1.0, 0.0, 0.0);
				glVertex3d(x[0], rand_up, z[0]);
			glEnd();

			}
					/* do mortar */
		if ( (mode == MORTAR_ONLY_PAVEMENT) ||	
			(mode == BOTH_PAVEMENT) )
			{
		  	glLineWidth(4.0);
			glColor3d(mortar_red, mortar_green, mortar_blue);
			glBegin(GL_LINE_LOOP);
				glVertex3d(x[0], rand_up + mortar_up, z[0]);
				glVertex3d(x[1], rand_up + mortar_up, z[1]);
				glVertex3d(x[2], rand_up + mortar_up, z[2]);
				glVertex3d(x[3], rand_up + mortar_up, z[3]);
			glEnd();
			}
		}  /* end of for (m = 0 ; m < columns ; m++) */
	    }  /* end of for (n = 0 ; n < rows ; n++) */
	glPopAttrib();
	}  /* end of gpavement */

/* ------------------------------------------------------------------ */

void grect_pillar(int mode,
		double north_red, double north_green, double north_blue,
		double south_red, double south_green, double south_blue)
		/* This function draws a pillar of dimensions
	( 0.0 < x < 1.0 , 0.0 < y < 1.0 , 0.0 < z < 1.0 ) interpolating
	the colors between the north pole and the south pole. Mode
	must be one of
				WIRE_FRAME_PILLAR 
				SOLID_SURFACE_PILLAR */
	{
	double	vertex[8][3];
	double	vertex_color[8][3];
	
			/* Draw the pillar cube by drawing the
		four faces in OUTWARD NORMAL orientation 

				        3 ------------ 2

				        |              |
				  7 ------------- 6    |
				  |     |         |    |
				  |     |         |    |
				  |     |         |    |
				  |     |         |    |
				  |     |         |    |
				  |               |    
				  |     0 ------------ 1
				  |               |

				  4 ------------- 5   */

	vertex[0][0] =  0.0;
	vertex[0][1] =  0.0;
	vertex[0][2] =  0.0;
	vertex[1][0] =  1.0;
	vertex[1][1] =  0.0;
	vertex[1][2] =  0.0;
	vertex[2][0] =  1.0;
	vertex[2][1] =  1.0;
	vertex[2][2] =  0.0;
	vertex[3][0] =  0.0;
	vertex[3][1] =  1.0;
	vertex[3][2] =  0.0;

	vertex[4][0] =  0.0;
	vertex[4][1] =  0.0;
	vertex[4][2] =  1.0;
	vertex[5][0] =  1.0;
	vertex[5][1] =  0.0;
	vertex[5][2] =  1.0;
	vertex[6][0] =  1.0;
	vertex[6][1] =  1.0;
	vertex[6][2] =  1.0;
	vertex[7][0] =  0.0;
	vertex[7][1] =  1.0;
	vertex[7][2] =  1.0;

	vertex_color[0][0] =  south_red;
	vertex_color[0][1] =  south_green;
	vertex_color[0][2] =  south_blue;
	vertex_color[1][0] =  vertex_color[0][0];
	vertex_color[1][1] =  vertex_color[0][1];
	vertex_color[1][2] =  vertex_color[0][2];
	vertex_color[4][0] =  vertex_color[0][0];
	vertex_color[4][1] =  vertex_color[0][1];
	vertex_color[4][2] =  vertex_color[0][2];
	vertex_color[5][0] =  vertex_color[0][0];
	vertex_color[5][1] =  vertex_color[0][1];
	vertex_color[5][2] =  vertex_color[0][2];

	vertex_color[2][0] =  north_red;
	vertex_color[2][1] =  north_green;
	vertex_color[2][2] =  north_blue;
	vertex_color[3][0] =  vertex_color[2][0];
	vertex_color[3][1] =  vertex_color[2][1];
	vertex_color[3][2] =  vertex_color[2][2];
	vertex_color[6][0] =  vertex_color[2][0];
	vertex_color[6][1] =  vertex_color[2][1];
	vertex_color[6][2] =  vertex_color[2][2];
	vertex_color[7][0] =  vertex_color[2][0];
	vertex_color[7][1] =  vertex_color[2][1];
	vertex_color[7][2] =  vertex_color[2][2];

	glPushAttrib(GL_ALL_ATTRIB_BITS);
		if (mode == WIRE_FRAME_PILLAR)
			glBegin(GL_LINE_LOOP);
		else					/* right face */
			glBegin(GL_POLYGON);
		glColor3d(vertex_color[1][0], vertex_color[1][1],
			vertex_color[1][2]);
		glNormal3d(1.0, 0.0, 0.0);
		glVertex3d(vertex[1][0], vertex[1][1], vertex[1][2]);
		glColor3d(vertex_color[2][0], vertex_color[2][1],
			vertex_color[2][2]);
		glNormal3d(1.0, 0.0, 0.0);
		glVertex3d(vertex[2][0], vertex[2][1], vertex[2][2]);
		glColor3d(vertex_color[6][0], vertex_color[6][1],
			vertex_color[6][2]);
		glNormal3d(1.0, 0.0, 0.0);
		glVertex3d(vertex[6][0], vertex[6][1], vertex[6][2]);
		glColor3d(vertex_color[5][0], vertex_color[5][1],
			vertex_color[5][2]);
		glNormal3d(1.0, 0.0, 0.0);
		glVertex3d(vertex[5][0], vertex[5][1], vertex[5][2]);
		glEnd();

		if (mode == WIRE_FRAME_PILLAR)
			glBegin(GL_LINE_LOOP);
		else					/* back face */
			glBegin(GL_POLYGON);
		glColor3d(vertex_color[0][0], vertex_color[0][1],
			vertex_color[0][2]);
		glNormal3d(0.0, 0.0, -1.0);
		glVertex3d(vertex[0][0], vertex[0][1], vertex[0][2]);
		glColor3d(vertex_color[3][0], vertex_color[3][1],
			vertex_color[3][2]);
		glNormal3d(0.0, 0.0, -1.0);
		glVertex3d(vertex[3][0], vertex[3][1], vertex[3][2]);
		glColor3d(vertex_color[2][0], vertex_color[2][1],
			vertex_color[2][2]);
		glNormal3d(0.0, 0.0, -1.0);
		glVertex3d(vertex[2][0], vertex[2][1], vertex[2][2]);
		glColor3d(vertex_color[1][0], vertex_color[1][1],
			vertex_color[1][2]);
		glNormal3d(0.0, 0.0, -1.0);
		glVertex3d(vertex[1][0], vertex[1][1], vertex[1][2]);
		glEnd();

		if (mode == WIRE_FRAME_PILLAR)
			glBegin(GL_LINE_LOOP);
		else					/* left face */
			glBegin(GL_POLYGON);
		glColor3d(vertex_color[0][0], vertex_color[0][1],
			vertex_color[0][2]);
		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3d(vertex[0][0], vertex[0][1], vertex[0][2]);
		glColor3d(vertex_color[4][0], vertex_color[4][1],
			vertex_color[4][2]);
		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3d(vertex[4][0], vertex[4][1], vertex[4][2]);
		glColor3d(vertex_color[7][0], vertex_color[7][1],
			vertex_color[7][2]);
		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3d(vertex[7][0], vertex[7][1], vertex[7][2]);
		glColor3d(vertex_color[3][0], vertex_color[3][1],
			vertex_color[3][2]);
		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3d(vertex[3][0], vertex[3][1], vertex[3][2]);
		glEnd();

		if (mode == WIRE_FRAME_PILLAR)
			glBegin(GL_LINE_LOOP);
		else					/* front face */
			glBegin(GL_POLYGON);
		glColor3d(vertex_color[4][0], vertex_color[4][1],
			vertex_color[4][2]);
		glNormal3d(0.0, 0.0, 1.0);
		glVertex3d(vertex[4][0], vertex[4][1], vertex[4][2]);
		glColor3d(vertex_color[5][0], vertex_color[5][1],
			vertex_color[5][2]);
		glNormal3d(0.0, 0.0, 1.0);
		glVertex3d(vertex[5][0], vertex[5][1], vertex[5][2]);
		glColor3d(vertex_color[6][0], vertex_color[6][1],
			vertex_color[6][2]);
		glNormal3d(0.0, 0.0, 1.0);
		glVertex3d(vertex[6][0], vertex[6][1], vertex[6][2]);
		glColor3d(vertex_color[7][0], vertex_color[7][1],
			vertex_color[7][2]);
		glNormal3d(0.0, 0.0, 1.0);
		glVertex3d(vertex[7][0], vertex[7][1], vertex[7][2]);
		glEnd();

	glPopAttrib();
	}  /* end of grect_pillar */

