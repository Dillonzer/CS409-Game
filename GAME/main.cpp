#include <iostream> //used for srand
#include <sstream>
#include "glut.h"
#include "Sleep.h"
#include <ctime> //used for random number
#include "ObjLibrary\SpriteFont.h"
#include <cmath> //used for distance
#include "ObjLibrary\Vector3.h"
#include <ctime> //used for clock
#include "ObjLibrary\ObjModel.h"
#include <fstream> //used to read in the seafloor
#include <string>
#include <sstream>
#include "ObjLibrary/TextureManager.h"


using namespace std;

struct SPACESHIP
{
	Vector3 coord_vector;
	Vector3 velocity;
	float theta, phi, turnangle, shipx, shipz;

};

struct METEOR
 {
	 Vector3 coord_vector;
	 Vector3 rotation_vector;
	 float theta, phi;
	 Vector3 cVelocity; //current velocity
	 Vector3 tVelocity; //target velocity
	 bool visible;
 };

struct BLACKHOLE
{
	 Vector3 coords;
	 bool active;
};

struct PLANET
{
	Vector3 coords;
	bool visited;
	float red, green, blue;
};

struct BULLET
{
	Vector3 velocity;
	Vector3 coords;
	bool visable;
};


// prototypes
// Some of the prototypes are using double since the Vector3 class has doubles instead of float, so float& will not work.
void display ();
void init();
void reshape(int w, int h);
void idle ();
void set_random_3d_coords(double& x, double& y, double& z); //used to generate random numbers for X, Y, Z value
void draw_meteors();
void draw_ship(); 
void draw_blackholes(); 
void draw_font(); 
void keyboard(unsigned char, int, int); 
void special(int, int, int); 
void rotation(float&, double&, double&, double&); //used to make a random rotation
void boundary_check(); 
float score(METEOR&, float, float, float); //USED TO CHECK DISTANCE
void physics_update(double);
void blackhole_placement(BLACKHOLE&, float, float, float);
void ship_physics_update(double);
void ship_fastAI(double);
void ship_slowAI();
void ship_increase_velocity();
void CCWturn();
void CWturn();
void ship_decrease_velocity();
void ship_increase_bouyancy();
void ship_decrease_bouyancy();
void meteor_rotation(int);
void gameover_text();
void draw_fog();
//collision detecting
void collision_detection();
void ship_vs_meteor();
void draw_planets();
float visiting(PLANET&);
void ship_outside();
void shoot();
void draw_shot();
void hit_meteor(METEOR&);
void win();
void get_angles();

//const variables
const int GOAL = 10;
const int MAX_HEIGHT = 1000;
const int MIN_HEIGHT = 0;
const int MAX_METEORS = 1000;
const int DISTANCE = 5; 
const int NUM_OF_BLACKHOLES = 500;
const int NUM_OF_PLANETS = 7;
const float DEFAULT_PHYSICS_UPDATE_RATE = 100.0; // per second
const float METEOR_MASS = 1.0;  // kg
const float METEOR_SPEED = 10.0 ; // m/s
const float METEOR_ACCELERATION = 5.0  ;// m/s^2
const float SHIP_MASS = 10.0; // kg
const float SHIP_DRAG = 0.6; // fraction of speed left after 1 second
const float SHIP_GRAVITY = -9.8;   // m*s^(-2)
const float SHIP_BUOYANT_FORCE = 980.0;  // kg*m*s^(-2)
const float SHIP_ENGINE_FORCE = 5000.0;  // kg*m*s^(-2)
const float SHIP_TURN_RATE = 180.0;   // degrees per second (or 2.0 radians/sec)
const float SHIP_TILT_MIN = -35.0;   // degrees  (or -0.1 for radians)
const float SHIP_TILT_MAX = 35.0;   // degrees (or 0.2 for radians)
const float PI = 4*atan(1);
const float METEOR_RADIUS = 425.0;   // m
const float PLANET_RADIUS = 2500.0;
const float FOG_DISTANCE = 400.0;
const float CULLING_DISTANCE = 50000;
const float SHOT_SPEED = 10;
const int NUM_OF_STARS = 15;

 //clocks and lag
 clock_t theclock = clock();
 double previous_time, current_time, physics_delta_time, physicsLag = 0;

 //objects
 ObjModel ship_model;
 ObjModel meteor_model;

 //toggles
 bool toggle = true;
 bool gameover = false;

 SPACESHIP ship;
 METEOR meteor_feild[MAX_METEORS]; 
 BLACKHOLE pools[NUM_OF_BLACKHOLES];
 PLANET planets[NUM_OF_PLANETS];
 BULLET shot;
 SpriteFont coord_font; //used to display the coords on the screen
 

 //fog
 bool fog_enabled = true; //on by default
 float bg_g, bg_b;

 //for collision
 float distance_MtS;

 float cc_g = 0.0;

 int visited = 0;


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);  // single is default
	glutInitWindowSize(500, 500);  // default is 300 x 300
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Spaceship Flight");
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	init();
	coord_font.load("Font.bmp"); //loading the font file 
	glutMainLoop();
	return 1;
}

