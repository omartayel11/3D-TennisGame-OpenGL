#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <glut.h>

#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)
#define M_PI 3.14159265358979323846


class Vector3f {
public:
	float x, y, z;
	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) : x(_x), y(_y), z(_z) {}

	Vector3f operator+(Vector3f v) { return Vector3f(x + v.x, y + v.y, z + v.z); }
	Vector3f operator-(Vector3f v) { return Vector3f(x - v.x, y - v.y, z - v.z); }
	Vector3f operator*(float n) { return Vector3f(x * n, y * n, z * n); }
	Vector3f operator/(float n) { return Vector3f(x / n, y / n, z / n); }
	Vector3f unit() { return *this / sqrt(x * x + y * y + z * z); }
	Vector3f cross(Vector3f v) { return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
};

Vector3f ballPosition(2.0f, 1.1f, 1.0f);  // Ball position
float ballBounceHeight = 1.0f;  // Maximum height of the bounce
float ballBounceSpeed = 4.0f;   // Speed of the bounce
float elapsedTime = 0.0f;       // Elapsed time for controlling bounce
Vector3f ballPosition2(-2.0f, 1.1f, 1.0f);  // Ball position

float courtMinX = -5.8f; // Left boundary
float courtMaxX = 5.8f;  // Right boundary
float courtMinZ = 0.0f; // Front boundary
float courtMaxZ = 13.3f;  // Back boundary

bool isForehand = false;  // Whether forehand motion is happening
float forehandAngle = 0.0f;  // The angle of the forehand motion
float forehandSpeed = 2.0f;  // Speed at which the forehand swing occurs

bool isScaling = false;   // Flag to determine if scaling should be active
float ballScaleFactor = 2.0f;   // Initial scale factor
bool scalingUp = true;         // Direction of scaling (true = increasing, false = decreasing)
bool isRotatingChair = false;  // Rotation state flag for the umpire chair
float chairRotationAngle = 0.0f;

float basketTranslationX = 0.0f;  // Initial X translation
float basketTranslationSpeed = 0.05f;  // Speed of translation
bool isMovingBasket = false;  // Flag for controlling basket movement
float basketRotationAngle = 0.0f;

bool isColorChanging = false;  // Flag for controlling color change
float colorChangeTime = 0.0f;  // Time counter for color change
GLfloat benchColor[3] = { 0.4f, 0.5f, 0.6f }; 

// Global Variables
bool isBallBoyJumping = false;  // Toggle for jumping animation
float ballBoyJumpHeight = 0.0f;  // Jump height variable
float ballBoyJumpSpeed = 0.05f;  // Jump speed for the up and down movement

int score = 0;
bool ballHit = false; 
bool ballHit2 = false;
float ballRadius = 0.5f;
bool gameWin = false;
bool gameLost = false;
int timer = 30;

enum GameState { PLAYING, GAME_WON, GAME_LOST };
GameState currentGameState = PLAYING;


class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 4.0f, float eyeY = 13.0f, float eyeZ = 25.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void setView(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}
};

Camera camera;


float playerX = -3.0f;  // Player's initial X position
float playerZ = 10.0f;  // Player's initial Z position
float playerAngle = 0.0f;  // Player's facing direction in degrees
float legAngle = 0.0f;  // Angle for leg movement (for walking effect)
bool legMovingForward = true;  // Direction of leg movement (for walking)

