set(SOURCES
	AStarSearch.cpp
	Command.cpp
	CommandManager.cpp
	DuneLegacyEngine.cpp
	engine_sand.cpp
	GameInitSettings.cpp
	ObjectBase.cpp
	ObjectData.cpp
	ObjectManager.cpp
	ObjectPointer.cpp
	FileClasses/INIFile.cpp
	)

set(PLAYERS_SOURCES
	players/AIPlayer.cpp
	players/CampaignAIPlayer.cpp
	players/HumanPlayer.cpp
	players/Player.cpp
	players/PlayerFactory.cpp
	players/QuantBot.cpp
	players/SmartBot.cpp)


set(TRIGGER_SOURCES
	Trigger/ReinforcementTrigger.cpp
	Trigger/TimeoutTrigger.cpp
	Trigger/TriggerManager.cpp
	)

set(HEADERS
	../../include/engine/AStarSearch.h
	../../include/engine/AITeamInfo.h
	../../include/engine/Command.h
	../../include/engine/CommandList.h
	../../include/engine/CommandManager.h
	../../include/engine/data.h
	../../include/engine/DuneLegacyEngine.h
	../../include/engine/EngineDataTypes.h
	../../include/engine/EngineDefinitions.h
	../../include/engine/engine_sand.h
	../../include/engine/GameInitSettings.h
	../../include/engine/ObjectBase.h
	../../include/engine/ObjectData.h
	../../include/engine/ObjectManager.h
	../../include/engine/ObjectPointer.h
	../../include/engine/FileClasses/INIFile.h
	)

set(PLAYERS_HEADERS
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

add_library(engine ${SOURCES} ${PLAYERS_SOURCES} ${HEADERS})
target_link_libraries(engine PUBLIC utility)
target_include_directories(engine PRIVATE ../../include/engine)

target_precompile_headers(engine PRIVATE ../../include/engine/pch.h)

