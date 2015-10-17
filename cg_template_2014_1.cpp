//=============================================================================================
// Szamitogepes grafika hazi feladat keret. Ervenyes 2014-tol.
// A //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// sorokon beluli reszben celszeru garazdalkodni, mert a tobbit ugyis toroljuk.
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni (printf is fajlmuvelet!)
// - new operatort hivni az onInitialization függvényt kivéve, a lefoglalt adat korrekt felszabadítása nélkül
// - felesleges programsorokat a beadott programban hagyni
// - tovabbi kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan gl/glu/glut fuggvenyek hasznalhatok, amelyek
// 1. Az oran a feladatkiadasig elhangzottak ES (logikai AND muvelet)
// 2. Az alabbi listaban szerepelnek:
// Rendering pass: glBegin, glVertex[2|3]f, glColor3f, glNormal3f, glTexCoord2f, glEnd, glDrawPixels
// Transzformaciok: glViewport, glMatrixMode, glLoadIdentity, glMultMatrixf, gluOrtho2D,
// glTranslatef, glRotatef, glScalef, gluLookAt, gluPerspective, glPushMatrix, glPopMatrix,
// Illuminacio: glMaterialfv, glMaterialfv, glMaterialf, glLightfv
// Texturazas: glGenTextures, glBindTexture, glTexParameteri, glTexImage2D, glTexEnvi,
// Pipeline vezerles: glShadeModel, glEnable/Disable a kovetkezokre:
// GL_LIGHTING, GL_NORMALIZE, GL_DEPTH_TEST, GL_CULL_FACE, GL_TEXTURE_2D, GL_BLEND, GL_LIGHT[0..7]
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : <VEZETEKNEV(EK)> <KERESZTNEV(EK)>
// Neptun : <NEPTUN KOD>
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================



#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Innentol modosithatod...

#warning "LOGGER FORWARD DECL"
void nwLogMousePos(int x, int y);
#include <iostream>
using namespace std;

//--------------------------------------------------------
// 3D Vektor
//--------------------------------------------------------
struct Vector {
   float x, y, z;

   Vector( ) {
	x = y = z = 0;
   }
   Vector(float x0, float y0, float z0 = 0) {
	x = x0; y = y0; z = z0;
   }
   Vector operator*(float a) {
	return Vector(x * a, y * a, z * a);
   }
   Vector operator+(const Vector& v) {
 	return Vector(x + v.x, y + v.y, z + v.z);
   }
   Vector operator-(const Vector& v) {
 	return Vector(x - v.x, y - v.y, z - v.z);
   }
   float operator*(const Vector& v) { 	// dot product
	return (x * v.x + y * v.y + z * v.z);
   }
   Vector operator%(const Vector& v) { 	// cross product
	return Vector(y*v.z-z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
   }
   float Length() { return sqrt(x * x + y * y + z * z); }
};

//--------------------------------------------------------
// Spektrum illetve szin
//--------------------------------------------------------
struct Color {
   float r, g, b;

   Color( ) {
	r = g = b = 0;
   }
   Color(float r0, float g0, float b0) {
	r = r0; g = g0; b = b0;
   }
   Color operator*(float a) {
	return Color(r * a, g * a, b * a);
   }
   Color operator*(const Color& c) {
	return Color(r * c.r, g * c.g, b * c.b);
   }
   Color operator+(const Color& c) {
 	return Color(r + c.r, g + c.g, b + c.b);
   }
};

const int screenWidth = 600;  // alkalmazás ablak felbontása
const int screenHeight = 600;

const Vector worldSize(1000, 1000);
const int MAX_CTRL_POINTS_NUM = 50;

bool startMovingAround = false;
long animationStartTime = 0;

Vector scaling(1, 1, 1);
Vector translating(0, 0, 0);

struct nwIDrawable
{
	virtual void drawMe() const = 0;
};

struct nwCircle : public nwIDrawable
{
	Vector center;
	float radius;
	int resolution;
	Color fillColor;
	Color borderColor;

	nwCircle(float r=5.0f,
			int res=16,
			Color fillC=Color(1, 0, 0),
			Color borderC=Color(1, 1, 1)) :
				radius(r),
				resolution(res),
				fillColor(fillC),
				borderColor(borderC) {}

