#include "Config.hpp"
#include <ThirdParty/cereal/types/map.hpp>

static std::map<uint64, KeyConfig> keyConfig;

uint64 calcId(const GamepadInfo& info)
{
	return info.productID | uint64(info.vendorID) << 32;
}

KeyConfig GetKeyConfig(const GamepadInfo& info)
{
	return keyConfig[calcId(info)];
}

void SetKeyConfig(const GamepadInfo& info, KeyConfig config)
{
	keyConfig[calcId(info)] = config;
}

void LoadConfig()
{
	Deserializer<BinaryReader> archive(ConfigPath);
	if (archive->isOpen())
	{
		try
		{
			archive(CEREAL_NVP(keyConfig));
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
			archive(CEREAL_NVP(keyConfig));
		}
		catch (cereal::Exception)
		{ }
	}
}
