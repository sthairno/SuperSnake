#include "Config.hpp"
#include <ThirdParty/cereal/types/map.hpp>

static std::map<uint64, KeyConfig> keyConfig;
static HashTable<String, GameSettings> gamePresets;

KeyConfig GetKeyConfig(const GamepadInfo& info)
{
	return keyConfig[GameController::FromGamepadInfo(info).gamepadUid];
}

void SetKeyConfig(const GamepadInfo& info, KeyConfig config)
{
	keyConfig[GameController::FromGamepadInfo(info).gamepadUid] = config;
}

const HashTable<String, GameSettings>& GetGamePresets()
{
	return gamePresets;
}

void SetGamePresets(const HashTable<String, GameSettings>& presets)
{
	gamePresets = presets;
}

void LoadConfig()
{
	Deserializer<BinaryReader> archive(ConfigPath);
	if (archive->isOpen())
	{
		try
		{
			archive(CEREAL_NVP(keyConfig), CEREAL_NVP(gamePresets));
		}
		catch (cereal::Exception)
		{ }
	}
}

void SaveConfig()
{
	Serializer<BinaryWriter> archive(ConfigPath);
	if (archive->isOpen())
	{
		try
		{
			archive(CEREAL_NVP(keyConfig), CEREAL_NVP(gamePresets));
		}
		catch (cereal::Exception)
		{ }
	}
}
