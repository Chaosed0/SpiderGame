
#include "Scene.h"

#include <cstdio>
#include <cstdint>
#include <sstream>
#include <random>
#include <ctime>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Renderer/Camera.h"
#include "Util.h"

#include "Game/Components/ModelRenderComponent.h"
#include "Game/Components/CameraComponent.h"
#include "Game/Components/CollisionComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/PlayerComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/FollowComponent.h"
#include "Game/Components/HealthComponent.h"
#include "Game/Components/SpiderComponent.h"
#include "Game/Components/ExpiresComponent.h"
#include "Game/Components/HurtboxComponent.h"
#include "Game/Components/VelocityComponent.h"
#include "Game/Components/AudioListenerComponent.h"
#include "Game/Components/AudioSourceComponent.h"
#include "Game/Components/PointLightComponent.h"
#include "Game/Components/SpawnerComponent.h"
#include "Game/Components/LevelComponent.h"

#include "Game/Events/HealthChangedEvent.h"
#include "Game/Events/GemCountChangedEvent.h"
#include "Game/Events/BulletCountChangedEvent.h"
#include "Game/Events/ShotEvent.h"

#include "Game/Extra/PrefabConstructionInfo.h"

#include "Renderer/UI/Label.h"
#include "Renderer/UI/UIQuad.h"

const static int gemCount = 3;
const static int spiderCount = 6;

const static glm::vec3 tableDimensions(2.0f, 1.0f, 0.785f);
const static glm::vec3 bulletDimensions(0.2f, 0.07f, 0.2f);
const static glm::vec3 platformDimensions1(6.0f, 0.25f, 6.0f);
const static glm::vec3 platformDimensions2(5.0f, 0.25f, 5.0f);
const static glm::vec3 gemSlabDimensions(0.5f, 0.1f, 0.5f);

glm::vec3 roomBoxCenter(const RoomBox& box);

Scene::Scene(const SceneInfo& info)
	: world(*info.world),
	renderer(*info.renderer),
	uiRenderer(*info.uiRenderer),
	soundManager(*info.soundManager),
	eventManager(*info.eventManager),
	dynamicsWorld(info.dynamicsWorld),
	generator(*info.generator),
	windowWidth(info.windowWidth),
	windowHeight(info.windowHeight),
	prefabsSetup(false)
{
	Material defaultMaterial;
	defaultMaterial.setProperty("shininess", FLT_MAX);
	defaultMaterial.setProperty("diffuseTint", glm::vec3(1.0f));
	defaultMaterial.setProperty("texture_specular", textureLoader.loadFromFile(TextureType_specular, "assets/img/default_specular.png"));
	modelLoader.setDefaultMaterialProperties(defaultMaterial);
}

