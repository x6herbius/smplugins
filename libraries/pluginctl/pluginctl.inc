/* =================================================
    PluginCtl - X6Herbius, 11/02/2017
   ================================================= */

#if defined _PLUGINCTL_INC
 #endinput
#endif
#define _PLUGINCTL_INC

#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>

#include "stringextra/stringextra.inc"

#define PCTL_PLUGIN_IDENT_MAX_LENGTH 16

static char _strPluginIdent[PCTL_PLUGIN_IDENT_MAX_LENGTH] = "";
static ConVar _cvPluginVersion = null;
static ConVar _cvPluginEnabled = null;

// This is only set on Initialise(), and should be considered undefined
// if isValid() returns false. This is because a function pointer can't
// be set to null.
static ConVarChanged _cbEnabledChanged;

/**
 * Initialises the global data for this plugin.
 *
 * Translations are loaded automatically. This process assumes that for a plugin with a given IDENT:
 * - Translated phrases are contained within translations/IDENT/IDENT_phrases.txt
 * - Plugin config is located at cfg/sourcemod/IDENT/IDENT.cfg
 *
 * @param strPluginIdent    Unique identifier for this plugin. Cannot be longer than PCTL_PLUGIN_IDENT_MAX_LENGTH.
 * @param strPluginVersion  Version string for this plugin. Recommended format is "maj.min.rev.build".
 * @param cbEnabledChanged  Callback to call when the plugin's enabled state convar is changed.
 *
 * @noreturn
 */
void PCtl_Initialise(const char[] strPluginIdent, const char[] strPluginVersion, ConVarChanged cbEnabledChanged)
{
    if ( isValid() )
    {
        ThrowError("PluginCtl: Already initialised for plugin '%s'.", _strPluginIdent);
    }

    int identLength = strlen(strPluginIdent);
    if ( identLength < 1 || identLength >= PCTL_PLUGIN_IDENT_MAX_LENGTH )
    {
        ThrowError("PluginCtl: Plugin ident '%s' was %s.", strPluginIdent, identLength < 1 ? "empty" : "too long");
    }

    strcopy(_strPluginIdent, sizeof(_strPluginIdent), strPluginIdent);

    char strPluginIdentLower[PCTL_PLUGIN_IDENT_MAX_LENGTH];
    StrEx_ToCase(strPluginIdentLower, sizeof(strPluginIdentLower), _strPluginIdent, StrEx_Lowercase);

    char strBuffer[64];

    Format(strBuffer, sizeof(strBuffer), "%s_enabled", strPluginIdentLower);
    _cvPluginEnabled = CreateConVar(strBuffer, "1", "Enables or disables the plugin.",
        FCVAR_ARCHIVE | FCVAR_NOTIFY,
        true, 0.0,
        true, 1.0);

    Format(strBuffer, sizeof(strBuffer), "%s_version", strPluginIdentLower);
    _cvPluginVersion = CreateConVar(strBuffer, strPluginVersion, "Plugin version.",
        FCVAR_CHEAT);

    _cbEnabledChanged = cbEnabledChanged;
    _cvPluginEnabled.AddChangeHook(_cbEnabledChanged);

    PerformPluginLoadTasks();
}

/**
 * Cleans up the global data for this plugin.
 *
 * @noreturn
 */
void PCtl_Shutdown()
{
    if ( !isValid() )
    {
        return;
    }

    _cvPluginEnabled.RemoveChangeHook(_cbEnabledChanged);

    delete _cvPluginEnabled;
    delete _cvPluginVersion;
}

/**
 * Returns the plugin version convar.
 *
 * @return Version convar.
 */
stock ConVar PCtl_PluginVersion()
{
    if ( !isValid() )
    {
        ThrowError("PluginCtl: Not initialised.");
    }

    return _cvPluginVersion;
}

/**
 * Returns the plugin enabled state convar.
 *
 * @return Enabled state convar.
 */
stock ConVar PCtl_PluginEnabled()
{
    if ( !isValid() )
    {
        ThrowError("PluginCtl: Not initialised.");
    }

    return _cvPluginEnabled;
}

/**
 * Returns the plugin identifier string.
 *
 * @param strBuffer   Output buffer.
 * @param nMaxLength  Length of the output buffer.
 *
 * @noreturn
 */
stock void PCtl_PluginIdent(char[] strBuffer, int nMaxLength)
{
    if ( !isValid() )
    {
        ThrowError("PluginCtl: Not initialised.");
    }

    strcopy(strBuffer, nMaxLength, _strPluginIdent);
}

/**
 * Checks all the required libraries exist, and sets a failure state if any do not.
 * This should be called from OnAllPluginsLoaded().
 *
 * @param libraries Array of library name strings to check for.
 * @param count The total number of strings in the array.
 *
 * @noreturn
 */
stock void PCtl_EnsureDependenciesExist(const char[][] libraries, int count)
{
    for ( int library = 0; library < count; ++library )
    {
        if ( !LibraryExists(libraries[library]) )
        {
            SetFailState("PluginCtl: Required library '%s' not found.", libraries[library]);
        }
    }
}

stock ConVar PCtl_CreatePrefixedConVar(const char[] strName,
                                       const char[] strDefaultValue,
                                       const char[] strDescription = "",
                                       int nFlags = 0,
                                       bool bHasMin = false,
                                       float fMin = 0.0,
                                       bool bHasMax = false,
                                       float fMax = 0.0)
{
    if ( !isValid() )
    {
        ThrowError("PluginCtl: Not initialised.");
    }

    char buffer[PCTL_PLUGIN_IDENT_MAX_LENGTH + 64];
    FormatEx(buffer, sizeof(buffer), "%s_%s", _strPluginIdent, strName);

    ConVar cVar = CreateConVar(buffer, strDefaultValue, strDescription, nFlags, bHasMin, fMin, bHasMax, fMax);

    if ( cVar == null )
    {
        ThrowError("PluginCtl: Unable to create ConVar %s.", buffer);
    }

    return cVar;
}

static stock bool isValid()
{
    return _cvPluginVersion != null;
}

static stock void PerformPluginLoadTasks()
{
    LoadTranslations("common.phrases");

    char buffer[64];

    Format(buffer, sizeof(buffer), "%s/%s_phrases", _strPluginIdent, _strPluginIdent);
    if ( FileExists(buffer, false, NULL_STRING) )
    {
        LoadTranslations(buffer);
    }
    else
    {
        LogMessage("Translations file %s not found, not loading.", buffer);
    }

    Format(buffer, sizeof(buffer), "sourcemod/%s", _strPluginIdent);
    if ( FileExists(buffer, false, NULL_STRING) )
    {
        AutoExecConfig(true, _strPluginIdent, buffer);
    }
    else
    {
        LogMessage("Plugin config %s not found, not executing.", buffer);
    }
}
