#include "Game.h"
#include "Thrower.h"

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
#include "UI.h"
#include "Sprite.h"
#include "Stairs.h"
#include "Analytics.h"

#include <ctime>
#include <cstdio>

#ifdef _MSC_VER
#define snprintf sprintf_s
#endif

using namespace Urho3D;

DEFINE_APPLICATION_MAIN(Game)

Game::Game(Context *context):
    Application(context), crashHandler_(context),
    currentLevel_(0), levelTime_(0.0f), gameState_(GS_PLAYING),
    debugGeometry_(false), debugPhysics_(false), debugNavigation_(false), debugDepthTest_(true)
{
    // We need to call back from the components for level transitions,
    // this stores our instance for access in the global store.
    context->RegisterSubsystem(this);

    context->RegisterSubsystem(new Analytics(context));
}

void Game::Setup()
{
    engineParameters_["WindowTitle"] = "The Traitor";
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

    // ResourceCache handles loading files from disk.
    ResourceCache *cache = GetSubsystem<ResourceCache>();

    debugHud_ = engine_->CreateDebugHud();
    debugHud_->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
    debugHud_->SetUseRendererStats(true);

    scene_ = new Scene(context_);

    // We need to register our custom component classes before they can be used.
    CameraController::RegisterObject(context_);
    Door::RegisterObject(context_);
    Guard::RegisterObject(context_);
    Inventory::RegisterObject(context_);
    Person::RegisterObject(context_);
    Pickup::RegisterObject(context_);
    Terminal::RegisterObject(context_);
    Stairs::RegisterObject(context_);
    Thrower::RegisterObject(context_);

    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, NULL));

#if 1
    RenderPath *renderPath = viewport->GetRenderPath();

    renderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA3.xml"));

    renderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
    renderPath->SetShaderParameter("BloomHDRMix", Vector2(1.0f, 1.0f));
#endif

    Renderer *renderer = GetSubsystem<Renderer>();
    renderer->SetViewport(0, viewport);

    // Increase shadow quality to max.
    renderer->SetShadowMapSize(2048);
    renderer->SetShadowQuality(QUALITY_MAX);

    // We need HDR rendering due to the simple materials and number of lights.
    // Without this, there are numerous lighting artifacts.
    renderer->SetHDRRendering(true);

    Input *input = GetSubsystem<Input>();
    input->SetMouseVisible(true);
    input->SetMouseMode(MM_ABSOLUTE);

    LoadLevel();

    SubscribeToEvent(E_UPDATE, HANDLER(Game, HandleUpdate));
    SubscribeToEvent(E_POSTRENDERUPDATE, HANDLER(Game, HandlePostRenderUpdate));

    GetSubsystem<Analytics>()->SendLaunchEvent();
}

void Game::Stop()
{
    engine_->DumpResources(true);
}

