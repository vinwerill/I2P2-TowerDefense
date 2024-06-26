#include <allegro5/color.h>
#include <allegro5/allegro_primitives.h>
#include <cmath>
#include <utility>

#include "Engine/IScene.hpp"
#include "Enemy/Enemy.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/IObject.hpp"
#include "Engine/IScene.hpp"
#include "Scene/PlayScene.hpp"
#include "Scene/NormalPlayScene.hpp"
#include "Scene/SurvivalPlayScene.hpp"
#include "Scene/BlackPlayScene.hpp"
#include "Engine/Point.hpp"
#include "Turret.hpp"
#include "Engine/Collider.hpp"
#include "UI/Component/ImageButton.hpp"

PlayScene* Turret::getPlayScene() {
	return dynamic_cast<PlayScene*>(Engine::GameEngine::GetInstance().GetActiveScene());
}

Turret::Turret(std::string imgBase, std::string imgTurret, float x, float y, float radius, int price, float coolDown, Turret_Type _type) :
	Sprite(imgTurret, x, y), price(price), coolDown(coolDown), imgBase(imgBase, x, y), type(_type)

{
	CollisionRadius = radius;
	Freeze=0;
	Berserker=0;	
	mouseIn=0;
	MenuVisible=0;
	init = 10;
	FrostUpdate = 0;
	AbletocastSnowball=0;
	Frostbtn=nullptr;
	Rangebtn=nullptr;
	Reloadbtn=nullptr;
}

Turret::~Turret()
{
	PlayScene* scene = getPlayScene();
	if (type!=TURRET) return ;
	if (Frostbtn)
	{
		scene->UIGroup->RemoveControlObject(Frostbtn->GetControlIterator(), Frostbtn->GetObjectIterator());
		Frostbtn=nullptr;
	}
	if (Rangebtn)
	{
		scene->UIGroup->RemoveControlObject(Rangebtn->GetControlIterator(), Rangebtn->GetObjectIterator());
		Rangebtn=nullptr;
	}
	if (Reloadbtn)
	{
		scene->UIGroup->RemoveControlObject(Reloadbtn->GetControlIterator(), Reloadbtn->GetObjectIterator());
		Reloadbtn=nullptr;
	}
}

