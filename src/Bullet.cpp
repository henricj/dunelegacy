/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Bullet.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <SoundPlayer.h>
#include <ObjectBase.h>
#include <Game.h>
#include <Map.h>
#include <House.h>
#include <Explosion.h>

#include <misc/draw_util.h>
#include <misc/exceptions.h>

#include <algorithm>


Bullet::Bullet(uint32_t shooterID, const Coord* newRealLocation, const Coord* newRealDestination, uint32_t bulletID, int damage, bool air, const ObjectBase* pTarget)
{
    airAttack = air;

    this->shooterID = shooterID;

    this->owner = currentGame->getObjectManager().getObject(shooterID)->getOwner();

    this->bulletID = bulletID;

    this->damage = damage;

    target.pointTo(pTarget);

    Bullet::init();

    destination = *newRealDestination;

    if(bulletID == Bullet_Sonic) {
        const auto diffX = destination.x - newRealLocation->x;
        auto diffY = destination.y - newRealLocation->y;

        int weaponrange = currentGame->objectData.data[Unit_SonicTank][static_cast<int>(owner->getHouseID())].weaponrange;

        if((diffX == 0) && (diffY == 0)) {
            diffY = weaponrange*TILESIZE;
        }

        const auto square_root = FixPoint::sqrt(diffX*diffX + diffY*diffY);
        const auto ratio = (weaponrange*TILESIZE)/square_root;
        destination.x = newRealLocation->x + floor(diffX*ratio);
        destination.y = newRealLocation->y + floor(diffY*ratio);
    } else if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket) {
        const auto distance = distanceFrom(*newRealLocation, *newRealDestination);


        const auto randAngle = 2 * FixPt_PI * currentGame->randomGen.randFixPoint();
        const auto radius = currentGame->randomGen.rand(0,lround(TILESIZE/2 + (distance/TILESIZE)));

        destination.x += lround(FixPoint::cos(randAngle) * radius);
        destination.y -= lround(FixPoint::sin(randAngle) * radius);

    }

    realX = newRealLocation->x;
    realY = newRealLocation->y;
    source.x = newRealLocation->x;
    source.y = newRealLocation->y;
    location.x = newRealLocation->x/TILESIZE;
    location.y = newRealLocation->y/TILESIZE;

    const auto angleRad =  destinationAngleRad(*newRealLocation, *newRealDestination);
    angle = RadToDeg256(angleRad);
    drawnAngle = lround(numFrames*angle/256) % numFrames;

    xSpeed = speed * FixPoint::cos(angleRad);
    ySpeed = speed * -FixPoint::sin(angleRad);
}

Bullet::Bullet(InputStream& stream)
{
    bulletID = stream.readUint32();

    airAttack = stream.readBool();
    target.load(stream);
    damage = stream.readSint32();

    shooterID  = stream.readUint32();
    uint32_t x = stream.readUint32();
    if(x < static_cast<uint32_t>(HOUSETYPE::NUM_HOUSES)) {
        owner = currentGame->getHouse(static_cast<HOUSETYPE>(x));
    } else {
        owner = currentGame->getHouse(static_cast<HOUSETYPE>(0));
    }

    source.x = stream.readSint32();
    source.y = stream.readSint32();
    destination.x = stream.readSint32();
    destination.y = stream.readSint32();
    location.x = stream.readSint32();
    location.y = stream.readSint32();
    realX = stream.readFixPoint();
    realY = stream.readFixPoint();

    xSpeed = stream.readFixPoint();
    ySpeed = stream.readFixPoint();

    drawnAngle = stream.readSint8();
    angle = stream.readFixPoint();

    Bullet::init();

    detonationTimer = stream.readSint8();
}