void init(void)
{
	ship.coord_vector.x = 100.0;
	ship.coord_vector.y = 500.0;
	ship.coord_vector.z = 500.0;
	ship.theta = 0; //so it is facing up
	ship.phi = 0;
	ship.velocity.getRandomUnitVector();

	//setting the shot
	shot.visable = false;
	shot.velocity.getRandomUnitVector();
	

	for(int i = 0; i < MAX_METEORS; i++) //generates all the meteors
	{
		set_random_3d_coords(meteor_feild[i].coord_vector.x, meteor_feild[i].coord_vector.y, meteor_feild[i].coord_vector.z);
		//rotation(meteor_feild[i].theta, meteor_feild[i].rotation_vector.x, meteor_feild[i].rotation_vector.y, meteor_feild[i].rotation_vector.z);
		meteor_feild[i].visible = true; 
		set_random_3d_coords(meteor_feild[i].cVelocity.x, meteor_feild[i].cVelocity.y, meteor_feild[i].cVelocity.z);
		meteor_feild[i].tVelocity = meteor_feild[i].cVelocity;
	}

	

	for(int i = 0; i < NUM_OF_BLACKHOLES; i++)	//this for loop is generating all the whirlpools
	{
		set_random_3d_coords(pools[i].coords.x, pools[i].coords.y, pools[i].coords.z);
		pools[i].active = true;
	}

	for(int i = 0; i < NUM_OF_PLANETS; i++) //this loop makes all the planets and places them in random locations
	{
		set_random_3d_coords(planets[i].coords.x, planets[i].coords.y, planets[i].coords.z);
		planets[i].visited = false;
		planets[i].red = planets[i].green = planets[i].blue = 0.0;
	}
	



	//setting colours
	planets[0].red = 1.0;
	planets[1].blue = 1.0;
	planets[2].green = 1.0;
	planets[3].red = planets[3].blue = 1.0;
	planets[4].blue = planets[4].green = 1.0;
	planets[5].red = planets[5].green = 1.0;
	planets[6].red = 0.4;
	planets[6].blue = 0.4;
	planets[6].green = 0.4;


	//loading models
	ship_model.load("SpaceShip.obj");
	meteor_model.load("asteroid1.obj");

	//fog enabled
	glEnable(GL_FOG);
	bg_g = 0.0; 
	bg_b = 0.0;

	glColor3f(0.0, 0.0, 0.0);		
	glClearColor(0.0,0.0,0.0,1.0); 
	glLineWidth(2.0f);
	glEnable(GL_DEPTH_TEST);
	
	
}

void reshape(int w , int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (double)w / (double)h, 0.1, 500.0);
}

void special(int special_key, int x, int y)
{
	if (gameover == false)
	{
	  switch(special_key)
	  {
	  case GLUT_KEY_DOWN:
		  ship_decrease_bouyancy();
		   break;
	  case GLUT_KEY_UP:
		  ship_increase_bouyancy();
		   break;
	  }
	}
}