	void drawMe() const
	{
		glBegin(GL_TRIANGLE_FAN);
		    glColor3f(fillColor.r, fillColor.g, fillColor.b);
		    glVertex2f(center.x, center.y);
		    for(int i = 0;i<=resolution;i++)
		    {
		      float angle = float(i) / resolution * 2.0f * M_PI;
		      glVertex2f(center.x + radius * cos(angle), center.y + radius * sin(angle));
		    }
		  glEnd();

		  glBegin(GL_LINE_LOOP);
		    glColor3f(borderColor.r, borderColor.g, borderColor.b);
		    for(int i =0;i<resolution;i++)
		    {
		      float angle = float(i) / resolution * 2.0f * M_PI;
		      glVertex2f(center.x + radius * cos(angle), center.y + radius * sin(angle));
		    }
		  glEnd();
	}
};

struct nwControlPoint : public nwCircle
{
	Vector velocity;
	long time;

	void calculateVelocity(nwControlPoint ri, nwControlPoint rj, nwControlPoint rk)
	{
		long ti = ri.time;
		long tj = rj.time;
		long tk = rk.time;
		float firstMemX = 0.5 * (rk.center.x - rj.center.x) / (tk - tj);
		float firstMemY = 0.5 * (rk.center.y - rj.center.y) / (tk - tj);
		float secMemX = 0.5 * (rj.center.x - ri.center.x) / (tj - ti);
		float secMemY = 0.5 * (rj.center.y - ri.center.y) / (tj - ti);
		velocity.x = firstMemX + secMemX;
		velocity.y = firstMemY + secMemY;
	}
};



struct nwLine : public nwIDrawable
{
	Vector points[2];
	Color lineColor;
	nwLine(Color lineC=Color(0, 0, 0)) : lineColor(lineC) {}
	void drawMe() const
	{
		glColor3f(lineColor.r, lineColor.g, lineColor.b);
		glBegin(GL_LINES);
			glVertex2f(points[0].x, points[0].y);
			glVertex2f(points[1].x, points[1].y);
		glEnd();
	}
};

Vector nwWindowToWorld(int x, int y)
{
  //float scale = 10.0f/6.0f;
  float scaleX = float(worldSize.x)/screenWidth/scaling.x;
  float scaleY = float(worldSize.y)/screenHeight/scaling.y;
  float worldX = x * scaleX - 1.0f/scaling.x * worldSize.x/2 - translating.x; // mert a window klikk olyan jol van helyezve a window x y tengelyen
  float worldY = (-1) * (y * scaleY - 1.0f/scaling.y * worldSize.y/2 + translating.y);
  return Vector(worldX, worldY);
}

void nwDrawParabola(Vector parabolaPoints[], bool drawInside=true)
{
	float A = (-1) * (parabolaPoints[1].y - parabolaPoints[0].y);
	float B = parabolaPoints[1].x - parabolaPoints[0].x;
	float C = (-1) * (A * parabolaPoints[0].x + B * parabolaPoints[0].y);
	Vector focusPoint = parabolaPoints[2];
	glBegin(GL_POINTS);
	for (int x = 0;x < screenWidth;x++)
	{
		for (int y = 0;y < screenHeight;y++)
		{
			Vector worldPos = nwWindowToWorld(x, y);
			float eqLeft = (fabs(A*worldPos.x + B*worldPos.y + C))/sqrt(A*A + B*B);
			float eqRight = sqrt((worldPos.x-focusPoint.x)*(worldPos.x-focusPoint.x) + (worldPos.y-focusPoint.y)*(worldPos.y-focusPoint.y));
			if (drawInside)
			  {
				  if (eqLeft - eqRight > 0)
				  {
					  glVertex2f(worldPos.x, worldPos.y);
				  }
			  }
			  else
			  {
				  if (!(eqLeft - eqRight > 0)) // ha nem a belsejet, akkor a kulso reszt rajzoljuk
				  {
					  glVertex2f(worldPos.x, worldPos.y);
				  }
			  }
		}
	}
	glEnd();
}

struct nwScene
{
	nwControlPoint ctrlPoints[MAX_CTRL_POINTS_NUM];
	int cpsCount;
	nwLine line;

	bool drawLine;
	bool drawParabola;
	bool drawCRSpline;

