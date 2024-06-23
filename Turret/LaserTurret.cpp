#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "Engine/AudioHelper.hpp"
#include "Engine/Group.hpp"
#include "Bullet/LaserBullet.hpp"
#include "Bullet/FrostBullet.hpp"
#include "LaserTurret.hpp"
#include "Scene/PlayScene.hpp"
#include "Engine/Point.hpp"

const int LaserTurret::Price = 200;
const int LaserTurret::Range = 300;
const int LaserTurret::Damage = LaserBullet::Damage;
const float LaserTurret::Reload= 0.5;

LaserTurret::LaserTurret(float x, float y) :
	Turret("play/tower-base.png", "play/turret-2.png", x, y, Range, Price, Reload, TURRET) {
	// Move center downward, since we the turret head is slightly biased upward.
	Anchor.y += 8.0f / GetBitmapHeight();
	bullet_speed = LaserBullet::Speed;
	AbletocastSnowball=0;
}
void LaserTurret::CreateBullet() {
	Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
	float rotation = atan2(diff.y, diff.x);
	Engine::Point normalized = diff.Normalize();
	Engine::Point normal = Engine::Point(-normalized.y, normalized.x);
	// Change bullet position to the front of the gun barrel.
	if (FrostUpdate)
	{
		getPlayScene()->BulletGroup->AddNewObject(new FrostBullet(Position + normalized * 36 - normal * 6, diff, rotation, this));
		getPlayScene()->BulletGroup->AddNewObject(new FrostBullet(Position + normalized * 36 + normal * 6, diff, rotation, this));
	}
	else
	{
		getPlayScene()->BulletGroup->AddNewObject(new LaserBullet(Position + normalized * 36 - normal * 6, diff, rotation, this));
		getPlayScene()->BulletGroup->AddNewObject(new LaserBullet(Position + normalized * 36 + normal * 6, diff, rotation, this));
	}
	AudioHelper::PlayAudio("laser.wav");
}