void keyboard(unsigned char key, int x, int y)
{
	if (gameover == false)
	{
		  switch(key)
		  {
		  case 'A':
		  case 'a':
				CCWturn();
				break;
		  case 'D':
		  case 'd':
				CWturn();
				break;
		  case 'W':
		  case 'w':
				ship_increase_velocity();
				break;
		  case 'S':
		  case 's':
				ship_decrease_velocity();
				break;
			  //used to toggle the controls on or off the screen
		  case 'M':
		  case 'm':
			  if(toggle == false)
				  toggle = true;
			  else 
				  toggle = false;
			  break;
		  case 'O':
		  case 'o':
			  fog_enabled = true;
			  glEnable(GL_FOG);
			  break;
		  case 'L':
		  case 'l':
			  fog_enabled = false;
			  glDisable(GL_FOG);
			  break;
		  case 32:
			  shoot();
			  break;
		  case 27: // on [ESC]
			   exit(0); // normal exit
			   break;
		  }
	}
	else
	{

	 switch(key)
	 {
		case 27: // on [ESC]
	    exit(0); // normal exit
	    break;
	}

}
}

void idle()
{
  sleep(0.01);  // wait for 0.01 seconds
  glutPostRedisplay();
}

void display (void)
{
	physics_delta_time = (1 / DEFAULT_PHYSICS_UPDATE_RATE)/100;
	theclock = clock(); 
	get_angles();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);	// which matrix
	glLoadIdentity();
	gluLookAt // use sin and cos and pass it and minus it form z and x
	(ship.coord_vector.x - ship.shipx, ship.coord_vector.y+0.3, (ship.coord_vector.z - ship.shipz),		// x,y,z of eye and keeps it always on the ship (where we are looking)
	ship.coord_vector.x, ship.coord_vector.y, ship.coord_vector.z,	    // x,y,z of look at point and keeps it always on the ship (point)
	0.0, 1.0, 0.0);		// x,y,z of up vector (camera tilt)

	//boundary_check(); //checking the boundary before drawing the ship
	//drawing functions
	collision_detection();
	if (toggle == true)
		draw_font();
	if(shot.visable == true)
		draw_shot();
	draw_ship();
	draw_meteors();
	draw_blackholes();
	draw_planets();

	if(fog_enabled == true)
		draw_fog(); 

	if (gameover == true)
		gameover_text();
	else if(visited == NUM_OF_PLANETS)
		win();
	else
	{
		//updating
		current_time = (theclock / (double) CLOCKS_PER_SEC) / 1000;
	
		physicsLag += (current_time - previous_time);
		while(physicsLag > physics_delta_time)
		{
			//doing the physics lag
			if (physicsLag > physics_delta_time)
			{
				physics_update(physics_delta_time);
				ship_physics_update(physics_delta_time * 1000);
				physicsLag -= physics_delta_time; 
			}

		}
		previous_time = current_time;
	}



	// ensure that all buffered OpenGL commands get done
	glFlush();	
	glutSwapBuffers();		// display it, double buffering
}

void set_random_3d_coords(double& x, double& y, double& z)
{
	
	x = (rand() % (1000)); //chooses a number between 0 and 10000 for x
	y = (rand() % (1000)); //chooses a number between 0 and 10000 for y
	z = (rand() % (1000)); //chooses a number between 0 and 10000 for z
	

}	

void draw_meteors()
{
			for(int i = 0; i < MAX_METEORS; i++) 
			{
				distance_MtS = score(meteor_feild[i], ship.coord_vector.x, ship.coord_vector.y, ship.coord_vector.z);
				if (meteor_feild[i].visible == true)
				{
					if(((distance_MtS  + METEOR_RADIUS < CULLING_DISTANCE) && fog_enabled == true) || fog_enabled == false)
					{
						glMatrixMode(GL_MODELVIEW);	// which matrix
						glPushMatrix();
							glColor3f(0.2,0.2,0.2);
							glTranslatef(meteor_feild[i].coord_vector.x, meteor_feild[i].coord_vector.y, meteor_feild[i].coord_vector.z); //used to set the position for the fish when they spawn
							glRotatef(i, 1,0,0); //used i so it spins constantly without using another variable.
							glScalef(0.6f, 0.6f, 0.6f);
							glTranslatef(0, 0, 0.5); //centers it on the origin
							meteor_model.draw();
						glPopMatrix();

						if (shot.visable == true)
						{
							hit_meteor(meteor_feild[i]);
						}
					
					}
				}

			}
}

