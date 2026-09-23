/************************************************************************************************
 *  S2Dumper is a dumper for various properties of Source2-based games.
 *  Copyright (C) 2026 Sava Andrei-Sebastian and it's contributors
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 ************************************************************************************************/

#include <map>
#include <string>
#include "public/icvar.h"
#include "public/datamap.h"

std::map<uint64_t, std::string> g_mFlags = {
    {FCVAR_LINKED_CONCOMMAND, "linked_concommand"},
	{FCVAR_DEVELOPMENTONLY, "developmentonly"},
	{FCVAR_GAMEDLL, "gamedll"},
	{FCVAR_CLIENTDLL, "clientdll"},
	{FCVAR_HIDDEN, "hidden"},
	{FCVAR_PROTECTED, "protected"},
	{FCVAR_SPONLY, "sponly"},
	{FCVAR_ARCHIVE, "archive"},
	{FCVAR_NOTIFY, "notify"},
	{FCVAR_USERINFO, "userinfo"},
	{FCVAR_REFERENCE, "reference"},
	{FCVAR_UNLOGGED, "unlogged"},
	{FCVAR_INITIAL_SETVALUE, "initial_setvalue"},
	{FCVAR_REPLICATED, "replicated"},
	{FCVAR_CHEAT, "cheat"},
	{FCVAR_PER_USER, "per_user"},
	{FCVAR_DEMO, "demo"},
	{FCVAR_DONTRECORD, "dontrecord"},
	{FCVAR_PERFORMING_CALLBACKS, "performing_callbacks"},
	{FCVAR_RELEASE, "release"},
	{FCVAR_MENUBAR_ITEM, "menubar_item"},
	{FCVAR_COMMANDLINE_ENFORCED, "commandline_enforced"},
	{FCVAR_NOT_CONNECTED, "notconnected"},
	{FCVAR_VCONSOLE_FUZZY_MATCHING, "vconsole_fuzzy_matching"},
	{FCVAR_SERVER_CAN_EXECUTE, "server_can_execute"},
	{FCVAR_CLIENT_CAN_EXECUTE, "client_can_execute"},
	{FCVAR_SERVER_CANNOT_QUERY, "server_cannot_query"},
	{FCVAR_VCONSOLE_SET_FOCUS, "vconsole_set_focus"},
	{FCVAR_CLIENTCMD_CAN_EXECUTE, "clientcmd_can_execute"},
	{FCVAR_EXECUTE_PER_TICK, "execute_per_tick"},
	{FCVAR_SNAPSHOT_IGNORED, "snapshot_ignored"},
	{FCVAR_GAMEINFO_CANNOT_OVERRIDE, "gameinfo_cannot_override"},
	{FCVAR_DEFENSIVE, "defensive"}
};

