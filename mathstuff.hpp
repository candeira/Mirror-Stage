
/**************************
 * Rendering Functions
 *
 **************************/

 #ifdef __APPLE__
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

#include <string>
#include <math.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

const GLfloat Pi=3.1415926535f;

inline GLfloat min(GLfloat a, GLfloat b)
{
  return(a < b ? a : b);
}

inline GLfloat max(GLfloat a, GLfloat b)
{
  return(a > b ? a : b);
}

const GLfloat flip_y[4][4]  =
{
{1.0f,0.0f,0.0f,0.0f},
{0.0f,-1.0f,0.0f,0.0f},
{0.0f,0.0f,1.0f,0.0f},
{0.0f,0.0f,0.0f,1.0f}
};

const GLfloat flip_x[4][4]  =
{
{-1.0f,0.0f,0.0f,0.0f},
{0.0f,1.0f,0.0f,0.0f},
{0.0f,0.0f,1.0f,0.0f},
{0.0f,0.0f,0.0f,1.0f}
};

class CVector3
	{
    public:
        GLfloat x;
        GLfloat y;
        GLfloat z;
	};

std::string stringify(int x)
{
	std::ostringstream o;
	o << x;
	return o.str();
}

GLfloat lengthsq(GLfloat x, GLfloat y)
{
    return (x*x+y*y);
}

GLfloat length(GLfloat x, GLfloat y)
{
    return sqrt(x*x+y*y);
}

//returns relative orientation of two vectors
int parity(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    return ( x1*y2-x2*y1>0 ? -1 : +1 );
}

GLfloat angle(GLfloat x,GLfloat y)
{
    return -atan2f(y,x);
}

GLfloat angle(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    return (atan2f(y2,x2)-atan2f(y1,x1));
}

GLfloat MidpointRatio(GLfloat p1, GLfloat p2, GLfloat q1, GLfloat q2, GLfloat r1, GLfloat r2)
{
    GLfloat as = lengthsq(p1-r1,p2-r2);
    GLfloat bs = lengthsq(q1-p1,q2-p2);
    GLfloat cs = lengthsq(q1-r1,q2-r2);

    return 0.5* ( (as-cs)/(bs)+1);
}

/*
 entity movement stuff
 */
inline GLfloat slope(GLfloat x,GLfloat y)
{
	return y/x;
}

GLfloat distfromline(GLfloat x1,GLfloat y1,GLfloat x2,GLfloat y2,GLfloat x,GLfloat y)
{
	GLfloat A = x - x1;
	GLfloat B = y - y1;
	GLfloat C = x2 - x1;
	GLfloat D = y2 - y1;

	GLfloat dot = A * C + B * D;
	GLfloat len_sq = C * C + D * D;
	GLfloat param = dot / len_sq;

	GLfloat xx,yy;

	if(param < 0)
	{
		xx = x1;
		yy = y1;
	}
	else if(param > 1)
	{
		xx = x2;
		yy = y2;
	}
	else
	{
		xx = x1 + param * C;
		yy = y1 + param * D;
	}

	return length(yy-y,xx-x);//your distance function
}

GLfloat distfromlinesq(GLfloat x1,GLfloat y1,GLfloat x2,GLfloat y2,GLfloat x,GLfloat y)
{
	GLfloat A = x - x1;
	GLfloat B = y - y1;
	GLfloat C = x2 - x1;
	GLfloat D = y2 - y1;

	GLfloat dot = A * C + B * D;
	GLfloat len_sq = C * C + D * D;
	GLfloat param = dot / len_sq;

	GLfloat xx,yy;

	if(param < 0)
	{
		xx = x1;
		yy = y1;
	}
	else if(param > 1)
	{
		xx = x2;
		yy = y2;
	}
	else
	{
		xx = x1 + param * C;
		yy = y1 + param * D;
	}

	return lengthsq(yy-y,xx-x);//your distance function
}

//replaces the fourth point with the point on the segment P1-P2 closest to P3
void closest_point (GLfloat x1,GLfloat y1,GLfloat x2,GLfloat y2,GLfloat x3,GLfloat y3, GLfloat* x4,GLfloat* y4)
{
	GLfloat	u= ((x3-x1)*(x2-x1)+(y3-y1)*(y2-y1))/lengthsq(x2-x1,y2-y1);
	*x4 =x1+u*(x2-x1);
	*y4 =y1+u*(y2-y1);
}