void Bullet::init()
{
    explodesAtGroundObjects = false;

    const auto houseID = owner->getHouseID();

    switch(bulletID) {
        case Bullet_DRocket: {
            damageRadius = TILESIZE/2;
            speed = 20;
            detonationTimer = 19;
            numFrames = 16;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_MediumRocket, houseID);
        } break;

        case Bullet_LargeRocket: {
            damageRadius = TILESIZE;
            speed = 20;
            detonationTimer = -1;
            numFrames = 16;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_LargeRocket, houseID);
        } break;

        case Bullet_Rocket: {
            damageRadius = TILESIZE/2;
            speed = 17.5_fix;
            detonationTimer = 22;
            numFrames = 16;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_MediumRocket, houseID);
        } break;

        case Bullet_TurretRocket: {
            damageRadius = TILESIZE/2;
            speed = 20;
            detonationTimer = -1;
            numFrames = 16;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_MediumRocket, houseID);
        } break;

        case Bullet_ShellSmall: {
            damageRadius = TILESIZE/2;
            explodesAtGroundObjects = true;
            speed = 20;
            detonationTimer = -1;
            numFrames = 1;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_Small, houseID);
        } break;

        case Bullet_ShellMedium: {
            damageRadius = TILESIZE/2;
            explodesAtGroundObjects = true;
            speed = 20;
            detonationTimer = -1;
            numFrames = 1;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_Medium, houseID);
        } break;

        case Bullet_ShellLarge: {
            damageRadius = TILESIZE/2;
            explodesAtGroundObjects = true;
            speed = 20;
            detonationTimer = -1;
            numFrames = 1;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_Large, houseID);
        } break;

        case Bullet_ShellTurret: {
            damageRadius = TILESIZE/2;
            explodesAtGroundObjects = true;
            speed = 20;
            detonationTimer = -1;
            numFrames = 1;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_Medium, houseID);
        } break;

        case Bullet_SmallRocket: {
            damageRadius = TILESIZE/2;
            speed = 20;
            detonationTimer = 7;
            numFrames = 16;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_SmallRocket, houseID);
        } break;

        case Bullet_Sonic: {
            damageRadius = (TILESIZE*3)/4;
            speed = 6;  // For Sonic bullets this is only half the actual speed; see Bullet::update()
            numFrames = 1;
            detonationTimer = 45;
            graphic = pGFXManager->getObjPic(ObjPic_Bullet_Sonic, HOUSETYPE::HOUSE_HARKONNEN);    // no color remapping
        } break;

        case Bullet_Sandworm: {
            THROW(std::domain_error, "Cannot init 'Bullet_Sandworm': Not allowed!");
        } break;

        default: {
            THROW(std::domain_error, "Unknown Bullet type %d!", bulletID);
        } break;
    }
}


Bullet::~Bullet() = default;

void Bullet::save(OutputStream& stream) const
{
    stream.writeUint32(bulletID);

    stream.writeBool(airAttack);
    target.save(stream);
    stream.writeSint32(damage);

    stream.writeUint32(shooterID);
    stream.writeUint32(static_cast<uint32_t>(owner->getHouseID()));

    stream.writeSint32(source.x);
    stream.writeSint32(source.y);
    stream.writeSint32(destination.x);
    stream.writeSint32(destination.y);
    stream.writeSint32(location.x);
    stream.writeSint32(location.y);
    stream.writeFixPoint(realX);
    stream.writeFixPoint(realY);

    stream.writeFixPoint(xSpeed);
    stream.writeFixPoint(ySpeed);

    stream.writeSint8(drawnAngle);
    stream.writeFixPoint(angle);

    stream.writeSint8(detonationTimer);
}


