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

#include "ScreenBorder.h"
#include "mmath.h"
#include <Explosion.h>
#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ObjectBase.h>
#include <SoundPlayer.h>

#include <math.h>
#include <misc/draw_util.h>
#include <misc/exceptions.h>

#include <algorithm>

Bullet::Bullet(uint32_t shooterID, const Coord* newRealLocation, const Coord* newRealDestination, uint32_t bulletID,
               int damage, bool air, const ObjectBase* pTarget)
    : bulletID_(bulletID), damage_(damage), shooterID_(shooterID),
      owner_(dune::globals::currentGame->getObjectManager().getObject(shooterID)->getOwner()),
      destination_(*newRealDestination), airAttack_(air) {

    target_.pointTo(pTarget);

    Bullet::init();

    auto* const game = dune::globals::currentGame.get();

    if (bulletID == Bullet_Sonic) {
        const auto diffX = destination_.x - newRealLocation->x;
        auto diffY       = destination_.y - newRealLocation->y;

        const int weaponrange =
            game->objectData.data[Unit_SonicTank][static_cast<int>(owner_->getHouseID())].weaponrange;

        if ((diffX == 0) && (diffY == 0)) {
            diffY = weaponrange * TILESIZE;
        }

        const auto square_root = FixPoint::sqrt(diffX * diffX + diffY * diffY);
        const auto ratio       = (weaponrange * TILESIZE) / square_root;
        destination_.x         = newRealLocation->x + floor(diffX * ratio);
        destination_.y         = newRealLocation->y + floor(diffY * ratio);
    } else if (bulletID == Bullet_Rocket || bulletID == Bullet_DRocket) {
        const auto distance = distanceFrom(*newRealLocation, *newRealDestination);

        const auto randAngle = 2 * FixPt_PI * game->randomGen.randFixPoint();
        const auto radius    = game->randomGen.rand(0, lround(TILESIZE / 2 + (distance / TILESIZE)));

        destination_.x += lround(FixPoint::cos(randAngle) * radius);
        destination_.y -= lround(FixPoint::sin(randAngle) * radius);
    }

    realX_      = newRealLocation->x;
    realY_      = newRealLocation->y;
    source_.x   = newRealLocation->x;
    source_.y   = newRealLocation->y;
    location_.x = newRealLocation->x / TILESIZE;
    location_.y = newRealLocation->y / TILESIZE;

    const auto angleRad = destinationAngleRad(*newRealLocation, *newRealDestination);
    angle_              = RadToDeg256(angleRad);
    drawnAngle_         = static_cast<uint8_t>(lround(numFrames_ * angle_ / 256) % numFrames_);

    xSpeed_ = speed_ * FixPoint::cos(angleRad);
    ySpeed_ = speed_ * -FixPoint::sin(angleRad);
}

Bullet::Bullet(InputStream& stream) {
    bulletID_ = stream.readUint32();

    airAttack_ = stream.readBool();
    target_.load(stream);
    damage_ = stream.readSint32();

    shooterID_ = stream.readUint32();
    uint32_t x = stream.readUint32();

    const auto* const game = dune::globals::currentGame.get();
    if (x < static_cast<uint32_t>(HOUSETYPE::NUM_HOUSES)) {
        owner_ = game->getHouse(static_cast<HOUSETYPE>(x));
    } else {
        owner_ = game->getHouse(static_cast<HOUSETYPE>(0));
    }

    source_.x      = stream.readSint32();
    source_.y      = stream.readSint32();
    destination_.x = stream.readSint32();
    destination_.y = stream.readSint32();
    location_.x    = stream.readSint32();
    location_.y    = stream.readSint32();
    realX_         = stream.readFixPoint();
    realY_         = stream.readFixPoint();

    xSpeed_ = stream.readFixPoint();
    ySpeed_ = stream.readFixPoint();

    drawnAngle_ = stream.readSint8();
    angle_      = stream.readFixPoint();

    Bullet::init();

    detonationTimer_ = stream.readSint8();
}