bool segmentsIntersect(
							  GLfloat Ax, GLfloat Ay,
							  GLfloat Bx, GLfloat By,
							  GLfloat Cx, GLfloat Cy,
							  GLfloat Dx, GLfloat Dy)
{
	GLfloat  distAB, theCos, theSin, newX, ABpos ;

	//  (1) Translate the system so that point A is on the origin.
    Bx-=Ax; By-=Ay;
    Cx-=Ax; Cy-=Ay;
    Dx-=Ax; Dy-=Ay;

	//  Discover the length of segment A-B.
    distAB=sqrt(Bx*Bx+By*By);

	//  (2) Rotate the system so that point B is on the positive X axis.
    theCos=Bx/distAB;
    theSin=By/distAB;
    newX=Cx*theCos+Cy*theSin;
    Cy  =Cy*theCos-Cx*theSin; Cx=newX;
    newX=Dx*theCos+Dy*theSin;
    Dy  =Dy*theCos-Dx*theSin; Dx=newX;

	//  Fail if segment C-D doesn't cross line A-B.
    if (Cy<0.0f && Dy<0.0f || Cy>=0.0f && Dy>=0.0f) return false;

	//  (3) Discover the position of the intersection point along line A-B.
    ABpos=Dx+(Cx-Dx)*Dy/(Dy-Cy);

	//  Fail if segment C-D crosses line A-B outside of segment A-B.
    if (ABpos<0.0f || ABpos>distAB) return false;

	//  Success.
    return true;
}


bool lineSegmentIntersection(
									GLfloat Ax, GLfloat Ay,
									GLfloat Bx, GLfloat By,
									GLfloat Cx, GLfloat Cy,
									GLfloat Dx, GLfloat Dy,
									GLfloat *X, GLfloat *Y)
{

    GLfloat  distAB, theCos, theSin, newX, ABpos ;

	//  (1) Translate the system so that point A is on the origin.
    Bx-=Ax; By-=Ay;
    Cx-=Ax; Cy-=Ay;
    Dx-=Ax; Dy-=Ay;

	//  Discover the length of segment A-B.
    distAB=sqrt(Bx*Bx+By*By);

	//  (2) Rotate the system so that point B is on the positive X axis.
    theCos=Bx/distAB;
    theSin=By/distAB;
    newX=Cx*theCos+Cy*theSin;
    Cy  =Cy*theCos-Cx*theSin; Cx=newX;
    newX=Dx*theCos+Dy*theSin;
    Dy  =Dy*theCos-Dx*theSin; Dx=newX;

	//  Fail if segment C-D doesn't cross line A-B.
    if (Cy<0.0f && Dy<0.0f || Cy>=0.0f && Dy>=0.0f) return false;

	//  (3) Discover the position of the intersection point along line A-B.
    ABpos=Dx+(Cx-Dx)*Dy/(Dy-Cy);

	//  Fail if segment C-D crosses line A-B outside of segment A-B.
    if (ABpos<0.0f || ABpos>distAB) return false;

	//  (4) Apply the discovered position to line A-B in the original coordinate system.
    *X=Ax+ABpos*theCos;
    *Y=Ay+ABpos*theSin;

	//  Success.
    return true;
}


//test if either of two points is in a triangle
bool triangleintersect(GLfloat tx1,GLfloat ty1,GLfloat tx2,GLfloat ty2,GLfloat tx3,GLfloat ty3,GLfloat px,GLfloat py, GLfloat qx, GLfloat qy)
{

	// Compute vectors
    GLfloat vx0 = tx3 - tx1;
    GLfloat vy0 = ty3 - ty1;
    GLfloat vx1 = tx2 - tx1;
    GLfloat vy1 = ty2 - ty1;
    GLfloat vx2a = px - tx1;
    GLfloat vy2a = py - ty1;
    GLfloat vx2b = qx - tx1;
    GLfloat vy2b = qy - ty1;

	// Compute dot products
    GLfloat dot00 = vx0*vx0+vy0*vy0;
    GLfloat dot01 = vx0*vx1+vy0*vy1;
    GLfloat dot02a = vx0*vx2a+vy0*vy2a;
    GLfloat dot02b = vx0*vx2b+vy0*vy2b;
    GLfloat dot11 = vx1*vx1+vy1*vy1;
    GLfloat dot12a = vx1*vx2a+vy1*vy2a;
    GLfloat dot12b = vx1*vx2b+vy1*vy2b;

	// Compute barycentric coordinates
    GLfloat invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    GLfloat ua = (dot11 * dot02a - dot01 * dot12a) * invDenom;
    GLfloat ub = (dot11 * dot02b - dot01 * dot12b) * invDenom;
    GLfloat va = (dot00 * dot12a - dot01 * dot02a) * invDenom;
    GLfloat vb = (dot00 * dot12b - dot01 * dot02b) * invDenom;

	// Check if point is in triangle
    return ((ua > 0) && (va > 0) && ((ua + va) < 1)) || ((ub > 0) && (vb > 0) && ((ub + vb) < 1)) ;

}




