#include "SecurityCamera.h"

#include "Context.h"
#include "Node.h"
#include "Scene.h"
#include "Log.h"
#include "Octree.h"
#include "DebugRenderer.h"
#include "Frustum.h"
#include "Guard.h"
#include "Light.h"
#include "RigidBody.h"
#include "NavigationMesh.h"

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
	startRotation_ = node_->GetWorldRotation();
	rigidBody_ = node_->GetComponent<RigidBody>();
	light_ = node_->CreateComponent<Light>();
	light_->SetLightType(LIGHT_SPOT);
}

void SecurityCamera::Update(float timeStep)
{
	Node *personNode = GetScene()->GetChild("Person", true);
	yaw_ += (sweepingBack_ ? -1.0f : 1.0f) * 20.0f * timeStep;

	if (Abs(yaw_) >= 45.0f) {
		sweepingBack_ = !sweepingBack_;
	}

	node_->SetWorldRotation(startRotation_);
	node_->Rotate(Quaternion(pitch_, Vector3::LEFT));
	node_->Rotate(Quaternion(yaw_, Vector3::UP));

	Vector3 velocity = rigidBody_->GetLinearVelocity();
	Quaternion rotation = Quaternion(Vector3::FORWARD, velocity);

	SearchForPlayer(personNode);
}

bool SecurityCamera::SearchForPlayer(Node* player)
{
	Vector3 cameraPosition = node_->GetWorldPosition();
	Vector3 cameraDirection = node_->GetWorldDirection();
	Vector3 playerPosition = player->GetWorldPosition();
	Vector3 difference = (playerPosition - cameraPosition);

	difference.Normalize();

	Frustum frustrum;
	frustrum.Define(30.0f / 2.0f, 1.0f, 1.0f, M_MIN_NEARCLIP, 10.0f, Matrix3x4(cameraPosition, node_->GetWorldRotation(), 1.0f));

	DebugRenderer *debug = node_->GetScene()->GetComponent<DebugRenderer>();
	debug->AddFrustum(frustrum, Color::BLUE);

	if (cameraDirection.DotProduct(difference) < Cos(30.0f / 2.0f))
	{
		Node *roomNode = node_->GetParent()->GetParent();

		PODVector<Node *> nodes;

		roomNode->GetChildrenWithComponent<Light>(nodes, true);

		for (PODVector<Node *>::ConstIterator i = nodes.Begin(); i != nodes.End(); ++i) {
			Node *lightNode = *i;

			Light *light = lightNode->GetComponent<Light>();

			light->SetColor(Color::WHITE);
		}
		return false;
	}

	Ray ray(cameraPosition + Vector3(0.0f, 4.6f, 0.0f), difference);
	PODVector<RayQueryResult> result;
	RayOctreeQuery query(result, ray, RAY_TRIANGLE, DRAWABLE_GEOMETRY);

	Octree *octree = GetScene()->GetComponent<Octree>();
	octree->RaycastSingle(query);


	AlertGuards();

	return true;
}

void SecurityCamera::AlertGuards()
{
	Node *personNode = GetScene()->GetChild("Person", true);

	Node *roomNode = node_->GetParent()->GetParent();
	PODVector<Node *> nodes;

	roomNode->GetChildrenWithComponent<Light>(nodes, true);

	for (PODVector<Node *>::ConstIterator i = nodes.Begin(); i != nodes.End(); ++i) {
		Node *lightNode = *i;

		Light *light = lightNode->GetComponent<Light>();

		light->SetColor(Color::RED);
	}

	NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();

	Vector3 pointPath = personNode->GetWorldPosition();

	roomNode->GetChildrenWithComponent<Guard>(nodes, true);

	for (PODVector<Node *>::ConstIterator i = nodes.Begin(); i != nodes.End(); ++i) {
		Node *guardNode = *i;

		Guard *guard = guardNode->GetComponent<Guard>();

		navMesh->FindPath(path_, guardNode->GetWorldPosition(), pointPath);
		guard->SetPath(path_);
	}
}