// Draw the player at its current position and orientation
void drawPlayer() {
	//glDisable(GL_LIGHTING);

	/*GLfloat whiteAmbient[] = {0.8f, 0.8f, 0.8f, 1.0f};  // Ambient light component
	GLfloat whiteDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };  // Diffuse light component
	GLfloat whiteSpecular[] = { 0.5f, 0.5f, 0.5f, 1.0f }; // Specular light component
	GLfloat shininess = 50.0f;                           // Shininess for specular highlights

	glMaterialfv(GL_FRONT, GL_AMBIENT, whiteAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, whiteDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);*/
	glPushMatrix();
	glTranslatef(playerX, -0.3f, playerZ); // Player's position
	glRotatef(playerAngle, 0.0f, 1.0f, 0.0f); // Rotate to face movement direction

	// Head
	glColor3f(1.0f, 0.8f, 0.6f); // Skin tone
	glPushMatrix();
	glTranslatef(0.0f, 1.8f, 0.0f);
	glutSolidSphere(0.2f, 20, 20); // Head
	glPopMatrix();

	// Eyes (two small spheres)
	glColor3f(0.0f, 0.0f, 0.0f); // Black color for the eyes
	// Right Eye
	glPushMatrix();
	glTranslatef(0.1f, 1.9f, -0.16f); // Position on the right side of the face
	glutSolidSphere(0.05f, 10, 10); // Right eye
	glPopMatrix();
	// Left Eye
	glPushMatrix();
	glTranslatef(-0.1f, 1.9f, -0.16f); // Position on the left side of the face
	glutSolidSphere(0.05f, 10, 10); // Left eye
	glPopMatrix();

	// Nose (small cone)
	glColor3f(1.0f, 0.8f, 0.6f); // Skin tone
	glPushMatrix();
	glTranslatef(0.0f, 1.7f, 0.2f); // Nose position
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotate cone to point outwards
	glutSolidCone(0.05f, 0.1f, 10, 10); // Nose
	glPopMatrix();

	// Mouth (simple box to approximate a mouth)
	glColor3f(0.8f, 0.0f, 0.0f); // Red color for the mouth
	glPushMatrix();
	glTranslatef(0.0f, 1.5f, 0.2f); // Position the mouth below the nose
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotate to horizontal
	glScalef(0.2f, 0.05f, 0.05f); // Scale to make a wide mouth shape
	glutSolidCube(0.05f); // Mouth as a small box (cube)
	glPopMatrix();

	// Body
	glColor3f(0.0f, 0.5f, 1.0f); // Blue shirt
	glPushMatrix();
	glTranslatef(0.0f, 1.2f, 0.0f);
	glScalef(0.3f, 0.6f, 0.2f);
	glutSolidCube(1.0f); // Torso
	glPopMatrix();

	// Arms (fixed in position)
	glColor3f(1.0f, 0.8f, 0.6f); // Skin tone

	// Right Arm (Forehand motion)
	glPushMatrix();
	glTranslatef(0.25f, 1.3f, 0.0f);  // Position of right arm

	glRotatef(30, 0, 0, 1);  // Rotate slightly for forehand motion

	if (isForehand) {
		// Apply forehand motion (rotation over time)
		glRotatef(forehandAngle, 1.0f, 0.0f, 0.0f);
		if (forehandAngle > -60.0f) {
			forehandAngle -= forehandSpeed; 
		}
		else {
			isForehand = false;  // Stop after swing completes
			forehandAngle = 0.0f; // Reset angle for next swing
		}
	}
	glScalef(0.1f, 0.5f, 0.1f); // Arm size
	glutSolidCube(1.0f); // Right arm
	glPopMatrix();  // End arm transformation

	// Left Arm (no forehand motion)
	glPushMatrix();
	glTranslatef(-0.25f, 1.3f, 0.0f);
	glRotatef(-30, 0, 0, 1);
	glScalef(0.1f, 0.5f, 0.1f);
	glutSolidCube(1.0f); // Left arm
	glPopMatrix();

	// Right Leg with walking motion
	glColor3f(0.0f, 0.5f, 1.0f); // Shorts color
	glPushMatrix();
	glTranslatef(0.1f, 0.6f, 0.0f);
	glRotatef(legAngle, 1, 0, 0); // Forward-backward leg swing
	glScalef(0.15f, 0.5f, 0.15f);
	glutSolidCube(1.0f); // Right leg
	glPopMatrix();

	// Left Leg with walking motion
	glPushMatrix();
	glTranslatef(-0.1f, 0.6f, 0.0f);
	glRotatef(-legAngle, 1, 0, 0); // Forward-backward leg swing opposite to right leg
	glScalef(0.15f, 0.5f, 0.15f);
	glutSolidCube(1.0f); // Left leg
	glPopMatrix();

	// Tennis racket in right hand (move with right arm)
	glColor3f(0.4f, 0.2f, 0.0f); // Brown handle
	glPushMatrix();
	glTranslatef(0.35f, 1.2f, 0.0f);  // Position racket at the hand
	glRotatef(-30, 0, 0, 1);  // Tilt racket
	glRotatef(-20, 0, 1, 0);  // Additional rotation

	// Apply forehand rotation to the racket, just like the arm
	if (isForehand) {
		glRotatef(forehandAngle, 1.0f, 0.0f, 0.0f); // Rotate racket with arm
	}

	// Handle of the racket
	glPushMatrix();
	glScalef(0.05f, 0.4f, 0.05f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Racket head
	glColor3f(0.9f, 0.9f, 0.9f); // Light gray
	glTranslatef(0.0f, 0.3f, 0.0f);
	glutSolidTorus(0.02f, 0.15f, 10, 10); // Racket head (with forehand rotation)
	glPopMatrix();  // End racket transformation

	glPopMatrix();  // End player transformation
	//glEnable(GL_LIGHTING);  // Restore lighting
}

void renderText(float x, float y, const char* text) {
	glDisable(GL_LIGHTING);
	glColor3f(1.0f, 1.0f, 1.0f); // White color for text
	glRasterPos2f(x, y); // Position of the text on the screen
	for (int i = 0; text[i] != '\0'; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]); // Render each character
	}
	glEnable(GL_LIGHTING);
}

void renderTextTimer(float x, float y, const char* text, void* font) {
	glRasterPos2f(x, y);  // Set the position for the text
	for (const char* c = text; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);  // Render each character
	}
}

void displayTimer() {
	glColor3f(1.0f, 1.0f, 1.0f);  // Set text color (white)
	char timerText[16];
	sprintf(timerText, "Time: %d", timer);  // Convert timer value to string
	renderTextTimer(-0.95f, 0.9f, timerText, GLUT_BITMAP_HELVETICA_18);  // Position and render
}

void timerCallback(int value) {
	if (timer > 0) {
		timer--;  // Decrease timer by 1 second
		glutTimerFunc(1000, timerCallback, 0);  // Call this function again in 1 second
	}
	else {
		gameLost = true;  // Set gameLost to true when timer reaches 0
	}

	glutPostRedisplay();  // Redraw the screen to update the timer display
}


bool checkCollision(float playerX, float playerZ, float ballX, float ballZ, float collisionRadius) {
	// Calculate the distance between the player and the ball
	float distance = sqrt(pow(playerX - ballX, 2) + pow(playerZ - ballZ, 2));
	// If the distance between the player and the ball is less than the collision radius, consider it a collision 
	if (distance < collisionRadius) {
		return true;
	}

	return false;
}

void update() {
	if (isForehand && checkCollision(playerX, playerZ, ballPosition.x, ballPosition.z, ballRadius)) {  // Adjust the racket size for collision detection
		if (!ballHit) {  // Only increment score once
			score += 100;
			ballHit = true;  // Set flag to prevent multiple score increments
		}
	}
	//printf("Score: %d\n", score);
}

void update2() {
	if (isForehand && checkCollision(playerX, playerZ, ballPosition2.x, ballPosition2.z, ballRadius)) {  // Adjust the racket size for collision detection
		if (!ballHit2) {  // Only increment score once
			score += 100;
			ballHit2 = true;  // Set flag to prevent multiple score increments
		}
	}
	//printf("Score: %d\n", score);
}

void drawWall(double width, double height, double thickness) {
	glPushMatrix();
	glScaled(width, height, thickness);
	glutSolidCube(1.0);
	glPopMatrix();
}

