#include"stdafx.h"
#include <GL/glut.h>
#include "cs470.h"		
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define MAX_CPTS  25
#define	RGBA_COLOR	1
#define	BGROUND_RED	 		0.8
#define	BGROUND_GREEN		0.8
#define	BGROUND_BLUE	 	0.8
#define	HIGHL_RED		0.0
#define	HIGHL_GREEN		0.0
#define	HIGHL_BLUE		1.0

#define	Z_NEAR		 1.0
#define	Z_FAR		-1.0

#define	INITIAL_WIDTH	1000
#define	INITIAL_HEIGHT	 500
#define	START_RASTX	  20
#define	START_RASTY	  20
#define BASELINE_SKIP	  15

struct 	_text_box	message_box;

#define	NONE			0
#define NEW				1
#define	LINE			2
#define	CURVE			3
#define	OVAL			4
#define	R_RECTANGLE		5
#define	HEXAGON			6
#define	FILL			7
#define	CROP			8
#define	ERASE			9
#define	SAVE			10
#define	TRIANGLE		11
#define	RED 			12
#define	BLUE			13
#define	GREEN			14
#define	EXIT			15

float	PAINT_RED	= 0.8f;
float	PAINT_GREEN = 0.5f;
float	PAINT_BLUE	= 0.3f;

struct	_user_status {
	int	oper;
	int	oper_complete;
	int	oper_stage;
} user;

struct	_button_box user_button[] =	{
	{ "0=None"          ,   0.0,   0.0,   0.0,   0.0 },
	{ "New"             ,  20.0, 452.0, 250.0, 480.0 },
	{ "Line"            ,  20.0, 418.0, 250.0, 447.0 },
	{ "Curve"           ,  20.0, 384.0, 250.0, 413.0 }, //curve
	{ "Cube"            ,  20.0, 350.0, 250.0, 379.0 },  //oval 
	{ "Cuboid"          ,  20.0, 316.0, 250.0, 345.0 },  // r rect
	{ "Cylinder"        ,  20.0, 282.0, 250.0, 311.0 }, // hexagon
	{ "Fill"            ,  20.0, 248.0, 250.0, 277.0 },
	{ "Crop"            ,  20.0, 214.0, 250.0, 243.0 },
	{ "Erase"           ,  20.0, 180.0, 250.0, 209.0 },
	{ "Save"            ,  20.0, 146.0, 250.0, 175.0 },
	{ "Cone"            ,  20.0, 112.0, 250.0, 141.0 }, //triangle
	{ "Red"             ,  20.0,  78.0, 250.0, 107.0 },
	{ "Blue"            ,  20.0,  44.0, 250.0,  73.0 },
	{ "Green"           ,  20.0,  10.0, 250.0,  39.0 },
	{ "_EOINPUT"        ,   0.0,   0.0,   0.0,   0.0 } };

#define	BUTTON_TEXT_SIZE	22.0

struct	_last_events	ev;	

void myinit(void);		

void display(void);
void point_event(int button, int state, int x, int y);
void motion_event(int x, int y);


GLfloat cpts[MAX_CPTS][3];
int ncpts = 0;

int mousex, mousey;
int a = 0;
unsigned int pixels1[INITIAL_WIDTH][INITIAL_HEIGHT] = {0};
unsigned int pixels2[INITIAL_WIDTH][INITIAL_HEIGHT] = {0};
int startdrawing = 0;

/* ------------------------------------------------------------------ */

void drawCurves()
{
	for (int y=0; y<INITIAL_HEIGHT; y++)
	{
		for (int x=0; x<INITIAL_WIDTH; x++)
		{
			pixels1[x][y] = pixels2[x][y];
		}
	}
	display();
    int i;
	glColor3d(PAINT_RED, PAINT_GREEN, PAINT_BLUE);
    for(i=0; i<ncpts-3; i +=3)
    {
		printf("here");
        glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, cpts[i]);
        glMapGrid1f(30, 0.0, 1.0);
        glEvalMesh1(GL_LINE, 0, 30);
    }
    glFlush();
	user.oper_complete = 1;
	startdrawing = 0;
	user.oper = NONE;
	user.oper_stage = 0;
	glReadPixels(0,0,INITIAL_WIDTH,INITIAL_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,pixels1);
	glColor3d(1.0, 0.0, 0.0);
}