	nwScene(int ctrlPC=0) :
		cpsCount(0),
		drawLine(false),
		drawParabola(false),
		drawCRSpline(false) {}

	nwControlPoint Hermite(nwControlPoint r00, Vector v0, long t0,
			nwControlPoint r11, Vector v1, long t1,
			long t)
	{
		Vector r0 = r00.center;
		Vector r1 = r11.center;

		Vector a0 = r00.center;
		Vector a1 = r00.velocity;
		Vector a2;
		a2.x = (3*(r1.x-r0.x)/pow(t1-t, 2)) - ((v1.x + 2*v0.x)/(t1 - t0));
		a2.y = (3*(r1.y-r0.y)/pow(t1-t, 2)) - ((v1.y + 2*v0.y)/(t1 - t0));
		Vector a3;
		a3.x = (2*(r0.x-r1.x)/pow(t1-t0, 3)) + ((v1.x+v0.x)/pow(t1-t0, 2));
		a3.y = (2*(r0.y-r1.y)/pow(t1-t0, 3)) + ((v1.y+v0.y)/pow(t1-t0, 2));

		nwControlPoint ret;
		ret.center.x = a3.x * pow(t-t0, 3) + a2.x * pow(t-t0, 2) + a1.x * (t-t0) + a0.x;
		ret.center.y = a3.y * pow(t-t0, 3) + a2.y * pow(t-t0, 2) + a1.y * (t-t0) + a0.y;
		ret.velocity.x = 3*a3.x*pow(t-t0, 2) + 2*a2.x*(t-t0) + a1.x;
		ret.velocity.y = 3*a3.y*pow(t-t0, 2) + 2*a2.y*(t-t0) + a1.y;
		return ret;
	}

	nwControlPoint r(long t)
	{
		for (int i =0;i<cpsCount-1;i++)
		{
			if (ctrlPoints[i].time <= t && t <= ctrlPoints[i+1].time)
			{
				return Hermite(ctrlPoints[i], ctrlPoints[i].velocity, ctrlPoints[i].time,
						ctrlPoints[i+1], ctrlPoints[i+1].velocity, ctrlPoints[i+1].time,
						t);
			}
		}
		return nwControlPoint();
	}

	void nwDrawCRSpline()
	{
		glBegin(GL_POINTS);
		glColor3f(0.1f, 0.2f, 0.3f);
		for(long t=ctrlPoints[0].time;t<ctrlPoints[cpsCount-1].time;t+=10)
		{
			glVertex2f(r(t).center.x, r(t).center.y);
		}
		glEnd();
	}

	void changeState()
	{
		ctrlPoints[cpsCount-1].time = glutGet(GLUT_ELAPSED_TIME);
		if (cpsCount > 2)
		{
			drawParabola = true;
			drawCRSpline = true;
			ctrlPoints[cpsCount - 2].calculateVelocity(ctrlPoints[cpsCount-3], ctrlPoints[cpsCount-2], ctrlPoints[cpsCount-1]);
		}
		else if(cpsCount > 1)
		{
			drawLine = true;
			line.points[0] = ctrlPoints[0].center;
			line.points[1] = ctrlPoints[1].center;
		}
	}

	bool registerPoint(int windowPosX, int windowPosY)
	{
		if (cpsCount < MAX_CTRL_POINTS_NUM)
		{
			ctrlPoints[cpsCount].center.x = nwWindowToWorld(windowPosX, windowPosY).x;
			ctrlPoints[cpsCount].center.y = nwWindowToWorld(windowPosX, windowPosY).y;
			cpsCount++;
			changeState();
			return true;
		}
		return false;
	}
	void drawMe()
	{
		if (drawParabola)
		{
			Vector parabolaPoints[3];
			parabolaPoints[0] = ctrlPoints[0].center;
			parabolaPoints[1] = ctrlPoints[1].center;
			parabolaPoints[2] = ctrlPoints[2].center;
			glColor3f(1, 1, 0);
			nwDrawParabola(parabolaPoints);
			glColor3f(0, 1, 1);
			nwDrawParabola(parabolaPoints, false);
		}
		if (drawLine)
		{
			line.drawMe();
		}
		if (drawCRSpline)
		{
			nwDrawCRSpline();
		}
		for (int i =0;i<cpsCount;i++)
		{
			ctrlPoints[i].drawMe();
		}
	}
};

nwScene scene;

// Inicializacio, a program futasanak kezdeten, az OpenGL kontextus letrehozasa utan hivodik meg (ld. main() fv.)
void onInitialization( ) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(worldSize.x/-2.0f, worldSize.x/2.0f, worldSize.y/-2.0f, worldSize.y/2.0f); // left right bottom top
}

