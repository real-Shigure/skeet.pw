#include "discord_rpc.h"
#include "discord_register.h"
#pragma comment(lib, "discord-rpc.lib")
#include <iostream>;
#include <time.h>
#include <wtypes.h>
using namespace std;
DiscordRichPresence discordPresence;
const char* token = "768309659646951434"; //not necessary
void UpdatePresence()
{
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.state = "Under our Title";
	discordPresence.details = "Details Text";
	discordPresence.startTimestamp = time(0); //initlialize time
	discordPresence.largeImageKey = "large image"; //large image file name no extension
	discordPresence.largeImageText = "hitting p ";
	discordPresence.smallImageKey = "small image"; //same as large
	discordPresence.smallImageText = "cute for looking ;)"; //displays on hover
	Discord_UpdatePresence(&discordPresence); //do the do
}

void Initialize()
{
	DiscordEventHandlers handlers;
	std::memset(&handlers, 0, sizeof(handlers));
	Discord_Initialize("768309659646951434", &handlers, TRUE, nullptr);
}

 void Shutdown()
{
	Discord_Shutdown(); //goodbye
}