void Bullet::blitToScreen(uint32_t cycleCount) const {
    const auto imageW = getWidth(graphic[currentZoomlevel])/numFrames;
    const auto imageH = getHeight(graphic[currentZoomlevel]);

    if(!screenborder->isInsideScreen( Coord(lround(realX), lround(realY)), Coord(imageW, imageH))) {
        return;
    }

    auto dest = calcSpriteDrawingRect(graphic[currentZoomlevel], screenborder->world2screenX(realX), screenborder->world2screenY(realY),
        numFrames, 1, HAlign::Center, VAlign::Center);

    if(bulletID == Bullet_Sonic) {
        static constexpr uint8_t shimmerOffset[] = {1, 3, 2, 5, 4, 3, 2, 1};

        auto* const shimmerMaskSurface = pGFXManager->getZoomedObjSurface(ObjPic_Bullet_Sonic, currentZoomlevel);
        auto* const shimmerTex =
            pGFXManager->getTempStreamingTexture(renderer, shimmerMaskSurface->w, shimmerMaskSurface->h);

        auto source = dest;

        const auto shimmerOffsetIndex = ((cycleCount + getBulletID()) % 24) / 3;
        source.x += shimmerOffset[shimmerOffsetIndex % 8] * 2;

        uint32_t format;
        int      access, w, h;
        SDL_QueryTexture(shimmerTex, &format, &access, &w, &h);

        float scaleX, scaleY;
        SDL_RenderGetScale(renderer, &scaleX, &scaleY);

        // Even after this scale adjustment, there is an unknown offset between the effective coordinates
        // used to read the pixels compared to the coordinates used to copy the final texture to the screen.
        // Note also that if we are partly off the screen, we will get the mask's black appearing in the
        // transparent areas of surface_copy.
        const SDL_Rect scaled_source{lround(static_cast<float>(source.x) * scaleX), lround(static_cast<float>(source.y) * scaleY),
                                     lround(static_cast<float>(w) * scaleX), lround(static_cast<float>(h) * scaleY)};

        const sdl2::surface_ptr screen_copy{
            SDL_CreateRGBSurfaceWithFormat(0, scaled_source.w, scaled_source.h, SDL_BITSPERPIXEL(32), SCREEN_FORMAT)};

        { // Scope
            const sdl2::surface_lock lock{screen_copy.get()};

            if(SDL_RenderReadPixels(renderer, &scaled_source, screen_copy->format->format, lock.pixels(),
                                    lock.pitch())) {
                const auto sdl_error = SDL_GetError();
            }
        }

        // If we are close
        const sdl2::surface_ptr shimmer_work{
            SDL_CreateRGBSurfaceWithFormat(0, w, h, SDL_BITSPERPIXEL(32), SCREEN_FORMAT)};

        SDL_SetSurfaceBlendMode(shimmer_work.get(), SDL_BlendMode::SDL_BLENDMODE_BLEND);

        SDL_SetSurfaceBlendMode(shimmerMaskSurface, SDL_BlendMode::SDL_BLENDMODE_NONE);
        const auto blit1_ret = SDL_BlitSurface(shimmerMaskSurface, nullptr, shimmer_work.get(), nullptr);
        SDL_SetSurfaceBlendMode(screen_copy.get(), SDL_BlendMode::SDL_BLENDMODE_ADD);
        const auto blit2_ret = SDL_BlitSurface(screen_copy.get(), nullptr, shimmer_work.get(), nullptr);

        { // Scope
            const sdl2::surface_lock src{shimmer_work.get()};

            const auto update_ret = SDL_UpdateTexture(shimmerTex, nullptr, src.pixels(), src.pitch());
        }

#if 0
        // switch to texture 'shimmerTex' for rendering
        auto *const oldRenderTarget = SDL_GetRenderTarget(renderer);
        SDL_SetRenderTarget(renderer, shimmerTex);

        // copy complete mask
        // contains solid black (0,0,0,255) for pixels to take from screen
        // and transparent (0,0,0,0) for pixels that should not be copied over
        SDL_SetTextureBlendMode(shimmerMaskTex, SDL_BLENDMODE_NONE);
        Dune_RenderCopy(renderer, shimmerMaskTex, nullptr, nullptr);
        SDL_SetTextureBlendMode(shimmerMaskTex, SDL_BLENDMODE_BLEND);

        // now copy r,g,b colors from screen but don't change alpha values in mask
        SDL_SetTextureBlendMode(screenTexture, SDL_BLENDMODE_ADD);
        auto source = dest;
        const auto shimmerOffsetIndex = ((cycleCount + getBulletID()) % 24)/3;
        source.x += shimmerOffset[shimmerOffsetIndex%8]*2;
        Dune_RenderCopy(renderer, screenTexture, &source, nullptr);
        SDL_SetTextureBlendMode(screenTexture, SDL_BLENDMODE_NONE);

        // switch back to old rendering target (from texture 'shimmerTex')
        SDL_SetRenderTarget(renderer, oldRenderTarget);
#endif // 0

        // now blend shimmerTex to screen (= make use of alpha values in mask)
        SDL_SetTextureBlendMode(shimmerTex, SDL_BLENDMODE_BLEND);
        Dune_RenderCopy(renderer, shimmerTex, nullptr, &dest);
    } else {
        const auto source = calcSpriteSourceRect(graphic[currentZoomlevel], (numFrames > 1) ? drawnAngle: 0, numFrames);
        Dune_RenderCopy(renderer, graphic[currentZoomlevel], &source, &dest);
    }
}