// Draw Tennis Net
void drawNet() {
	// Draw the two posts for the net
	glColor3f(1.0f, 1.0f, 1.0f); // White color for posts
	glPushMatrix();
	glTranslatef(-5.5f, 1.0f, 0.0f);  // Left post position
	glRotatef(90, 1, 0, 0);  // Rotate to make it vertical
	gluCylinder(gluNewQuadric(), 0.1f, 0.1f, 2.0f, 10, 10);  // Post dimensions: radius 0.1, height 2
	glPopMatrix();

	glPushMatrix();
	glTranslatef(5.5f, 1.0f, 0.0f);   // Right post position
	glRotatef(90, 1, 0, 0);  // Rotate to make it vertical
	gluCylinder(gluNewQuadric(), 0.1f, 0.1f, 2.0f, 10, 10);  // Post dimensions: radius 0.1, height 2
	glPopMatrix();

	// Draw the net mesh
	glLineWidth(1.0f);  // Net line width
	glColor3f(1.0f, 1.0f, 1.0f); // White color for the net

	glBegin(GL_LINES);
	for (float y = 0.0f; y <= 1.0f; y += 0.1f) {
		glVertex3f(-5.5f, y, 0.0f); // Left side of the net
		glVertex3f(5.5f, y, 0.0f);  // Right side of the net
	}
	glEnd();

	glBegin(GL_LINES);
	for (float x = -5.5f; x <= 5.5f; x += 0.1f) {
		glVertex3f(x, 1.0f, 0.0f); // Left side of the net
		glVertex3f(x, 0.0f, 0.0f);  // Right side of the net
	}
	glVertex3f(-5.5f, 1.0f, 0.0f); // Left side of the net
	glVertex3f(5.5f, 1.0f, 0.0f);  // Right side of the net

	glEnd();
}

void drawBall() {
	// Disable lighting to ensure colors are directly applied
	float rad = 0.2;
	//glDisable(GL_LIGHTING);

	// Set ball's tennis ball color (yellow-greenish)
	glColor3f(0.8f, 0.9f, 0.2f); // Bright yellow-green color for the ball

	// Draw the main body of the ball as a solid sphere (smooth and realistic)
	glPushMatrix();
	glTranslatef(ballPosition.x, ballPosition.y, ballPosition.z);  // Position the ball
	glutSolidSphere(rad, 40, 40); // Radius 0.3, 40 slices, 40 stacks for smoother sphere
	glPopMatrix();

	// Draw the seam lines (representing the real tennis ball seam)
	glColor3f(0.9f, 0.9f, 0.0f); // Lighter yellow for the seams

	glPushMatrix();
	glTranslatef(2.0f, 0.0f, 1.0f);  // Position the seams to match the ball

	glPopMatrix();

	// Re-enable lighting to ensure subsequent objects are lit
	//glEnable(GL_LIGHTING);
}

void drawBall2() {
	// Disable lighting to ensure colors are directly applied
	float rad = 0.2;
	//glDisable(GL_LIGHTING);

	// Set ball's tennis ball color (yellow-greenish)
	glColor3f(0.8f, 0.9f, 0.2f); // Bright yellow-green color for the ball

	// Draw the main body of the ball as a solid sphere (smooth and realistic)
	glPushMatrix();
	glTranslatef(ballPosition2.x, ballPosition2.y, ballPosition2.z);  // Position the ball
	glutSolidSphere(rad, 40, 40); // Radius 0.3, 40 slices, 40 stacks for smoother sphere
	glPopMatrix();

	// Draw the seam lines (representing the real tennis ball seam)
	glColor3f(0.9f, 0.9f, 0.0f); // Lighter yellow for the seams

	glPushMatrix();
	glTranslatef(2.0f, 0.0f, 1.0f);  // Position the seams to match the ball

	glPopMatrix();

	// Re-enable lighting to ensure subsequent objects are lit
	//glEnable(GL_LIGHTING);
}

void updateBall() {
	// Increment the elapsed time
	elapsedTime += 0.016f;  // Assuming the timer is set for ~60 FPS

	// Set the ball's vertical position using a sine wave function
	ballPosition.y = 1.1f + ballBounceHeight * sin(elapsedTime * ballBounceSpeed);
}

void updateBall2() {
	// Increment the elapsed time
	elapsedTime += 0.016f;  // Assuming the timer is set for ~60 FPS

	// Set the ball's vertical position using a sine wave function
	ballPosition2.y = 1.1f + ballBounceHeight * sin(elapsedTime * ballBounceSpeed);
}

void drawScoreboard() {
	//glDisable(GL_LIGHTING);
	glPushMatrix();
	glTranslatef(0.0f, 2.5f, -13.5f);  // Position above the back wall

	// 1. Base Frame (Cuboid)
	glColor3f(0.1f, 0.1f, 0.1f);  // Dark gray
	glPushMatrix();
	glScalef(5.0f, 1.5f, 0.1f);  // Base shape of the scoreboard
	glutSolidCube(1.0f);
	glPopMatrix();

	// 2. Outer Border (Scaled Cuboid)
	glColor3f(0.0f, 0.0f, 0.5f);  // Blue border for US Open theme
	glPushMatrix();
	glScalef(5.2f, 1.7f, 0.12f);  // Slightly larger than the base
	glutSolidCube(1.0f);
	glPopMatrix();

	// 3. Text Panel (Cuboid)
	glColor3f(0.2f, 0.2f, 0.2f);  // Slightly lighter gray for contrast
	glPushMatrix();
	glScalef(4.8f, 1.3f, 0.1f);  // Slightly smaller than the base
	glutSolidCube(1.0f);
	glPopMatrix();

	// Render Text
	glColor3f(1.0f, 1.0f, 1.0f);  // White text
	renderTextTimer(-0.9f, 0.2f, "US OPEN", GLUT_BITMAP_HELVETICA_18);
	char scoreText[20];
	sprintf(scoreText, "Score: %d", score);
	renderTextTimer(-0.9f, -0.2f, scoreText, GLUT_BITMAP_HELVETICA_18);

	// 4. Decorative Tennis Rackets (Cylinder + Torus)
	// Left Racket
	glColor3f(0.8f, 0.8f, 0.8f);  // Silver for the racket frame
	glPushMatrix();
	glTranslatef(-2.0f, 0.2f, 0.05f);
	glRotatef(45, 0.0f, 0.0f, 1.0f);
	GLUquadric* quad = gluNewQuadric();
	gluCylinder(quad, 0.05f, 0.05f, 1.0f, 20, 20);  // Racket handle
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-1.5f, 0.5f, 0.05f);
	glutSolidTorus(0.03f, 0.4f, 20, 20);  // Racket head
	glPopMatrix();

	// Right Racket
	glPushMatrix();
	glTranslatef(2.0f, 0.2f, 0.05f);
	glRotatef(-45, 0.0f, 0.0f, 1.0f);
	gluCylinder(quad, 0.05f, 0.05f, 1.0f, 20, 20);  // Racket handle
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.5f, 0.5f, 0.05f);
	glutSolidTorus(0.03f, 0.4f, 20, 20);  // Racket head
	glPopMatrix();
	glPopMatrix();

	gluDeleteQuadric(quad);
	glPopMatrix();
	//glEnable(GL_LIGHTING);
}

