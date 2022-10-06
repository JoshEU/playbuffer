#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

// Set the size and resolution of the window
int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

// Helps remember overall state of game
struct GameState {
	float timer = 0;
	int spriteId = 0;
	int score = 0;
};

GameState gameState; // Can access GameState struct anywhere in the code

// Defines all different types of GameObjects in the game - starts at -1 null
enum GameObjectType {
	TYPE_NULL = -1,
	TYPE_AGENT8,	
};

void HandlePlayerControls();

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE) {
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::StartAudioLoop("music");
	Play::CreateGameObject(TYPE_AGENT8, { 115, 0 }, 50, "agent8");
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	gameState.timer += elapsedTime; // increments the timer by the number of seconds that have passed since the last frame (elapsed time - should be 60)
	Play::ClearDrawingBuffer(Play::cBlack); // Passes the screen colour as a parameter
	
	// Draws name of currently selected sprite
	Play::DrawDebugText({ DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 },
		Play::GetSpriteName(gameState.spriteId),
		Play::cWhite);

	Play::DrawSprite(gameState.spriteId, Play::GetMousePos(), gameState.timer); // Draws the sprite using its spriteId, passing through the mouse position and using the GameState timer to provide an index of the animation frame.

	if (Play::KeyPressed(VK_SPACE)) {
		gameState.spriteId++; // add one to the spriteId when spacebar is pressed
	}
	
	Play::DrawBackground();
	HandlePlayerControls();
	Play::PresentDrawingBuffer(); // Finally presents the drawing buffer to the player by copying it - uses "Painters Algorithm - clear drawing buffer and then draw background first and then objects on top - 
    return Play::KeyDown(VK_ESCAPE); // Uses the order back to front then present drawing buffer
}

void HandlePlayerControls() {
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8); // Creates a variable of type GameObject
	if (Play::KeyDown(VK_UP)) {
		obj_agent8.velocity = {0, -4}; // Accesses the GameObject from the function 'enum GameObjectType' and alters its velocity in the upwards direction when the UP Arrow Key is pressed.
		Play::SetSprite(obj_agent8, "agent8_climb", 0.25f); // Play climb animation
	}
	else if (Play::KeyDown(VK_DOWN)) {
		obj_agent8.acceleration = { 0, 1 }; // Alters the GameObjects acceleration in the downward direction when the DOWN arrow key is pressed.
		Play::SetSprite(obj_agent8, "agent8_fall", 0); // Play fall animation
	}
	else {
		Play::SetSprite(obj_agent8, "agent8_hang", 0.02f);
		obj_agent8.velocity *= 0.5f; // Multiplies 0.5 by the value stored in velcoity and assigns the result to velocity
		obj_agent8.acceleration = { 0, 0 }; 
	}
	Play::UpdateGameObject(obj_agent8); 
	
	// Prevents the GameObject from traveling out of the game display area.
	if (Play::IsLeavingDisplayArea(obj_agent8)) {
		obj_agent8.pos = obj_agent8.oldPos;
	}

	Play::DrawLine({ obj_agent8.pos.x, 0 }, obj_agent8.pos, Play::cWhite); // Draws a trail behin the GameObject - acts as a cobweb
	Play::DrawObjectRotated(obj_agent8); // Draws the GameObject to the Screen - allows the changing of opacity to be passed as second arguement 
}





// Gets called once when the player quits the game 
int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}