void main(int argc, char *argv[], char *envp[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);  
	glutInitWindowSize(INITIAL_WIDTH, INITIAL_HEIGHT);	
	glutInitWindowPosition(10, 10);	

	glutCreateWindow("Welcome to Peindre");	

	myinit(); 			
	glEnable(GL_MAP1_VERTEX_3);
	glutMouseFunc(point_event);
	glutMotionFunc(motion_event);
	glutDisplayFunc(display);
	glutMainLoop();
}
    
void myinit(void)
{
	gflush_keybbuf(&ev);
	ev.reshape_width = INITIAL_WIDTH;
	ev.reshape_height = INITIAL_HEIGHT;
	ev.keyboard_khar = '\0';
	ev.keyboard_x = 0;
	ev.keyboard_y = 0;
	ev.speckey_button = 0;
	ev.speckey_x = 0;
	ev.speckey_y = 0;
	ev.mouse_button = 0;
	ev.mouse_state = 0;	
	ev.mouse_x = 0;
	ev.mouse_y = 0;
	ev.display_time = 0;	
	ev.keyboard_time = 0;	
	ev.mouse_time = 0;
	ev.reshape_time = 0;

	user.oper = NONE; 
	user.oper_complete = 0;
	user.oper_stage = 0;

	glClearColor((float)BGROUND_RED, (float)BGROUND_GREEN, (float)BGROUND_BLUE, (float)1.0);
	glColor3d(1.0, 0.0, 0.0);
	ginit_stroke(GLUT_STROKE_ROMAN);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, (double)INITIAL_WIDTH, 0.0,(double)INITIAL_HEIGHT, - 1000,  1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, INITIAL_WIDTH, INITIAL_HEIGHT);	
	glFlush();
}

void motion_event(int x, int y)
{
	mousex = x;
	mousey = y;
	glutPostRedisplay();
}

void floodfill(int x, int y, float z[3])
{
	float mm[3] = {0};
	glReadPixels(x,y,1,1,GL_RGB,GL_FLOAT,mm);
	if ( mm[0] == z[0] && mm[1] == z[1] && mm[2] == z[2] ) 
	{
		glBegin(GL_POINTS);
			glVertex2i(x, y);
		glEnd();

		floodfill ( x+1, y, z );
		floodfill ( x-1, y, z );
		floodfill ( x, y+1, z );
		floodfill ( x, y-1, z );
	}
}

void memxor1(int bytes, char *dst, char *src)
{
	while(bytes--)
		*dst++ ^= *src++;
}