void draw_ship()
{
	 glColor3f(0.0,0.0,0.4); //colour
	 glMatrixMode(GL_MODELVIEW);
	 glPushMatrix();
		glTranslatef(ship.coord_vector.x, ship.coord_vector.y, ship.coord_vector.z);
		glRotatef(ship.theta,0,1,0);
		glRotatef(ship.phi,-1,0,0); 
		glRotatef(ship.turnangle,0,0,1);
		glScalef(0.06f, 0.06f, 0.06f);
		ship_model.draw();
	glPopMatrix();

}

void draw_font()
{
	stringstream ss, sss;

	ss << "Planets visited: " << visited;
	string vis = ss.str();


	sss << "Ship's Position: " << ship.coord_vector.x << ", " << ship.coord_vector.y << ", " << ship.coord_vector.z << ".";
	string crd = sss.str();

	SpriteFont::setUp2DView(650,650); 
		coord_font.draw(vis, 0, 0, 255, 0, 30); //x, y, R, G, B
		coord_font.draw(crd, 240, 0, 255, 0, 30); //x, y, R, G, B
	SpriteFont::unsetUp2DView();
}

void draw_planets()
{
	static float dummy;

	 glColor3f(1.0,1.0,1.0); //colour
	 glMatrixMode(GL_MODELVIEW);
	 for (int i = 0; i <= NUM_OF_PLANETS; i++)
	 {
		 glPushMatrix();
			glColor3f(planets[i].red, planets[i].green, planets[i].blue);
			glTranslatef(planets[i].coords.x, planets[i].coords.y, planets[i].coords.z);
			glScalef(5.0f,5.0f,5.0f);
			glutSolidSphere(10.0,20,40);
		glPopMatrix();

		dummy = visiting(planets[i]); //gets distance to planet
		if(dummy <= 10000 && planets[i].visited == false)
		{
			planets[i].visited = true;
			visited++;
		}

		if (dummy < PLANET_RADIUS)
			gameover = true;
	 }
}

void rotation(float& theta, double& x, double& y, double& z)
{
	x = (rand() % (2)); 
	y = (rand() % (2)); 
	z = (rand() % (2)); 
	theta = (rand() % (3600)) / 10.0;
}

float score(METEOR& tar, float x, float y, float z) 
{

	return(pow((tar.coord_vector.x - x), 2)+pow((tar.coord_vector.y - y), 2)+pow((tar.coord_vector.z - z),2));

}

void physics_update(double deltatime)
{
	for(int i = 0; i < MAX_METEORS; i++)
	{
		distance_MtS = score(meteor_feild[i], ship.coord_vector.x, ship.coord_vector.y, ship.coord_vector.z);
		if((((distance_MtS + METEOR_RADIUS < CULLING_DISTANCE) && fog_enabled == true) || fog_enabled == false) && meteor_feild[i].visible == true)
			meteor_feild[i].coord_vector = ((meteor_feild[i].cVelocity + meteor_feild[i].tVelocity) / 2) * (deltatime * METEOR_SPEED) + meteor_feild[i].coord_vector;
	}
	
	
}

void draw_blackholes()
{
	//drawing blackholes
	for (int i = 0; i < NUM_OF_BLACKHOLES; i++)
				{

					glColor3f(0.1,0.1,0.1); //colour
					glMatrixMode(GL_MODELVIEW);
					glPushMatrix();
						glTranslatef(pools[i].coords.x, pools[i].coords.y, pools[i].coords.z); 
						glRotatef(180, 0, 1, 0);
						glScalef(1.0f, 1.0f, 1.0f);   
						glutWireCone(5,10,5,5);
					glPopMatrix();

					blackhole_placement(pools[i], ship.coord_vector.x, ship.coord_vector.y, ship.coord_vector.z); //checking for if they entered a blackhole
				}
}