void Scene::setupPrefabs()
{
	if (prefabsSetup) {
		return;
	}

	/* Shaders */
	shader = shaderLoader.compileAndLink("Shaders/basic.vert", "Shaders/lightcolor.frag");
	skinnedShader = shaderLoader.compileAndLink("Shaders/skinned.vert", "Shaders/lightcolor.frag");
	singleColorShader = shaderLoader.compileAndLink("Shaders/basic.vert", "Shaders/singlecolor.frag");
	textShader = shaderLoader.compileAndLink("Shaders/basic2d.vert", "Shaders/text.frag");
	imageShader = shaderLoader.compileAndLink("Shaders/basic2d.vert", "Shaders/texture2d.frag");

	/* Health label */
	std::shared_ptr<Font> font = std::make_shared<Font>("assets/font/Inconsolata.otf", 50);
	gui.healthLabel = std::make_shared<Label>(font);
	gui.healthLabel->material.setProperty("textColor", MaterialProperty(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	gui.healthLabel->transform = Transform(glm::vec3(50.0f, windowHeight - 10.0f, 0.0f)).matrix();
	gui.healthLabelHandle = uiRenderer.getEntityHandle(gui.healthLabel, textShader);

	/* Health image */
	gui.healthImage = std::make_shared<UIQuad>(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/heart.png"), glm::vec2(32.0f, 32.0f));
	gui.healthImage->transform = Transform(glm::vec3(10.0f, windowHeight - 42.0f, 0.0f)).matrix();
	gui.healthImageHandle = uiRenderer.getEntityHandle(gui.healthImage, imageShader);

	/* Bullet label */
	gui.bulletLabel = std::make_shared<Label>(font);
	gui.bulletLabel->setAlignment(Label::Alignment_right);
	gui.bulletLabel->material.setProperty("textColor", MaterialProperty(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	gui.bulletLabel->transform = Transform(glm::vec3(windowWidth - 50.0f, windowHeight - 10.0f, 0.0f)).matrix();
	gui.bulletLabelHandle = uiRenderer.getEntityHandle(gui.bulletLabel, textShader);

	/* Bullet image */
	gui.bulletImage = std::make_shared<UIQuad>(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/bullet.png"), glm::vec2(32.0f, 32.0f));
	gui.bulletImage->transform = Transform(glm::vec3(windowWidth - 42.0f, windowHeight - 42.0f, 0.0f)).matrix();
	gui.bulletImageHandle = uiRenderer.getEntityHandle(gui.bulletImage, imageShader);

	/* Gem images */
	gui.redGemImage = std::make_shared<UIQuad>(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/gemred.png"), glm::vec2(32.0f, 32.0f));
	gui.redGemImage->transform = Transform(glm::vec3(windowWidth / 2.0f - 48.0f, windowHeight - 42.0f, 0.0f)).matrix();
	gui.redGemImage->isVisible = false;
	gui.redGemImageHandle = uiRenderer.getEntityHandle(gui.redGemImage, imageShader);

	gui.greenGemImage = std::make_shared<UIQuad>(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/gemgreen.png"), glm::vec2(32.0f, 32.0f));
	gui.greenGemImage->transform = Transform(glm::vec3(windowWidth / 2.0f - 16.0f, windowHeight - 42.0f, 0.0f)).matrix();
	gui.greenGemImage->isVisible = false;
	gui.greenGemImageHandle = uiRenderer.getEntityHandle(gui.greenGemImage, imageShader);

	gui.blueGemImage = std::make_shared<UIQuad>(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/gemblue.png"), glm::vec2(32.0f, 32.0f));
	gui.blueGemImage->transform = Transform(glm::vec3(windowWidth / 2.0f + 16.0f, windowHeight - 42.0f, 0.0f)).matrix();
	gui.blueGemImage->isVisible = false;
	gui.blueGemImageHandle = uiRenderer.getEntityHandle(gui.blueGemImage, imageShader);

	/* Notification label */
	font = std::make_shared<Font>("assets/font/Inconsolata.otf", 30);
	gui.facingLabel = std::make_shared<Label>(font);
	gui.facingLabel->material.setProperty("textColor", MaterialProperty(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	gui.facingLabel->transform = Transform(glm::vec3(windowWidth / 2.0f + 40.0f, windowHeight / 2.0f - 40.0f, 0.0f)).matrix();
	gui.facingLabelHandle = uiRenderer.getEntityHandle(gui.facingLabel, textShader);
	font.reset();

	/* Aiming reticle*/
	gui.reticleImage = std::make_shared<UIQuad>(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/reticle.png"), glm::vec2(32.0f, 32.0f));
	gui.reticleImage->transform = Transform(glm::vec3(windowWidth / 2.0f - 16.0f, windowHeight / 2.0f - 16.0f, 0.0f)).matrix();
	gui.reticleHandle = uiRenderer.getEntityHandle(gui.reticleImage, imageShader);

	/* Pedestal */
	Model pedestalModel = modelLoader.loadModelFromPath("assets/models/pedestal.fbx");
	Renderer::ModelHandle pedestalModelHandle = renderer.getModelHandle(pedestalModel);
	pedestalPrefab.addConstructor(new TransformConstructor());
	pedestalPrefab.addConstructor(new ModelRenderConstructor(renderer, pedestalModelHandle, shader));

	/* Barrel */
	Model barrelModel = modelLoader.loadModelFromPath("assets/models/barrel.fbx");
	barrelModel.material.setProperty("shininess", 16.0f);
	Renderer::ModelHandle barrelModelHandle = renderer.getModelHandle(barrelModel);

	btVector3 barrelHalfExtents(0.5f, 0.75f, 0.5f);
	btCollisionShape* barrelShape = new btBoxShape(barrelHalfExtents);
	btCompoundShape* barrelCompoundShape = new btCompoundShape();
	barrelCompoundShape->addChildShape(btTransform(btQuaternion::getIdentity(), btVector3(0.0f, barrelHalfExtents.y(), 0.0f)), barrelShape);
	btRigidBody::btRigidBodyConstructionInfo barrelBodyInfo(0.0f, new btDefaultMotionState(), barrelShape);

	barrelPrefab.addConstructor(new TransformConstructor());
	barrelPrefab.addConstructor(new CollisionConstructor(dynamicsWorld, barrelBodyInfo));
	barrelPrefab.addConstructor(new ModelRenderConstructor(renderer, barrelModelHandle, shader));

	/* Table */
	Model tableModel = modelLoader.loadModelFromPath("assets/models/table.fbx");
	Renderer::ModelHandle tableModelHandle = renderer.getModelHandle(tableModel);

	btCollisionShape* tableCollisionShape = new btBoxShape(Util::glmToBt(tableDimensions / 2.0f));
	btCompoundShape* tableCompoundShape = new btCompoundShape();
	tableCompoundShape->addChildShape(btTransform(btQuaternion::getIdentity(), btVector3(0.0f, tableDimensions.y / 2.0f, 0.0f)), tableCollisionShape);
	btRigidBody::btRigidBodyConstructionInfo tableInfo(0.0f, new btDefaultMotionState(), tableCompoundShape);

	tablePrefab.addConstructor(new TransformConstructor());
	tablePrefab.addConstructor(new CollisionConstructor(dynamicsWorld, tableInfo));
	tablePrefab.addConstructor(new ModelRenderConstructor(renderer, tableModelHandle, shader));

	/* Platform */
	Model platformModel = modelLoader.loadModelFromPath("assets/models/platform.fbx");
	Renderer::ModelHandle platformModelHandle = renderer.getModelHandle(platformModel);

	btCollisionShape* platformShape1 = new btBoxShape(Util::glmToBt(platformDimensions1 / 2.0f));
	btCollisionShape* platformShape2 = new btBoxShape(Util::glmToBt(platformDimensions2 / 2.0f));
	btCompoundShape* platformCompoundShape = new btCompoundShape();
	platformCompoundShape->addChildShape(btTransform(btQuaternion::getIdentity(), btVector3(0.0f, platformDimensions1.y / 2.0f, 0.0f)), platformShape1);
	platformCompoundShape->addChildShape(btTransform(btQuaternion::getIdentity(), btVector3(0.0f, platformDimensions1.y + platformDimensions2.y / 2.0f, 0.0f)), platformShape2);
	btRigidBody::btRigidBodyConstructionInfo platformBodyInfo(0.0f, new btDefaultMotionState(), platformCompoundShape);

	platformPrefab.addConstructor(new TransformConstructor());
	platformPrefab.addConstructor(new CollisionConstructor(dynamicsWorld, platformBodyInfo));
	platformPrefab.addConstructor(new ModelRenderConstructor(renderer, platformModelHandle, shader));

	/* Bullets */
	Model bulletModel = modelLoader.loadModelFromPath("assets/models/bullets.fbx");
	Renderer::ModelHandle bulletModelHandle = renderer.getModelHandle(bulletModel);

	btCollisionShape* bulletCollisionShape = new btBoxShape(Util::glmToBt(bulletDimensions / 2.0f));
	btCompoundShape* bulletCompoundShape = new btCompoundShape();
	bulletCompoundShape->addChildShape(btTransform(btQuaternion::getIdentity(), btVector3(0.0f, bulletDimensions.y / 2.0f, 0.0f)), bulletCollisionShape);
	btRigidBody::btRigidBodyConstructionInfo bulletInfo(0.0f, new btDefaultMotionState(), bulletCompoundShape);

	bulletPrefab.setName("Bullets");
	bulletPrefab.addConstructor(new TransformConstructor());
	bulletPrefab.addConstructor(new CollisionConstructor(dynamicsWorld, bulletInfo));
	bulletPrefab.addConstructor(new ModelRenderConstructor(renderer, bulletModelHandle, shader));

	/* Gems */
	std::vector<glm::vec3> gemColors = {
		glm::vec3(1.0f, 0.0f, 0.2f),
		glm::vec3(0.2f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.2f, 1.0f)
	};
	gemPrefabs.resize(gemColors.size());
	gemLightPrefabs.resize(gemColors.size());
	gemSlabPrefabs.resize(gemColors.size());

	Model gemModel = modelLoader.loadModelFromPath("assets/models/gem.fbx");
	gemModel.material.setProperty("shininess", 32.0f);

	btCollisionShape* gemCollisionShape = new btBoxShape(btVector3(0.1f, 0.1f, 0.05f));
	btRigidBody::btRigidBodyConstructionInfo gemBodyInfo(0.0f, new btDefaultMotionState(), gemCollisionShape);

	Model gemSlabModel = getBox({ textureLoader.loadFromFile(TextureType_diffuse, "assets/img/gem.png") }, gemSlabDimensions);

	btCollisionShape* gemSlabCollisionShape = new btBoxShape(Util::glmToBt(gemSlabDimensions / 2.0f));
	btRigidBody::btRigidBodyConstructionInfo gemSlabBodyInfo(0.0f, new btDefaultMotionState(), gemSlabCollisionShape);

	for (unsigned i = 0; i < gemColors.size(); i++) {
		glm::vec3 color = gemColors[i];

		PointLight light;
		light.constant = 1.0f;
		light.linear = 0.18f;
		light.quadratic = 0.064f;
		light.ambient = color * 0.1f;
		light.diffuse = color * 0.6f;
		light.specular = color * 1.0f;

		std::stringstream namestream;
		namestream << "Light " << i;

		gemLightPrefabs[i].setName(namestream.str());
		gemLightPrefabs[i].addConstructor(new TransformConstructor());
		gemLightPrefabs[i].addConstructor(new PointLightConstructor(renderer, light));

		gemModel.material.setProperty("diffuseTint", color);
		Renderer::ModelHandle gemModelHandle = renderer.getModelHandle(gemModel);

		namestream.str("");
		namestream << "Gem " << i;

		gemPrefabs[i].setName(namestream.str());
		gemPrefabs[i].addConstructor(new TransformConstructor());
		gemPrefabs[i].addConstructor(new ModelRenderConstructor(renderer, gemModelHandle, shader));
		gemPrefabs[i].addConstructor(new CollisionConstructor(dynamicsWorld, gemBodyInfo, CollisionGroupDefault, CollisionGroupAll, false));
		gemPrefabs[i].addConstructor(new VelocityConstructor(VelocityComponent::Data(1.0f, glm::vec3(0.0f, 1.0f, 0.0f))));

		gemSlabModel.material.setProperty("diffuseTint", color);
		Renderer::ModelHandle gemSlabModelHandle = renderer.getModelHandle(gemSlabModel);

		namestream.str("");
		namestream << "Slab " << i;

		gemSlabPrefabs[i].setName(namestream.str());
		gemSlabPrefabs[i].addConstructor(new TransformConstructor());
		gemSlabPrefabs[i].addConstructor(new ModelRenderConstructor(renderer, gemSlabModelHandle, shader));
		gemSlabPrefabs[i].addConstructor(new CollisionConstructor(dynamicsWorld, gemSlabBodyInfo));
	}

	/* Spider */
	std::vector<AudioClip> spiderSounds = {
		AudioClip("assets/sound/minecraft/spider/say1.ogg"),
		AudioClip("assets/sound/minecraft/spider/say2.ogg"),
		AudioClip("assets/sound/minecraft/spider/say3.ogg"),
		AudioClip("assets/sound/minecraft/spider/say4.ogg")
	};
	AudioClip spiderDeathSound("assets/sound/minecraft/spider/death.ogg");

	Model spiderModel = modelLoader.loadModelFromPath("assets/models/spider/spider-tex.fbx");
	auto spiderModelHandle = renderer.getModelHandle(spiderModel);

	glm::vec3 spiderHalfExtents = glm::vec3(125.0f, 75.0f, 120.0f) * 0.005f;
	btCapsuleShapeZ* spiderShape = new btCapsuleShapeZ(spiderHalfExtents.y, spiderHalfExtents.x);
	btCompoundShape* spiderCompoundShape = new btCompoundShape();
	float spiderMass = 40.0f;
	btVector3 spiderInertia;
	spiderCompoundShape->calculateLocalInertia(spiderMass, spiderInertia);
	spiderCompoundShape->addChildShape(btTransform(btQuaternion::getIdentity(), btVector3(0.0f, spiderHalfExtents.y, 0.0f)), spiderShape);
	btRigidBody::btRigidBodyConstructionInfo spiderBodyInfo(spiderMass, new btDefaultMotionState(), spiderCompoundShape, spiderInertia);

	FollowComponent::Data followData;
	followData.repathTime = 3.0f;
	followData.raycastStartOffset = glm::vec3(0.0f, spiderHalfExtents.y, 0.0f);

	SpiderComponent::Data spiderData;
	spiderData.normalMoveSpeed = 3.0f;
	spiderData.attackTime = 0.5f;
	spiderData.sounds = spiderSounds;
	spiderData.deathSound = spiderDeathSound;
	spiderData.leapMoveSpeed = 7.5f;

	spiderPrefab.setName("Spider");
	spiderPrefab.addConstructor(new TransformConstructor(Transform(glm::vec3(0.0f), glm::quat(), glm::vec3(0.005f))));
	spiderPrefab.addConstructor(new ModelRenderConstructor(renderer, spiderModelHandle, skinnedShader));
	spiderPrefab.addConstructor(new CollisionConstructor(dynamicsWorld, spiderBodyInfo));
	spiderPrefab.addConstructor(new FollowConstructor(followData));
	spiderPrefab.addConstructor(new RigidbodyMotorConstructor(RigidbodyMotorComponent::Data(spiderData.normalMoveSpeed, 3.5f)));
	spiderPrefab.addConstructor(new HealthConstructor(HealthComponent::Data(100)));
	spiderPrefab.addConstructor(new AudioSourceConstructor(soundManager));
	spiderPrefab.addConstructor(new SpiderConstructor(spiderData));

	/* Bullet tracer */
	Vertex fromVert;
	fromVert.position = glm::vec3(0.0f);
	Vertex toVert;
	toVert.position = Util::forward * 3.0f;
	Mesh lineMesh(std::vector<Vertex>{fromVert, toVert}, std::vector<unsigned>{0,1});
	Material material;
	material.drawType = MaterialDrawType_Lines;
	material.setProperty("color", MaterialProperty(glm::vec4(0.5f, 0.5f, 0.0f, 1.0f)));
	auto bulletMeshHandle = renderer.getModelHandle(Model(lineMesh, material));

	bulletTracer.setName("BulletTracer");
	bulletTracer.addConstructor(new TransformConstructor());
	bulletTracer.addConstructor(new ModelRenderConstructor(renderer, bulletMeshHandle, singleColorShader));
	bulletTracer.addConstructor(new ExpiresConstructor(ExpiresComponent::Data(0.2f)));
	bulletTracer.addConstructor(new VelocityConstructor(75.0f));

	/* Muzzle flash */
	Texture muzzleFlashTexture(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/flash.png"));
	Model muzzleFlashPlane = getPlane(std::vector<Texture> { muzzleFlashTexture }, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.3f, 0.3f));
	auto muzzleFlashModelHandle = renderer.getModelHandle(muzzleFlashPlane);

	muzzleFlash.setName("MuzzleFlash");
	muzzleFlash.addConstructor(new TransformConstructor());
	muzzleFlash.addConstructor(new ModelRenderConstructor(renderer, muzzleFlashModelHandle, shader));
	muzzleFlash.addConstructor(new ExpiresConstructor(ExpiresComponent::Data(0.05f)));

	/* Spider spawner */
	SpawnerComponent::Data spawnerData;
	spawnerData.prefab = spiderPrefab;

	spiderSpawnerPrefab.setName("Spawner");
	spiderSpawnerPrefab.addConstructor(new SpawnerConstructor(spawnerData));

	/* Gun */
	Model gunModel = modelLoader.loadModelFromPath("assets/models/gun.fbx");
	auto gunModelHandle = renderer.getModelHandle(gunModel);

	playerGunPrefab.setName("PlayerGun");
	playerGunPrefab.addConstructor(new TransformConstructor(Transform(glm::vec3(0.2f, -0.3f, -0.15f))));
	playerGunPrefab.addConstructor(new ModelRenderConstructor(renderer, gunModelHandle, shader, "AnimStack::Gun|Idle"));

	/* Camera */
	cameraPrefab.setName("Camera");
	cameraPrefab.addConstructor(new TransformConstructor(glm::vec3(0.0f, 0.85f, 0.0f)));
	cameraPrefab.addConstructor(new CameraConstructor(Camera(glm::radians(90.0f), (unsigned)windowWidth, (unsigned)windowHeight, 0.1f, 1000000.0f)));

	/* Player's light */
	PointLight playerLight;
	playerLight.constant = 1.0f;
	playerLight.linear = 0.3f;
	playerLight.quadratic = 0.7f;
	playerLight.ambient = glm::vec3(0.2f);
	playerLight.diffuse = glm::vec3(0.6f);
	playerLight.specular = glm::vec3(1.0f);

	playerLightPrefab.setName("Light");
	playerLightPrefab.addConstructor(new TransformConstructor());
	playerLightPrefab.addConstructor(new PointLightConstructor(renderer, playerLight));

	/* Player */
	btCapsuleShape* shape = new btCapsuleShape(0.5f, 0.7f);
	float playerMass = 70.0f;
	btVector3 playerInertia;
	shape->calculateLocalInertia(playerMass, playerInertia);
	btRigidBody::btRigidBodyConstructionInfo playerInfo(playerMass, new btDefaultMotionState(), shape, playerInertia);

	PlayerComponent::Data playerData;
	playerData.shotCooldown = 0.3f;
	playerData.shotDamage = 100;
	playerData.facingLabel = gui.facingLabel;

	playerData.shotClip = AudioClip("assets/sound/shot.wav");
	playerData.dryFireClip = AudioClip("assets/sound/dryfire.wav");
	playerData.hurtClip = AudioClip("assets/sound/minecraft/classic_hurt.ogg");
	playerData.gemPickupClip = AudioClip("assets/sound/pickup.wav");
	playerData.reloadClip = AudioClip("assets/sound/reload.wav");
	playerData.gunBarrelOffset = glm::vec3(0.0f, 0.19f, -0.665f);

	playerData.shotTracerPrefab = bulletTracer;
	playerData.muzzleFlashPrefab = muzzleFlash;
	playerData.facingLabel = gui.facingLabel;

	playerData.gemPrefabs = gemPrefabs;
	playerData.gemLightPrefabs = gemLightPrefabs;

	playerPrefab.setName("Player");
	playerPrefab.addConstructor(new TransformConstructor());
	playerPrefab.addConstructor(new CollisionConstructor(dynamicsWorld, playerInfo, CollisionGroupPlayer));
	playerPrefab.addConstructor(new PlayerConstructor(playerData));
	playerPrefab.addConstructor(new RigidbodyMotorConstructor(RigidbodyMotorComponent::Data(5.0f, 4.0f)));
	playerPrefab.addConstructor(new HealthConstructor(HealthComponent::Data(100)));
	playerPrefab.addConstructor(new AudioListenerConstructor(AudioListenerComponent::Data()));
	playerPrefab.addConstructor(new AudioSourceConstructor(soundManager));

	damageEventResponder = std::make_unique<DamageEventResponder>(world, eventManager);
	playerJumpResponder = std::make_shared<PlayerJumpResponder>(world, eventManager);
	hurtboxPlayerResponder = std::make_shared<HurtboxPlayerResponder>(world, eventManager);

	std::function<void(const ShotEvent& event)> shotCallback =
		[world = &world, soundManager = &soundManager](const ShotEvent& event) {
			PlayerComponent* playerComponent = world->getComponent<PlayerComponent>(event.source);
			AudioSourceComponent* audioSourceComponent = world->getComponent<AudioSourceComponent>(event.source);
			AudioClip clip;
			if (event.actuallyShot) {
				clip = playerComponent->data.shotClip;
			} else {
				clip = playerComponent->data.dryFireClip;
			}
			soundManager->playClipAtSource(clip, audioSourceComponent->sourceHandle);
		};
	eventManager.registerForEvent<ShotEvent>(shotCallback);

	std::function<void(const HealthChangedEvent& event)> healthChangedCallback =
		[world = &world, soundManager = &soundManager, healthLabel = gui.healthLabel](const HealthChangedEvent& event) {
			PlayerComponent* playerComponent = world->getComponent<PlayerComponent>(event.entity);

			if (playerComponent == nullptr) {
				return;
			}

			std::stringstream sstream;
			sstream << event.newHealth;
			healthLabel->setText(sstream.str());

			if (event.healthChange < 0) {
				AudioSourceComponent* audioSourceComponent = world->getComponent<AudioSourceComponent>(event.entity);
				soundManager->playClipAtSource(playerComponent->data.hurtClip, audioSourceComponent->sourceHandle);
			}

			if (event.newHealth <= 0) {
				CollisionComponent* collisionComponent = world->getComponent<CollisionComponent>(event.entity);
				RigidbodyMotorComponent* motorComponent = world->getComponent<RigidbodyMotorComponent>(event.entity);
				btRigidBody* playerBody = (btRigidBody*)collisionComponent->collisionObject;
				playerBody->setAngularFactor(btVector3(1.0f, 1.0f, 1.0f));
				playerComponent->isDead = true;
				motorComponent->canMove = false;
			}
		};
	eventManager.registerForEvent<HealthChangedEvent>(healthChangedCallback);

	std::function<void(const GemCountChangedEvent& event)> gemCountChangedCallback =
		[world = &world, soundManager = &soundManager, gui = &gui](const GemCountChangedEvent& event) {
			PlayerComponent* playerComponent = world->getComponent<PlayerComponent>(event.source);
			AudioSourceComponent* audioSourceComponent = world->getComponent<AudioSourceComponent>(event.source);

			for (unsigned i = 0; i < playerComponent->gemStates.size(); i++) {
				std::shared_ptr<UIQuad> gemImage(nullptr);
				if (i == GemColor_Red) {
					gemImage = gui->redGemImage;
				} else if (i == GemColor_Green) {
					gemImage = gui->greenGemImage;
				} else if (i == GemColor_Blue) {
					gemImage = gui->blueGemImage;
				} else {
					continue;
				}

				gemImage->isVisible = playerComponent->gemStates[i] == GemState_PickedUp;
			}

			soundManager->playClipAtSource(playerComponent->data.gemPickupClip, audioSourceComponent->sourceHandle);
		};
	eventManager.registerForEvent<GemCountChangedEvent>(gemCountChangedCallback);

	std::function<void(const BulletCountChangedEvent& event)> bulletCountChangedCallback =
		[world = &world, soundManager = &soundManager, bulletLabel = gui.bulletLabel](const BulletCountChangedEvent& event) {
			PlayerComponent* playerComponent = world->getComponent<PlayerComponent>(event.source);
			AudioSourceComponent* audioSourceComponent = world->getComponent<AudioSourceComponent>(event.source);

			std::stringstream sstream;
			sstream << event.newBulletsInGun << "/" << event.newBulletCount;
			bulletLabel->setText(sstream.str());
		};
	eventManager.registerForEvent<BulletCountChangedEvent>(bulletCountChangedCallback);

	std::function<void(const ReloadStartEvent& event)> reloadStartCallback =
		[world = &world, soundManager = &soundManager](const ReloadStartEvent& event) {
			PlayerComponent* playerComponent = world->getComponent<PlayerComponent>(event.source);
			AudioSourceComponent* audioSourceComponent = world->getComponent<AudioSourceComponent>(event.source);

			soundManager->playClipAtSource(playerComponent->data.reloadClip, audioSourceComponent->sourceHandle);
		};
	eventManager.registerForEvent<ReloadStartEvent>(reloadStartCallback);

	prefabsSetup = true;
}

void Scene::setup()
{
	setupPrefabs();

	/* Scene */
	DirLight dirLight;
	dirLight.direction = glm::vec3(0.2f, -1.0f, 0.3f);
	dirLight.ambient = glm::vec3(0.2f);
	dirLight.diffuse = glm::vec3(0.6f);
	dirLight.specular = glm::vec3(1.0f);
	renderer.setDirLight(dirLight);

	/* Dungeon */
	std::uniform_int_distribution<int> seedRand(INT_MIN, INT_MAX);
	roomGenerator = RoomGenerator(seedRand(generator));
	Room room = roomGenerator.generate();
	const unsigned roomHeight = 6;

	roomData.room = room;
	roomData.meshBuilder.reset();
	roomData.meshBuilder.addRoom(room, (float)roomHeight);
	roomData.meshBuilder.construct();

	// Add the room to collision
	btCollisionShape* roomShape = roomData.meshBuilder.getCollisionMesh();
	btRigidBody::btRigidBodyConstructionInfo roomConstructionInfo(0.0f, new btDefaultMotionState(), roomShape);

	// Render the room
	// TODO: This actually introduces a bit of a leak where we add this to VRAM every time the scene loads
	Texture roomTexture(textureLoader.loadFromFile(TextureType_diffuse, "assets/img/brick.png"));
	Model roomModel = roomData.meshBuilder.getModel(std::vector<Texture>{ roomTexture });
	roomModel.material.setProperty("shininess", MaterialProperty(FLT_MAX));
	Renderer::ModelHandle roomModelHandle = renderer.getModelHandle(roomModel);

	Prefab roomPrefab("Level");
	roomPrefab.addConstructor(new CollisionConstructor(dynamicsWorld, roomConstructionInfo, CollisionGroupWall, CollisionGroupAll));
	roomPrefab.addConstructor(new ModelRenderConstructor(renderer, roomModelHandle, shader));
	roomPrefab.addConstructor(new LevelConstructor(LevelComponent::Data(roomData.room)));
	eid_t roomEntity = world.constructPrefab(roomPrefab);

	const RoomBox& centerRoomBox = room.boxes[0];
	const RoomBox& topmostRoomBox = room.boxes[room.topmostBox];
	const RoomBox& bottommostRoomBox = room.boxes[room.bottommostBox];
	const RoomBox& rightmostRoomBox = room.boxes[room.rightmostBox];
	const RoomBox& leftmostRoomBox = room.boxes[room.leftmostBox];

	std::vector<glm::vec3> gemFloorPositions = {
		roomBoxCenter(leftmostRoomBox),
		roomBoxCenter(rightmostRoomBox),
		roomBoxCenter(bottommostRoomBox)
	};

	// Put a light in the center room and the rooms that are farthest out
	for (unsigned i = 0; i < gemFloorPositions.size(); i++) {
		glm::vec3 floorPosition = gemFloorPositions[i];

		glm::vec3 lightPosition = floorPosition + glm::vec3(0.0f, roomHeight / 2.0f, 0.0f);
		PrefabConstructionInfo lightInfo = PrefabConstructionInfo(Transform(lightPosition));
		eid_t lightEntity = world.constructPrefab(gemLightPrefabs[i], World::NullEntity, &lightInfo);

		PrefabConstructionInfo pedestalInfo = PrefabConstructionInfo(Transform(floorPosition));
		eid_t pedestalEntity = world.constructPrefab(pedestalPrefab, World::NullEntity, &pedestalInfo);

		glm::vec3 gemPosition = floorPosition + glm::vec3(0.0f, 1.5f, 0.0f);
		PrefabConstructionInfo gemInfo = PrefabConstructionInfo(Transform(gemPosition));
		eid_t gemEntity = world.constructPrefab(gemPrefabs[i], World::NullEntity, &gemInfo);
	}

	// Create the platform in the center room with some gem slabs on it
	glm::vec3 centerBoxCenter = roomBoxCenter(centerRoomBox);
	PrefabConstructionInfo platformInfo = PrefabConstructionInfo(Transform(centerBoxCenter));
	eid_t platformEntity = world.constructPrefab(platformPrefab, World::NullEntity, &platformInfo);

	glm::vec3 gemSlabCenter = centerBoxCenter + glm::vec3(0.0f, platformDimensions1.y + platformDimensions2.y + gemSlabDimensions.y / 2.0f + 0.01f, 0.0f);
	std::vector<glm::vec3> gemSlabPositions = {
		gemSlabCenter + glm::vec3(-2.0f, 0.0f, 0.0f),
		gemSlabCenter + glm::vec3(0.0f, 0.0f, -2.0f),
		gemSlabCenter + glm::vec3(2.0f, 0.0f, 0.0f)
	};

	for (unsigned i = 0; i < gemSlabPositions.size(); i++) {
		glm::vec3 gemSlabPosition = gemSlabPositions[i];
		PrefabConstructionInfo slabInfo = PrefabConstructionInfo(Transform(gemSlabPosition));
		eid_t slabEntity = world.constructPrefab(gemSlabPrefabs[i], World::NullEntity, &slabInfo);
	}

	// Clutter
	std::uniform_real_distribution<float> barrelRand(0.0f, 1.0f);

	for (unsigned i = 0; i < room.sides.size(); i++) {
		RoomSide& side = room.sides[i];

		// Don't clutter the extreme rooms, we want to put other things there
		std::vector<RoomBox> extremeRooms = { leftmostRoomBox, rightmostRoomBox, topmostRoomBox, bottommostRoomBox };
		bool skip = false;
		for (unsigned i = 0; i < extremeRooms.size(); i++) {
			if (side.x1 <= extremeRooms[i].right && side.x0 >= extremeRooms[i].left &&
				side.y1 <= extremeRooms[i].top && side.y0 >= extremeRooms[i].bottom)
			{
				skip = true;
				break;
			}
		}

		if (skip) {
			continue;
		}

		bool horizontal = (side.y0 == side.y1);
		int min = horizontal ? side.x0 : side.y0;
		int max = horizontal ? side.x1 : side.y1;
		for (int i = min; i < max; ++i) {
			float rand = barrelRand(generator);
			int x = horizontal ? i : side.x0 + side.normal.x;
			int y = horizontal ? side.y0 + side.normal.y : i;
			if (rand < 0.1f) {
				PrefabConstructionInfo info(Transform(glm::vec3((float)x, 0.0f, (float)y)));
				eid_t entity = world.constructPrefab(barrelPrefab, World::NullEntity, &info);
			}
		}
	}

	// Put down tables with bullets
	std::vector<Transform> bulletSpawnLocations = {
		Transform(roomBoxCenter(topmostRoomBox)),
		Transform(glm::vec3(leftmostRoomBox.left + 2.0f, 0.0f, (leftmostRoomBox.bottom + leftmostRoomBox.top) / 2.0f), glm::angleAxis(glm::half_pi<float>(), Util::up)),
		Transform(glm::vec3((bottommostRoomBox.left + bottommostRoomBox.right) / 2.0f, 0.0f, bottommostRoomBox.bottom + 2.0f)),
		Transform(glm::vec3(rightmostRoomBox.right - 2.0f, 0.0f, (rightmostRoomBox.bottom + rightmostRoomBox.top) / 2.0f), glm::angleAxis(glm::half_pi<float>(), Util::up)),
	};

	std::uniform_real_distribution<float> angleRand(-glm::half_pi<float>(), glm::half_pi<float>());
	for (unsigned i = 0; i < bulletSpawnLocations.size(); i++) {
		Transform initialTransform = bulletSpawnLocations[i];

		// Table
		PrefabConstructionInfo tableInfo(initialTransform);
		eid_t table = world.constructPrefab(tablePrefab, World::NullEntity, &tableInfo);

		// Bullet
		glm::vec3 bulletPosition(initialTransform.getPosition() + glm::vec3(0.0f, tableDimensions.y + bulletDimensions.y / 2.0f, 0.0f));
		Transform bulletTransform(bulletPosition, glm::angleAxis(angleRand(generator), glm::vec3(0.0f, 1.0f, 0.0f)));
		PrefabConstructionInfo bulletInfo(bulletTransform);
		eid_t bullet = world.constructPrefab(bulletPrefab, World::NullEntity, &bulletInfo);
	}

	// Initialize the player
	glm::vec3 playerSpawn = glm::vec3((topmostRoomBox.left + topmostRoomBox.right) / 2.0f, 0.5f, topmostRoomBox.top - 1.0f);

	PrefabConstructionInfo playerInfo = PrefabConstructionInfo(Transform(playerSpawn));
	eid_t player = world.constructPrefab(playerPrefab, World::NullEntity, &playerInfo);
	eid_t camera = world.constructPrefab(cameraPrefab, player);
	eid_t playerLight = world.constructPrefab(playerLightPrefab, player);
	eid_t gun = world.constructPrefab(playerGunPrefab, camera);

	PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(player);
	playerComponent->data.gun = gun;
	playerComponent->data.camera = camera;

	btRigidBody* playerBody = (btRigidBody*)world.getComponent<CollisionComponent>(player)->collisionObject;
	playerBody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
	playerBody->setActivationState(DISABLE_DEACTIVATION);

	eid_t spawner = world.constructPrefab(spiderSpawnerPrefab);
	SpawnerComponent* spawnerComponent = world.getComponent<SpawnerComponent>(spawner);
	for (unsigned i = 0; i < roomData.room.boxes.size(); i++) {
		RoomBox& box = roomData.room.boxes[i];
		spawnerComponent->data.candidatePositions.push_back(roomBoxCenter(box) + glm::vec3(0.0f, 1.0f, 0.0f));
	}

	// Initialize the GUI
	HealthComponent* playerHealthComponent = world.getComponent<HealthComponent>(player);
	std::stringstream sstream;
	sstream << playerHealthComponent->data.health;
	gui.healthLabel->setText(sstream.str());

	sstream.str("");
	sstream << playerComponent->bulletsInGun << "/" << playerComponent->bulletCount;
	gui.bulletLabel->setText(sstream.str());
}

glm::vec3 roomBoxCenter(const RoomBox& box)
{
	return glm::vec3((box.left + box.right) / 2.0f, 0.0f, (box.bottom + box.top) / 2.0f);
}
