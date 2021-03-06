#include "ModuleGame.hpp"
#include <sstream>
#include "../ElDorito.hpp"
#include "../Patches/Ui.hpp"
#include "../Patches/Logging.hpp"
#include "../Blam/BlamTypes.hpp"

namespace
{
	bool CommandGameLogMode(const std::vector<std::string>& Arguments, std::string& returnInfo)
	{
		auto newFlags = Modules::ModuleGame::Instance().DebugFlags;

		if (Arguments.size() > 0)
		{
			for (auto arg : Arguments)
			{
				if (arg.compare("off") == 0)
				{
					// Disable it.
					newFlags = 0;

					Patches::Logging::EnableNetworkLog(false);
					Patches::Logging::EnableSslLog(false);
					Patches::Logging::EnableUiLog(false);
					Patches::Logging::EnableGame1Log(false);
					Patches::Logging::EnableGame2Log(false);
				}
				else
				{
					auto hookNetwork = arg.compare("network") == 0;
					auto hookSSL = arg.compare("ssl") == 0;
					auto hookUI = arg.compare("ui") == 0;
					auto hookGame1 = arg.compare("game1") == 0;
					auto hookGame2 = arg.compare("game2") == 0;
					if (arg.compare("all") == 0 || arg.compare("on") == 0)
						hookNetwork = hookSSL = hookUI = hookGame1 = hookGame2 = true;

					if (hookNetwork)
					{
						newFlags |= DebugLoggingModes::eDebugLoggingModeNetwork;
						Patches::Logging::EnableNetworkLog(true);
					}

					if (hookSSL)
					{
						newFlags |= DebugLoggingModes::eDebugLoggingModeSSL;
						Patches::Logging::EnableSslLog(true);
					}

					if (hookUI)
					{
						newFlags |= DebugLoggingModes::eDebugLoggingModeUI;
						Patches::Logging::EnableUiLog(true);
					}

					if (hookGame1)
					{
						newFlags |= DebugLoggingModes::eDebugLoggingModeGame1;
						Patches::Logging::EnableGame1Log(true);
					}

					if (hookGame2)
					{
						newFlags |= DebugLoggingModes::eDebugLoggingModeGame2;
						Patches::Logging::EnableGame2Log(true);
					}
				}
			}
		}

		Modules::ModuleGame::Instance().DebugFlags = newFlags;

		std::stringstream ss;
		ss << "Debug logging: ";
		if (newFlags == 0)
			ss << "disabled";
		else
		{
			ss << "enabled: ";
			if (newFlags & DebugLoggingModes::eDebugLoggingModeNetwork)
				ss << "Network ";
			if (newFlags & DebugLoggingModes::eDebugLoggingModeSSL)
				ss << "SSL ";
			if (newFlags & DebugLoggingModes::eDebugLoggingModeUI)
				ss << "UI ";
			if (newFlags & DebugLoggingModes::eDebugLoggingModeGame1)
				ss << "Game1 ";
			if (newFlags & DebugLoggingModes::eDebugLoggingModeGame2)
				ss << "Game2 ";
		}
		if (Arguments.size() <= 0)
		{
			ss << std::endl << "Usage: Game.DebugMode <network | ssl | ui | game1 | game2 | all | off>";
		}
		returnInfo = ss.str();
		return true;
	}

	bool CommandGameLogFilter(const std::vector<std::string>& Arguments, std::string& returnInfo)
	{
		std::stringstream ss;
		if (Arguments.size() != 3)
		{
			ss << "Usage: Game.LogFilter <include/exclude> <add/remove> <string>" << std::endl << std::endl;
		}
		else
		{
			bool exclude = (Arguments[0].compare("exclude") == 0);
			bool remove = Arguments[1].compare("remove") == 0;
			auto vect = exclude ? &Modules::ModuleGame::Instance().FiltersExclude : &Modules::ModuleGame::Instance().FiltersInclude;
			auto str = Arguments[2];
			bool exists = std::find(vect->begin(), vect->end(), str) != vect->end();
			if (remove && !exists)
			{
				ss << "Failed to find string \"" << str << "\" inside " << (exclude ? "exclude" : "include") << " filters list" << std::endl << std::endl;
			}
			else if (remove)
			{
				auto pos = std::find(vect->begin(), vect->end(), str);
				vect->erase(pos);
				ss << "Removed \"" << str << "\" from " << (exclude ? "exclude" : "include") << " filters list" << std::endl << std::endl;
			}
			else
			{
				vect->push_back(str);
				ss << "Added \"" << str << "\" to " << (exclude ? "exclude" : "include") << " filters list" << std::endl << std::endl;
			}
		}

		ss << "Include filters (message must contain these strings):";

		for (auto filter : Modules::ModuleGame::Instance().FiltersInclude)
			ss << std::endl << filter;

		ss << std::endl << std::endl << "Exclude filters (message must not contain these strings):";

		for (auto filter : Modules::ModuleGame::Instance().FiltersExclude)
			ss << std::endl << filter;

		returnInfo = ss.str();
		return true;
	}