void Bullet::init() {
    explodesAtGroundObjects_ = false;

    const auto houseID = owner_->getHouseID();

    const auto* const gfx = dune::globals::pGFXManager.get();

    switch (bulletID_) {
        case Bullet_DRocket: {
            damageRadius_    = TILESIZE / 2;
            speed_           = 20;
            detonationTimer_ = 19;
            numFrames_       = 16;
            graphic_         = gfx->getObjPic(ObjPic_Bullet_MediumRocket, houseID);
        } break;

        case Bullet_LargeRocket: {
            damageRadius_    = TILESIZE;
            speed_           = 20;
            detonationTimer_ = -1;
            numFrames_       = 16;
            graphic_         = gfx->getObjPic(ObjPic_Bullet_LargeRocket, houseID);
        } break;

        case Bullet_Rocket: {
            damageRadius_    = TILESIZE / 2;
            speed_           = 17.5_fix;
            detonationTimer_ = 22;
            numFrames_       = 16;
            graphic_         = gfx->getObjPic(ObjPic_Bullet_MediumRocket, houseID);
        } break;

        case Bullet_TurretRocket: {
            damageRadius_    = TILESIZE / 2;
            speed_           = 20;
            detonationTimer_ = -1;
            numFrames_       = 16;
            graphic_         = gfx->getObjPic(ObjPic_Bullet_MediumRocket, houseID);
        } break;

        case Bullet_ShellSmall: {
            damageRadius_            = TILESIZE / 2;
            explodesAtGroundObjects_ = true;
            speed_                   = 20;
            detonationTimer_         = -1;
            numFrames_               = 1;
            graphic_                 = gfx->getObjPic(ObjPic_Bullet_Small, houseID);
        } break;

        case Bullet_ShellMedium: {
            damageRadius_            = TILESIZE / 2;
            explodesAtGroundObjects_ = true;
            speed_                   = 20;
            detonationTimer_         = -1;
            numFrames_               = 1;
            graphic_                 = gfx->getObjPic(ObjPic_Bullet_Medium, houseID);
        } break;

        case Bullet_ShellLarge: {
            damageRadius_            = TILESIZE / 2;
            explodesAtGroundObjects_ = true;
            speed_                   = 20;
            detonationTimer_         = -1;
            numFrames_               = 1;
            graphic_                 = gfx->getObjPic(ObjPic_Bullet_Large, houseID);
        } break;

        case Bullet_ShellTurret: {
            damageRadius_            = TILESIZE / 2;
            explodesAtGroundObjects_ = true;
            speed_                   = 20;
            detonationTimer_         = -1;
            numFrames_               = 1;
            graphic_                 = gfx->getObjPic(ObjPic_Bullet_Medium, houseID);
        } break;

        case Bullet_SmallRocket: {
            damageRadius_    = TILESIZE / 2;
            speed_           = 20;
            detonationTimer_ = 7;
            numFrames_       = 16;
            graphic_         = gfx->getObjPic(ObjPic_Bullet_SmallRocket, houseID);
        } break;

        case Bullet_Sonic: {
            damageRadius_    = (TILESIZE * 3) / 4;
            speed_           = 6; // For Sonic bullets this is only half the actual speed; see Bullet::update()
            numFrames_       = 1;
            detonationTimer_ = 45;
            graphic_         = gfx->getObjPic(ObjPic_Bullet_Sonic, HOUSETYPE::HOUSE_HARKONNEN); // no color remapping
        } break;

        case Bullet_Sandworm: {
            THROW(std::domain_error, "Cannot init 'Bullet_Sandworm': Not allowed!");
        }

        default: {
            THROW(std::domain_error, "Unknown Bullet type {}!", bulletID_);
        }
    }
}

Bullet::~Bullet() = default;

