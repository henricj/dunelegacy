set(SOURCES
	AStarSearch.cpp
	Bullet.cpp
	Command.cpp
	CommandManager.cpp
	DuneLegacyEngine.cpp
	engine_sand.cpp
	Explosion.cpp
	GameInitSettings.cpp
	ObjectBase.cpp
	ObjectData.cpp
	ObjectManager.cpp
	ObjectPointer.cpp
	FileClasses/INIFile.cpp
	)

set(PLAYER_SOURCES
	players/AIPlayer.cpp
	players/CampaignAIPlayer.cpp
	players/HumanPlayer.cpp
	players/Player.cpp
	players/PlayerFactory.cpp
	players/QuantBot.cpp
	players/SmartBot.cpp)


set(STRUCTURE_SOURCES
	structures/Barracks.cpp
	structures/BuilderBase.cpp
	structures/ConstructionYard.cpp
	structures/GunTurret.cpp
	structures/HeavyFactory.cpp
	structures/HighTechFactory.cpp
	structures/IX.cpp
	structures/LightFactory.cpp
	structures/Palace.cpp
	structures/Radar.cpp
	structures/Refinery.cpp
	structures/RepairYard.cpp
	structures/RocketTurret.cpp
	structures/Silo.cpp
	structures/StarPort.cpp
	structures/StructureBase.cpp
	structures/TurretBase.cpp
	structures/WOR.cpp
	structures/Wall.cpp
	structures/WindTrap.cpp
	)


set(TRIGGER_SOURCES
	Trigger/ReinforcementTrigger.cpp
	Trigger/TimeoutTrigger.cpp
	Trigger/TriggerManager.cpp
	)

set(UNIT_SOURCES
	units/AirUnit.cpp
	units/Carryall.cpp
	units/Devastator.cpp
	units/Deviator.cpp
	units/Frigate.cpp
	units/GroundUnit.cpp
	units/Harvester.cpp
	units/InfantryBase.cpp
	units/Launcher.cpp
	units/MCV.cpp
	units/Ornithopter.cpp
	units/Quad.cpp
	units/RaiderTrike.cpp
	units/Saboteur.cpp
	units/SandWorm.cpp
	units/SiegeTank.cpp
	units/Soldier.cpp
	units/SonicTank.cpp
	units/Tank.cpp
	units/TankBase.cpp
	units/TrackedUnit.cpp
	units/Trike.cpp
	units/Trooper.cpp
	units/UnitBase.cpp
	)


set(HEADERS
	../../include/engine/AStarSearch.h
	../../include/engine/AITeamInfo.h
	../../include/engine/Bullet.h
	../../include/engine/Command.h
	../../include/engine/CommandManager.h
	../../include/engine/data.h
	../../include/engine/DuneLegacyEngine.h
	../../include/engine/EngineDataTypes.h
	../../include/engine/EngineDefinitions.h
	../../include/engine/engine_sand.h
	../../include/engine/engine_mmath.h
	../../include/engine/Explosion.h
	../../include/engine/GameInitSettings.h
	../../include/engine/ObjectBase.h
	../../include/engine/ObjectData.h
	../../include/engine/ObjectManager.h
	../../include/engine/ObjectPointer.h
	../../include/engine/FileClasses/INIFile.h
	)

set(PLAYER_HEADERS
	../../include/engine/players/AIPlayer.h
	../../include/engine/players/CampaignAIPlayer.h
	../../include/engine/players/HumanPlayer.h
	../../include/engine/players/Player.h
	../../include/engine/players/PlayerFactory.h
	../../include/engine/players/QuantBot.h
	../../include/engine/players/SmartBot.h
	)

set(TRIGGER_HEADERS
	../../include/engine/Trigger/ReinforcementTrigger.h
	../../include/engine/Trigger/TimeoutTrigger.h
	../../include/engine/Trigger/Trigger.h
	../../include/engine/Trigger/TriggerManager.h
)

set(STRUCTURE_HEADERS
	../../include/engine/structures/Barracks.h
	../../include/engine/structures/BuilderBase.h
	../../include/engine/structures/ConstructionYard.h
	../../include/engine/structures/GunTurret.h
	../../include/engine/structures/HeavyFactory.h
	../../include/engine/structures/HighTechFactory.h
	../../include/engine/structures/IX.h
	../../include/engine/structures/LightFactory.h
	../../include/engine/structures/Palace.h
	../../include/engine/structures/Radar.h
	../../include/engine/structures/Refinery.h
	../../include/engine/structures/RepairYard.h
	../../include/engine/structures/RocketTurret.h
	../../include/engine/structures/Silo.h
	../../include/engine/structures/StarPort.h
	../../include/engine/structures/StructureBase.h
	../../include/engine/structures/TurretBase.h
	../../include/engine/structures/Wall.h
	../../include/engine/structures/WindTrap.h
	../../include/engine/structures/WOR.h
)

set(UNIT_HEADERS
	../../include/engine/units/AirUnit.h
	../../include/engine/units/Carryall.h
	../../include/engine/units/Devastator.h
	../../include/engine/units/Deviator.h
	../../include/engine/units/Frigate.h
	../../include/engine/units/GroundUnit.h
	../../include/engine/units/Harvester.h
	../../include/engine/units/InfantryBase.h
	../../include/engine/units/Launcher.h
	../../include/engine/units/MCV.h
	../../include/engine/units/Ornithopter.h
	../../include/engine/units/Quad.h
	../../include/engine/units/RaiderTrike.h
	../../include/engine/units/Saboteur.h
	../../include/engine/units/SandWorm.h
	../../include/engine/units/SiegeTank.h
	../../include/engine/units/Soldier.h
	../../include/engine/units/SonicTank.h
	../../include/engine/units/Tank.h
	../../include/engine/units/TankBase.h
	../../include/engine/units/TrackedUnit.h
	../../include/engine/units/Trike.h
	../../include/engine/units/Trooper.h
	../../include/engine/units/UnitBase.h
)

set(ALL_SOURCES ${SOURCES} ${PLAYER_SOURCES} ${STRUCTURE_SOURCES} ${TRIGGER_SOURCES} ${UNIT_SOURCES})
set(ALL_HEADERS ${HEADERS} ${PLAYER_HEADERS} ${STRUCTURE_HEADERS} ${TRIGGER_HEADERS} ${UNIT_HEADERS})

add_library(engine ${ALL_SOURCES} ${ALL_HEADERS})
target_link_libraries(engine PUBLIC utility)
target_include_directories(engine PRIVATE ../../include/engine)

target_precompile_headers(engine PRIVATE ../../include/engine/pch.h)

