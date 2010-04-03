//entity stuff


const int EntityCount =8;

enum Etype {
	e_EmptyRoom=0,
	e_Doctor=1,
	e_Mover=2,
	e_Sentry=3,
	e_ParityMover=4,
	e_Wall=5,//laser wall..store an x, a y, and a dir...
	e_Goal_Table=6,
	e_Goal_Flower=7,
	e_Table=8,
	e_Bedroom=9,
	e_Goal_Bedroom=10,
	e_Bathroom=11,
	e_TableMover=12,
	e_Decoration=13,
	e_BigMover=14,
	e_BigDoctor=15,
};

/*
PLAN FOR ADDITIONAL ENTITY TYPE:

decorations: by default, place on player position facing in player direction;
 allow players to scale
 options will be:

 model (range,)
 place (hit space to place)
 scale (range, *=)
 angle (rang, +/-=)
 parity (toggle)

 */

/*
  I use several of the latter variables in the Entity struct
  for different things depending on the type.  It's not great, and
  has resulted in some odd encodings.
*/

struct Entity {
	Etype type;
	GLfloat dat;//used for other data...angle of rotation for sentry..velocity for mover
	GLfloat x;
	GLfloat y;
	GLfloat x2;//used for sentry //used in line wall
	GLfloat y2;
	GLfloat x3;//used in line wall
	GLfloat y3;
};