int mstate;
int sposx,sposy;
void point_event(int button, int state, int x, int y)
{
	mstate = state;
	mousex = x;
	mousey = y;
	if ( (state == GLUT_UP) && ( (user.oper == NONE) || (user.oper_complete) ) )
	{
		user.oper_complete = 0;
		user.oper_stage = 0;
		user.oper = gget_pick(user_button, x, y);
		gpost_buttons(user_button, BUTTON_TEXT_SIZE, user.oper, HIGHL_RED, HIGHL_GREEN, HIGHL_BLUE);
		glFlush();
		if( (state == GLUT_UP) && (user.oper == NEW) && (user.oper_complete == 0) )
		{
			a = 0;
			user.oper_complete = 1;
			startdrawing = 0;
			user.oper = NONE;
			user.oper_stage = 0;
		}
		if( (state == GLUT_UP) && (user.oper == SAVE) && (user.oper_complete == 0) )
		{
			char * pix = (char *)malloc(INITIAL_WIDTH * INITIAL_HEIGHT * 3);
			glReadPixels(0, 0, INITIAL_WIDTH, INITIAL_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE ,(void *)pix);
			int i;
			for(i=0;i<(INITIAL_HEIGHT/2);i++)
			{
				char *c = pix + i*(INITIAL_WIDTH*3);
				char *d = pix + (INITIAL_HEIGHT-1-i)*(INITIAL_WIDTH*3);
				memxor1((INITIAL_WIDTH*3), c, d);
				memxor1((INITIAL_WIDTH*3), d, c);
				memxor1((INITIAL_WIDTH*3), c, d);
			}
			FILE *f = fopen("out.ppm", "wb");
			fprintf(f, "P6\n%i %i\n255\n", INITIAL_WIDTH, INITIAL_HEIGHT);
			fwrite(pix, 1, INITIAL_WIDTH*INITIAL_HEIGHT*3, f);
			fclose(f);
			free(pix);
			user.oper_complete = 1;
			startdrawing = 0;
			user.oper = NONE;
			user.oper_stage = 0;
		}
		if( (state == GLUT_UP) && (user.oper == RED) && (user.oper_complete == 0) )
		{
			PAINT_RED	= 1.0f;
			PAINT_GREEN = 0.0f;
			PAINT_BLUE	= 0.0f;
			user.oper_complete = 1;
			startdrawing = 0;
			user.oper = NONE;
			user.oper_stage = 0;
		}
		if( (state == GLUT_UP) && (user.oper == BLUE) && (user.oper_complete == 0) )
		{
			PAINT_RED	= 0.0f;
			PAINT_GREEN = 0.0f;
			PAINT_BLUE	= 1.0f;
			user.oper_complete = 1;
			startdrawing = 0;
			user.oper = NONE;
			user.oper_stage = 0;
		}
		if( (state == GLUT_UP) && (user.oper == GREEN) && (user.oper_complete == 0) )
		{
			PAINT_RED	= 0.0f;
			PAINT_GREEN = 1.0f;
			PAINT_BLUE	= 0.0f;
			user.oper_complete = 1;
			startdrawing = 0;
			user.oper = NONE;
			user.oper_stage = 0;
		}
		if( (state == GLUT_UP) && (user.oper == EXIT) && (user.oper_complete == 0) )
		{
			exit(0);
		}
		if( (state == GLUT_UP) && (user.oper == CURVE) && (user.oper_complete == 0) )
		{
			ncpts = 0;
			for (int y=0; y<INITIAL_HEIGHT; y++)
			{
				for (int x=0; x<INITIAL_WIDTH; x++)
				{
					pixels2[x][y] = pixels1[x][y];
				}
			}
		}
	}
	else if (state == GLUT_DOWN && user.oper == CURVE && !user.oper_complete && button == GLUT_RIGHT_BUTTON)
	{
		int wx, wy;
		gwininput2output(x, y, &wx, &wy);

		if (ncpts == MAX_CPTS) return;

		cpts[ncpts][0] = wx;
		cpts[ncpts][1] = wy;
		cpts[ncpts][2] = 0.0;
		ncpts++;

		glColor3d(PAINT_RED, PAINT_GREEN, PAINT_BLUE);
		glPointSize(5.0);
		glBegin(GL_POINTS);
			glVertex2i(wx, wy);
		glEnd();
		glFlush();
		glPointSize(1.0);
		glColor3d(1.0, 0.0, 0.0);
		glReadPixels(0,0,INITIAL_WIDTH,INITIAL_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,pixels1);
	}
	else if (state == GLUT_DOWN && user.oper == CURVE && !user.oper_complete && button == GLUT_LEFT_BUTTON)
	{
		drawCurves();
	}
	else if (state == GLUT_DOWN && user.oper != NONE && !user.oper_complete && user.oper_stage == 0 && user.oper != FILL)
	{
		startdrawing = 1;
		mstate = state;
		user.oper_stage = 1;
		gwininput2output(x, y, &sposx, &sposy);
	}
	else if( (state == GLUT_UP) && (user.oper == FILL) && (user.oper_complete == 0) )
	{
		float mm[3] = {0};
		int s,t;
		gwininput2output(x, y, &s, &t);
		glReadPixels(s,t,1,1,GL_RGB,GL_FLOAT,mm);
		glColor3d(PAINT_RED, PAINT_GREEN, PAINT_BLUE);
		floodfill(s,t,mm);
		glColor3f(1.0, 0.0, 0.0);
		glFlush();
		user.oper_complete = 1;
		startdrawing = 0;
		user.oper = NONE;
		user.oper_stage = 0;
		glReadPixels(0,0,INITIAL_WIDTH,INITIAL_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,pixels1);
	}
	glutPostRedisplay();
}

void clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(a == 1)
		glDrawPixels(INITIAL_WIDTH,INITIAL_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,pixels1);
}

