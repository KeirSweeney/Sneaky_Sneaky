#include "Game.h"

#include "Camera.h"
#include "CollisionShape.h"
#include "CoreEvents.h"
#include "DebugHud.h"
#include "DebugRenderer.h"
#include "Engine.h"
#include "Input.h"
#include "Light.h"
#include "Material.h"
#include "Model.h"
#include "Navigable.h"
#include "NavigationMesh.h"
#include "Octree.h"
#include "Person.h"
#include "PhysicsWorld.h"
#include "RenderPath.h"
#include "Renderer.h"
#include "ResourceCache.h"
#include "RigidBody.h"
#include "Scene.h"
#include "StaticModel.h"
#include "Viewport.h"
#include "XMLFile.h"
#include "Zone.h"

#include <ctime>

using namespace Urho3D;

DEFINE_APPLICATION_MAIN(Game)

Game::Game(Context *context):
    Application(context),
    yaw_(0.0f), pitch_(45.0f)
{
}

void Game::Setup()
{
    engineParameters_["WindowTitle"] = "SneakySneaky";
    engineParameters_["FullScreen"] = false;
    engineParameters_["Multisample"] = 8;
    engineParameters_["VSync"] = true;
    engineParameters_["TextureFilterMode"] = FILTER_ANISOTROPIC;
    engineParameters_["TextureAnisotropy"] = 16;

    // Override these because the defaults are horrible for cross-platform compat.
    engineParameters_["ResourcePaths"] = "data";
    engineParameters_["AutoloadPaths"] = "";
}

void Game::Start()
{
    // Seed the random number generator.
    SetRandomSeed((unsigned int)time(NULL));

    ResourceCache *cache = GetSubsystem<ResourceCache>();

    DebugHud *debugHud = engine_->CreateDebugHud();
    debugHud->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
    debugHud->SetUseRendererStats(true);
    debugHud->SetMode(DEBUGHUD_SHOW_STATS/* | DEBUGHUD_SHOW_PROFILER*/);

    scene_ = new Scene(context_);

    // Required components for 3D rendering.
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<DebugRenderer>();

    // The Zone component handles ambient lighting and fog.
    Zone *zone = scene_->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetFogStart(50.0f);
    zone->SetFogEnd(200.0f);

    // Global lighting.
    Node *lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.0f, -1.0f, 0.0f));
    Light *light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetPerVertex(true);

    lightNode = lightNode->Clone();
    lightNode->SetDirection(Vector3(1.0f, 0.5f, 1.0f));

    lightNode = lightNode->Clone();
    lightNode->SetDirection(Vector3(-1.0f, 0.5f, -1.0f));

    ////////////////////////////////////////////////////////////////////////////////

    NavigationMesh *navigationMesh = scene_->CreateComponent<NavigationMesh>();
    navigationMesh->SetCellSize(0.2f);
    navigationMesh->SetAgentRadius(0.25f);
    navigationMesh->SetAgentHeight(1.8f - navigationMesh->GetCellHeight());
    navigationMesh->SetAgentMaxClimb(0.0f);
    navigationMesh->SetAgentMaxSlope(5.0f);

    PhysicsWorld *physicsWorld = scene_->CreateComponent<PhysicsWorld>();

#if 1
    Node *floorNode = scene_->CreateChild();

    floorNode->CreateComponent<Navigable>();
    floorNode->CreateComponent<RigidBody>();

    CollisionShape *floorCollisionShape = floorNode->CreateComponent<CollisionShape>();
    floorCollisionShape->SetBox(Vector3(11.0f, 1.0f, 11.0f), Vector3(0.0f, -0.5f, 0.0f));

    StaticModel *floorModel = floorNode->CreateComponent<StaticModel>();
    floorModel->SetModel(cache->GetResource<Model>("Models/Floor.mdl"));
    floorModel->SetMaterial(cache->GetResource<Material>("Materials/FlatGrey.xml"));
    floorModel->SetOccluder(true);

    floorNode = floorNode->Clone();
    floorNode->SetPosition(Vector3(11.0f, 0.0f, 0.0f));

    floorNode = floorNode->Clone();
    floorNode->SetPosition(Vector3(11.0f, 0.0f, 11.0f));
