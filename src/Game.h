#pragma once

#include "Urho3D/Engine/Application.h"

#include "CrashHandler.h"

namespace Urho3D {
	class DebugHud;
	class Scene;
	class Camera;
	class Node;
}

class Game: public Urho3D::Application
{
	OBJECT(Game)

public:
	Game(Urho3D::Context *context);

	void Setup();
	void Start();
	void Stop();

public:
	void LoadLevel();
	void EndLevel(bool died, bool suppressScore);

	bool IsDeveloper();
	int GetCurrentJoystick();

private:
	void HandleControllerInputStart(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
	void HandleControllerInputEnd(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
	void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
	void HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

private:
	CrashHandler crashHandler_;

	Urho3D::SharedPtr<Urho3D::DebugHud> debugHud_;
	Urho3D::SharedPtr<Urho3D::Scene> scene_;
	unsigned screenJoystickIndex_;
	int joystickIndex_;
	
	int currentLevel_;
	float levelTime_;
	enum GameState {
		GS_MENU,
		GS_INTRO,
		GS_PLAYING,
		GS_DEAD,
		GS_STAIRS,
		GS_CREDITS,
		GS_FINISHED,
	} gameState_;

	float totalTime_;
	int totalScore_;

	int exitTimer_;

	bool unceUnceUnceWubWubWub_;
	Urho3D::PODVector<int> cheatCode_;

	bool developerMode_;
	bool debugGeometry_;
	bool debugPhysics_;
	bool debugNavigation_;
	bool debugDepthTest_;
};
