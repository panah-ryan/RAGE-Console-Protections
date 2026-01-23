#pragma once
#include "caller.h"

extern Caller* call;

static BOOL IS_THIS_MODEL_A_PED(uint32_t model_hash) { return call->call<BOOL>("IS_THIS_MODEL_A_PED", model_hash); }
static BOOL IS_THIS_MODEL_A_VEHICLE(uint32_t model_hash) { return call->call<BOOL>("IS_THIS_MODEL_A_VEHICLE", model_hash); }
static BOOL IS_THIS_MODEL_A_BIKE(uint32_t model_hash) { return call->call<BOOL>("IS_THIS_MODEL_A_BIKE", model_hash); }
static BOOL IS_THIS_MODEL_A_BOAT(uint32_t model_hash) { return call->call<BOOL>("IS_THIS_MODEL_A_BOAT", model_hash); }
static BOOL IS_THIS_MODEL_A_CAR(uint32_t model_hash) { return call->call<BOOL>("IS_THIS_MODEL_A_CAR", model_hash); }
static BOOL IS_THIS_MODEL_A_HELI(uint32_t model_hash) { return call->call<BOOL>("IS_THIS_MODEL_A_HELI", model_hash); }
static BOOL IS_THIS_MODEL_A_TRAIN(uint32_t model_hash) { return call->call<BOOL>("IS_THIS_MODEL_A_TRAIN", model_hash); }
static void GET_DISTANCE_BETWEEN_COORDS_3D(float x1, float y1, float z1, float x2, float y2, float z2, float* pDist) { call->call<int>("GET_DISTANCE_BETWEEN_COORDS_3D", x1, y1, z1, x2, y2, z2, pDist); }
static void GET_CHAR_COORDINATES(int ped, float* pX, float* pY, float* pZ) { call->call<int>("GET_CHAR_COORDINATES", ped, pX, pY, pZ); }
static void GET_PLAYER_CHAR(int playerIndex, int* pPed) { call->call<int>("GET_PLAYER_CHAR", playerIndex, pPed); }
static void CLEAR_PRINTS() { call->call<int>("CLEAR_PRINTS"); }
static void PRINT_STRING_WITH_LITERAL_STRING_NOW(const char* gxtName, const char* text, int timeMS, int color) { call->call<int>("PRINT_STRING_WITH_LITERAL_STRING_NOW", gxtName, text, timeMS, color); }
static BOOL IS_SCREEN_FADED_OUT() { return call->call<BOOL>("IS_SCREEN_FADED_OUT"); }
static int GET_CURRENT_EPISODE() { return call->call<int>("GET_CURRENT_EPISODE"); }
static int GET_PLAYER_ID() { return call->call<int>("GET_PLAYER_ID"); }
static int GET_HOST_ID() { return call->call<int>("GET_HOST_ID"); }
static uint32_t GET_HASH_KEY(char* key) { return call->call<uint32_t>("GET_HASH_KEY", key); }
static BOOL DOES_OBJECT_EXIST_WITH_NETWORK_ID(int networkId) { return call->call<BOOL>("DOES_OBJECT_EXIST_WITH_NETWORK_ID", networkId); }
static void GET_OBJECT_FROM_NETWORK_ID(int networkId, int* object) { call->call<int>("GET_OBJECT_FROM_NETWORK_ID", networkId, object); }
static void ADD_TO_PREVIOUS_BRIEF(const char* string) { call->call<int>("ADD_TO_PREVIOUS_BRIEF", string); }
static void SET_OBJECT_LIGHTS(int obj, bool lights) { call->call<int>("SET_OBJECT_LIGHTS", lights); }
static void SET_OBJECT_COLLISION(int obj, bool collision) { call->call<int>("SET_OBJECT_COLLISION", collision); }