void Game::LoadLevel()
{
    UI *ui = GetSubsystem<UI>();
    ResourceCache *cache = GetSubsystem<ResourceCache>();

    // If the player has completed the level (rather than dying), advance to the next.
    if (gameState_ == GS_COMPLETED) {
        currentLevel_++;
    }
    gameState_ = GS_PLAYING;

    // The floor layout is defined using a pixel image.
    // If we can't load the next level, try and go back to the first level.
    Image *levelImage = cache->GetResource<Image>(ToString("Levels/%d.png", currentLevel_ + 1));
    if (!levelImage) {
        if (currentLevel_ != 0) {
            currentLevel_ = 0;
            LoadLevel();
        }

        return;
    }

    // Reset the score counter.
    levelTime_ = 0.0f;

    // Remove all UI and scene elements,
    // they will be recreated for the level by the rest of this function.
    ui->GetRoot()->RemoveAllChildren();
    scene_->Clear();

    // Required components for 3D rendering.
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<DebugRenderer>();

    // The Zone component handles ambient lighting and fog.
    Zone *zone = scene_->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color::BLACK);
    zone->SetFogStart(50.0f);
    zone->SetFogEnd(200.0f);

    // The NavigationMesh is used for pathfinding.
    NavigationMesh *navigationMesh = scene_->CreateComponent<NavigationMesh>();
    navigationMesh->SetCellSize(0.2f);
    navigationMesh->SetAgentRadius(0.25f);
    navigationMesh->SetAgentHeight(1.8f - navigationMesh->GetCellHeight());
    navigationMesh->SetAgentMaxClimb(0.0f);
    navigationMesh->SetAgentMaxSlope(5.0f);

    // PhysicsWorld holds all physics objects (and can be used to setup things like gravity).
    scene_->CreateComponent<PhysicsWorld>();

    // ooooooooooo
    // oxoxoxoxoxo
    // ooooooooooo
    // oxoxoxoxoxo
    // ooooooooooo
    // The above 11x5 pixel "image" shows which pixels would affect the floor.
    // Loop through the image pixels used to define the floor tiles.
    int imageWidth = levelImage->GetWidth();
    int imageHeight = levelImage->GetHeight();
    int width = (imageWidth - 1) / 2;
    int height = (imageHeight - 1) / 2;
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            // If the pixel for this tile is transparent (less than 50% alpha), skip over it.
            if (levelImage->GetPixel(1 + (x * 2), (imageHeight - 1) - (1 + (y * 2))).a_ < 0.5f) {
                continue;
            }

            // Create a node and place it at the correct world coordinates.
            // Floor tiles are 11m x 11m.
            Node *floorNode = scene_->CreateChild();
            floorNode->SetPosition(Vector3(x * 11.0f, 0.0f, y * 11.0f));

            floorNode->CreateComponent<Navigable>()->SetRecursive(false);
            RigidBody * floorRigidBody = floorNode->CreateComponent<RigidBody>();
            floorRigidBody->SetRestitution(0.5f);

            CollisionShape *floorCollisionShape = floorNode->CreateComponent<CollisionShape>();
            floorCollisionShape->SetBox(Vector3(11.0f, 1.0f, 11.0f), Vector3(0.0f, -0.5f, 0.0f));

            StaticModel *floorModel = floorNode->CreateComponent<StaticModel>();
            floorModel->SetModel(cache->GetResource<Model>("Models/Floor.mdl"));
            floorModel->SetMaterial(cache->GetResource<Material>("Materials/FlatGrey.xml"));

            // Create 4 evenly spaced lights for each room.
            // Each light has a brightness of 20% to provide full lighting (when combiend with the 20% camera light).
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

            // Label the room with its number for development.
            Node *roomLabelNode = floorNode->CreateChild();
            roomLabelNode->SetPosition(Vector3(0.0f, 0.1f, 0.0f));
            roomLabelNode->SetRotation(Quaternion(90.0f, Vector3::RIGHT));
            roomLabelNode->SetScale(3.0f);

            Text3D *roomLabel = roomLabelNode->CreateComponent<Text3D>();
            roomLabel->SetText(ToString("%dx%d", x + 1, y + 1));
            roomLabel->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.sdf"));
            roomLabel->SetColor(Color::WHITE);
            roomLabel->SetAlignment(HA_CENTER, VA_CENTER);
            roomLabel->SetFaceCameraMode(FC_ROTATE_Y);

            // Try and load the XML file that defines the contents for the room.
            XMLFile *roomContents = cache->GetResource<XMLFile>(ToString("Levels/%d/%dx%d.xml", currentLevel_ + 1, x + 1, y + 1));
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

                    // If the object specifies a component for interaction, create it, else mark it for navmesh generation.
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
                        // The guard waypoints need to be converted to world space.
                        waypoints.Push(roomContentsNode->GetWorldPosition() + waypoint.GetVector3("position"));
                        waypoint = waypoint.GetNext("waypoint");
                    }

                    if (waypoints.Empty()) {
                        continue;
                    }

                    Node *guardNode = roomContentsNode->CreateChild("Guard");
                    guardNode->SetWorldPosition(waypoints[0]);
                    guardNode->Scale(Vector3(0.6f, 1.8f, 1.0f));

                    StaticModel *guardModel = guardNode->CreateComponent<StaticModel>();
                    guardModel->SetModel(cache->GetResource<Model>("Models/PersonFlat.mdl"));
                    guardModel->SetMaterial(cache->GetResource<Material>("Materials/Person.xml"));
                    guardModel->SetCastShadows(true);

                    RigidBody *guardRigidBody = guardNode->CreateComponent<RigidBody>();
                    guardRigidBody->SetMass(100.0f);
                    guardRigidBody->SetFriction(0.0f);
                    guardRigidBody->SetAngularFactor(Vector3::ZERO);

                    CollisionShape *guardCollisionShape = guardNode->CreateComponent<CollisionShape>();
                    guardCollisionShape->SetCylinder(1.0f, 1.0f, Vector3(0.0f, 0.5f, 0.0f));

                    Node *guardLightNode = guardNode->CreateChild("SearchLight");
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

    // oxoxoxoxoxo
    // xoxoxoxoxox
    // oxoxoxoxoxo
    // xoxoxoxoxox
    // oxoxoxoxoxo
    // The above 11x5 pixel "image" shows which pixels would affect the walls.
    // Loop through the image pixels used to define the wall types.
    for (int x = 0; x < imageWidth; ++x) {
        for (int y = 0; y < imageHeight; ++y) {
            // (x % 2) returns a boolean indicated whether x is even or odd.
            // If the x and y coordinates are both even, or the x and y coordinates are both odd, this isn't a pixel we're interested in.
            if ((x % 2) == (y % 2)) {
                continue;
            }

            // Get the color of the image pixel. The y coordinate needs to be flipped.
            Color color = levelImage->GetPixel(x, (imageHeight - 1) - y);

            // If it's transparent, pass on it.
            if (color.a_ < 0.5f) {
                continue;
            }

            Node *wallNode = scene_->CreateChild();

            // Walls have to be offset by half the room size.
            Vector3 position(-5.5f + (x * 5.5f), 0.0f, -5.5f + (y * 5.5f));

            // If the wall is vertical, we need to rotate the model.
            if ((y % 2) != 0) {
                wallNode->SetRotation(Quaternion(90.0f, Vector3::UP));
            }

            wallNode->SetPosition(position);

            wallNode->CreateComponent<Navigable>()->SetRecursive(false);
            wallNode->CreateComponent<RigidBody>();

            // Select which model to load based on the pixel color.
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

    Node *personNode = scene_->CreateChild("Person");
    personNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    personNode->Scale(Vector3(0.6f, 1.8f, 1.0f));

    StaticModel *personModel = personNode->CreateComponent<StaticModel>();
    personModel->SetModel(cache->GetResource<Model>("Models/PersonFlat.mdl"));
    personModel->SetMaterial(cache->GetResource<Material>("Materials/MaverickFront.xml"));
    personModel->SetCastShadows(true);

    Node *personShadowNode = personNode->CreateChild("ShadowCaster");
    personShadowNode->SetRotation(Quaternion(90.0f, Vector3::UP));

    StaticModel *personShadowModel = personShadowNode->CreateComponent<StaticModel>();
    personShadowModel->SetModel(cache->GetResource<Model>("Models/PersonFlat.mdl"));
    personShadowModel->SetMaterial(cache->GetResource<Material>("Materials/MaverickLeftShadow.xml"));
    personShadowModel->SetCastShadows(true);

    RigidBody *personRigidBody = personNode->CreateComponent<RigidBody>();
    personRigidBody->SetMass(100.0f);
    personRigidBody->SetFriction(0.0f);
    personRigidBody->SetAngularFactor(Vector3::ZERO);

    CollisionShape *personCollisionShape = personNode->CreateComponent<CollisionShape>();
    personCollisionShape->SetCylinder(1.0f, 1.0f, Vector3(0.0f, 0.5f, 0.0f));

    personNode->CreateComponent<Person>();
    personNode->CreateComponent<Inventory>();
    personNode->CreateComponent<Thrower>();

    Node *cameraTargetNode = scene_->CreateChild();
    cameraTargetNode->CreateComponent<CameraController>();

    Node *cameraNode = cameraTargetNode->CreateChild("Camera");
    cameraNode->SetPosition(Vector3(0.0f, 14.0f, -8.0f));
    cameraNode->SetRotation(Quaternion(60.0f, Vector3::RIGHT));

    Camera *camera = cameraNode->CreateComponent<Camera>();
    camera->SetFarClip(zone->GetFogEnd());

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

    Renderer *renderer = GetSubsystem<Renderer>();
    renderer->GetViewport(0)->SetCamera(camera);
}

void Game::EndLevel(bool died)
{
    PODVector<Node *> nodes;
    scene_->GetChildrenWithComponent<StaticModel>(nodes, true);

    // Loop through all scene nodes with a StaticModel component.
    for (PODVector<Node *>::ConstIterator i = nodes.Begin(); i != nodes.End(); ++i) {
        Node *node = *i;

        // Loop through all its components and disable all of them apart from the StaticModel.
        Vector<SharedPtr<Component>> components = node->GetComponents();
        for (Vector<SharedPtr<Component>>::ConstIterator ii = components.Begin(); ii != components.End(); ++ii) {
            Component *component = *ii;

            if (component->GetType() == "StaticModel") {
                continue;
            }

            component->SetEnabled(false);
        }
    }

    UI *ui = GetSubsystem<UI>();

    UIElement *panel = ui->GetRoot()->CreateChild<UIElement>();
    panel->SetFixedSize(panel->GetParent()->GetSize());
    panel->SetAlignment(HA_CENTER, VA_CENTER);

    Sprite *background = panel->CreateChild<Sprite>();
    background->SetFixedSize(panel->GetSize());
    background->SetColor(Color::BLACK);
    background->SetOpacity(0.6f);

    Text *label = panel->CreateChild<Text>();
    label->SetFont("Fonts/Anonymous Pro.ttf");
    label->SetColor(Color::WHITE);
    label->SetAlignment(HA_CENTER, VA_CENTER);

    PODVector<Node *> guards;
    scene_->GetChildrenWithComponent<Guard>(guards, true);

    int guardCount = 0;
    for (PODVector<Node *>::ConstIterator i = guards.Begin(); i != guards.End(); ++i) {
        if ((*i)->GetComponent<Guard>()->HasSeenPlayer()) {
            guardCount++;
        }
    }

    Node *person = scene_->GetChild("Person", true);
    int pickupCount = person->GetComponent<Inventory>()->GetItemCount();

    char buffer[512];
    if (died) {
        GetSubsystem<Analytics>()->SendLevelFailedEvent(currentLevel_, levelTime_, guardCount, pickupCount, person->GetWorldPosition());

        snprintf(buffer, sizeof(buffer),
                 "You've been brutally murdered for your crimes against the company.\n"
                 "\n"
                 "Press [space] to try your hand again.");
    } else {
        int m = (int)(levelTime_ / 60.0f);
        int s = (int)levelTime_ - (m * 60);
        int ms = (int)(levelTime_ * 100.0f) - (s * 100);

        int score = (int)(((300.0f - levelTime_) + (guardCount * -100.0f) + (pickupCount * 50.0f)) * 5.0f);

        GetSubsystem<Analytics>()->SendLevelCompletedEvent(currentLevel_, levelTime_, guardCount, pickupCount, score);

        snprintf(buffer, sizeof(buffer),
                 "Against all odds, you made it past security and on to the next floor.\n"
                 "\n"
                 "Press [space] to continue the adventure.\n"
                 "\n"
                 "\n"
                 "           Completion Time: %02d:%02d.%03d\n"
                 "            Seen By Guards: %d time%s\n"
                 "         Pickups Collected: %d\n"
                 "      ---------------------------------\n"
                 "                     Score: %d",
                 m, s, ms, guardCount, (guardCount != 1) ? "s" : "", pickupCount, score);
    }

    label->SetText(buffer);

    gameState_ = died ? GS_DEAD : GS_COMPLETED;
}

void Game::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
    (void)eventType;

    Input *input = GetSubsystem<Input>();

    float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

    if (gameState_ == GS_PLAYING) {
        levelTime_ += timeStep;
    } else if (input->GetKeyPress(KEY_SPACE)) {
        LoadLevel();
    }

#if 1
    if (input->GetKeyPress(KEY_0)) {
        debugDepthTest_ = !debugDepthTest_;
    }

    if (input->GetKeyPress(KEY_1)) {
        debugGeometry_ = !debugGeometry_;
    }

    if (input->GetKeyPress(KEY_2)) {
        debugPhysics_ = !debugPhysics_;
    }

    if (input->GetKeyPress(KEY_3)) {
        debugNavigation_ = !debugNavigation_;
    }

    if (input->GetKeyPress(KEY_D)) {
        debugHud_->SetMode(debugHud_->GetMode() == DEBUGHUD_SHOW_NONE ? DEBUGHUD_SHOW_ALL : DEBUGHUD_SHOW_NONE);
    }

    if (gameState_ == GS_PLAYING && input->GetKeyPress(KEY_R)) {
        GetSubsystem<ResourceCache>()->ReleaseAllResources(true);
        LoadLevel();
    }

    if (gameState_ == GS_PLAYING && input->GetKeyPress(KEY_N)) {
        EndLevel(false);
    }

    if (input->GetKeyPress(KEY_C)) {
        // CRASH EVERYTHING!
        volatile int *lol = NULL;
        *lol = 0xDEADBEEF;
    }

    Renderer *renderer = GetSubsystem<Renderer>();
    RenderPath *renderPath = renderer->GetDefaultRenderPath();

    // The post-processing effects don't play nicely with the debug rendering.
    bool debugRendering = debugGeometry_ || debugPhysics_ || debugNavigation_;
    renderPath->SetEnabled("FXAA3", !debugRendering);
    renderPath->SetEnabled("BloomHDR", !debugRendering);
#endif

#if 0
    static float colorTimer = 0.0f;

    if (colorTimer >= 0.2f) {
        colorTimer = 0.0f;

        PODVector<Node *> nodes;
        scene_->GetChildrenWithComponent<Light>(nodes, true);
        for (PODVector<Node *>::ConstIterator i = nodes.Begin(); i != nodes.End(); ++i) {
            Color c;
            c.FromHSV(Random(1.0f), 1.0f, 1.0f);
            Light *l = (*i)->GetComponent<Light>();
            l->SetColor(c);
            l->SetCastShadows((Rand() % 2) == 0);
        }
    }

    colorTimer += timeStep;
#endif
}

void Game::HandlePostRenderUpdate(StringHash eventType, VariantMap &eventData)
{
    (void)eventType; (void)eventData;

    if (debugGeometry_) {
        GetSubsystem<Renderer>()->DrawDebugGeometry(debugDepthTest_);
    }

    if (debugPhysics_) {
        scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(debugDepthTest_);
    }

    if (debugNavigation_) {
        scene_->GetComponent<NavigationMesh>()->DrawDebugGeometry(debugDepthTest_);
    }
}
