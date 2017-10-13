#ifdef _MSC_VER
#pragma warning(disable:4305)
#pragma warning(disable:4244)
#define _CRT_SECURE_NO_WARNINGS
#endif

#define PI 3.14159265359

#include <iostream>
#include <gl/glut.h>

#include "vector3.h"


GLfloat light_diffuse_0[] = {1, 1, 1, 1.0};
GLfloat light_positio_0[] = {0, 0, 0.35, 1};	//1 vuol dire che è una luce posizionale e non a distanza infinita
GLfloat light_spot_0[] = {0, 0, 1};

// size of the window
int window_size_x = 800, window_size_y = 480;

// used for the trackball implementation
const double m_ROTSCALE = 90.0;
const double m_ZOOMSCALE = 0.008;
float fit_factor = 1.f;
Vector3 trackBallMapping(int x, int y);    // Utility routine to convert mouse locations to a virtual hemisphere
Vector3 lastPoint;                         // Keep track of the last mouse location
enum MovementType { ROTATE, ZOOM, NONE };  // Keep track of the current mode of interaction (which mouse button)
MovementType Movement;                     //    Left-mouse => ROTATE, Right-mouse => ZOOM
Vector3 mouse2D, mouse3D;

GLint FPS = 60;		

float angular_speed = 0.1;

float current, elapsed, last;

GLUquadric *qobj;

GLfloat first_arm_rotation = 0;
GLfloat second_arm_rotation = 60;
GLfloat lamp_rotation = 90;

// implementation of printf with GLUT
void glPrint(float* c, float x, float y, float z, const char *fmt, ...);

void mglTranslate(float x, float y, float z)
{
	GLfloat m[16] ={
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		x, y, z, 1
	};
	glMultMatrixf(m);
}

void mglRotateX(float angle)
{
	GLfloat angle_rad = angle * (PI/180.0f);
	GLfloat m[16] ={
		1,		0,			0,		0,
		0, cos(angle_rad),  sin(angle_rad), 0,
		0, -sin(angle_rad), cos(angle_rad), 0,
		0,		0,			0,		1
	};
	glMultMatrixf(m);
}

void mglRotateY(float angle)
{
	GLfloat angle_rad = angle * (PI/180.0f);
	GLfloat m[16] ={
		cos(angle_rad),	0, -sin(angle_rad), 0,
		0,				1,		0,			0,
		sin(angle_rad), 0, cos(angle_rad),  0,
		0,				0,		0,			1
	};
	glMultMatrixf(m);
}

void mglRotateZ(float angle)
{
	GLfloat angle_rad = angle * (PI/180.0f);
	GLfloat m[16] ={
		cos(angle_rad), sin(angle_rad), 0, 0,
		-sin(angle_rad), cos(angle_rad),0, 0,
			0			,		0		,1, 0,
			0			,		0		,0, 1
	};
	glMultMatrixf(m);
}

void mglRotate(float angle, float ux, float uy, float uz)
{
	Vector3 v;
	v.x = ux;
	v.y = uy;
	v.z = uz;
	v.Normalize();

	float angle_rad = angle * (PI/180.0f);

	float c = cos(angle_rad);
	float s = sin(angle_rad);
	float t = 1 - c;

	float tx = t*v.x;
	float ty = t*v.y;
	float tz = t*v.z;
	
	float txy = tx * v.y;
	float txz = tx * v.z;
	float tyz = ty * v.z;

	float sx = s * v.x;
	float sy = s * v.y;
	float sz = s * v.z;

	GLfloat m[16] ={
		t*powf(v.x,2) + c, txy + sz, txz - sy, 0,
		txy - sz, t*powf(v.y,2) + c, tyz + sx, 0,
		txz + sy, tyz - sx, t*powf(v.z,2) + c, 0,
			0	,	0	,	0	,	1
	};
	glMultMatrixf(m);
}

void mglScale(float s)
{
	GLfloat m[16] ={
		s, 0, 0, 0,
		0, s, 0, 0,
		0, 0, s, 0,
		0, 0, 0, 1
	};
	glMultMatrixf(m);
}


void mgluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear,GLdouble zFar)
{
	GLdouble f = 1 / tan(fovy * PI / 360);
	GLdouble m[16] ={
		f/aspect, 0, 0, 0,
		0, f, 0, 0,
		0, 0, (zNear+zFar)/(zNear-zFar), -1,
		0, 0, (2*zFar*zNear)/(zNear-zFar), 0
	};
	glMultMatrixd(m);
}

void mglOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	GLdouble m[16] ={
		2/(right - left), 0, 0, 0,
		0, 2/(top - bottom), 0, 0,
		0, 0, -2/(zFar - zNear), 0,
		-(right+left)/(right-left), -(top+bottom)/(top-bottom), -(zFar+zNear)/(zFar-zNear), 1
	};
	glMultMatrixd(m);	
}

// handles key down event
void keyboardDown(unsigned char key, int x, int y) 
{
	switch(key) 
	{
	case 'a':
		first_arm_rotation += elapsed * angular_speed;
		if(first_arm_rotation > 360.0)
			first_arm_rotation -= 360.0;
		break;
	case 's':
		first_arm_rotation -= elapsed * angular_speed;
		if(first_arm_rotation < -360.0)
			first_arm_rotation += 360.0;
		break;
	case 'd':
		second_arm_rotation += elapsed * angular_speed;
		if(second_arm_rotation > 360.0)
			second_arm_rotation -= 360.0;
		break;
	case 'f':
		second_arm_rotation -= elapsed * angular_speed;
		if(second_arm_rotation < -360.0)
			second_arm_rotation += 360.0;
		break;
	case 'g':
		lamp_rotation += elapsed * angular_speed;
		if(lamp_rotation > 360.0)
			lamp_rotation -= 360.0;
		break;
	case 'h':
		lamp_rotation -= elapsed * angular_speed;
		if(lamp_rotation < -360.0)
			lamp_rotation += 360.0;
		break;
	case 'Q':
	case 'q':
	case  27:   // ESC
		exit(0);
	}

	glutPostRedisplay();
}

// by pressing left and right cursors, ...
void specialDown(int key, int x, int y)
{
	switch(key) 
	{
	case GLUT_KEY_LEFT:
		if (angular_speed > 0)
			angular_speed -= 0.1;
		break;
	case GLUT_KEY_RIGHT:
		angular_speed += 0.1;
		break;
	}

	glutPostRedisplay();
}


void reshape(int width, int height) 
{
	window_size_x = width;
	window_size_y = height;

	// Determine the new aspect ratio
	GLdouble gldAspect = (GLdouble) width/ (GLdouble) height;

	// Reset the projection matrix with the new aspect ratio.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(40.0, gldAspect, 0.01, 60.0);
	mgluPerspective(40.0, gldAspect, 0.01, 60.0);
	//glTranslatef( 0.0, 0.0, -2.0 );
	mglTranslate( 0.0, 0.0, -2.0 );

	// Set the viewport to take up the entire window.
	glViewport(0, 0, width, height);
}

// handles when a mouse button is pressed / released
void mouseClick(int button, int state, int x, int y) 
{
	mouse2D = Vector3(x, window_size_y - y, 0);

	if (state == GLUT_UP)
	{
		// Turn-off rotations and zoom.
		Movement = NONE;
		glutPostRedisplay();
		return;
	}

	switch (button)
	{
	case (GLUT_LEFT_BUTTON):

		// Turn on user interactive rotations.
		// As the user moves the mouse, the scene will rotate.
		Movement = ROTATE;

		// Map the mouse position to a logical sphere location.
		// Keep it in the class variable lastPoint.
		lastPoint = trackBallMapping( x, y );

		break;

	case (GLUT_RIGHT_BUTTON):

		// Turn on user interactive zooming.
		// As the user moves the mouse, the scene will zoom in or out
		//   depending on the x-direction of travel.
		Movement = ZOOM;

		// Set the last point, so future mouse movements can determine
		//   the distance moved.
		lastPoint.x = (double) x;
		lastPoint.y = (double) y;

		break;

	case (GLUT_MIDDLE_BUTTON):
		Movement = NONE;	
		break;
	}

	glutPostRedisplay();
}


