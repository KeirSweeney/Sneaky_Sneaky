#include "SecurityCamera.h"

#include "Context.h"
#include "Node.h"
#include "Scene.h"
#include "NavigationMesh.h"
#include "Log.h"
#include "DebugRenderer.h"
#include "Profiler.h"
#include "RigidBody.h"
#include "Input.h"
#include "Camera.h"
#include "Graphics.h"
#include "Octree.h"
#include "UI.h"
#include "Light.h"
#include "Game.h"
#include "Material.h"
#include "StaticModel.h"
#include "ResourceCache.h"
#include "Renderer.h"
#include "CameraController.h"
#include "Log.h"

using namespace Urho3D;

const float SecurityCamera::VIEW_DISTANCE = 5.0f; //Distance the camera can see
const float SecurityCamera::VIEW_ANGLE = 90.0f; //How much the camera can see in terms of angle

SecurityCamera::SecurityCamera(Context *context) :
	InteractableComponent(context),
    hasSeenPlayer_(false),
	rotatingToPlayer_(false)
{
}

void SecurityCamera::RegisterObject(Context* context)
{
	context->RegisterFactory<SecurityCamera>("Logic");
    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void SecurityCamera::DelayedStart()
{
    rigidBody_ = node_->GetComponent<RigidBody>();

	CAMERA_START_ROT = node_->GetRotation().YawAngle();

	//Give the security camera an initial angular velocity (We want it to rotate)
}

void SecurityCamera::Update(float timeStep)
{
	Node *personNode = GetScene()->GetChild("Person", true);

	Vector3 securityCameraAngularVelocity = rigidBody_->GetAngularVelocity();//Store security camera angular velocity

	//Now check if it has or doesn't have angular velocity
	bool camersHasAngularVelocity = false;
//	if (securityCameraAngularVelocity.Data > 0) //Check if camera has angular velocity more than 0
	//	camersHasAngularVelocity = true; //If it does, then its rotating

	//DebugRenderer *debug = node_->GetScene()->GetComponent<DebugRenderer>();
	//debug->AddLine(cameraRotating, cameraRotating, Color::BLUE);

	node_->Rotate(Quaternion(0.0f, CAMERA_ROT_SPEED * timeStep, 0.0f),TS_PARENT);

	//If 'DetectPlayer' returns true camera angular attributes should be adjusted to look at the player
	
	/*bool playerDetected = DetectPlayer(personNode);

	if (playerDetected) {
		hasSeenPlayer_ = true;
		CameraRotateOnPlayer(timeStep, personNode);//Rotate camera to the node the player is standing on
	}
	else {
		ResetCamera(timeStep); //Reset camera when the player is no longer detected and reset its behaviour back to 'searching' (Pivoting left and right by flipping angular velocity)
		hasSeenPlayer_ = false;
	}*/

	Seeking(timeStep);
}

void SecurityCamera::Seeking(float timeStep) //In this function we want to give the camera an angular velocity, but we want to flip the angular velocity when it reaches an angle threshold (So it doesnt rotate 180 degrees)
{
	//When player runs out of range reset camera behaviour to just rotate left and right with a given angle
	//We want to reset camera behaviour when the player is no longer detected!

	
	float currCameraRotation = (node_->GetRotation().YawAngle());

	float diff = (CAMERA_START_ROT - currCameraRotation);

	bool right = true;
	

	if (diff <= -45.0f ) {
		CAMERA_ROT_SPEED *= -1.0f;
		right = false;
		
	}

	LOGERRORF("Camera difference: %f", diff);

	//^Camera rotates right and left until player detected.^

	node_->Rotate(Quaternion(0.0f, CAMERA_ROT_SPEED * timeStep, 0.0f), TS_PARENT);

}

bool SecurityCamera::DetectPlayer(Node *player)
{
	Vector3 cameraPosition = node_->GetWorldPosition(); //Get security camera position
	Vector3 personPosition = player->GetWorldPosition();//Get player camera position


	Vector3 difference = (personPosition - cameraPosition); //Get vector difference between camera and player

	if (difference.LengthSquared() > (VIEW_DISTANCE * VIEW_DISTANCE)) //If difference is more than the distance, return false (Player isn't detected)
	{
		return false;
	}

	Vector3 forward = rigidBody_->GetLinearVelocity(); //this stores the direction the camera is looking

	forward.Normalize(); //normalize is used for when we use dot product.
	difference.Normalize();

	//DebugRenderer *debug = node_->GetScene()->GetComponent<DebugRenderer>();
	//debug->AddLine(guardPosition, guardPosition + forward, Color::BLUE);

	//Check to see if player is in camera FOV
	if (forward.DotProduct(difference) < Cos(VIEW_ANGLE / 2.0f)) { //At this point, the player is close enough to the camera, but we must check if it's in the FOV
		return false;
	}


	Ray ray(cameraPosition + Vector3(0.0f, 1.6f, 0.0f) + (forward * 0.25f), difference);
	PODVector<RayQueryResult> result; //stores all of the objects the ray collides with into a vector.
	RayOctreeQuery query(result, ray, RAY_TRIANGLE, difference.LengthSquared(), DRAWABLE_GEOMETRY);

	Octree *octree = GetScene()->GetComponent<Octree>();
	octree->RaycastSingle(query);

	if (!result.Empty() && result[0].node_ != player && result[0].node_->GetParent() != player) {
		return false;
	}
	return true;
}


void SecurityCamera::CameraRotateOnPlayer(float timeStep, Urho3D::Node *player)
{
	//Player is in reach and in the FOV of the camera, we must now rotate the camera to the player
	//Rotate camera to player and update camera angle according to player..
	//We dont need to run any checks in terms of distance and FOV here because this function wont be called if the player wasn't in FOV & distance.


	Vector3 cameraPosition = node_->GetWorldPosition(); //Get security camera position
	Vector3 personPosition = player->GetWorldPosition();//Get player camera position

	Vector3 difference = (personPosition - cameraPosition);


}