void drawScalingTennisBall() {
	glPushMatrix();
	glTranslatef(0.0f, 3.5f, -13.5f);  // Position above the scoreboard

	// Apply scaling only if scaling is active
	if (isScaling) {
		// Update the scaling factor
		if (scalingUp) {
			ballScaleFactor += 0.01f;  // Increment the scale
			if (ballScaleFactor >= 1.5f) scalingUp = false;  // Reverse direction if max scale reached
		}
		else {
			ballScaleFactor -= 0.01f;  // Decrement the scale
			if (ballScaleFactor <= 0.8f) scalingUp = true;   // Reverse direction if min scale reached
		}

		// Apply scaling
		glScalef(ballScaleFactor, ballScaleFactor, ballScaleFactor);
	}

	// Draw the tennis ball
	glColor3f(0.9f, 0.9f, 0.0f);  // Yellow color
	glutSolidSphere(0.3f, 20, 20);

	// Add white curved stripes to resemble a tennis ball
	glColor3f(1.0f, 1.0f, 1.0f);
	glPushMatrix();
	glRotatef(90, 0.0f, 1.0f, 0.0f);
	glutSolidTorus(0.02f, 0.28f, 10, 10);
	glPopMatrix();

	glPopMatrix();
}

GLUquadric* quadric = gluNewQuadric();

void drawTennisBallBasket() {
	glPushMatrix();
	glTranslatef(basketTranslationX, 2.0f, -6.0f);  // Position the basket initially at (0, 0, -6)

	// 1. Basket's Body (Cylinder) - main cylindrical part
	glColor3f(0.7f, 0.3f, 0.1f);  // Wooden brown color for the basket
	glPushMatrix();
	glScalef(1.0f, 1.5f, 1.0f);  // Make the cylinder taller and slightly wider
	gluCylinder(quadric, 0.6f, 0.6f, 1.8f, 20, 10);  // Create a solid cylinder for the body of the basket
	glPopMatrix();

	// 2. Vertical Bars (Grid-like structure)
	glColor3f(0.5f, 0.3f, 0.1f);  // Darker color for the vertical bars

	// Left side vertical bars (3 bars)
	for (float y = -0.5f; y <= 0.5f; y += 0.5f) {
		glPushMatrix();
		glTranslatef(-0.6f, y, 0.0f);  // Positioning left bars
		glRotatef(90, 1.0f, 0.0f, 0.0f);  // Rotate to make vertical bars
		gluCylinder(quadric, 0.05f, 0.05f, 1.8f, 10, 10);  // Create a vertical cylinder
		glPopMatrix();
	}

	// Right side vertical bars (3 bars)
	for (float y = -0.5f; y <= 0.5f; y += 0.5f) {
		glPushMatrix();
		glTranslatef(0.6f, y, 0.0f);  // Positioning right bars
		glRotatef(90, 1.0f, 0.0f, 0.0f);  // Rotate to make vertical bars
		gluCylinder(quadric, 0.05f, 0.05f, 1.8f, 10, 10);  // Create a vertical cylinder
		glPopMatrix();
	}

	// 3. Horizontal Bars (Supports for the basket)
	glColor3f(0.6f, 0.3f, 0.1f);  // Darker color for horizontal bars

	// Upper horizontal bar
	glPushMatrix();
	glTranslatef(0.0f, 0.9f, 0.0f);  // Position the upper horizontal bar
	glScalef(1.2f, 0.05f, 0.8f);  // Adjust the size of the bar
	glutSolidCube(1.0f);  // Use a cube for the bar
	glPopMatrix();

	// Lower horizontal bar
	glPushMatrix();
	glTranslatef(0.0f, -0.9f, 0.0f);  // Position the lower horizontal bar
	glScalef(1.2f, 0.05f, 0.8f);  // Adjust the size of the bar
	glutSolidCube(1.0f);  // Use a cube for the bar
	glPopMatrix();

	// 4. Base (Circular support at the bottom)
	glColor3f(0.7f, 0.3f, 0.1f);  // Same color as the body of the basket
	glPushMatrix();
	glTranslatef(0.0f, -1.0f, 0.0f);  // Positioning the base at the bottom
	gluDisk(quadric, 0.6f, 0.7f, 20, 10);  // Create a disk to represent the base
	glPopMatrix();

	// 5. Tennis Balls Inside the Basket (Multiple Spheres)
	glColor3f(1.0f, 1.0f, 0.0f);  // Yellow color for tennis balls
	float ballSpacing = 0.3f;  // Space between balls
	int ballCount = 3;  // Number of balls inside the basket

	// Draw tennis balls inside the basket (arranged in a grid)
	for (int i = 0; i < ballCount; i++) {
		for (int j = 0; j < ballCount; j++) {
			float ballX = (i - ballCount / 2) * ballSpacing;
			float ballY = (j - ballCount / 2) * ballSpacing;

			glPushMatrix();
			glTranslatef(ballX, ballY, 0.5f);  // Positioning balls inside the basket
			glutSolidSphere(0.2f, 20, 20);  // Draw each ball as a sphere
			glPopMatrix();
		}
	}

	glPopMatrix();  // End of basket positioning
}
void updateBasketPosition() {
	if (isMovingBasket) {
		basketTranslationX += basketTranslationSpeed;  // Move basket along X-axis
		if (basketTranslationX > 2.0f)  // If the basket moves too far
			basketTranslationSpeed = -basketTranslationSpeed;  // Reverse direction
		if (basketTranslationX < -2.0f)  // If the basket moves too far in the negative direction
			basketTranslationSpeed = -basketTranslationSpeed;  // Reverse direction
	}
}