// handle any necessary mouse movements through the trackball
void mouseMotion(int x, int y) 
{
	Vector3 direction;
	double pixel_diff;
	double rot_angle, zoom_factor;
	Vector3 curPoint;

	switch (Movement) 
	{
	case ROTATE :  // Left-mouse button is being held down
		{
			curPoint = trackBallMapping( x, y );  // Map the mouse position to a logical sphere location.
			direction = curPoint - lastPoint;
			double velocity = direction.Length(); 
			if( velocity > 0.0001 )
			{
				// Rotate about the axis that is perpendicular to the great circle connecting the mouse movements.
				Vector3 rotAxis;
				rotAxis = lastPoint ^ curPoint ;
				rotAxis.Normalize();
				rot_angle = velocity * m_ROTSCALE;

				// We need to apply the rotation as the last transformation.
				//   1. Get the current matrix and save it.
				//   2. Set the matrix to the identity matrix (clear it).
				//   3. Apply the trackball rotation.
				//   4. Pre-multiply it by the saved matrix.
				static GLdouble m[4][4];
				glGetFloatv( GL_MODELVIEW_MATRIX, (GLfloat *) m );
				glLoadIdentity();
				glRotatef( rot_angle, rotAxis.x, rotAxis.y, rotAxis.z );
				glMultMatrixf( (GLfloat *) m );

				//  If we want to see it, we need to force the system to redraw the scene.
				glutPostRedisplay();
			}
			break;
		}
	case ZOOM :  // Right-mouse button is being held down
		//
		// Zoom into or away from the scene based upon how far the mouse moved in the x-direction.
		//   This implementation does this by scaling the eye-space.
		//   This should be the first operation performed by the GL_PROJECTION matrix.
		//   1. Calculate the signed distance
		//       a. movement to the left is negative (zoom out).
		//       b. movement to the right is positive (zoom in).
		//   2. Calculate a scale factor for the scene s = 1 + a*dx
		//   3. Call glScalef to have the scale be the first transformation.
		// 
		pixel_diff = y - lastPoint.y; 
		zoom_factor = 1.0 + pixel_diff * m_ZOOMSCALE;
		glScalef( zoom_factor, zoom_factor, zoom_factor );

		// Set the current point, so the lastPoint will be saved properly below.
		curPoint.x = (double) x;  curPoint.y = (double) y;  (double) curPoint.z = 0;

		//  If we want to see it, we need to force the system to redraw the scene.
		glutPostRedisplay();
		break;
	}

	// Save the location of the current point for the next movement. 
	lastPoint = curPoint;	// in spherical coordinates
	mouse2D = Vector3(x, window_size_y - y, 0);	// in window coordinates
}

// draw the coordinate axes
void DrawAxes(double length)
{
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glScalef(length, length, length);

	glLineWidth(2.f);
	glBegin(GL_LINES);

	// x red
	glColor3f(1.f, 0.f, 0.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(1.f, 0.f, 0.f);

	// y green
	glColor3f(0.f, 1.f, 0.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 1.f, 0.f);

	// z blue
	glColor3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 0.f, 1.f);

	glEnd();
	glLineWidth(1.f);

	glPopMatrix();
}

// draw information on the screen
void DrawInfo()
{
	glDisable(GL_LIGHTING);
	// -- start Ortographic Mode
	glMatrixMode(GL_PROJECTION);					//Select the projection matrix
	glPushMatrix();									//Store the projection matrix
	glLoadIdentity();								//Reset the projection matrix
	glOrtho(0, window_size_x, window_size_y, 0, -1, 1);		//Set up an ortho screen

	glMatrixMode(GL_MODELVIEW);						//Select the modelview matrix
	glPushMatrix();									//Store the projection matrix

	glLoadIdentity();								//Reset the projection matrix

	float c[3] = {1, 1, 1};
	float y = 25;

	glPrint(c, 10, y, 0, "Hello World"); y += 20;

	glPopMatrix();									//Restore the old projection matrix
	glMatrixMode(GL_PROJECTION);					//Select the projection matrix
	glPopMatrix();									//Restore the old projection matrix
	glMatrixMode(GL_MODELVIEW);
	// -- end Ortographic mode
}

// draw a cylinder
// br: bottom radius
// tr: top radius
// h: height
void DrawCylinder(float br, float tr, float h)
{
	glPushMatrix();

	gluDisk(gluNewQuadric(), 0, br, 20, 20);

	gluCylinder(qobj, br, tr, h, 20, 20);
	glTranslatef(0, 0, h);
	gluDisk(gluNewQuadric(), 0, tr, 20, 20);

	glPopMatrix();
}