void blackhole_placement(BLACKHOLE& pool, float x, float y, float z)
{
	int live_or_die =  1 + (rand() % (2));
	//checking distance
	if (pow((pool.coords.x - x), 2)+pow((pool.coords.y - y), 2)+pow((pool.coords.z - z),2) < 2)
	{
		if(pool.active == true)
		{
			switch(live_or_die)
			{
				case 1:
					set_random_3d_coords(ship.coord_vector.x, ship.coord_vector.y, ship.coord_vector.z); //if it's 1, then teleports
					break;
				case 2:
					gameover = true;
					toggle = false;
					break; 
			}
			pool.active = false;
		}
	}
}

void gameover_text()
{
	stringstream go; 
	go << "GAME OVER";
				
	string over = go.str();
	SpriteFont::setUp2DView(750,650); 
		coord_font.draw(over, 300, 300, 255, 150, 30); //x, y, R, G, B
	SpriteFont::unsetUp2DView();

	//blow up
	 glColor3f(1.0,0.0,0.0); //colour
	 glMatrixMode(GL_MODELVIEW);
	 glPushMatrix();
		glTranslatef(ship.coord_vector.x, ship.coord_vector.y, ship.coord_vector.z);
		glScalef(0.5f, 0.5f, 0.5f);
		glutSolidSphere(1.0,10.0,10.0);
	glPopMatrix();


}

void ship_physics_update(double deltatime)
{
	ship.velocity = ship.velocity * pow(SHIP_DRAG, deltatime);
	ship.coord_vector += ship.velocity * deltatime;

	if (shot.visable == true)
	{
		shot.coords += shot.velocity * deltatime;
	}
}

void CCWturn()
{
	ship.theta += SHIP_TURN_RATE * 0.1;

	if(ship.turnangle != -40)
		ship.turnangle -= 40;
}

void CWturn()
{
	ship.theta -= SHIP_TURN_RATE * 0.1;
	
	if(ship.turnangle != 40)
		ship.turnangle += 40;
	
}

void ship_increase_velocity()
{
	
	float vel = (SHIP_ENGINE_FORCE / SHIP_MASS * 0.01);
	float theta_radians = ship.theta * (PI/180);
	
	if(ship.theta < 0)
	{
		ship.theta +=360;
	}
	if (ship.theta > 360)
	{
		ship.theta -= 360;
	}

	if(ship.theta == 0)
	{
		ship.velocity.z += vel;
	}
	else if(ship.theta > 270)//quad 4
	{
		ship.velocity.x -= (sin(-theta_radians));
		ship.velocity.z -= (cos(-theta_radians));
	}
	else if(ship.theta == 360)
	{
		ship.velocity.z -=vel;
	}
	else if(ship.theta > 180) //quad 3
	{
		ship.velocity.z -= vel * (cos(theta_radians-PI));
		ship.velocity.x -= vel * (sin(theta_radians-PI));
	}
	else if(ship.theta == 90)
	{
		ship.velocity.x +=vel;
	}
	else if(ship.theta > 90)//quad 2
	{
		ship.velocity.z -=vel*(cos(PI-theta_radians));
		ship.velocity.x +=vel*(sin(PI-theta_radians));
	}
	else if(ship.theta == 180)
	{
		ship.velocity.z -= vel;
	}
	else if(ship.theta > 0) //quad1
	{
		ship.velocity.z += vel*(cos(theta_radians));
		ship.velocity.x += vel*(sin(theta_radians));
	}
	else if(ship.theta==0)
	{
		ship.velocity.z += vel;
	}

	cout << ship.theta << endl;

}

