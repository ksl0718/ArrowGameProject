// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "ArrowProjectile.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
class AActor;
class UPrimitiveComponent;
struct FHitResult;
#ifdef ARROWGAME_ArrowProjectile_generated_h
#error "ArrowProjectile.generated.h already included, missing '#pragma once' in ArrowProjectile.h"
#endif
#define ARROWGAME_ArrowProjectile_generated_h

#define FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowProjectile_h_12_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execInitVelocity); \
	DECLARE_FUNCTION(execOnHit);


#define FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowProjectile_h_12_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAArrowProjectile(); \
	friend struct Z_Construct_UClass_AArrowProjectile_Statics; \
public: \
	DECLARE_CLASS(AArrowProjectile, AActor, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/ArrowGame"), NO_API) \
	DECLARE_SERIALIZER(AArrowProjectile)


#define FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowProjectile_h_12_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	AArrowProjectile(AArrowProjectile&&); \
	AArrowProjectile(const AArrowProjectile&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AArrowProjectile); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AArrowProjectile); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(AArrowProjectile) \
	NO_API virtual ~AArrowProjectile();


#define FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowProjectile_h_9_PROLOG
#define FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowProjectile_h_12_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowProjectile_h_12_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowProjectile_h_12_INCLASS_NO_PURE_DECLS \
	FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowProjectile_h_12_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> ARROWGAME_API UClass* StaticClass<class AArrowProjectile>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowProjectile_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