	bool CommandGameInfo(const std::vector<std::string>& Arguments, std::string& returnInfo)
	{
		std::stringstream ss;
		std::string ArgList((char*)Pointer(0x199C0A4)[0]);
		std::string LocalSecureKey((char*)Pointer(0x50CCDB4 + 1));
		std::string Xnkid;
		std::string Xnaddr;
		std::string Build((char*)Pointer(0x199C0F0));
		std::string SystemID((char*)Pointer(0x199C130));
		std::string SessionID((char*)Pointer(0x199C1D0));
		std::string DevKitName((char*)Pointer(0x160C8C8));
		std::string DevKitVersion((char*)Pointer(0x199C350));
		std::string FlashVersion((char*)Pointer(0x199C350));
		std::string MapName((char*)Pointer(0x22AB018)(0x1A4));
		std::wstring VariantName((wchar_t*)Pointer(0x23DAF4C));

		Utils::String::BytesToHexString((char*)Pointer(0x2247b80), 0x10, Xnkid);
		Utils::String::BytesToHexString((char*)Pointer(0x2247b90), 0x10, Xnaddr);

		ss << std::hex << "ThreadLocalStorage: 0x" << std::hex << (size_t)(void*)ElDorito::GetMainTls() << std::endl;

		ss << "Command line args: " << (ArgList.empty() ? "(null)" : ArgList) << std::endl;
		ss << "Local Secure Key: " << (LocalSecureKey.empty() ? "(null)" : LocalSecureKey) << std::endl;
		ss << "XNKID: " << Xnkid << std::endl;
		ss << "XNAddr: " << Xnaddr << std::endl;
		ss << "Server Port: " << std::dec << Modules::ModuleServer::Instance().VarServerPort->ValueInt << std::endl;
		ss << "Server Endpoint Port: " << std::dec << Pointer(0x1860454).Read<uint32_t>() << std::endl;
		ss << "Build: " << (Build.empty() ? "(null)" : Build) << std::endl;
		ss << "SystemID: " << (SystemID.empty() ? "(null)" : SystemID) << std::endl;
		ss << "SessionID: " << (SessionID.empty() ? "(null)" : SessionID) << std::endl;
		ss << "SDK Info: " << (DevKitName.empty() ? "(null)" : DevKitName) << '|';
		ss << (DevKitVersion.empty() ? "(null)" : DevKitVersion) << std::endl;

		ss << "Flash Version: " << (FlashVersion.empty() ? "(null)" : FlashVersion) << std::endl;
		ss << "Current Map: " << (MapName.empty() ? "(null)" : MapName) << std::endl;
		ss << "Current Map Cache Size: " << std::dec << Pointer(0x22AB018)(0x8).Read<int32_t>() << std::endl;
		ss << "Loaded Game Variant: " << (VariantName.empty() ? "(null)" : Utils::String::ThinString(VariantName)) << std::endl;
		ss << "Loaded Game Type: 0x" << std::hex << Pointer(0x023DAF18).Read<int32_t>() << std::endl;
		ss << "Tag Table Offset: 0x" << std::hex << Pointer(0x22AAFF4).Read<uint32_t>() << std::endl;
		ss << "Tag Bank Offset: 0x" << std::hex << Pointer(0x22AAFF8).Read<uint32_t>() << std::endl;
		ss << "Players global addr: 0x" << std::hex << ElDorito::GetMainTls(GameGlobals::Players::TLSOffset).Read<uint32_t>() << std::endl;

		returnInfo = ss.str();
		return true;
	}