void ship_decrease_velocity()
{

	float vel = (SHIP_ENGINE_FORCE / SHIP_MASS * 0.01);
	float theta_radians = ship.theta * (PI/180);
	
	if(ship.theta < 0)
	{
		ship.theta +=360;
	}
	if (ship.theta > 360)
	{
		ship.theta -= 360;
	}

	if(ship.theta == 0)
	{
		ship.velocity.z -= vel;
	}
	else if(ship.theta > 270)//quad 4
	{
		ship.velocity.x += vel * (sin(-theta_radians));
		ship.velocity.z -= vel * (cos(-theta_radians));
	}
	else if(ship.theta == 360)
	{
		ship.velocity.x +=vel;
	}
	else if(ship.theta > 180) //quad 3
	{
		ship.velocity.z += vel * (cos(theta_radians-PI));
		ship.velocity.x += vel * (sin(theta_radians-PI));
	}
	else if(ship.theta == 90)
	{
		ship.velocity.z +=vel;
	}
	else if(ship.theta > 90)//quad 2
	{
		ship.velocity.z +=vel*(cos(PI-theta_radians));
		ship.velocity.x -=vel*(sin(PI-theta_radians));
	}
	else if(ship.theta == 180)
	{
		ship.velocity.z += vel;
	}
	else if(ship.theta > 0) //quad1
	{
		ship.velocity.z -= vel*(cos(theta_radians));
		ship.velocity.x -= vel*(sin(theta_radians));
	}
	else if(ship.theta==0)
	{
		ship.velocity.z -= vel;
		
	}
}

void ship_increase_bouyancy()
{

	if(ship.phi < SHIP_TILT_MAX)
		ship.phi += 6;
	else 
		ship.phi = SHIP_TILT_MAX;

	float vel = (SHIP_ENGINE_FORCE / SHIP_MASS * 0.01);
	ship.velocity.y += vel;
	
}

void ship_decrease_bouyancy()
{
	if(ship.phi > SHIP_TILT_MIN)
		ship.phi -= 6;
	else 
		ship.phi = SHIP_TILT_MIN;

	float vel = (SHIP_ENGINE_FORCE / SHIP_MASS * 0.01);
	ship.velocity.y -= vel;
}

void draw_fog()
{
    float fog_color[4] = {0.0, bg_g, bg_b, 1.0};
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glHint(GL_FOG_HINT, GL_DONT_CARE);
	glFogfv(GL_FOG_COLOR, fog_color);
	glFogf (GL_FOG_START, 0);
	glFogf (GL_FOG_END, FOG_DISTANCE);
}

void collision_detection()
{
	//calls all the other ones
	ship_vs_meteor();
	if (ship.coord_vector.x < 0 || ship.coord_vector.y < 0 || ship.coord_vector.z < 0 || ship.coord_vector.x > 1000 || ship.coord_vector.y > 1000 || ship.coord_vector.z > 1000)
		ship_outside();

}

void ship_vs_meteor()
{
	for (int i = 0; i < MAX_METEORS; i++)
	{
		distance_MtS = score(meteor_feild[i], ship.coord_vector.x, ship.coord_vector.y, ship.coord_vector.z);
	
		if (distance_MtS < METEOR_RADIUS &&  meteor_feild[i].visible == true)
			gameover = true;
	}
}

void ship_outside()
{
	//if the ship goes outside the boundaries it acts as a black hole and sends them back into the game.
	set_random_3d_coords(ship.coord_vector.x, ship.coord_vector.y, ship.coord_vector.z);

}

float visiting(PLANET& tar)
{
	return(pow((tar.coords.x - ship.coord_vector.x), 2)+pow((tar.coords.y - ship.coord_vector.y), 2)+pow((tar.coords.z - ship.coord_vector.z),2));
}