GLUquadricObj *quadratic; 
int hexx,hexy;
void display(void)
{
	clear();
	if(a == 0)
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_POLYGON);
			glVertex3f(270.0, 10.0,  0.0);
			glVertex3f(270.0, 490.0, 0.0);
			glVertex3f(990.0, 490.0, 0.0);
			glVertex3f(990.0, 10.0, 0.0);
		glEnd();
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_POLYGON);
			glVertex3f(271.0, 11.0,  0.0);
			glVertex3f(271.0, 489.0, 0.0);
			glVertex3f(989.0, 489.0, 0.0);
			glVertex3f(989.0, 11.0, 0.0);
		glEnd();
		glPopAttrib();
		glReadPixels(0,0,INITIAL_WIDTH,INITIAL_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,pixels1);
		a = 1;
	}
	
	int xx1,yy1;
	if(user.oper != NONE && !user.oper_complete && startdrawing == 1)
	{
		switch(user.oper) 
		{
			case LINE:
				gwininput2output(mousex, mousey, &xx1, &yy1);
				glPushAttrib(GL_ALL_ATTRIB_BITS);
				glColor3d(PAINT_RED, PAINT_GREEN, PAINT_BLUE);
				glBegin(GL_LINES);
					glVertex3f(sposx, sposy, 0.0);
					glVertex3f(xx1, yy1, 0.0);
				glEnd();
				glPopAttrib();
				//glFlush();
				if(mstate == GLUT_UP)
				{
					user.oper_complete = 1;
					startdrawing = 0;
					user.oper = NONE;
					user.oper_stage = 0;
					glReadPixels(0,0,INITIAL_WIDTH,INITIAL_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,pixels1);
				}
				break;
			case OVAL:
				int i;
				{
					gwininput2output(mousex, mousey, &xx1, &yy1);
					glPushAttrib(GL_ALL_ATTRIB_BITS);
					glColor3d(PAINT_RED, PAINT_GREEN, PAINT_BLUE);
					double a;
					if(sposx>xx1 || sposx<xx1) a=(sposx-xx1)/2.41; else if(sposx==xx1) a=0;
					glBegin(GL_LINES);
						glVertex3f(xx1-a,yy1-abs(a),-10);
						glVertex3f(xx1-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(xx1,yy1,10);
						glVertex3f(xx1-a,yy1-abs(a),-10);
						glVertex3f(xx1,sposx-xx1+yy1,10);
						glVertex3f(xx1-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(xx1,yy1,10);
						glVertex3f(xx1,sposx-xx1+yy1,10);
						glVertex3f(sposx-a,yy1-abs(a),-10);
						glVertex3f(sposx-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(sposx,yy1,10);
						glVertex3f(sposx-a,yy1-abs(a),-10);
						glVertex3f(sposx,sposx-xx1+yy1,10);
						glVertex3f(sposx-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(sposx,yy1,10);
						glVertex3f(sposx,sposx-xx1+yy1,10);
						glVertex3f(xx1,sposx-xx1+yy1,10);
						glVertex3f(sposx,sposx-xx1+yy1,10);
						glVertex3f(xx1-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(sposx-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(xx1,sposx-xx1+yy1,10);
						glVertex3f(xx1-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(sposx,sposx-xx1+yy1,10);
						glVertex3f(sposx-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(xx1,yy1,10);
						glVertex3f(sposx,yy1,10);
						glVertex3f(xx1-a,yy1-abs(a),-10);
						glVertex3f(sposx-a,yy1-abs(a),-10);
						glVertex3f(xx1,yy1,10);
						glVertex3f(xx1-a,yy1-abs(a),-10);
						glVertex3f(sposx,yy1,10);
						glVertex3f(sposx-a,yy1-abs(a),-10);
					glEnd();
					glPopAttrib();
					//glFlush();
					if(mstate == GLUT_UP)
					{
						user.oper_complete = 1;
						startdrawing = 0;
						user.oper = NONE;
						user.oper_stage = 0;
						glReadPixels(0,0,INITIAL_WIDTH,INITIAL_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,pixels1);
					}
				}
				break;
			case HEXAGON:
				gwininput2output(mousex, mousey, &xx1, &yy1);
				glPushMatrix();
				glPushAttrib(GL_ALL_ATTRIB_BITS);
				quadratic=gluNewQuadric();
				gluQuadricDrawStyle(quadratic, GLU_LINE);
				gluQuadricNormals(quadratic, GLU_SMOOTH);
				gluQuadricTexture(quadratic, GL_TRUE);
				
				glTranslatef(sposx,sposy,0);		

				if(user.oper_stage == 1)
				{
					glRotatef(+(atan (((GLfloat)yy1-sposy)/((GLfloat)xx1-sposx)) * 180 / 3.14)+90, 1.0f, 0.0f, 0.0f);
					glRotatef(+(atan (((GLfloat)yy1-sposy)/((GLfloat)xx1-sposx)) * 180 / 3.14)-180, 1.0f, 0.0f, 0.0f);
					glColor3d(PAINT_RED, PAINT_GREEN, PAINT_BLUE);
					gluCylinder(quadratic,sqrt(pow(sposx-(GLfloat)xx1,2)+pow(sposy-(GLfloat)yy1,2))/2,sqrt(pow(sposx-(GLfloat)xx1,2)+pow(sposy-(GLfloat)yy1,2))/2,sqrt(pow(sposx-(GLfloat)xx1,2)+pow(sposy-(GLfloat)yy1,2)),32,32);
				}
				/*else
				{
					glRotatef(+(atan (((GLfloat)yy1-sposy)/((GLfloat)xx1-sposx)) * 180 / 3.14)+90, 0.0f, 1.0f, 0.0f);
					glRotatef(+(atan (((GLfloat)yy1-sposy)/((GLfloat)xx1-sposx)) * 180 / 3.14)-180, 0.0f, 1.0f, 0.0f);
					//xx1 = hexx;
					//yy1 = hexy;
				}*/

				
				
				glPopAttrib();
				glPopMatrix();
				
				//glFlush();
				if(mstate == GLUT_UP)
				{
					/*if(user.oper_stage == 1)
					{
						hexx = xx1;
						hexy = yy1;
						user.oper_stage++;
					}
					else*/
					{
						//glFlush();
						user.oper_complete = 1;
						startdrawing = 0;
						user.oper = NONE;
						user.oper_stage = 0;
					}
					glReadPixels(0,0,INITIAL_WIDTH,INITIAL_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,pixels1);
				}
				break;
			case R_RECTANGLE:
				{
					gwininput2output(mousex, mousey, &xx1, &yy1);
					int maxx = sposx;
					int minx = xx1;
					if(minx > sposx)
					{
						minx = sposx;
						maxx = xx1;
					}
					int maxy = sposy;
					int miny = yy1;
					if(miny > sposy)
					{
						miny = sposy;
						maxy = yy1;
					}
					glPushAttrib(GL_ALL_ATTRIB_BITS);
					glColor3d(PAINT_RED, PAINT_GREEN, PAINT_BLUE);
					const int sides = 20;  // The amount of segment to create the circle
					const double radius = 20; // The radius of the circle
						int a;
						if(sposx>xx1)
							a=50;
						else if(sposx==xx1)
							a=0;
						else
							a=-50;
					glBegin(GL_LINES);
						glVertex3f(xx1-a,yy1-abs(a),-10);
						glVertex3f(sposx-a,yy1-abs(a),-10);
						glVertex3f(xx1,yy1,10);
						glVertex3f(xx1-a,yy1-abs(a),-10);
						glVertex3f(sposx,yy1,10);
						glVertex3f(sposx-a,yy1-abs(a),-10);
						glVertex3f(xx1,yy1,10);
						glVertex3f(sposx,yy1,10);
						glVertex3f(sposx-a,yy1-abs(a),-10);
						glVertex3f(sposx-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(sposx,yy1,10);
						glVertex3f(sposx-a,yy1-abs(a),-10);
						glVertex3f(sposx,sposx-xx1+yy1,10);
						glVertex3f(sposx-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(sposx,yy1,10);
						glVertex3f(sposx,sposx-xx1+yy1,10);
						glVertex3f(xx1,yy1,10);
						glVertex3f(xx1,sposx-xx1+yy1,10);
						glVertex3f(xx1-a,yy1-abs(a),-10);
						glVertex3f(xx1-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(xx1,yy1,10);
						glVertex3f(xx1-a,yy1-abs(a),-10);
						glVertex3f(xx1,sposx-xx1+yy1,10);
						glVertex3f(xx1-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(xx1,sposx-xx1+yy1,10);
						glVertex3f(sposx,sposx-xx1+yy1,10);
						glVertex3f(xx1-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(sposx-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(xx1,sposx-xx1+yy1,10);
						glVertex3f(xx1-a,sposx-xx1+yy1-abs(a),-10);
						glVertex3f(sposx,sposx-xx1+yy1,10);
						glVertex3f(sposx-a,sposx-xx1+yy1-abs(a),-10);
					glEnd();
					glPopAttrib();
					//glFlush();
					if(mstate == GLUT_UP)
					{
						user.oper_complete = 1;
						startdrawing = 0;
						user.oper = NONE;
						user.oper_stage = 0;
						glReadPixels(0,0,INITIAL_WIDTH,INITIAL_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,pixels1);
					}
				}
				break;
			case ERASE:
				gwininput2output(mousex, mousey, &xx1, &yy1);
				glPushAttrib(GL_ALL_ATTRIB_BITS);
				glColor3d(1.0, 1.0, 1.0);
				glPointSize(10.0);
				glBegin(GL_POINTS);
					glVertex2i(xx1,yy1);
				glEnd();
				glPopAttrib();
				//glFlush();
				glPointSize(1.0);
				glReadPixels(0,0,INITIAL_WIDTH,INITIAL_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,pixels1);
				if(mstate == GLUT_UP)
				{
					user.oper_complete = 1;
					startdrawing = 0;
					user.oper = NONE;
					user.oper_stage = 0;
				}
				break;
			case CURVE:
				glPushAttrib(GL_ALL_ATTRIB_BITS);
				glColor3d(PAINT_RED, PAINT_GREEN, PAINT_BLUE);
				glPointSize(5.0);
				glBegin(GL_POINTS);
				for (i = 0; i < ncpts; i++)
					glVertex3fv(cpts[i]);
				glEnd();
				glPopAttrib();
				//glFlush();
				glPointSize(1.0);
				glColor3d(1.0, 0.0, 0.0);
				break;
			case CROP:
				gwininput2output(mousex, mousey, &xx1, &yy1);
				if(mstate == GLUT_UP)
				{
					int maxx = sposx;
					int minx = xx1;
					if(minx > sposx)
					{
						minx = sposx;
						maxx = xx1;
					}
					int maxy = sposy;
					int miny = yy1;
					if(miny > sposy)
					{
						miny = sposy;
						maxy = yy1;
					}
					
					glPushAttrib(GL_ALL_ATTRIB_BITS);
					glColor3f(1.0, 1.0, 1.0);
					glBegin(GL_POLYGON);
						glVertex3f(271.0, 11.0,  0.0);
						glVertex3f(271.0, miny, 0.0);
						glVertex3f(989.0, miny, 0.0);
						glVertex3f(989.0, 11.0, 0.0);					
					glEnd();
					glBegin(GL_POLYGON);
						glVertex3f(271.0, 489.0,  0.0);
						glVertex3f(989.0, 489.0, 0.0);
						glVertex3f(989.0, maxy, 0.0);
						glVertex3f(271.0, maxy, 0.0);
					glEnd();
					glBegin(GL_POLYGON);
						glVertex3f(271.0, 489.0,  0.0);
						glVertex3f(minx, 489.0, 0.0);
						glVertex3f(minx, 11.0, 0.0);
						glVertex3f(271.0, 11.0, 0.0);
					glEnd();
					glBegin(GL_POLYGON);
						glVertex3f(maxx, 489.0,  0.0);
						glVertex3f(989.0, 489.0, 0.0);
						glVertex3f(989.0, 11.0, 0.0);
						glVertex3f(maxx, 11.0, 0.0);
					glEnd();
					glColor3f(1.0, 0.0, 0.0);
					glPopAttrib();
					glFlush();
				    
					glReadPixels(0,0,INITIAL_WIDTH,INITIAL_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,pixels2);
					user.oper_complete = 1;
					startdrawing = 0;
					user.oper = NONE;
					user.oper_stage = 0;
					glReadPixels(0,0,INITIAL_WIDTH,INITIAL_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,pixels1);
				}
				else
				{
					glPushAttrib(GL_ALL_ATTRIB_BITS);
					glColor3d(PAINT_RED, PAINT_GREEN, PAINT_BLUE);
					glBegin(GL_LINES);
						glVertex2i(sposx,sposy);
						glVertex2i(sposx,yy1);
						glVertex2i(sposx,sposy);
						glVertex2i(xx1,sposy);
						glVertex2i(xx1,yy1);
						glVertex2i(xx1,sposy);
						glVertex2i(xx1,yy1);
						glVertex2i(sposx,yy1);
					glEnd();
					glPopAttrib();
					//glFlush();
				}				
				break;
			case TRIANGLE:
				gwininput2output(mousex, mousey, &xx1, &yy1);
				glPushMatrix();
				glPushAttrib(GL_ALL_ATTRIB_BITS);
				quadratic=gluNewQuadric();
				gluQuadricDrawStyle(quadratic, GLU_LINE);
				gluQuadricNormals(quadratic, GLU_SMOOTH);
				gluQuadricTexture(quadratic, GL_TRUE);
				
				glTranslatef(sposx,sposy,0);		

				if(user.oper_stage == 1)
				{
					glRotatef(+(atan (((GLfloat)yy1-sposy)/((GLfloat)xx1-sposx)) * 180 / 3.14)+90, 1.0f, 0.0f, 0.0f);
					glRotatef(+(atan (((GLfloat)yy1-sposy)/((GLfloat)xx1-sposx)) * 180 / 3.14)-180, 1.0f, 0.0f, 0.0f);
					glColor3d(PAINT_RED, PAINT_GREEN, PAINT_BLUE);
					glutWireCone(sqrt(pow(sposx-(GLfloat)xx1,2)+pow(sposy-(GLfloat)yy1,2))/2,sqrt(pow(sposx-(GLfloat)xx1,2)+pow(sposy-(GLfloat)yy1,2))/2,25,15);
				}
				/*else
				{
					glRotatef(+(atan (((GLfloat)yy1-sposy)/((GLfloat)xx1-sposx)) * 180 / 3.14)+90, 0.0f, 1.0f, 0.0f);
					glRotatef(+(atan (((GLfloat)yy1-sposy)/((GLfloat)xx1-sposx)) * 180 / 3.14)-180, 0.0f, 1.0f, 0.0f);
					//xx1 = hexx;
					//yy1 = hexy;
				}*/

				
				
				glPopAttrib();
				glPopMatrix();
				
				//glFlush();
				if(mstate == GLUT_UP)
				{
					/*if(user.oper_stage == 1)
					{
						hexx = xx1;
						hexy = yy1;
						user.oper_stage++;
					}
					else*/
					{
						//glFlush();
						user.oper_complete = 1;
						startdrawing = 0;
						user.oper = NONE;
						user.oper_stage = 0;
					}
					glReadPixels(0,0,INITIAL_WIDTH,INITIAL_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,pixels1);
				}
				break;
	  	} 
	}

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glColor3f(BGROUND_RED, BGROUND_GREEN, BGROUND_BLUE);
	glBegin(GL_POLYGON);
		glVertex3f(0.0, 0.0,  0.0);
		glVertex3f(0.0, 499.0, 0.0);
		glVertex3f(270.0, 499.0, 0.0);
		glVertex3f(270.0, 0.0, 0.0);		
	glEnd();
	glBegin(GL_POLYGON);
		glVertex3f(270.0, 0.0,  0.0);
		glVertex3f(270.0, 10.0, 0.0);
		glVertex3f(999.0, 10.0, 0.0);
		glVertex3f(999.0, 0.0, 0.0);	
	glEnd();
	glBegin(GL_POLYGON);
		glVertex3f(270.0, 499.0,  0.0);
		glVertex3f(270.0, 490.0, 0.0);
		glVertex3f(999.0, 490.0, 0.0);
		glVertex3f(999.0, 499.0, 0.0);					
	glEnd();
	glBegin(GL_POLYGON);
		glVertex3f(990.0, 499.0,  0.0);
		glVertex3f(999.0, 499.0, 0.0);
		glVertex3f(999.0, 0.0, 0.0);
		glVertex3f(990.0, 0.0, 0.0);
	glEnd();
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
		glVertex2i(271,10);
		glVertex2i(271,490);
		glVertex2i(270,11);
		glVertex2i(990,11);
		glVertex2i(270,490);
		glVertex2i(990,490);
		glVertex2i(990,490);
		glVertex2i(990,10);
	glEnd();
	glPopAttrib();
	
	gpost_buttons(user_button, BUTTON_TEXT_SIZE, user.oper,HIGHL_RED, HIGHL_GREEN, HIGHL_BLUE);

	glFlush();
} 