	bool CommandGameExit(const std::vector<std::string>& Arguments, std::string& returnInfo)
	{
		std::exit(0);
		return true;
	}

	bool CommandGameLoadMap(const std::vector<std::string>& Arguments, std::string& returnInfo)
	{
		std::stringstream ss;
		if (Arguments.size() <= 0)
		{
			ss << "Current map: " << std::string((char*)(0x2391824)) << std::endl;
			ss << "Usage: Game.Map <mapname> [gametype] [maptype]" << std::endl;
			ss << "Available maps:";

			for (auto map : Modules::ModuleGame::Instance().MapList)
				ss << std::endl << "\t" << map;

			ss << std::endl << std::endl << "Valid gametypes:";
			for (size_t i = 0; i < Blam::GameTypeCount - 1; i++)
				ss << std::endl << "\t" << "[" << i << "] " << Blam::GameTypeNames[i];

			returnInfo = ss.str();
			return false;
		}

		auto mapName = Arguments[0];

		auto gameTypeStr = "none";
		Blam::GameType gameType = Blam::GameType::None;
		Blam::GameMode gameMode = Blam::GameMode::Multiplayer;

		if (std::find(Modules::ModuleGame::Instance().MapList.begin(), Modules::ModuleGame::Instance().MapList.end(), mapName) == Modules::ModuleGame::Instance().MapList.end())
		{
			returnInfo = "Unable to find map " + mapName;
			return false;
		}

		mapName = "maps\\" + mapName;

		if (Arguments.size() >= 2)
		{
			//Look up gametype string.
			size_t i;
			for (i = 0; i < Blam::GameTypeCount; i++)
			{
				// Todo: case insensiive
				if (!Blam::GameTypeNames[i].compare(Arguments[1]))
				{
					gameType = Blam::GameType(i);
					break;
				}
			}

			if (i == Blam::GameTypeCount)
				gameType = (Blam::GameType)std::atoi(Arguments[1].c_str());

			if (gameType > Blam::GameTypeCount) // only valid gametypes are 1 to 10
				gameType = Blam::GameType::Slayer;
		}

		if (Arguments.size() >= 3)
		{
			//Look up gamemode string.
			size_t i;
			for (i = 0; i < Blam::GameModeCount; i++)
			{
				// Todo: case insensiive
				if (!Blam::GameModeNames[i].compare(Arguments[2]))
				{
					gameMode = Blam::GameMode(i);
					break;
				}
			}

			if (i == Blam::GameModeCount)
				gameMode = (Blam::GameMode)std::atoi(Arguments[2].c_str());

			if (gameMode > Blam::GameModeCount) // only valid gametypes are 1 to 10
				gameMode = Blam::GameMode::Multiplayer;
		}

		ss << "Loading " << mapName << " gametype: " << Blam::GameTypeNames[gameType] << " gamemode: " << Blam::GameModeNames[gameMode];

		Pointer(0x2391B2C).Write<uint32_t>(gameType);

		// Infinite play time
		Pointer(0x2391C51).Write<uint8_t>(0);

		// Game Mode
		Pointer(0x2391800).Write<uint32_t>(gameMode);

		// Map Name
		Pointer(0x2391824).Write(mapName.c_str(), mapName.length() + 1);

		// Map Reset
		Pointer(0x23917F0).Write<uint8_t>(0x1);

		returnInfo = ss.str();
		return true;
	}

	bool CommandGameShowUI(const std::vector<std::string>& Arguments, std::string& returnInfo)
	{
		if (Arguments.size() <= 0)
		{
			returnInfo = "You must specify at least a dialog ID!";
			return false;
		}

		unsigned int dialogStringId = 0;
		int dialogArg1 = 0; // todo: figure out a better name for this
		int dialogFlags = 4;
		unsigned int dialogParentStringId = 0x1000C;
		try
		{
			dialogStringId = std::stoul(Arguments[0], 0, 0);
			if (Arguments.size() > 1)
				dialogArg1 = std::stoi(Arguments[1], 0, 0);
			if (Arguments.size() > 2)
				dialogFlags = std::stoi(Arguments[2], 0, 0);
			if (Arguments.size() > 3)
				dialogParentStringId = std::stoul(Arguments[3], 0, 0);
		}
		catch (std::invalid_argument)
		{
			returnInfo = "Invalid argument given.";
			return false;
		}
		catch (std::out_of_range)
		{
			returnInfo = "Invalid argument given.";
			return false;
		}

		Patches::Ui::DialogStringId = dialogStringId;
		Patches::Ui::DialogArg1 = dialogArg1;
		Patches::Ui::DialogFlags = dialogFlags;
		Patches::Ui::DialogParentStringId = dialogParentStringId;
		Patches::Ui::DialogShow = true;

		returnInfo = "Sent Show_UI notification to game.";
		return true;
	}

