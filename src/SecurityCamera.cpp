#include "SecurityCamera.h"

#include "Guard.h"

#include "Context.h"
#include "DebugRenderer.h"
#include "Frustum.h"
#include "Light.h"
#include "Log.h"
#include "NavigationMesh.h"
#include "Node.h"
#include "Octree.h"
#include "ResourceCache.h"
#include "RigidBody.h"
#include "Scene.h"
#include "Texture2D.h"
#include "PhysicsWorld.h"

using namespace Urho3D;

SecurityCamera::SecurityCamera(Context *context) :
	InteractableComponent(context),
	sweepingBack_(false),
	pitch_(-55.0f), yaw_(0.0f)
{
}

void SecurityCamera::RegisterObject(Context* context)
{
	context->RegisterFactory<SecurityCamera>("Logic");
	COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void SecurityCamera::DelayedStart()
{
	ResourceCache *cache = GetSubsystem<ResourceCache>();

	startRotation_ = node_->GetWorldRotation();

	light_ = node_->CreateComponent<Light>();
	light_->SetLightType(LIGHT_SPOT);
	light_->SetShapeTexture(cache->GetResource<Texture2D>("Textures/SpotHard.png"));
	light_->SetBrightness(0.2f);
	light_->SetColor(Color::WHITE);
	light_->SetCastShadows(true);
	light_->SetFov(30.0f);
	light_->SetSpecularIntensity(10.0f);
}

void SecurityCamera::Update(float timeStep)
{
	yaw_ += (sweepingBack_ ? -1.0f : 1.0f) * 10.0f * timeStep;

	if (Abs(yaw_) >= 45.0f) {
		sweepingBack_ = !sweepingBack_;
	}

	node_->SetWorldRotation(startRotation_);
	node_->Rotate(Quaternion(yaw_, Vector3::UP));
	node_->Rotate(Quaternion(pitch_, Vector3::LEFT));

	Node *personNode = GetScene()->GetChild("Person", true);
	SearchForPlayer(personNode);
}

bool SecurityCamera::SearchForPlayer(Node* player)
{
	Vector3 cameraPosition = node_->GetWorldPosition();
	Vector3 cameraDirection = node_->GetWorldDirection();
	Vector3 playerPosition = player->GetWorldPosition();

#if 0
	Frustum frustrum;
	frustrum.Define(30.0f / 2.0f, 1.0f, 1.0f, M_MIN_NEARCLIP, 10.0f, Matrix3x4(cameraPosition, node_->GetWorldRotation(), 1.0f));

	DebugRenderer *debug = node_->GetScene()->GetComponent<DebugRenderer>();
	debug->AddFrustum(frustrum, Color::BLUE);
#endif

	Vector3 difference = (playerPosition - cameraPosition);
	difference.Normalize();

	if (cameraDirection.DotProduct(difference) < Cos(30.0f / 2.0f)) {
		light_->SetColor(Color::WHITE);
		return false;
	}

	Ray ray(cameraPosition + (cameraDirection * 0.5f), difference + Vector3(0.0f, 0.01f, 0.0f));

	PhysicsWorld *physicsWorld = GetScene()->GetComponent<PhysicsWorld>();

	PhysicsRaycastResult result;
	physicsWorld->RaycastSingle(result, ray, 10.0f);

	if (result.body_ && result.body_->GetNode() != player) {
		light_->SetColor(Color::WHITE);
		return false;
	}

	light_->SetColor(Color::RED);
	AlertGuards();

	return true;
}

void SecurityCamera::AlertGuards()
{
	NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();

	Node *personNode = GetScene()->GetChild("Person", true);
	Node *roomNode = node_->GetParent()->GetParent();

	PODVector<Node *> nodes;
	roomNode->GetChildrenWithComponent<Guard>(nodes, true);

	Vector3 pointPath = personNode->GetWorldPosition();

	for (PODVector<Node *>::ConstIterator i = nodes.Begin(); i != nodes.End(); ++i) {
		Node *guardNode = *i;

		Guard *guard = guardNode->GetComponent<Guard>();

		Urho3D::PODVector<Urho3D::Vector3> path;
		navMesh->FindPath(path, guardNode->GetWorldPosition(), pointPath);
		guard->SetPath(path);
	}
}