// draw the scene
void draw() 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	DrawAxes(0.5);

	glEnable(GL_LIGHTING);
	DrawCylinder(1, 1, 0.5);
	glPushMatrix();
		glColor3f(0, 0, 1);
		glTranslatef(3, 0, 0.6);
		glRotatef(90, 1, 0, 0);
		//mglRotate(90, 0.2, 0.3, 0.5);
		glutSolidTeapot(0.5);
	glPopMatrix();

	glPushMatrix();
		glColor3f(0, 0, 1);
		glTranslatef(-3, 0, 0.6);
		glutSolidSphere(0.5, 20, 8);
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0, 0, 0.5);
		glRotatef(first_arm_rotation, 0, 0, 1.0);		//Controllare perchè va prima
		glRotatef(15, 0, 1.0, 0);
		DrawCylinder(0.2, 0.2, 2);
		glColor3f(1, 0.6, 0);
		glutSolidSphere(0.2, 20, 20);
			glTranslatef(0, 0, 2);
			glRotatef(second_arm_rotation, 0, 1, 0);
			glColor3f(0, 0, 1);
			DrawCylinder(0.2, 0.2, 2);
			glColor3f(1, 0.6, 0);
			glutSolidSphere(0.2, 20, 20);
				glTranslatef(0, 0, 2);
				glRotatef(lamp_rotation, 0, 1, 0);
				glColor3f(0, 0, 1);
				DrawCylinder(0.2, 0.7, 0.5);
				glColor3f(1, 0.6, 0);
				glutSolidSphere(0.2, 20, 20);
				glTranslatef(0, 0, 0.5);
				glColor3f(1, 0.6, 0);
				glutSolidSphere(0.2, 20, 20);
			    glLightfv(GL_LIGHT0, GL_POSITION, light_positio_0);
				glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_spot_0);
	glPopMatrix();

	//DrawInfo();

	glutSwapBuffers();
}


void idle() { }


void initGL(int width, int height) 
{
	glLightfv (GL_LIGHT0, GL_DIFFUSE,	light_diffuse_0);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 60.0);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	reshape(width, height);

	glClearColor(0.126f, 0.126f, 0.128f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//glShadeModel(GL_FLAT); //senza è smooth
//	glShadeModel(GL_SHADE_MODEL);
	glEnable(GL_NORMALIZE);

	qobj = gluNewQuadric();
}


void animation(int t)
{
	last = current;
	current = glutGet(GLUT_ELAPSED_TIME);
	elapsed = current - last;
	glutPostRedisplay();

	glutTimerFunc((int) 1000/FPS, animation, 0);
}


int main(int argc, char** argv) 
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(window_size_x, window_size_y);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("CG 2012/13 - Assignment #1");

	glutKeyboardFunc(keyboardDown);
	glutSpecialFunc(specialDown);

	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(reshape);
	glutDisplayFunc(draw);  
	glutIdleFunc(idle);
	glutTimerFunc((int) 1000/FPS, animation, 0);
	glutIgnoreKeyRepeat(false); // process keys held down

	initGL(window_size_x, window_size_y);

	glutMainLoop();
	return 0;
}

//
// Utility routine to calculate the 3D position of a 
// projected unit vector onto the xy-plane. Given any
// point on the xy-plane, we can think of it as the projection
// from a sphere down onto the plane. The inverse is what we
// are after.
//
Vector3 trackBallMapping(int x, int y)
{
	Vector3 v;
	double d;

	v.x = (2.0 * x - window_size_x) / window_size_x;
	v.y = (window_size_y - 2.0 * y) / window_size_y;
	v.z = 0.0;
	d = v.Length();
	d = (d < 1.0) ? d : 1.0;  // If d is > 1, then clamp it at one.
	v.z = sqrtf( 1.001 - d * d );  // project the line segment up to the surface of the sphere.

	v.Normalize();  // We forced d to be less than one, not v, so need to normalize somewhere.


	return v;
}



// Custom GL "Print" Routine
// needs glut
void glPrint(float* c, float x, float y, float z, const char *fmt, ...)
{
	if (fmt == NULL)	// If There's No Text
		return;			// Do Nothing

	char text[256];		// Holds Our String
	va_list ap;			// Pointer To List Of Arguments

	va_start(ap, fmt);								// Parses The String For Variables
	vsprintf(text,/* 256 * sizeof(char),*/ fmt, ap);	// And Converts Symbols To Actual Numbers
	va_end(ap);										// Results Are Stored In Text

	size_t len = strlen(text);

	if (c != NULL)
		glColor3fv(c);

	glRasterPos3f(x, y, z);
	for(size_t i = 0; i < len; i++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text[i]);

}


