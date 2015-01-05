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
#include "Texture2D.h"
#include "CameraController.h"
#include "Text3D.h"
#include "Font.h"
#include "StringUtils.h"
#include "Door.h"
#include "Terminal.h"
#include "Log.h"
#include "Pickup.h"
#include "Guard.h"
#include "Inventory.h"

#include <ctime>

using namespace Urho3D;

DEFINE_APPLICATION_MAIN(Game)

Game::Game(Context *context):
    Application(context)
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
    //debugHud->SetMode(DEBUGHUD_SHOW_ALL);

    scene_ = new Scene(context_);

    // Required components for 3D rendering.
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<DebugRenderer>();

    // The Zone component handles ambient lighting and fog.
    Zone *zone = scene_->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color::BLACK);
    zone->SetFogStart(50.0f);
    zone->SetFogEnd(200.0f);

    NavigationMesh *navigationMesh = scene_->CreateComponent<NavigationMesh>();
    navigationMesh->SetCellSize(0.2f);
    navigationMesh->SetAgentRadius(0.25f);
    navigationMesh->SetAgentHeight(1.8f - navigationMesh->GetCellHeight());
    navigationMesh->SetAgentMaxClimb(0.0f);
    navigationMesh->SetAgentMaxSlope(5.0f);

    PhysicsWorld *physicsWorld = scene_->CreateComponent<PhysicsWorld>();

    Terminal::RegisterObject(context_);
    Pickup::RegisterObject(context_);
    Guard::RegisterObject(context_);

    Image *levelImage = cache->GetResource<Image>("Levels/1.png");

    int imageWidth = levelImage->GetWidth();
    int imageHeight = levelImage->GetHeight();
    int width = (imageWidth - 1) / 2;
    int height = (imageHeight - 1) / 2;
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            if (levelImage->GetPixel(1 + (x * 2), (imageHeight - 1) - (1 + (y * 2))).a_ < 0.5f) {
                continue;
            }

            Node *floorNode = scene_->CreateChild();
            floorNode->SetPosition(Vector3(x * 11.0f, 0.0f, y * 11.0f));

            floorNode->CreateComponent<Navigable>()->SetRecursive(false);
            floorNode->CreateComponent<RigidBody>();

            CollisionShape *floorCollisionShape = floorNode->CreateComponent<CollisionShape>();
            floorCollisionShape->SetBox(Vector3(11.0f, 1.0f, 11.0f), Vector3(0.0f, -0.5f, 0.0f));

            StaticModel *floorModel = floorNode->CreateComponent<StaticModel>();
            floorModel->SetModel(cache->GetResource<Model>("Models/Floor.mdl"));
            floorModel->SetMaterial(cache->GetResource<Material>("Materials/FlatGrey.xml"));

            const float floorLightHeight = 3.0f;

            Node *floorLightNode = floorNode->CreateChild();
            floorLightNode->SetPosition(Vector3(-2.75f, floorLightHeight, -2.75f));
            floorLightNode->SetDirection(Vector3::DOWN);

            Light *floorLight = floorLightNode->CreateComponent<Light>();
            floorLight->SetLightType(LIGHT_POINT);
            floorLight->SetBrightness(0.2f);
            floorLight->SetColor(Color::WHITE);

            floorLightNode = floorLightNode->Clone();
            floorLightNode->SetPosition(Vector3(2.75f, floorLightHeight, -2.75f));

            floorLightNode = floorLightNode->Clone();
            floorLightNode->SetPosition(Vector3(-2.75f, floorLightHeight, 2.75f));

            floorLightNode = floorLightNode->Clone();
            floorLightNode->SetPosition(Vector3(2.75f, floorLightHeight, 2.75f));

            Node *roomLabelNode = floorNode->CreateChild();
            roomLabelNode->SetPosition(Vector3(0.0f, 0.1f, 0.0f));
            roomLabelNode->SetRotation(Quaternion(90.0f, Vector3::RIGHT));
            roomLabelNode->SetScale(3.0f);

            Text3D *roomLabel = roomLabelNode->CreateComponent<Text3D>();
            roomLabel->SetText(ToString("%dx%d", x + 1, y + 1));
            roomLabel->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.sdf"));
            roomLabel->SetColor(Color::WHITE);
            //roomLabel->SetTextEffect(TE_STROKE);
            //roomLabel->SetEffectColor(Color::BLACK);
            roomLabel->SetAlignment(HA_CENTER, VA_CENTER);
            roomLabel->SetFaceCameraMode(FC_ROTATE_Y);

            XMLFile *roomContents = cache->GetResource<XMLFile>(ToString("Levels/1/%dx%d.xml", x + 1, y + 1));
            if (roomContents) {
                Node *roomContentsNode = floorNode->CreateChild();

                XMLElement child = roomContents->GetRoot().GetChild("object");
                while (child) {
                    Node *childNode = roomContentsNode->CreateChild();
                    childNode->SetPosition(child.GetVector3("position"));
                    Vector3 angles = child.GetVector3("angles");
                    childNode->SetRotation(Quaternion(angles.x_, angles.y_, angles.z_));
                    if (child.HasAttribute("scale")) {
                        childNode->SetScale(child.GetFloat("scale"));
                    }

                    childNode->CreateComponent<RigidBody>();

                    StaticModel *model = childNode->CreateComponent<StaticModel>();
                    model->SetModel(cache->GetResource<Model>("Models/" + child.GetAttribute("model") + ".mdl"));
                    model->SetMaterial(cache->GetResource<Material>("Materials/" + child.GetAttribute("material") + ".xml"));
                    model->SetCastShadows(true);

                    CollisionShape *collisionShape = childNode->CreateComponent<CollisionShape>();
                    collisionShape->SetBox(model->GetBoundingBox().Size(), model->GetBoundingBox().Center());

                    if (child.HasAttribute("interaction")) {
                        childNode->CreateComponent(child.GetAttribute("interaction"));
                    } else {
                        childNode->CreateComponent<Navigable>();
                    }

                    child = child.GetNext("object");
                }

                child = roomContents->GetRoot().GetChild("guard");
                while (child) {
                    PODVector<Vector3> waypoints;
                    XMLElement waypoint = child.GetChild("waypoint");
                    while (waypoint) {
                        waypoints.Push(waypoint.GetVector3("position"));
                        waypoint = waypoint.GetNext("waypoint");
                    }

                    if (waypoints.Empty()) {
                        continue;
                    }

                    Node *guardNode = scene_->CreateChild("Guard");
                    guardNode->SetPosition(waypoints[0]);
                    guardNode->Scale(Vector3(1.0f, 1.8f, 1.0f));

                    StaticModel *guardModel = guardNode->CreateComponent<StaticModel>();
                    guardModel->SetModel(cache->GetResource<Model>("Models/Person.mdl"));
                    guardModel->SetMaterial(cache->GetResource<Material>("Materials/Person.xml"));
                    guardModel->SetCastShadows(true);

                    RigidBody *guardRigidBody = guardNode->CreateComponent<RigidBody>();
                    guardRigidBody->SetMass(100.0f);
                    guardRigidBody->SetFriction(0.0f);
                    guardRigidBody->SetAngularFactor(Vector3::ZERO);

                    CollisionShape *guardCollisionShape = guardNode->CreateComponent<CollisionShape>();
                    guardCollisionShape->SetCylinder(0.5f, 1.0f, Vector3(0.0f, 0.5f, 0.0f));

                    Node *guardLightNode = guardNode->CreateChild();
                    guardLightNode->SetPosition(Vector3(0.0f, 0.75f, 0.0f));
                    guardLightNode->SetRotation(Quaternion(30.0f, Vector3::RIGHT));

                    Light *guardLight = guardLightNode->CreateComponent<Light>();
                    guardLight->SetLightType(LIGHT_SPOT);
                    guardLight->SetBrightness(0.4f);
                    guardLight->SetColor(Color::WHITE);
                    guardLight->SetCastShadows(true);
                    guardLight->SetFov(90.0f);


                    Guard *guard = guardNode->CreateComponent<Guard>();
                    guard->SetWaypoints(waypoints);

                    child = child.GetNext("guard");
                }
            }
        }
    }

    for (int x = 0; x < imageWidth; ++x) {
        for (int y = 0; y < imageHeight; ++y) {
            if ((x % 2) == (y % 2)) {
                continue;
            }

            Color color = levelImage->GetPixel(x, (imageHeight - 1) - y);
            if (color.a_ < 0.5f) {
                continue;
            }

            Node *wallNode = scene_->CreateChild();

            Vector3 position(-5.5f + (x * 5.5f), 0.0f, -5.5f + (y * 5.5f));

            if ((y % 2) != 0) {
                wallNode->SetRotation(Quaternion(90.0f, Vector3::UP));
            }

            wallNode->SetPosition(position);

            wallNode->CreateComponent<Navigable>()->SetRecursive(false);
            wallNode->CreateComponent<RigidBody>();

            String wallType("");
            if (color == Color::WHITE) {
                wallType = "Filler";
            } else if (color == Color::RED || color == Color::GREEN) {
                wallType = "Door";
            }

            StaticModel *wallModel = wallNode->CreateComponent<StaticModel>();
            wallModel->SetModel(cache->GetResource<Model>("Models/Wall" + wallType + ".mdl"));
            wallModel->SetMaterial(cache->GetResource<Material>("Materials/FlatGrey.xml"));
            wallModel->SetCastShadows(true);

            CollisionShape *wallCollisionShape = wallNode->CreateComponent<CollisionShape>();
            if (wallType == "") {
                wallCollisionShape->SetBox(Vector3(11.0f, 3.0f, 1.0f), Vector3(0.0f, 1.5f, 0.0f));
            } else {
                wallCollisionShape->SetTriangleMesh(wallModel->GetModel());

                if (wallType == "Door") {
                    Door::RegisterObject(context_);

                    Node *doorNode = wallNode->CreateChild();

                    doorNode->CreateComponent<Navigable>();
                    doorNode->CreateComponent<RigidBody>();

                    StaticModel *doorModel = doorNode->CreateComponent<StaticModel>();
                    doorModel->SetModel(cache->GetResource<Model>("Models/Door.mdl"));
                    doorModel->SetMaterial(cache->GetResource<Material>("Materials/FlatGrey.xml"));
                    doorModel->SetCastShadows(true);

                    CollisionShape *doorCollisionShape = doorNode->CreateComponent<CollisionShape>();
                    doorCollisionShape->SetBox(Vector3(1.5f, 2.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f));

                    Node *doorTriggerNode = wallNode->CreateChild();

                    RigidBody *doorTriggerRigidBody = doorTriggerNode->CreateComponent<RigidBody>();
                    doorTriggerRigidBody->SetTrigger(true);

                    CollisionShape *doorTriggerCollisionShape = doorTriggerNode->CreateComponent<CollisionShape>();
                    doorTriggerCollisionShape->SetBox(Vector3(2.0f, 2.0f, 2.0f), Vector3(0.0f, 1.0f, 0.0f));

                    Door *door = doorNode->CreateComponent<Door>();
                    door->SetTriggerNode(doorTriggerNode);
                }
            }
        }
    }

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
    Inventory::RegisterObject(context_);

    Node *personNode = scene_->CreateChild("Person");
    personNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    personNode->Scale(Vector3(1.0f, 1.8f, 1.0f));

    StaticModel *personModel = personNode->CreateComponent<StaticModel>();
    personModel->SetModel(cache->GetResource<Model>("Models/Person.mdl"));
    personModel->SetMaterial(cache->GetResource<Material>("Materials/Person.xml"));
    personModel->SetCastShadows(true);

    RigidBody *personRigidBody = personNode->CreateComponent<RigidBody>();
    personRigidBody->SetMass(100.0f);
    personRigidBody->SetFriction(0.0f);
    personRigidBody->SetAngularFactor(Vector3::ZERO);

    CollisionShape *personCollisionShape = personNode->CreateComponent<CollisionShape>();
    personCollisionShape->SetCylinder(0.5f, 1.0f, Vector3(0.0f, 0.5f, 0.0f));

    personNode->CreateComponent<Person>();
    personNode->CreateComponent<Inventory>();