void Turret::Update(float deltaTime) {
	Sprite::Update(deltaTime);
	PlayScene* scene = getPlayScene();
	if (Freeze) return;
	imgBase.Position = Position;
	imgBase.Tint = Tint;
	if (!Enabled)
		return;
	if (Target) {
		if (init-->0) return ;
		Engine::Point diff = Target->Position - Position;
		if (diff.Magnitude() > CollisionRadius) {
			Target->lockedTurrets.erase(lockedTurretIterator);
			Target = nullptr;
			lockedTurretIterator = std::list<Turret*>::iterator();
		}
	}
	if (!Target) {
		// Lock first seen target.
		// Can be improved by Spatial Hash, Quad Tree, ...
		// However simply loop through all enemies is enough for this program.
		if (init-->0) return ;
		for (auto& it : scene->EnemyGroup->GetObjects()) {
			Engine::Point diff = it->Position - Position;
			if (diff.Magnitude() <= CollisionRadius) {
				Target = dynamic_cast<Enemy*>(it);
				Target->lockedTurrets.push_back(this);
				lockedTurretIterator = std::prev(Target->lockedTurrets.end());
				break;
			}
		}
	}

	// make the turret points to the target enemy
	// if (Target) {
	// 	Engine::Point originRotation = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
	// 	Engine::Point targetRotation = (Target->Position - Position).Normalize();
	// 	float maxRotateRadian = rotateRadian * deltaTime;
	// 	float cosTheta = originRotation.Dot(targetRotation);
	// 	// Might have floating-point precision error.
	// 	if (cosTheta > 1) cosTheta = 1;
	// 	else if (cosTheta < -1) cosTheta = -1;
	// 	float radian = acos(cosTheta);
	// 	Engine::Point rotation;
	// 	if (abs(radian) <= maxRotateRadian)
	// 		rotation = targetRotation;
	// 	else
	// 		rotation = ((abs(radian) - maxRotateRadian) * originRotation + maxRotateRadian * targetRotation) / radian;
	// 	// Add 90 degrees (PI/2 radian), since we assume the image is oriented upward.
	// 	Rotation = atan2(rotation.y, rotation.x) + ALLEGRO_PI / 2;
	// 	// Shoot reload.
	// 	reload -= deltaTime;
	// 	if (reload <= 0) {
	// 		// shoot.
	// 		reload = coolDown;
	// 		CreateBullet();
	// 	}
	// }

	// make the turret points to the target enemy with leading
	if (Target) {
		if (init-->0) return ;
		Engine::Point originRotation = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
		
		float estimatedTime = (Target->Position - this->Position).Magnitude() / bullet_speed;
		Engine::Point predictedPosistion = Target->Position + estimatedTime * Target->Velocity;

		Engine::Point targetRotation = (predictedPosistion - this->Position).Normalize();
		float maxRotateRadian = rotateRadian * deltaTime;
		float cosTheta = originRotation.Dot(targetRotation);
		// Might have floating-point precision error.
		if (cosTheta > 1) cosTheta = 1;
		else if (cosTheta < -1) cosTheta = -1;
		float radian = acos(cosTheta);
		Engine::Point rotation;
		if (abs(radian) <= maxRotateRadian)
			rotation = targetRotation;
		else
			rotation = ((abs(radian) - maxRotateRadian) * originRotation + maxRotateRadian * targetRotation) / radian;
		// Add 90 degrees (PI/2 radian), since we assume the image is oriented upward.
		Rotation = atan2(rotation.y, rotation.x) + ALLEGRO_PI / 2;
		// Shoot reload.
		reload -= deltaTime+(Berserker==1)*deltaTime;
		if (reload <= 0) {
			// shoot.
			reload = coolDown;
			CreateBullet();
		}
	}
	if (MenuVisible)
	{
		if (AbletocastSnowball)
		{
			if (scene->GetMoney()>=25) Frostbtn->Enabled=1;
			else Frostbtn->Enabled=0;
		}
		if (scene->GetMoney()>=25) Reloadbtn->Enabled=1;
		else Reloadbtn->Enabled=0;
		if (scene->GetMoney()>=25) Rangebtn->Enabled=1;
		else Rangebtn->Enabled=0;
	}
}
void Turret::Draw() const {
	if (Preview) {
		if (type==TURRET) al_draw_filled_circle(Position.x, Position.y, CollisionRadius, al_map_rgba(0, 255, 0, 50));
		else if (type==POTION) al_draw_filled_circle(Position.x, Position.y, CollisionRadius, al_map_rgba(0, 255, 0, 50));
	}
	imgBase.Draw();
	Sprite::Draw();
	if (PlayScene::DebugMode) {
		// Draw target radius.
		al_draw_circle(Position.x, Position.y, CollisionRadius, al_map_rgb(0, 0, 255), 2);
	}
	if (type==POTION && Enabled) al_draw_filled_circle(Position.x, Position.y, CollisionRadius, al_map_rgba(255, 0, 0, 50));
}
int Turret::GetPrice() const {
	return price;
}

Turret_Type Turret::GetType() const{
	return type;
}

void Turret::OnMouseMove(int mx, int my)
{
	if (Engine::GameEngine::GetInstance().GetSceneName(getPlayScene())=="play-reverse") return;
	PlayScene* scene = getPlayScene();
	int BlockSize=scene->BlockSize;
	Engine::Point diff = Position - Engine::Point(mx, my);
	mouseIn = diff.Magnitude()<15;
}