void drawBallBoy(float xPos, float zPos) {
	glPushMatrix();
	glTranslatef(xPos, ballBoyJumpHeight, zPos);  // Translate for jump animation

	glColor3f(1.0f, 0.8f, 0.6f); // Skin tone
	glPushMatrix();
	glTranslatef(0.0f, 1.8f, 0.0f);
	glutSolidSphere(0.2f, 20, 20); // Head
	glPopMatrix();

	// Eyes (two small spheres)
	glColor3f(0.0f, 0.0f, 0.0f); // Black color for the eyes
	// Right Eye
	glPushMatrix();
	glTranslatef(0.1f, 1.9f, -0.16f); // Position on the right side of the face
	glutSolidSphere(0.05f, 10, 10); // Right eye
	glPopMatrix();
	// Left Eye
	glPushMatrix();
	glTranslatef(-0.1f, 1.9f, -0.16f); // Position on the left side of the face
	glutSolidSphere(0.05f, 10, 10); // Left eye
	glPopMatrix();

	// Nose (small cone)
	glColor3f(1.0f, 0.8f, 0.6f); // Skin tone
	glPushMatrix();
	glTranslatef(0.0f, 1.7f, 0.2f); // Nose position
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotate cone to point outwards
	glutSolidCone(0.05f, 0.1f, 10, 10); // Nose
	glPopMatrix();

	// Mouth (simple box to approximate a mouth)
	glColor3f(0.8f, 0.0f, 0.0f); // Red color for the mouth
	glPushMatrix();
	glTranslatef(0.0f, 1.5f, 0.2f); // Position the mouth below the nose
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotate to horizontal
	glScalef(0.2f, 0.05f, 0.05f); // Scale to make a wide mouth shape
	glutSolidCube(0.05f); // Mouth as a small box (cube)
	glPopMatrix();

	// Body
	glColor3f(0.0f, 0.5f, 1.0f); // Blue shirt
	glPushMatrix();
	glTranslatef(0.0f, 1.2f, 0.0f);
	glScalef(0.3f, 0.6f, 0.2f);
	glutSolidCube(1.0f); // Torso
	glPopMatrix();

	// Arms (fixed in position)
	glColor3f(1.0f, 0.8f, 0.6f); // Skin tone

	// Right Arm (Forehand motion)
	glPushMatrix();
	glTranslatef(0.25f, 1.3f, 0.0f);  // Position of right arm

	glRotatef(30, 0, 0, 1);  // Rotate slightly for forehand motion

	glScalef(0.1f, 0.5f, 0.1f); // Arm size
	glutSolidCube(1.0f); // Right arm
	glPopMatrix();  // End arm transformation

	// Left Arm (no forehand motion)
	glPushMatrix();
	glTranslatef(-0.25f, 1.3f, 0.0f);
	glRotatef(-30, 0, 0, 1);
	glScalef(0.1f, 0.5f, 0.1f);
	glutSolidCube(1.0f); // Left arm
	glPopMatrix();

	// Right Leg with walking motion
	glColor3f(0.0f, 0.5f, 1.0f); // Shorts color
	glPushMatrix();
	glTranslatef(0.1f, 0.6f, 0.0f);
	//glRotatef(legAngle, 1, 0, 0); // Forward-backward leg swing
	glScalef(0.15f, 0.5f, 0.15f);
	glutSolidCube(1.0f); // Right leg
	glPopMatrix();

	// Left Leg with walking motion
	glPushMatrix();
	glTranslatef(-0.1f, 0.6f, 0.0f);
	//glRotatef(-legAngle, 1, 0, 0); // Forward-backward leg swing opposite to right leg
	glScalef(0.15f, 0.5f, 0.15f);
	glutSolidCube(1.0f); // Left leg
	glPopMatrix();

	glPopMatrix();  // End of ball boy positioning
}

// Function to handle the jumping animation when pressing 'b'
void updateBallBoyJump() {
	if (isBallBoyJumping) {
		ballBoyJumpHeight += ballBoyJumpSpeed;
		if (ballBoyJumpHeight > 0.2f || ballBoyJumpHeight < 0.0f) {
			ballBoyJumpSpeed = -ballBoyJumpSpeed;  // Reverse direction for jumping
		}
	}
}