std::map<fieldtype_t, std::string> g_mFieldTypes = {
    {FIELD_VOID, "void"},
    {FIELD_FLOAT32, "float"},
    {FIELD_STRING, "string"},
    {FIELD_VECTOR, "vector"},
    {FIELD_QUATERNION, "quaternion"},
    {FIELD_INT32, "int"},
    {FIELD_BOOLEAN, "boolean"},
    {FIELD_INT16, "int"},
    {FIELD_CHARACTER, "char"},
    {FIELD_COLOR32, "color32"},
    {FIELD_EMBEDDED, "embedded"},
    {FIELD_CUSTOM, "custom"},
    {FIELD_CLASSPTR, "classptr"},
    {FIELD_EHANDLE, "ehandle"},
    {FIELD_POSITION_VECTOR, "position_vector"},
    {FIELD_TIME, "time"},
    {FIELD_TICK, "tick"},
    {FIELD_SOUNDNAME, "soundname"},
    {FIELD_INPUT, "input"},
    {FIELD_FUNCTION, "function"},
    {FIELD_VMATRIX, "vmatrix"},
    {FIELD_VMATRIX_WORLDSPACE, "vmatrix_worldspace"},
    {FIELD_MATRIX3X4_WORLDSPACE, "matrix3x4_worldspace"},
    {FIELD_INTERVAL, "interval"},
    {FIELD_UNUSED, "unused"},
    {FIELD_VECTOR2D, "vector2d"},
    {FIELD_INT64, "int"},
    {FIELD_VECTOR4D, "vector4d"},
    {FIELD_RESOURCE, "resource"},
    {FIELD_TYPEUNKNOWN, "typeunknown"},
    {FIELD_CSTRING, "cstring"},
    {FIELD_HSCRIPT, "hscript"},
    {FIELD_VARIANT, "variant"},
    {FIELD_UINT64, "uint64"},
    {FIELD_FLOAT64, "float64"},
    {FIELD_POSITIVEINTEGER_OR_NULL, "positiveinteger_or_null"},
    {FIELD_HSCRIPT_NEW_INSTANCE, "hscript_new_instance"},
    {FIELD_UINT32, "uint32"},
    {FIELD_UTLSTRINGTOKEN, "utlstringtoken"},
    {FIELD_QANGLE, "qangle"},
    {FIELD_NETWORK_ORIGIN_CELL_QUANTIZED_VECTOR, "network_origin_cell_quantized_vector"},
    {FIELD_HMATERIAL, "hmaterial"},
    {FIELD_HMODEL, "hmodel"},
    {FIELD_NETWORK_QUANTIZED_VECTOR, "network_quantized_vector"},
    {FIELD_NETWORK_QUANTIZED_FLOAT, "network_quantized_float"},
    {FIELD_DIRECTION_VECTOR_WORLDSPACE, "direction_vector_worldspace"},
    {FIELD_QANGLE_WORLDSPACE, "qangle_worldspace"},
    {FIELD_QUATERNION_WORLDSPACE, "quaternion_worldspace"},
    {FIELD_HSCRIPT_LIGHTBINDING, "hscript_lightbinding"},
    {FIELD_V8_VALUE, "v8_value"},
    {FIELD_V8_OBJECT, "v8_object"},
    {FIELD_V8_ARRAY, "v8_array"},
    {FIELD_V8_CALLBACK_INFO, "v8_callback_info"},
    {FIELD_UTLSTRING, "utlstring"},
    {FIELD_NETWORK_ORIGIN_CELL_QUANTIZED_POSITION_VECTOR, "network_origin_cell_quantized_position_vector"},
    {FIELD_HRENDERTEXTURE, "hrendertexture"},
    {FIELD_HPARTICLESYSTEMDEFINITION, "hparticlesystemdefinition"},
    {FIELD_UINT8, "uint8"},
    {FIELD_UINT16, "uint16"},
    {FIELD_CTRANSFORM, "ctransform"},
    {FIELD_CTRANSFORM_WORLDSPACE, "ctransform_worldspace"},
    {FIELD_HPOSTPROCESSING, "hpostprocessing"},
    {FIELD_MATRIX3X4, "matrix3x4"},
    {FIELD_SHIM, "shim"},
    {FIELD_CMOTIONTRANSFORM, "cmotiontransform"},
    {FIELD_CMOTIONTRANSFORM_WORLDSPACE, "cmotiontransform_worldspace"},
    {FIELD_ATTACHMENT_HANDLE, "attachment_handle"},
    {FIELD_AMMO_INDEX, "ammo_index"},
    {FIELD_CONDITION_ID, "condition_id"},
    {FIELD_AI_SCHEDULE_BITS, "ai_schedule_bits"},
    {FIELD_MODIFIER_HANDLE, "modifier_handle"},
    {FIELD_ROTATION_VECTOR, "rotation_vector"},
    {FIELD_ROTATION_VECTOR_WORLDSPACE, "rotation_vector_worldspace"},
    {FIELD_HVDATA, "hvdata"},
    {FIELD_SCALE32, "scale32"},
    {FIELD_STRING_AND_TOKEN, "string_and_token"},
    {FIELD_ENGINE_TIME, "engine_time"},
    {FIELD_ENGINE_TICK, "engine_tick"},
    {FIELD_WORLD_GROUP_ID, "world_group_id"},
    {FIELD_GLOBALSYMBOL, "globalsymbol"},
    {FIELD_HNMGRAPHDEFINITION, "hnmgraphdefinition"},
    {FIELD_TYPECOUNT, "typecount"},
};