bool Bullet::update(const GameContext& context) {
    if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket || bulletID == Bullet_TurretRocket) {

        ObjectBase* pTarget = target.getObjPointer();
        if(pTarget && pTarget->isAFlyingUnit()) { destination = pTarget->getCenterPoint(); }

        const auto angleToDestinationRad = destinationAngleRad(Coord(lround(realX), lround(realY)), destination);
        const auto angleToDestination    = RadToDeg256(angleToDestinationRad);

        auto angleDifference = angleToDestination - angle;
        if(angleDifference > 128) {
            angleDifference -= 256;
        } else if(angleDifference < -128) {
            angleDifference += 256;
        }

        static const FixPoint turnSpeed = 4.5_fix;

        if(angleDifference >= turnSpeed) {
            angleDifference = turnSpeed;
        } else if(angleDifference <= -turnSpeed) {
            angleDifference = -turnSpeed;
        }

        angle += angleDifference;

        if(angle < 0) {
            angle += 256;
        } else if(angle >= 256) {
            angle -= 256;
        }

        xSpeed = speed * FixPoint::cos(Deg256ToRad(angle));
        ySpeed = speed * -FixPoint::sin(Deg256ToRad(angle));

        drawnAngle = lround(numFrames * angle / 256) % numFrames;
    }

    const auto oldDistanceToDestination = distanceFrom(realX, realY, destination.x, destination.y);

    realX += xSpeed; // keep the bullet moving by its current speeds
    realY += ySpeed;
    location.x = floor(realX / TILESIZE);
    location.y = floor(realY / TILESIZE);

    if((location.x < -5) || (location.x >= currentGameMap->getSizeX() + 5) || (location.y < -5) ||
       (location.y >= currentGameMap->getSizeY() + 5)) {
        // it's off the map => delete it
        return true;
    }

    const auto newDistanceToDestination = distanceFrom(realX, realY, destination.x, destination.y);

    if(detonationTimer > 0) { detonationTimer--; }

    auto& map = context.map;

    if(bulletID == Bullet_Sonic) {

        if(detonationTimer == 0) {
            destroy(context);
            return true;
        }

        FixPoint weaponDamage =
            context.game.objectData.data[Unit_SonicTank][static_cast<int>(owner->getHouseID())].weapondamage;

        FixPoint startDamage = (weaponDamage / 4 + 1) / 4.5_fix;
        FixPoint endDamage   = ((weaponDamage - 9) / 4 + 1) / 4.5_fix;

        const auto damageDecrease = -(startDamage - endDamage) / (45 * 2 * speed);
        const auto dist           = distanceFrom(source.x, source.y, realX, realY);

        const auto currentDamage = dist * damageDecrease + startDamage;

        auto realPos = Coord(lround(realX), lround(realY));
        map.damage(context, shooterID, owner, realPos, bulletID, currentDamage / 2, damageRadius, false);

        realX += xSpeed; // keep the bullet moving by its current speeds
        realY += ySpeed;

        realPos = Coord(lround(realX), lround(realY));
        map.damage(context, shooterID, owner, realPos, bulletID, currentDamage / 2, damageRadius, false);

        return false;
    }

    if(explodesAtGroundObjects) {
        auto* tile = map.tryGetTile(location.x, location.y);

        if(tile && tile->hasANonInfantryGroundObject()) {
            auto* structure = tile->getNonInfantryGroundObject(context.objectManager);

            if(structure && structure->isAStructure() &&
               ((bulletID != Bullet_ShellTurret) || (structure->getOwner() != owner))) {
                destroy(context);

                return true;
            }
        }
    }

    if(oldDistanceToDestination < newDistanceToDestination || newDistanceToDestination < 4) {

        if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket) {
            if(detonationTimer == 0) {
                destroy(context);

                return true;
            }
        } else {
            realX = destination.x;
            realY = destination.y;
            destroy(context);

            return true;
        }
    }

    return false;
}


void Bullet::destroy(const GameContext& context) const {
    auto position = Coord(lround(realX), lround(realY));

    auto& [game, map, objectManager] = context;

    const auto houseID = owner->getHouseID();

    switch(bulletID) {
        case Bullet_DRocket: {
            map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            soundPlayer->playSoundAt(Sound_ExplosionGas, position);
            game.addExplosion(Explosion_Gas, position, houseID);
        } break;

        case Bullet_LargeRocket: {
            soundPlayer->playSoundAt(Sound_ExplosionLarge, position);

            for(auto i = 0; i < 5; i++) {
                for(auto j = 0; j < 5; j++) {
                    if(((i != 0) && (i != 4)) || ((j != 0) && (j != 4))) {
                        position.x = lround(realX) + (i - 2) * TILESIZE;
                        position.y = lround(realY) + (j - 2) * TILESIZE;

                        map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);

                        uint32_t explosionID = game.randomGen.getRandOf(Explosion_Large1, Explosion_Large2);
                        game.addExplosion(explosionID, position, houseID);
                        screenborder->shakeScreen(22);
                    }
                }
            }
        } break;

        case Bullet_Rocket:
        case Bullet_TurretRocket:
        case Bullet_SmallRocket: {
            map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            game.addExplosion(Explosion_Small, position, houseID);
        } break;

        case Bullet_ShellSmall: {
            map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            game.addExplosion(Explosion_ShellSmall, position, houseID);
        } break;

        case Bullet_ShellMedium: {
            map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            game.addExplosion(Explosion_ShellMedium, position, houseID);
        } break;

        case Bullet_ShellLarge: {
            map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            game.addExplosion(Explosion_ShellLarge, position, houseID);
        } break;

        case Bullet_ShellTurret: {
            map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            game.addExplosion(Explosion_ShellMedium, position, houseID);
        } break;

        case Bullet_Sonic:
        case Bullet_Sandworm:
        default: {
            // do nothing
        } break;
    }
}