void Turret::OnMouseDown(int button, int mx, int my) {
	if (Engine::GameEngine::GetInstance().GetSceneName(getPlayScene())=="play-reverse") return;
	if (type!=TURRET) return ;
	PlayScene* scene = dynamic_cast<PlayScene*>(getPlayScene());
	if (scene->preview) return ;
	if ((button == 1) && mouseIn) {
		if (!MenuVisible)
		{
			ShowMenu();
			MenuVisible=1;
		}
		else 
		{
			DestroyMenu();
			MenuVisible=0;
		}
	}
	else if (!mouseIn && MenuVisible && !(Reloadbtn->GetMouseIn() || Rangebtn->GetMouseIn()))
	{
		DestroyMenu();
		MenuVisible=0;
	}
	else if (AbletocastSnowball)
	{
		if (Frostbtn->GetMouseIn())
		{
			DestroyMenu();
			MenuVisible=0;
		}
	}
}

void Turret::TurretClicked()
{
	if (Engine::GameEngine::GetInstance().GetSceneName(getPlayScene())=="play-reverse") return;
	if(!MenuVisible) DestroyMenu();
	else ShowMenu();
	MenuVisible=!MenuVisible;
}

void Turret::ShowMenu()
{
	if (Engine::GameEngine::GetInstance().GetSceneName(getPlayScene())=="play-reverse") return;
	if (type!=TURRET) return ;
	PlayScene* scene = getPlayScene();
	int BlockSize=scene->BlockSize;
	if (AbletocastSnowball)
	{
		Frostbtn = new Engine::ImageButton("play/Snowball.png", "play/Snowball_Hovered.png", Position.x + BlockSize/2, Position.y + BlockSize/2, scene->BlockSize, scene->BlockSize);
		Frostbtn->SetOnClickCallback(std::bind(&Turret::FrostClick, this));
		scene->UIGroup->AddNewControlObject(Frostbtn);
		Frostbtn->Enabled=1;
	}
	Rangebtn = new Engine::ImageButton("play/RangeUp.png", "play/RangeUp_Hovered.png", Position.x - BlockSize/2, Position.y + BlockSize/2, scene->BlockSize, scene->BlockSize);
	Rangebtn->SetOnClickCallback(std::bind(&Turret::RangeUpClick, this));
	Rangebtn->Enabled=1;
	scene->UIGroup->AddNewControlObject(Rangebtn);
	Reloadbtn = new Engine::ImageButton("play/ReloadUp.png", "play/ReloadUp_Hovered.png", Position.x - BlockSize/2*3, Position.y + BlockSize/2, scene->BlockSize, scene->BlockSize);
	Reloadbtn->Enabled=1;
	Reloadbtn->SetOnClickCallback(std::bind(&Turret::ReloadUpClick, this));
	scene->UIGroup->AddNewControlObject(Reloadbtn);
}

void Turret::DestroyMenu()
{
	PlayScene* scene = getPlayScene();
	if (type!=TURRET) return ;
	if (Frostbtn)
	{
		scene->UIGroup->RemoveControlObject(Frostbtn->GetControlIterator(), Frostbtn->GetObjectIterator());
		Frostbtn=nullptr;
	}
	if (Rangebtn)
	{
		scene->UIGroup->RemoveControlObject(Rangebtn->GetControlIterator(), Rangebtn->GetObjectIterator());
		Rangebtn=nullptr;
	}
	if (Reloadbtn)
	{
		scene->UIGroup->RemoveControlObject(Reloadbtn->GetControlIterator(), Reloadbtn->GetObjectIterator());
		Reloadbtn=nullptr;
	}
}

void Turret::ReloadUpClick()
{
	if (Engine::GameEngine::GetInstance().GetSceneName(getPlayScene())=="play-reverse") return;
	PlayScene* scene = getPlayScene();
	coolDown=coolDown/3*2;
	scene->EarnMoney(-1*25);
}

void Turret::RangeUpClick()
{
	if (Engine::GameEngine::GetInstance().GetSceneName(getPlayScene())=="play-reverse") return;
	PlayScene* scene = getPlayScene();
	CollisionRadius=CollisionRadius/2*3;
	scene->EarnMoney(-1*25);
}

void Turret::FrostClick()
{
	if (Engine::GameEngine::GetInstance().GetSceneName(getPlayScene())=="play-reverse") return;
	PlayScene* scene = getPlayScene();
	FrostUpdate=1;
	scene->EarnMoney(-1*25);
}