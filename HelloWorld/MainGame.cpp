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
	TYPE_FAN,
	TYPE_TOOL,
	TYPE_COIN,
	TYPE_STAR,
	TYPE_LASER,
	TYPE_DESTROYED,
};

void HandlePlayerControls();
void UpdateFan();
void UpdateTools();

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE) {
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::StartAudioLoop("Data\\Audio\\music.mp3");
	Play::CreateGameObject(TYPE_AGENT8, { 115, 0 }, 50, "agent8");
	int id_fan = Play::CreateGameObject(TYPE_FAN, { 1140, 217 }, 0, "fan"); // Defines the fan so you can change its values below
	Play::GetGameObject(id_fan).velocity = { 0,3 };
	Play::GetGameObject(id_fan).animSpeed = 1.0f;
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	gameState.timer += elapsedTime; // increments the timer by the number of seconds that have passed since the last frame (elapsed time - should be 60)
	
	// Draws name of currently selected sprite
	Play::DrawDebugText({ DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 },
		Play::GetSpriteName(gameState.spriteId),
		Play::cWhite);

	Play::DrawSprite(gameState.spriteId, Play::GetMousePos(), gameState.timer); // Draws the sprite using its spriteId, passing through the mouse position and using the GameState timer to provide an index of the animation frame.

	if (Play::KeyPressed(VK_SPACE)) {
		gameState.spriteId++; // add one to the spriteId when spacebar is pressed
	}
	
	
	Play::DrawBackground();
	Play::DrawFontText("64px", "ARROW KEYS TO MOVE UP AND DOWN AND SPACE TO FIRE", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 700}, Play::CENTRE);
	Play::DrawFontText("132px", "SCORE: " + std::to_string(gameState.score), { DISPLAY_WIDTH - 150, DISPLAY_HEIGHT - 50 }, Play::CENTRE);
	UpdateFan();
	UpdateTools();
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
	Play::DrawObjectRotated(obj_agent8); // Draws the GameObject to the Screen and can rotate - allows the changing of opacity to be passed as second arguement 
}

void UpdateFan() {
	GameObject& obj_fan = Play::GetGameObjectByType(TYPE_FAN);
	// Rolls a 50 sided die 60 times per second.
	// if it lands on 50, a new GameObject is created next to the fan
	if (Play::RandomRoll(50) == 50) {
		int id = Play::CreateGameObject(TYPE_TOOL, obj_fan.pos, 50, "driver");
		GameObject& obj_tool = Play::GetGameObject(id);
		obj_tool.velocity = Point2f(-8, Play::RandomRollRange(-1, -1) * 6);

		// If a 2 sided die lands on 1 the tool object is set to the spanner sprite
		if (Play::RandomRoll(2) == 1) {
			Play::SetSprite(obj_tool, "spanner", 0);
			// properties for the tool
			obj_tool.radius = 100;
			obj_tool.velocity.x = -4;
			obj_tool.rotSpeed = 0.1f;
		}
		Play::PlayAudio("tool");
	}
	Play::UpdateGameObject(obj_fan);

	// Updates fans position to its previous position so it doesn't leave the display area - applies the inverted velocity on the y axis so it doesn't travel off screen
	if (Play::IsLeavingDisplayArea(obj_fan)) {
		obj_fan.pos = obj_fan.oldPos;
		obj_fan.velocity.y *= -1;
	}
	Play::DrawObject(obj_fan); // Draws object and cannot rotate
}

void UpdateTools() {
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);

	// Assigns each item in turn their unique id to the local 'id' variable
	// this means that 'id' can be used to retrieve a reference ot the corresponding GameObject meaning that each object can be updated in turn. 
	for (int id : vTools) {
		GameObject& obj_tool = Play::GetGameObject(id);

		if (Play::IsColliding(obj_tool, obj_agent8)) {
			Play::StartAudioLoop("music");
			Play::PlayAudio("die");

			obj_agent8.pos = { -100, -100 };
		}
		Play::UpdateGameObject(obj_tool);

		// Updates tools position to its previous position so it doesn't leave the display area - applies the inverted velocity on the x axis so it doesn't travel off screen
		if (Play::IsLeavingDisplayArea(obj_tool, Play::VERTICAL)) {
			obj_tool.pos = obj_tool.oldPos;
			obj_tool.velocity.y *= -1;
		}
		Play::DrawObjectRotated(obj_tool);

		if (!Play::IsVisible(obj_tool)) {
			Play::DestroyGameObject(id);
		}
	}
}

// Gets called once when the player quits the game 
int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}

