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

	light_ = node_->CreateComponent<Light>();
	light_->SetLightType(LIGHT_SPOT);
	light_->SetBrightness(0.4f);
	light_->SetColor(Color::WHITE);
	light_->SetCastShadows(true);
	light_->SetFov(30.0f);
}

void SecurityCamera::Update(float timeStep)
{
	yaw_ += (sweepingBack_ ? -1.0f : 1.0f) * 10.0f * timeStep;

	if (Abs(yaw_) >= 45.0f) {
		sweepingBack_ = !sweepingBack_;
	}

	node_->SetWorldRotation(startRotation_);
	node_->Rotate(Quaternion(pitch_, Vector3::LEFT));
	node_->Rotate(Quaternion(yaw_, Vector3::UP));

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

#if 0
	// Don't do a raycast for now, it is currently self-intersecting with the camera body.

	Ray ray(cameraPosition + Vector3(0.0f, 4.6f, 0.0f), difference);
	PODVector<RayQueryResult> result;
	RayOctreeQuery query(result, ray, RAY_TRIANGLE, DRAWABLE_GEOMETRY);

	Octree *octree = GetScene()->GetComponent<Octree>();
	octree->RaycastSingle(query);

	if (!result.Empty() && result[0].node_ != player && result[0].node_->GetParent() != player) {
		light_->SetColor(Color::WHITE);
		return false;
	}
#endif

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