	//EXAMPLE:
	/*std::string VariableGameNameUpdate(const std::vector<std::string>& Arguments)
	{
		// arguments are only passed to variable update funcs in case they need them for something
		// CommandMap already updated the value of the variable so we'll just use that
		std::string name;
		if (!Modules::ModuleGame::Instance().GetVariableString("Name", name))
			// we could also call Modules::CommandMap::Instance()->GetVariableString("Game.Name", name);
			return ""; // should never happen

		return std::string("Our name is ") + name;
	}*/
}

namespace Modules
{
	ModuleGame::ModuleGame() : ModuleBase("Game")
	{
		AddCommand("LogMode", "debug", "Chooses which debug messages to print to the log file", eCommandFlagsNone, CommandGameLogMode, { "network|ssl|ui|game1|game2|all|off The log mode to enable" });

		AddCommand("LogFilter", "debug_filter", "Allows you to set filters to apply to the debug messages", eCommandFlagsNone, CommandGameLogFilter, { "include/exclude The type of filter", "add/remove Add or remove the filter", "string The filter to add" });

		AddCommand("Info", "info", "Displays information about the game", eCommandFlagsNone, CommandGameInfo);

		AddCommand("Exit", "exit", "Ends the game process", eCommandFlagsNone, CommandGameExit);

		AddCommand("Map", "map", "Loads a map", eCommandFlagsNone, CommandGameLoadMap, { "mapname(string) The name of the map to load", "gametype(int) The gametype to load", "gamemode(int) The type of gamemode to play", });

		AddCommand("ShowUI", "show_ui", "Attempts to force a UI widget to open", eCommandFlagsNone, CommandGameShowUI, { "dialogID(int) The dialog ID to open", "arg1(int) Unknown argument", "flags(int) Unknown argument", "parentdialogID(int) The ID of the parent dialog" });

		VarLanguageID = AddVariableInt("LanguageID", "languageid", "The index of the language to use", eCommandFlagsArchived, 0);
		VarLanguageID->ValueIntMin = 0;
		VarLanguageID->ValueIntMax = 11;

		VarSkipLauncher = AddVariableInt("SkipLauncher", "launcher", "Skip requiring the launcher", eCommandFlagsArchived, 0);
		VarSkipLauncher->ValueIntMin = 0;
		VarSkipLauncher->ValueIntMax = 0;

		VarLogName = AddVariableString("LogName", "debug_logname", "Filename to store debug log messages", eCommandFlagsArchived, "dorito.log");

		// Level load patch
		Patch::NopFill(Pointer::Base(0x2D26DF), 5);

		//populate map list on load
		WIN32_FIND_DATA Finder;
		HANDLE hFind = FindFirstFile((ElDorito::Instance().GetDirectory() + "\\maps\\*.map").c_str(), &Finder);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (!(Finder.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					std::string MapName(Finder.cFileName);
					//remove extension
					MapList.push_back(MapName.substr(0, MapName.find_last_of('.')));
				}
			} while (FindNextFile(hFind, &Finder) != 0);
		}

		/*EXAMPLES: adds a variable "Game.Name", default value ElDewrito, calls VariableGameNameUpdate when value is updated
		AddVariableString("Name", "gamename", "Title of the game", "ElDewrito", VariableGameNameUpdate);

		// adds a variable "Game.Year", default value 2015, clamped to [1994..3030]
		auto cmd = AddVariableInt("Year", "gameyear", "The current year", 2015);
		cmd->ValueIntMin = 1994;
		cmd->ValueIntMax = 3030;

		// adds a variable "Game.Money", default value 1.86
		AddVariableFloat("Money", "gamemoney", "Your mothers hourly rate", 1.86f);*/
	}
}