#endif

#if 1
    Node *wallNode = scene_->CreateChild();
    wallNode->SetPosition(Vector3(0.0f, 0.0f, 5.5f));

    wallNode->CreateComponent<Navigable>();
    wallNode->CreateComponent<RigidBody>();

    CollisionShape *wallCollisionShape = wallNode->CreateComponent<CollisionShape>();
    wallCollisionShape->SetBox(Vector3(11.0f, 3.0f, 1.0f), Vector3(0.0f, 1.5f, 0.0f));

    StaticModel *wallModel = wallNode->CreateComponent<StaticModel>();
    wallModel->SetModel(cache->GetResource<Model>("Models/Wall.mdl"));
    wallModel->SetMaterial(cache->GetResource<Material>("Materials/FlatGrey.xml"));
    wallModel->SetOccluder(true);

    wallNode = wallNode->Clone();
    wallNode->SetPosition(Vector3(0.0f, 0.0f, -5.5f));

    wallNode = wallNode->Clone();
    wallNode->SetPosition(Vector3(11.0f, 0.0f, -5.5f));

    wallNode = wallNode->Clone();
    wallNode->SetPosition(Vector3(11.0f, 0.0f, 16.5f));

    wallNode = wallNode->Clone();
    wallNode->SetPosition(Vector3(-5.5f, 0.0f, 0.0f));
    wallNode->SetRotation(Quaternion(90.0f, Vector3::UP));

    wallNode = wallNode->Clone();
    wallNode->SetPosition(Vector3(16.5f, 0.0f, 0.0f));

    wallNode = wallNode->Clone();
    wallNode->SetPosition(Vector3(16.5f, 0.0f, 11.0f));

    wallNode = wallNode->Clone();
    wallNode->SetPosition(Vector3(5.5f, 0.0f, 11.0f));

    // Ideally these would recreate the whole thing, but this was less to type.
    wallNode = wallNode->Clone();
    wallNode->SetPosition(Vector3(5.5f, 0.0f, 0.0f));
    wallNode->GetComponent<StaticModel>()->SetModel(cache->GetResource<Model>("Models/WallFiller.mdl"));
    wallNode->GetComponent<CollisionShape>()->SetTriangleMesh(cache->GetResource<Model>("Models/WallFiller.mdl"));

    wallNode = wallNode->Clone();
    wallNode->SetPosition(Vector3(11.0f, 0.0f, 5.5f));
    wallNode->SetRotation(Quaternion::IDENTITY);
    wallNode->GetComponent<StaticModel>()->SetModel(cache->GetResource<Model>("Models/WallDoor.mdl"));
    wallNode->GetComponent<CollisionShape>()->SetTriangleMesh(cache->GetResource<Model>("Models/WallDoor.mdl"));
#endif

    // All the navigable gemoetry needs to have been added to the scene by this point.
    navigationMesh->Build();

#if 1
    // Quick example of how to put a texture on a plane.
    Node *signNode = scene_->CreateChild("Sign");
    signNode->SetPosition(Vector3(0.0f, 1.5f, 5.0f));
    signNode->SetRotation(Quaternion(-90.0f, Vector3::RIGHT));
    signNode->SetScale(1.0f);

    StaticModel *sign = signNode->CreateComponent<StaticModel>();
    sign->SetModel(cache->GetResource<Model>("Models/Sign.mdl"));
    sign->SetMaterial(cache->GetResource<Material>("Materials/PosterMap.xml"));
#endif