#endif

    CameraController::RegisterObject(context_);

    Node *cameraTargetNode = scene_->CreateChild();
    cameraTargetNode->CreateComponent<CameraController>();

    Node *cameraNode = cameraTargetNode->CreateChild("Camera");
    cameraNode->SetPosition(Vector3(0.0f, 14.0f, -8.0f));
    cameraNode->SetRotation(Quaternion(60.0f, Vector3::RIGHT));

    Camera *camera = cameraNode->CreateComponent<Camera>();
    camera->SetFarClip(zone->GetFogEnd());

#if 1
    Node *cameraLightNode = cameraNode->CreateChild();
    cameraLightNode->SetPosition(Vector3(0.0f, -2.0f, 0.0f));
    cameraLightNode->SetRotation(Quaternion(-5.0f, Vector3::RIGHT));

    Light *cameraLight = cameraLightNode->CreateComponent<Light>();
    cameraLight->SetLightType(LIGHT_SPOT);
    cameraLight->SetRange(200.0f);
    cameraLight->SetBrightness(0.2f);
    cameraLight->SetFov(60.0f);
    cameraLight->SetColor(Color::WHITE);
    cameraLight->SetCastShadows(true);
    cameraLight->SetShapeTexture(cache->GetResource<Texture2D>("Textures/White.png"));
#endif

    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, camera));

#if 1
    RenderPath *renderPath = viewport->GetRenderPath();

    renderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA3.xml"));

    renderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
    renderPath->SetShaderParameter("BloomHDRMix", Vector2(1.0f, 1.0f));
#endif

    Renderer *renderer = GetSubsystem<Renderer>();
    renderer->SetViewport(0, viewport);
    renderer->SetShadowMapSize(2048);
    renderer->SetShadowQuality(QUALITY_MAX);
    renderer->SetHDRRendering(true);

    Input *input = GetSubsystem<Input>();
    input->SetMouseVisible(true);
    input->SetMouseMode(MM_ABSOLUTE);

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
    (void)eventType; (void)eventData;
}

void Game::HandlePostRenderUpdate(StringHash eventType, VariantMap &eventData)
{
    (void)eventType; (void)eventData;

    scene_->GetComponent<NavigationMesh>()->DrawDebugGeometry(true);
    scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
    //GetSubsystem<Renderer>()->DrawDebugGeometry(true);
}