void drawUmpireChair() {
	glPushMatrix();
	glTranslatef(-4.6f, 1.0f, -3.0f);  // Position the chair
	glRotatef(90, 0.0f, 1.0f, 0.0f);
	// Rotate if active
	if (isRotatingChair) {
		chairRotationAngle += 1.0f;  // Increment the rotation angle
		if (chairRotationAngle >= 360.0f) chairRotationAngle -= 360.0f;
	}
	glRotatef(chairRotationAngle, 0.0f, 1.0f, 0.0f);  // Rotate about Y-axis

	// 1. Seat (Cuboid)
	glColor3f(0.7f, 0.5f, 0.3f);  // Wood-like color
	glPushMatrix();
	glTranslatef(0.0f, 1.5f, 0.0f);  // Positioning the seat
	glScalef(1.2f, 0.2f, 1.5f);  // Adjust size of the seat
	glutSolidCube(1.0f);  // Seat part
	glPopMatrix();

	// 2. Backrest (Cuboid)
	glColor3f(0.7f, 0.5f, 0.3f);  // Same wood color for the backrest
	glPushMatrix();
	glTranslatef(0.0f, 2.2f, -0.8f);  // Positioning the backrest
	glScalef(1.5f, 1.0f, 0.2f);  // Size of the backrest
	glutSolidCube(1.0f);  // Backrest part
	glPopMatrix();

	// 3. Legs (4 Cylinders)
	glColor3f(0.5f, 0.3f, 0.1f);  // Darker color for legs

	// Front left leg (Taller)
	glPushMatrix();
	glTranslatef(-0.6f, 1.5f, 0.6f);
	glRotatef(90, 1.0f, 0.0f, 0.0f);  // Rotate to make the leg vertical
	gluCylinder(gluNewQuadric(), 0.05f, 0.05f, 3.0f, 10, 10);  // Increased height
	glPopMatrix();

	// Front right leg (Taller)
	glPushMatrix();
	glTranslatef(0.6f, 1.5f, 0.6f);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	gluCylinder(gluNewQuadric(), 0.05f, 0.05f, 3.0f, 10, 10);  // Increased height
	glPopMatrix();

	// Back left leg (Taller)
	glPushMatrix();
	glTranslatef(-0.6f, 1.5f, -0.6f);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	gluCylinder(gluNewQuadric(), 0.05f, 0.05f, 3.0f, 10, 10);  // Increased height
	glPopMatrix();

	// Back right leg (Taller)
	glPushMatrix();
	glTranslatef(0.6f, 1.5f, -0.6f);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	gluCylinder(gluNewQuadric(), 0.05f, 0.05f, 3.0f, 10, 10);  // Increased height
	glPopMatrix();

	// 4. Armrests (Cuboids)
	glColor3f(0.7f, 0.5f, 0.3f);  // Same wood color for the armrests

	// Left armrest
	glPushMatrix();
	glTranslatef(-0.9f, 1.8f, 0.4f);
	glScalef(0.2f, 0.1f, 0.8f);
	glutSolidCube(1.0f);  // Armrest part
	glPopMatrix();

	// Right armrest
	glPushMatrix();
	glTranslatef(0.9f, 1.8f, 0.4f);
	glScalef(0.2f, 0.1f, 0.8f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// 5. Footrest (Cuboid)
	glColor3f(0.7f, 0.5f, 0.3f);  // Wood-like color
	glPushMatrix();
	glTranslatef(0.0f, 0.4f, 0.0f);  // Positioning the footrest
	glScalef(1.5f, 0.1f, 1.0f);  // Size of the footrest
	glutSolidCube(1.0f);  // Footrest part
	glPopMatrix();

	glPopMatrix();  // End of chair positioning
}


// Function to draw a player bench
void drawPlayerBench(float xPos, float zPos) {
	glPushMatrix();
	glTranslatef(xPos, 0.0f, zPos);  // Positioning the bench at x = 5, z = -2 or z = 2
	glRotatef(-90, 0.0f, 1.0f, 0.0f);
	// Update the color if color change is enabled
	if (isColorChanging) {
		colorChangeTime += 1.0f;
		if (colorChangeTime >= 1.0f) {  // Change the color every 1 second
			// Randomly change the color (for demonstration purposes)
			benchColor[0] = (rand() % 100) / 100.0f;
			benchColor[1] = (rand() % 100) / 100.0f;
			benchColor[2] = (rand() % 100) / 100.0f;
			colorChangeTime = 0.0f;
		}
	}

	glColor3fv(benchColor);  // Set the current color for the bench

	// 1. Main Seat (Cuboid)
	glPushMatrix();
	glTranslatef(0.0f, 1.0f, 0.0f);  // Positioning the seat
	glScalef(3.0f, 0.3f, 1.0f);  // Size of the seat
	glutSolidCube(1.0f);  // Create the seat
	glPopMatrix();

	// 2. Backrest (Cuboid)
	glPushMatrix();
	glTranslatef(0.0f, 1.5f, -0.35f);  // Positioning the backrest
	glScalef(3.0f, 0.7f, 0.3f);  // Size of the backrest
	glutSolidCube(1.0f);  // Create the backrest
	glPopMatrix();

	// 3. Legs (4 Cylinders)
	glColor3f(0.3f, 0.1f, 0.05f);  // Darker color for the legs

	// Front left leg
	glPushMatrix();
	glTranslatef(-1.2f, 0.9f, 0.35f);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	gluCylinder(quadric, 0.05f, 0.05f, 1.0f, 10, 10);  // Create a vertical leg
	glPopMatrix();

	// Front right leg
	glPushMatrix();
	glTranslatef(1.2f, 0.9f, 0.35f);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	gluCylinder(quadric, 0.05f, 0.05f, 1.0f, 10, 10);  // Create a vertical leg
	glPopMatrix();

	// Back left leg
	glPushMatrix();
	glTranslatef(-1.2f, 0.9f, -0.35f);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	gluCylinder(quadric, 0.05f, 0.05f, 1.0f, 10, 10);  // Create a vertical leg
	glPopMatrix();

	// Back right leg
	glPushMatrix();
	glTranslatef(1.2f, 0.9f, -0.35f);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	gluCylinder(quadric, 0.05f, 0.05f, 1.0f, 10, 10);  // Create a vertical leg
	glPopMatrix();

	// 4. Crossbars (Supports connecting the legs)
	glColor3f(0.5f, 0.2f, 0.1f);  // Slightly darker color for the supports

	// Horizontal bar between front legs
	glPushMatrix();
	glTranslatef(0.0f, 0.7f, 0.35f);
	glScalef(2.4f, 0.1f, 0.2f);
	glutSolidCube(1.0f);  // Horizontal support bar
	glPopMatrix();

	// Horizontal bar between back legs
	glPushMatrix();
	glTranslatef(0.0f, 0.7f, -0.35f);
	glScalef(2.4f, 0.1f, 0.2f);
	glutSolidCube(1.0f);  // Horizontal support bar
	glPopMatrix();

	// 5. Side Supports (Connecting front and back legs)
	glPushMatrix();
	glTranslatef(-1.2f, 0.7f, 0.0f);  // Position the side support on the left side
	glScalef(0.1f, 0.1f, 0.7f);  // Size of the side support
	glutSolidCube(1.0f);  // Create the side support
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.2f, 0.7f, 0.0f);  // Position the side support on the right side
	glScalef(0.1f, 0.1f, 0.7f);  // Size of the side support
	glutSolidCube(1.0f);  // Create the side support
	glPopMatrix();

	glPopMatrix();  // End of bench positioning
}