void shoot()
{ 
	//depending on the angle, the ship will shoot a specific way
	shot.coords = ship.coord_vector;
	shot.velocity = ship.velocity;

	float vel = (SHOT_SPEED);
	float theta_radians = ship.theta * (PI/180);
	
	if(ship.theta < 0)
	{
		ship.theta +=360;
	}
	if (ship.theta > 360)
	{
		ship.theta -= 360;
	}

	if(ship.theta == 0)
	{
		shot.velocity.z += vel;
	}
	else if(ship.theta > 270)//quad 4
	{
		shot.velocity.x -= vel * (sin(-theta_radians));
		shot.velocity.z += vel * (cos(-theta_radians));
	}
	else if(ship.theta == 360)
	{
		shot.velocity.x -=vel;
	}
	else if(ship.theta > 180) //quad 3
	{
		shot.velocity.z -= vel * (cos(theta_radians-PI));
		shot.velocity.x -= vel * (sin(theta_radians-PI));
	}
	else if(ship.theta == 90)
	{
		shot.velocity.x += vel;
	}
	else if(ship.theta > 90)//quad 2
	{
		shot.velocity.z -=vel*(cos(PI-theta_radians));
		shot.velocity.x +=vel*(sin(PI-theta_radians));
	}
	else if(ship.theta == 180)
	{
		shot.velocity.z -= vel;
	}
	else if(ship.theta > 0) //quad1
	{
		shot.velocity.z += vel*(cos(theta_radians));
		shot.velocity.x += vel*(sin(theta_radians));
	}
	else if(ship.theta==0)
	{
		shot.velocity.z += vel;
		
	}

	shot.visable = true;

}

void draw_shot()
{
	static float distance_from_ship = pow((shot.coords.x - ship.coord_vector.x), 2)+pow((shot.coords.y - ship.coord_vector.y), 2)+pow((shot.coords.z - ship.coord_vector.z),2);

	glColor3f(1.0,1.0,1.0); //colour
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glTranslatef(shot.coords.x, shot.coords.y, shot.coords.z); 
		glScalef(0.4f, 0.4f, 0.4f);   
		glutSolidCube(0.3f);
	glPopMatrix();

	if(distance_from_ship > 30000)
		shot.visable = false;

}

void hit_meteor(METEOR& m)
{
	float bullet_to_m = score(m, shot.coords.x, shot.coords.y, shot.coords.z);

	if(bullet_to_m < METEOR_RADIUS)
	{
		m.visible = false;
		shot.visable = false;
	}
}

void win()
{
	stringstream go; 
	go << "YOU WIN!";
				
	string over = go.str();
	SpriteFont::setUp2DView(750,650); 
		coord_font.draw(over, 300, 300, 255, 150, 30); //x, y, R, G, B
	SpriteFont::unsetUp2DView();
}

void get_angles()
{
	float theta_radians = ship.theta * (PI/180);
	
	if(ship.theta < 0)
	{
		ship.theta +=360;
	}
	if (ship.theta > 360)
	{
		ship.theta -= 360;
	}

	if(ship.theta == 0)
	{
		ship.shipz = (cos(-theta_radians));
		ship.shipx = 0;
	}
	else if(ship.theta > 270)//quad 4
	{
		ship.shipx = (sin(theta_radians));
		ship.shipz = (cos(theta_radians));
	}
	else if(ship.theta == 360)
	{
		ship.shipx = -(sin(-theta_radians));
		ship.shipz = 0;
	}
	else if(ship.theta > 180) //quad 3
	{
		ship.shipz = -(cos(theta_radians-PI));
		ship.shipx = -(sin(theta_radians-PI));
	}
	else if(ship.theta == 90)
	{
		ship.shipx = (sin(theta_radians));
		ship.shipz = 0;
	}
	else if(ship.theta > 90)//quad 2
	{
		ship.shipz = -(cos(PI-theta_radians));
		ship.shipx = (sin(PI-theta_radians));
	}
	else if(ship.theta == 180)
	{
		ship.shipz = -(cos(theta_radians));
	}
	else if(ship.theta > 0) //quad1
	{
		ship.shipz = (cos(theta_radians));
		ship.shipx = (sin(theta_radians));
	}
	else if(ship.theta==0)
	{
		ship.shipz = (cos(theta_radians));
		ship.shipx = 0;
		
	}

}

