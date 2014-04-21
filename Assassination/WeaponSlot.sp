/* Plugin Template generated by Pawn Studio */

#include <sourcemod>
#include <sdktools>
#pragma semicolon 1

public Plugin:myinfo = 
{
	name = "Weapon Slot Identifier",
	author = "[X6] Herbius",
	description = "Displays the weapons in a player's weapon slots.",
	version = "1.0",
	url = "<- URL ->"
}

public OnPluginStart()
{
	RegConsoleCmd("say", Command_Say);
}

/* Parses our chat commands.	*/
public Action:Command_Say(client, args)
{
	new String:text[192];
	GetCmdArgString(text, sizeof(text));
 
	new startidx = 0;
	if ( text[0] == '"' )
	{
		startidx = 1;
		// Strip the ending quote, if there is one
		new len = strlen(text);
		if ( text[len-1] == '"' )
		{
			text[len-1] = '\0';
		}
	}
 
	if ( StrEqual(text[startidx], "/slots") )
	{
		// Output the client's weapon info to the chat.
		new slot0_weapon = GetPlayerWeaponSlot(client, 0);
		new slot1_weapon = GetPlayerWeaponSlot(client, 1);
		new slot2_weapon = GetPlayerWeaponSlot(client, 2);
		decl String:classname[64];
		classname[0] = '\0';
		
		GetEntityClassname(slot0_weapon, classname, sizeof(classname));
		PrintToChat(client, "Weapon in primary slot is ID %d, %s", slot0_weapon, classname);
		
		GetEntityClassname(slot1_weapon, classname, sizeof(classname));
		PrintToChat(client, "Weapon in secondary slot is ID %d, %s", slot1_weapon, classname);
		
		GetEntityClassname(slot2_weapon, classname, sizeof(classname));
		PrintToChat(client, "Weapon in melee slot is ID %d, %s", slot2_weapon, classname);
		
		// Block the client's messsage from broadcasting
		return Plugin_Handled;
	}
 
	// Let say continue normally
	return Plugin_Continue;
}