void drawCourt() {
	glPushMatrix();

	// Disable lighting temporarily to set the court color to green
	//glDisable(GL_LIGHTING);
	glColor3f(0.82f, 0.43f, 0.22f); // Clay court color
	glBegin(GL_QUADS);
	glVertex3f(-6.0f, 0.0f, -13.4f);
	glVertex3f(6.0f, 0.0f, -13.4f);
	glVertex3f(6.0f, 0.0f, 13.4f);
	glVertex3f(-6.0f, 0.0f, 13.4f);
	glEnd();

	// Set line color to white for the court markings
	glColor3f(1.0f, 1.0f, 1.0f);
	glLineWidth(2.0f);
	glBegin(GL_LINES);

	// Draw singles and doubles sidelines
	glVertex3f(-5.5f, 0.01f, -12.0f); glVertex3f(-5.5f, 0.01f, 12.0f); // Left doubles
	glVertex3f(5.5f, 0.01f, -12.0f); glVertex3f(5.5f, 0.01f, 12.0f);   // Right doubles
	glVertex3f(-4.1f, 0.01f, -12.0f); glVertex3f(-4.1f, 0.01f, 12.0f);  // Left singles
	glVertex3f(4.1f, 0.01f, -12.0f); glVertex3f(4.1f, 0.01f, 12.0f);    // Right singles
	glVertex3f(0.0f, 0.01f, -6.2f); glVertex3f(0.0f, 0.01f, 6.2f);    // 

	// Draw baselines and service lines
	glVertex3f(-5.5f, 0.01f, -12.0f); glVertex3f(5.5f, 0.01f, -12.0f);  // Far baseline
	glVertex3f(-5.5f, 0.01f, 12.0f); glVertex3f(5.5f, 0.01f, 12.0f);    // Near baseline
	glVertex3f(-4.1f, 0.01f, -6.4f); glVertex3f(4.1f, 0.01f, -6.4f);    // Far service line
	glVertex3f(-4.1f, 0.01f, 6.4f); glVertex3f(4.1f, 0.01f, 6.4f);      // Near service line

	// Draw net line
	glVertex3f(-5.5f, 0.01f, 0.0f); glVertex3f(5.5f, 0.01f, 0.0f);

	// Draw missing service box lines
	/*glVertex3f(-4.1f, 0.01f, 0.0f); glVertex3f(-4.1f, 0.01f, 6.4f);     // Near left service box
	glVertex3f(4.1f, 0.01f, 0.0f); glVertex3f(4.1f, 0.01f, 6.4f);       // Near right service box
	glVertex3f(-4.1f, 0.01f, 0.0f); glVertex3f(-4.1f, 0.01f, -6.4f);    // Far left service box
	glVertex3f(4.1f, 0.01f, 0.0f); glVertex3f(4.1f, 0.01f, -6.4f); */     // Far right service box

	glEnd();
	//glEnable(GL_LIGHTING);  // Re-enable lighting
	glPopMatrix();
}

