// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "UserCharacter.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef ARROWGAME_UserCharacter_generated_h
#error "UserCharacter.generated.h already included, missing '#pragma once' in UserCharacter.h"
#endif
#define ARROWGAME_UserCharacter_generated_h

#define FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacter_h_16_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAUserCharacter(); \
	friend struct Z_Construct_UClass_AUserCharacter_Statics; \
public: \
	DECLARE_CLASS(AUserCharacter, AArrowCharacter, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/ArrowGame"), NO_API) \
	DECLARE_SERIALIZER(AUserCharacter)


#define FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacter_h_16_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	AUserCharacter(AUserCharacter&&); \
	AUserCharacter(const AUserCharacter&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AUserCharacter); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AUserCharacter); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(AUserCharacter) \
	NO_API virtual ~AUserCharacter();


#define FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacter_h_13_PROLOG
#define FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacter_h_16_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacter_h_16_INCLASS_NO_PURE_DECLS \
	FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacter_h_16_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> ARROWGAME_API UClass* StaticClass<class AUserCharacter>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacter_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