// Rajzolas, ha az alkalmazas ablak ervenytelenne valik, akkor ez a fuggveny hivodik meg
void onDisplay( ) {
  glClearColor(0.1f, 0.2f, 0.3f, 1.0f);		// torlesi szin beallitasa
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // kepernyo torles

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glScalef(scaling.x, scaling.y, 1);
  glTranslatef(translating.x, translating.y, 0);

  scene.drawMe();

  glutSwapBuffers();     				// Buffercsere: rajzolas vege

}

// Billentyuzet esemenyeket lekezelo fuggveny (lenyomas)
void onKeyboard(unsigned char key, int x, int y) {
    if (key == 'd')
    {
      glutPostRedisplay( );
    }
    else if (key == ' ')
    {
      if (!startMovingAround) // csak 1x lehessen lenyomni a space-t
      {
        startMovingAround = true;
        animationStartTime = glutGet(GLUT_ELAPSED_TIME);
      }
    }

}

// Billentyuzet esemenyeket lekezelo fuggveny (felengedes)
void onKeyboardUp(unsigned char key, int x, int y) {

}

// Eger esemenyeket lekezelo fuggveny
void onMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
    	scene.registerPoint(x, y);
    	glutPostRedisplay();
    }
}

// Eger mozgast lekezelo fuggveny
void onMouseMotion(int x, int y)
{

}

// `Idle' esemenykezelo, jelzi, hogy az ido telik, az Idle esemenyek frekvenciajara csak a 0 a garantalt minimalis ertek
void onIdle( ) {
     long time = glutGet(GLUT_ELAPSED_TIME);		// program inditasa ota eltelt ido
     if (startMovingAround)
     {
      scaling.x = 2;
      scaling.y = 2;
      translating.x -= 2;
      translating.y -= 3;
      glutPostRedisplay();
     }
}

// REMOVE =======================================================
#warning "REMOVE IOSTREAM AND LOGGER"

void nwLogMousePos(int x, int y)
{
  cout << endl << "================" << endl;
  cout << "WINDOW:" << endl;
  cout << " X: " << x << endl;
  cout << " Y: " << y << endl;
  cout << "WORLD:" << endl;
  cout << " X: " << nwWindowToWorld(x, y).x << endl;
  cout << " Y: " << nwWindowToWorld(x, y).y << endl;
  cout << "================" << endl;
}
//================================================================

// ...Idaig modosithatod
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// A C++ program belepesi pontja, a main fuggvenyt mar nem szabad bantani
int main(int argc, char **argv) {
    glutInit(&argc, argv); 				// GLUT inicializalasa
    glutInitWindowSize(600, 600);			// Alkalmazas ablak kezdeti merete 600x600 pixel
    glutInitWindowPosition(100, 100);			// Az elozo alkalmazas ablakhoz kepest hol tunik fel
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);	// 8 bites R,G,B,A + dupla buffer + melyseg buffer

    glutCreateWindow("Grafika hazi feladat");		// Alkalmazas ablak megszuletik es megjelenik a kepernyon

    glMatrixMode(GL_MODELVIEW);				// A MODELVIEW transzformaciot egysegmatrixra inicializaljuk
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);			// A PROJECTION transzformaciot egysegmatrixra inicializaljuk
    glLoadIdentity();

    onInitialization();					// Az altalad irt inicializalast lefuttatjuk

    glutDisplayFunc(onDisplay);				// Esemenykezelok regisztralasa
    glutMouseFunc(onMouse);
    glutIdleFunc(onIdle);
    glutKeyboardFunc(onKeyboard);
    glutKeyboardUpFunc(onKeyboardUp);
    glutMotionFunc(onMouseMotion);

    glutMainLoop();					// Esemenykezelo hurok

    return 0;
}