void setupLights() {
	GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
	GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
	GLfloat shininess[] = { 50 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	GLfloat lightIntensity[] = { 0.7f, 0.7f, 1, 1.0f };
	GLfloat lightPosition[] = { -7.0f, 6.0f, 3.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
}

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 1.0, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

void checkGameOver() {
	if (gameLost) {
		currentGameState = GAME_LOST;
		glutPostRedisplay();  // Request a redraw to show the new scene
	}
}

void checkGameWin() {
	if (ballHit && ballHit2) {
		currentGameState = GAME_WON;
		glutPostRedisplay();  // Request a redraw to show the new scene
	}
}

GLfloat wallColor[3] = { 1.0f, 0.0f, 0.0f };  // Initial wall color (red)
void updateWallColor(int value) {
	// Generate random RGB values for the wall color
	wallColor[0] = static_cast<float>(rand() % 100) / 100.0f;  // Red component
	wallColor[1] = static_cast<float>(rand() % 100) / 100.0f;  // Green component
	wallColor[2] = static_cast<float>(rand() % 100) / 100.0f;  // Blue component

	// Request to redisplay the scene
	glutPostRedisplay();

	// Set the timer to call this function again after 2000 ms
	glutTimerFunc(2000, updateWallColor, 0);
}


void Display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (currentGameState) {
	case PLAYING:
		setupCamera();
		setupLights();

		// Draw game elements
		drawCourt();
		drawNet();
		drawScoreboard();
		drawScalingTennisBall();
		drawUmpireChair();
		drawTennisBallBasket();
		updateBasketPosition();
		drawPlayerBench(5.0f, -2.0f);
		drawPlayerBench(5.0f, 2.0f);
		drawBallBoy(-3.0f, -11.0f);
		drawBallBoy(3.0f, -11.0f);
		updateBallBoyJump();

		if (!ballHit) {
			drawBall();
			updateBall();
		}
		update();
		if (!ballHit2) {
			drawBall2();
			updateBall2();
		}
		update2();
		drawPlayer();

		// Display score
		char scoreText[20];
		sprintf(scoreText, "Score: %d", score);  // Format score as string
		renderText(0.8f, 0.9f, scoreText);  // Adjust the position of the score

		// Display timer
		displayTimer();

		// Draw boundary walls around the court
		glColor3f(1.0f, 0.0f, 0.0f);

		// Back wall
		glPushMatrix();
		glColor3fv(wallColor);  // Use the global wall color
		glTranslatef(0.0f, 1.0f, -13.4f);  // Adjusted to match the court boundary
		drawWall(12.0, 2.0, 0.1);
		glPopMatrix();

		// Left wall
		glPushMatrix();
		glColor3fv(wallColor);  // Use the global wall color
		glTranslatef(-6.0f, 1.0f, 0.0f);   // Adjusted to match the court boundary
		glRotatef(90, 0, 1, 0);
		drawWall(27.0, 2.0, 0.1);
		glPopMatrix();

		// Right wall
		glPushMatrix();
		glColor3fv(wallColor);  // Use the global wall color
		glTranslatef(6.0f, 1.0f, 0.0f);    // Adjusted to match the court boundary
		glRotatef(90, 0, 1, 0);
		drawWall(27.0, 2.0, 0.1);
		glPopMatrix();


		// Check game status
		checkGameOver();
		checkGameWin();
		break;

	case GAME_LOST:
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Black background
		glColor3f(1.0f, 0.0f, 0.0f);           // Red text
		renderTextTimer(-0.4f, 0.0f, "YOU LOST!", GLUT_BITMAP_HELVETICA_18);
		break;

	case GAME_WON:
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Black background
		glColor3f(0.0f, 1.0f, 0.0f);           // Green text
		renderTextTimer(-0.6f, 0.0f, "CONGRATULATIONS, YOU WON!", GLUT_BITMAP_HELVETICA_18);
		break;
	}

	glFlush();
}


/*void Display() {
	
	setupCamera();
	setupLights();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw tennis court floor and lines
	drawCourt();
	drawNet();
	if (!ballHit) {
		drawBall();
		updateBall();
	}
	update();
	if (!ballHit2) {
		drawBall2();
		updateBall2();
	}
	update2();
	drawPlayer();

	char scoreText[20];
	sprintf(scoreText, "Score: %d", score);  // Format score as string
	renderText(0.8f, 0.9f, scoreText);  // Adjust the position of the score
	displayTimer();
	checkGameOver();
	checkGameWin();
	// Draw boundary walls around the court
	glColor3f(1.0f, 0.0f, 0.0f);

	// Back wall
	glPushMatrix();
	glTranslatef(0.0f, 1.0f, -13.4f);  // Adjusted to match the court boundary
	drawWall(12.0, 2.0, 0.1);
	glPopMatrix();

	// Left wall
	glPushMatrix();
	glTranslatef(-6.0f, 1.0f, 0.0f);   // Adjusted to match the court boundary
	glRotatef(90, 0, 1, 0);
	drawWall(27.0, 2.0, 0.1);
	glPopMatrix();

	// Right wall
	glPushMatrix();
	glTranslatef(6.0f, 1.0f, 0.0f);    // Adjusted to match the court boundary
	glRotatef(90, 0, 1, 0);
	drawWall(27.0, 2.0, 0.1);
	glPopMatrix();


	glFlush();
}*/

void Keyboard(unsigned char key, int x, int y) {
	float d = 0.2;
	float stepSize = 0.1f;  // Movement step size
	float rotationAngle = 90.0f;  // Rotate 90 degrees to face the direction

	float newPlayerX = playerX;
	float newPlayerZ = playerZ;

	switch (key) {
	case 'w':
		camera.moveY(d);
		break;
	case 's':
		camera.moveY(-d);
		break;
	case 'a':
		camera.moveX(d);
		break;
	case 'd':
		camera.moveX(-d);
		break;
	case 'q':
		camera.moveZ(d);
		break;
	case 'e':
		camera.moveZ(-d);
		break;
	case '1':  // Front view
		camera.setView(0.0f, 2.5f, 18.0f, 0.0f, 0.0f, 0.0f);
		break;
	case '2':  // Top view
		camera.setView(0.0f, 40.0f, 0.1f, 0.0f, 0.0f, 0.0f);
		break;
	case '3':  // Side view
		camera.setView(30.0f, 14.5f, 0.0f, -9.0f, 0.0f, 0.0f);
		break;
	case '4':  // Side view
		camera.setView(4.0f, 13.0f, 25.0f, 0.0f, 0.0f, 0.0f);
		break;
	case 'u':  // Move forward and face forward (0 degrees)
		playerAngle = 0.0f;  // Face forward
		newPlayerZ -= stepSize;  // Move forward in the Z direction
		break;

	case 'j':  // Move backward and face backward (180 degrees)
		playerAngle = 180.0f;  // Face backward
		newPlayerZ += stepSize;  // Move backward in the Z direction
		break;

	case 'h':  // Move left and face left (90 degrees)
		playerAngle = 90.0f;  // Face left
		newPlayerX -= stepSize;  // Move left in the X direction
		break;

	case 'k':  // Move right and face right (-90 degrees)
		playerAngle = -90.0f;  // Face right
		newPlayerX += stepSize;  // Move right in the X direction
		break;
	case 'z':
		isScaling = !isScaling;  // Toggle rotation
		break;
	case 'x':
		isRotatingChair = !isRotatingChair;  // Toggle rotation
		break;
	case 'c':
		isMovingBasket = !isMovingBasket;  // Toggle rotation
		break;
	case 'v':
		isColorChanging = !isColorChanging;  // Toggle rotation
		break;
	case 'b':
		isBallBoyJumping = !isBallBoyJumping;  // Toggle rotation
		break;
	case ' ':  // Space bar for forehand action
		isForehand = true;
		break;
	/*case 'r':
		if (currentGameState == GAME_LOST || currentGameState == GAME_WON) {
			// Reset game state
			currentGameState = PLAYING;
			score = 0;
			gameLost = false;
			gameWin = false;
			ballHit = false;
			ballHit2 = false;
			timer = 30;  // Reset timer
			glutPostRedisplay();
		}
		break;*/

	case GLUT_KEY_ESCAPE:
		exit(EXIT_SUCCESS);
	}

	if (newPlayerX >= courtMinX && newPlayerX <= courtMaxX && newPlayerZ >= courtMinZ && newPlayerZ <= courtMaxZ) {
		// If the new position is valid, update the player's position
		playerX = newPlayerX;
		playerZ = newPlayerZ;
	}

	// Update leg movement for walking effect
	if (legMovingForward) {
		legAngle += 5.0f;
		if (legAngle >= 20.0f) legMovingForward = false;
	}
	else {
		legAngle -= 5.0f;
		if (legAngle <= -20.0f) legMovingForward = true;
	}

	glutPostRedisplay();
}

void Special(int key, int x, int y) {
	float a = 1.7;

	switch (key) {
	case GLUT_KEY_UP:
		camera.rotateX(a);
		break;
	case GLUT_KEY_DOWN:
		camera.rotateX(-a);
		break;
	case GLUT_KEY_LEFT:
		camera.rotateY(a);
		break;
	case GLUT_KEY_RIGHT:
		camera.rotateY(-a);
		break;
	}

	glutPostRedisplay();
}

void Timer(int value) {
	updateBall();            // Update ball position
	glutPostRedisplay();      // Request a redraw
	glutTimerFunc(16, Timer, 0);  // Call Timer again after 16 ms (approx. 60 FPS)
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(640, 480);
    glutCreateWindow("Tennis Court");

    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(Special);
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f); // Sky blue background

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

    // Start the timer
    glutTimerFunc(0, Timer, 0);
	glutTimerFunc(1000, timerCallback, 0);
	glutTimerFunc(2000, updateWallColor, 0);  // Start the wall color update timer


    glutMainLoop();
    return 0;
}