void Bullet::save(OutputStream& stream) const {
    stream.writeUint32(bulletID_);

    stream.writeBool(airAttack_);
    target_.save(stream);
    stream.writeSint32(damage_);

    stream.writeUint32(shooterID_);
    stream.writeUint32(static_cast<uint32_t>(owner_->getHouseID()));

    stream.writeSint32(source_.x);
    stream.writeSint32(source_.y);
    stream.writeSint32(destination_.x);
    stream.writeSint32(destination_.y);
    stream.writeSint32(location_.x);
    stream.writeSint32(location_.y);
    stream.writeFixPoint(realX_);
    stream.writeFixPoint(realY_);

    stream.writeFixPoint(xSpeed_);
    stream.writeFixPoint(ySpeed_);

    stream.writeSint8(drawnAngle_);
    stream.writeFixPoint(angle_);

    stream.writeSint8(detonationTimer_);
}

void Bullet::blitToScreen(uint32_t cycleCount) const {
    const auto zoom                = dune::globals::currentZoomlevel;
    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();

    const auto imageW = getWidth(graphic_[zoom]) / numFrames_;
    const auto imageH = getHeight(graphic_[zoom]);

    if (!screenborder->isInsideScreen(
            Coord(lround(realX_), lround(realY_)),
            Coord(static_cast<int>(std::ceil(imageW)), static_cast<int>(std::ceil(imageH))))) {
        return;
    }

    const auto dest =
        calcSpriteDrawingRect(graphic_[zoom], screenborder->world2screenX(realX_), screenborder->world2screenY(realY_),
                              numFrames_, 1, HAlign::Center, VAlign::Center);

    if (bulletID_ == Bullet_Sonic) {
        static constexpr uint8_t shimmerOffset[] = {1, 3, 2, 5, 4, 3, 2, 1};

        auto* const gfx = dune::globals::pGFXManager.get();

        auto* const shimmerMaskSurface = gfx->getZoomedObjSurface(ObjPic_Bullet_Sonic, zoom);
        auto* const shimmerTex = gfx->getTempStreamingTexture(renderer, shimmerMaskSurface->w, shimmerMaskSurface->h);

        auto [sx, sy, sw, sh] = dest;

        const auto shimmerOffsetIndex = ((cycleCount + getBulletID()) % 24) / 3;
        sx += shimmerOffset[shimmerOffsetIndex % 8] * 2;

        uint32_t format = 0;
        int access = 0, w = 0, h = 0;
        SDL_QueryTexture(shimmerTex, &format, &access, &w, &h);

        float scaleX = NAN, scaleY = NAN;
        SDL_RenderGetScale(renderer, &scaleX, &scaleY);

        // Even after this scale adjustment, there is an unknown offset between the effective coordinates
        // used to read the pixels compared to the coordinates used to copy the final texture to the screen.
        // Note also that if we are partly off the screen, we will get the mask's black appearing in the
        // transparent areas of surface_copy.
        const SDL_Rect scaled_source{static_cast<int>(lround(sx * scaleX)), static_cast<int>(lround(sy * scaleY)),
                                     static_cast<int>(lround(static_cast<float>(w) * scaleX)),
                                     static_cast<int>(lround(static_cast<float>(h) * scaleY))};

        const sdl2::surface_ptr screen_copy{
            SDL_CreateRGBSurfaceWithFormat(0, scaled_source.w, scaled_source.h, SDL_BITSPERPIXEL(32), SCREEN_FORMAT)};

        { // Scope
            const sdl2::surface_lock lock{screen_copy.get()};

            if (SDL_RenderReadPixels(renderer, &scaled_source, screen_copy->format->format, lock.pixels(),
                                     lock.pitch())) {
                sdl2::log_error("Bullet render pixels failed: %s!", SDL_GetError());
            }
        }

        // If we are close
        const sdl2::surface_ptr shimmer_work{
            SDL_CreateRGBSurfaceWithFormat(0, w, h, SDL_BITSPERPIXEL(32), SCREEN_FORMAT)};

        SDL_SetSurfaceBlendMode(shimmer_work.get(), SDL_BlendMode::SDL_BLENDMODE_BLEND);

        SDL_SetSurfaceBlendMode(shimmerMaskSurface, SDL_BlendMode::SDL_BLENDMODE_NONE);
        if (0 != SDL_BlitSurface(shimmerMaskSurface, nullptr, shimmer_work.get(), nullptr))
            sdl2::log_error("Bullet draw failed to copy surface: %s!", SDL_GetError());
        if (0 != SDL_SetSurfaceBlendMode(screen_copy.get(), SDL_BlendMode::SDL_BLENDMODE_ADD))
            sdl2::log_error("Bullet draw failed to set surface blend mode: %s!", SDL_GetError());
        if (0 != SDL_BlitSurface(screen_copy.get(), nullptr, shimmer_work.get(), nullptr))
            sdl2::log_error("Bullet draw failed copy surface: %s!", SDL_GetError());

        { // Scope
            const sdl2::surface_lock src{shimmer_work.get()};

            if (0 != SDL_UpdateTexture(shimmerTex, nullptr, src.pixels(), src.pitch()))
                sdl2::log_error("Bullet draw failed: %s!", SDL_GetError());
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
        Dune_RenderCopyF(renderer, shimmerTex, nullptr, &dest);
    } else {
        const auto source = calcSpriteSourceRect(graphic_[zoom], (numFrames_ > 1) ? drawnAngle_ : 0, numFrames_);
        Dune_RenderCopyF(renderer, graphic_[zoom], &source, &dest);
    }
}

bool Bullet::update(const GameContext& context) {
    auto& map = context.map;

    if (bulletID_ == Bullet_Rocket || bulletID_ == Bullet_DRocket || bulletID_ == Bullet_TurretRocket) {

        const auto* pTarget = target_.getObjPointer();
        if (pTarget && pTarget->isAFlyingUnit()) {
            destination_ = pTarget->getCenterPoint();
        }

        const auto angleToDestinationRad = destinationAngleRad(Coord(lround(realX_), lround(realY_)), destination_);
        const auto angleToDestination    = RadToDeg256(angleToDestinationRad);

        auto angleDifference = angleToDestination - angle_;
        if (angleDifference > 128) {
            angleDifference -= 256;
        } else if (angleDifference < -128) {
            angleDifference += 256;
        }

        static constexpr auto turnSpeed = 4.5_fix;

        if (angleDifference >= turnSpeed) {
            angleDifference = turnSpeed;
        } else if (angleDifference <= -turnSpeed) {
            angleDifference = -turnSpeed;
        }

        angle_ += angleDifference;

        if (angle_ < 0) {
            angle_ += 256;
        } else if (angle_ >= 256) {
            angle_ -= 256;
        }

        xSpeed_ = speed_ * FixPoint::cos(Deg256ToRad(angle_));
        ySpeed_ = speed_ * -FixPoint::sin(Deg256ToRad(angle_));

        drawnAngle_ = static_cast<int8_t>(lround(numFrames_ * angle_ / 256) % numFrames_);
    }

    const auto oldDistanceToDestination = distanceFrom(realX_, realY_, destination_.x, destination_.y);

    realX_ += xSpeed_; // keep the bullet moving by its current speeds
    realY_ += ySpeed_;
    location_.x = floor(realX_ / TILESIZE);
    location_.y = floor(realY_ / TILESIZE);

    if ((location_.x < -5) || (location_.x >= map.getSizeX() + 5) || (location_.y < -5)
        || (location_.y >= map.getSizeY() + 5)) {
        // it's off the map => delete it
        return true;
    }

    const auto newDistanceToDestination = distanceFrom(realX_, realY_, destination_.x, destination_.y);

    if (detonationTimer_ > 0) {
        detonationTimer_--;
    }

    if (bulletID_ == Bullet_Sonic) {

        if (detonationTimer_ == 0) {
            destroy(context);
            return true;
        }

        const FixPoint weaponDamage =
            context.game.objectData.data[Unit_SonicTank][static_cast<int>(owner_->getHouseID())].weapondamage;

        const FixPoint startDamage = (weaponDamage / 4 + 1) / 4.5_fix;
        const FixPoint endDamage   = ((weaponDamage - 9) / 4 + 1) / 4.5_fix;

        const auto damageDecrease = -(startDamage - endDamage) / (45 * 2 * speed_);
        const auto dist           = distanceFrom(source_.x, source_.y, realX_, realY_);

        const auto currentDamage = dist * damageDecrease + startDamage;

        auto realPos = Coord(lround(realX_), lround(realY_));
        map.damage(context, shooterID_, owner_, realPos, bulletID_, currentDamage / 2, damageRadius_, false);

        realX_ += xSpeed_; // keep the bullet moving by its current speeds
        realY_ += ySpeed_;

        realPos = Coord(lround(realX_), lround(realY_));
        map.damage(context, shooterID_, owner_, realPos, bulletID_, currentDamage / 2, damageRadius_, false);

        return false;
    }

    if (explodesAtGroundObjects_) {
        const auto* tile = map.tryGetTile(location_.x, location_.y);

        if (tile && tile->hasANonInfantryGroundObject()) {
            const auto* structure = tile->getNonInfantryGroundObject(context.objectManager);

            if (structure && structure->isAStructure()
                && ((bulletID_ != Bullet_ShellTurret) || (structure->getOwner() != owner_))) {
                destroy(context);

                return true;
            }
        }
    }

    if (oldDistanceToDestination < newDistanceToDestination || newDistanceToDestination < 4) {

        if (bulletID_ == Bullet_Rocket || bulletID_ == Bullet_DRocket) {
            if (detonationTimer_ == 0) {
                destroy(context);

                return true;
            }
        } else {
            realX_ = destination_.x;
            realY_ = destination_.y;
            destroy(context);

            return true;
        }
    }

    return false;
}

void Bullet::destroy(const GameContext& context) const {
    auto position = Coord(lround(realX_), lround(realY_));

    auto& [game, map, objectManager] = context;

    const auto houseID = owner_->getHouseID();

    switch (bulletID_) {
        case Bullet_DRocket: {
            map.damage(context, shooterID_, owner_, position, bulletID_, damage_, damageRadius_, airAttack_);
            dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionGas, position);
            game.addExplosion(Explosion_Gas, position, houseID);
        } break;

        case Bullet_LargeRocket: {
            dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionLarge, position);

            for (auto i = 0; i < 5; i++) {
                for (auto j = 0; j < 5; j++) {
                    if (((i != 0) && (i != 4)) || ((j != 0) && (j != 4))) {
                        position.x = lround(realX_) + (i - 2) * TILESIZE;
                        position.y = lround(realY_) + (j - 2) * TILESIZE;

                        map.damage(context, shooterID_, owner_, position, bulletID_, damage_, damageRadius_,
                                   airAttack_);

                        uint32_t explosionID = game.randomGen.getRandOf(Explosion_Large1, Explosion_Large2);
                        game.addExplosion(explosionID, position, houseID);
                        dune::globals::screenborder->shakeScreen(22);
                    }
                }
            }
        } break;

        case Bullet_Rocket:
        case Bullet_TurretRocket:
        case Bullet_SmallRocket: {
            map.damage(context, shooterID_, owner_, position, bulletID_, damage_, damageRadius_, airAttack_);
            game.addExplosion(Explosion_Small, position, houseID);
        } break;

        case Bullet_ShellSmall: {
            map.damage(context, shooterID_, owner_, position, bulletID_, damage_, damageRadius_, airAttack_);
            game.addExplosion(Explosion_ShellSmall, position, houseID);
        } break;

        case Bullet_ShellMedium: {
            map.damage(context, shooterID_, owner_, position, bulletID_, damage_, damageRadius_, airAttack_);
            game.addExplosion(Explosion_ShellMedium, position, houseID);
        } break;

        case Bullet_ShellLarge: {
            map.damage(context, shooterID_, owner_, position, bulletID_, damage_, damageRadius_, airAttack_);
            game.addExplosion(Explosion_ShellLarge, position, houseID);
        } break;

        case Bullet_ShellTurret: {
            map.damage(context, shooterID_, owner_, position, bulletID_, damage_, damageRadius_, airAttack_);
            game.addExplosion(Explosion_ShellMedium, position, houseID);
        } break;

        case Bullet_Sonic:
        case Bullet_Sandworm:
        default: {
            // do nothing
        } break;
    }
}