#if 1
    Person::RegisterObject(context_);

    Node *personNode = scene_->CreateChild("Person");
    personNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    personNode->Scale(Vector3(1.0f, 1.8f, 1.0f));

    StaticModel *personModel = personNode->CreateComponent<StaticModel>();
    personModel->SetModel(cache->GetResource<Model>("Models/Person.mdl"));
    personModel->SetMaterial(cache->GetResource<Material>("Materials/Person.xml"));

    RigidBody *personRigidBody = personNode->CreateComponent<RigidBody>();
    personRigidBody->SetMass(100.0f);
    personRigidBody->SetFriction(0.01f);
    personRigidBody->SetAngularFactor(Vector3::ZERO);
    personRigidBody->SetLinearDamping(0.75f);

    CollisionShape *personCollisionShape = personNode->CreateComponent<CollisionShape>();
    personCollisionShape->SetCylinder(0.5f, 1.0f, Vector3(0.0f, 0.5f, 0.0f));

    personNode->CreateComponent<Person>();

    // Using the template we've created, create 9 more people.
    for (int i = 1; i < 10; ++i) {
        personNode = personNode->Clone();
        personNode->SetPosition(navigationMesh->GetRandomPoint());
    }
#endif

    ////////////////////////////////////////////////////////////////////////////////

    Node *cameraNode = scene_->CreateChild("Camera");
    cameraNode->SetPosition(Vector3(0.0f, 15.0f, -15.0f));
    // Don't set rotation here, use the constructor way up top.

    camera_ = cameraNode->CreateComponent<Camera>();
    camera_->SetFarClip(zone->GetFogEnd());

    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, camera_));

#if 0
    RenderPath *renderPath = viewport->GetRenderPath();
    renderPath->Append(cache->GetResource<XMLFile>("PostProcess/Bloom.xml"));
    renderPath->SetShaderParameter("BloomMix", Vector2(1.0f, 1.0f));
    renderPath->SetEnabled("Bloom", true);
#endif

    Renderer *renderer = GetSubsystem<Renderer>();
    renderer->SetViewport(0, viewport);

    SubscribeToEvent(E_UPDATE, HANDLER(Game, HandleUpdate));

    // Uncomment this to show all the debug rendering.
    //SubscribeToEvent(E_POSTRENDERUPDATE, HANDLER(Game, HandlePostRenderUpdate));
}

void Game::Stop()
{
    engine_->DumpResources(true);
}

void Game::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
    (void)eventType;

    float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

    Input *input = GetSubsystem<Input>();

    // Movement speed as world units per second
    const float MOVE_SPEED = 10.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    Node *cameraNode = camera_->GetNode();

#if 1
    if (input->GetNumJoysticks() > 0) {
        JoystickState *state = input->GetJoystickByIndex(0);
        GetSubsystem<DebugHud>()->SetAppStats("Joystick", state->name_);

        yaw_ += state->GetAxisPosition(5) * MOUSE_SENSITIVITY * timeStep * 2000.0f;
        pitch_ -= state->GetAxisPosition(3) * MOUSE_SENSITIVITY * timeStep * 2000.0f;

        Vector3 movement(state->GetAxisPosition(0), -state->GetAxisPosition(2), -state->GetAxisPosition(1));
        cameraNode->Translate(movement * MOVE_SPEED * timeStep * 2.0f);
    } else {
        GetSubsystem<DebugHud>()->ResetAppStats("Joystick");
    }
#endif

    IntVector2 mouseMove = input->GetMouseMove();
    yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
    pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
    pitch_ = Clamp(pitch_, -90.0f, 90.0f);

    cameraNode->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

    if (input->GetKeyDown('W'))
        cameraNode->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('S'))
        cameraNode->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('A'))
        cameraNode->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('D'))
        cameraNode->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);

    GetSubsystem<DebugHud>()->SetAppStats("Position", cameraNode->GetPosition());
    GetSubsystem<DebugHud>()->SetAppStats("Rotation", Vector2(pitch_, cameraNode->GetRotation().YawAngle()));
}

void Game::HandlePostRenderUpdate(StringHash eventType, VariantMap &eventData)
{
    (void)eventType; (void)eventData;

    scene_->GetComponent<NavigationMesh>()->DrawDebugGeometry(true);
    scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
    GetSubsystem<Renderer>()->DrawDebugGeometry(true);
}
