// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "ArrowGame/UserCharacter.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeUserCharacter() {}

// Begin Cross Module References
ARROWGAME_API UClass* Z_Construct_UClass_AArrowCharacter();
ARROWGAME_API UClass* Z_Construct_UClass_AUserCharacter();
ARROWGAME_API UClass* Z_Construct_UClass_AUserCharacter_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_UCameraComponent_NoRegister();
ENHANCEDINPUT_API UClass* Z_Construct_UClass_UInputAction_NoRegister();
ENHANCEDINPUT_API UClass* Z_Construct_UClass_UInputMappingContext_NoRegister();
UPackage* Z_Construct_UPackage__Script_ArrowGame();
// End Cross Module References

// Begin Class AUserCharacter
void AUserCharacter::StaticRegisterNativesAUserCharacter()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AUserCharacter);
UClass* Z_Construct_UClass_AUserCharacter_NoRegister()
{
	return AUserCharacter::StaticClass();
}
struct Z_Construct_UClass_AUserCharacter_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "HideCategories", "Navigation" },
		{ "IncludePath", "UserCharacter.h" },
		{ "ModuleRelativePath", "UserCharacter.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DefaultMappingContext_MetaData[] = {
		{ "Category", "Input" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Enhanced Input\n" },
#endif
		{ "ModuleRelativePath", "UserCharacter.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Enhanced Input" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_MoveAction_MetaData[] = {
		{ "Category", "Input" },
		{ "ModuleRelativePath", "UserCharacter.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_LookAction_MetaData[] = {
		{ "Category", "Input" },
		{ "ModuleRelativePath", "UserCharacter.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_JumpAction_MetaData[] = {
		{ "Category", "Input" },
		{ "ModuleRelativePath", "UserCharacter.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_AimAction_MetaData[] = {
		{ "Category", "Input" },
		{ "ModuleRelativePath", "UserCharacter.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ShootAction_MetaData[] = {
		{ "Category", "Input" },
		{ "ModuleRelativePath", "UserCharacter.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_FollowCamera_MetaData[] = {
		{ "Category", "Camera" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xc4\xab\xef\xbf\xbd\xde\xb6\xef\xbf\xbd\n" },
#endif
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "UserCharacter.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xc4\xab\xef\xbf\xbd\xde\xb6\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_NormalFOV_MetaData[] = {
		{ "Category", "Camera" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// FOV \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "UserCharacter.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "FOV \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_AimFOV_MetaData[] = {
		{ "Category", "Camera" },
		{ "ModuleRelativePath", "UserCharacter.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_AimInterpSpeed_MetaData[] = {
		{ "Category", "Camera" },
		{ "ModuleRelativePath", "UserCharacter.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_MaxChargeTime_MetaData[] = {
		{ "Category", "Arrow|Charge" },
		{ "ModuleRelativePath", "UserCharacter.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_TiredThreshold_MetaData[] = {
		{ "Category", "Arrow|Charge" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xef\xbf\xbd\xd6\xb4\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xc2\xa1 \xef\xbf\xbd\xc3\xb0\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "UserCharacter.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xef\xbf\xbd\xd6\xb4\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xc2\xa1 \xef\xbf\xbd\xc3\xb0\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_AutoReleaseTime_MetaData[] = {
		{ "Category", "Arrow|Charge" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "UserCharacter.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_MinArrowSpeed_MetaData[] = {
		{ "Category", "Arrow|Charge" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xdf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "UserCharacter.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xdf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_MaxArrowSpeed_MetaData[] = {
		{ "Category", "Arrow|Charge" },
		{ "ModuleRelativePath", "UserCharacter.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_DefaultMappingContext;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_MoveAction;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_LookAction;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_JumpAction;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_AimAction;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_ShootAction;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_FollowCamera;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_NormalFOV;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_AimFOV;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_AimInterpSpeed;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_MaxChargeTime;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_TiredThreshold;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_AutoReleaseTime;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_MinArrowSpeed;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_MaxArrowSpeed;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AUserCharacter>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_DefaultMappingContext = { "DefaultMappingContext", nullptr, (EPropertyFlags)0x0020080000000015, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, DefaultMappingContext), Z_Construct_UClass_UInputMappingContext_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DefaultMappingContext_MetaData), NewProp_DefaultMappingContext_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_MoveAction = { "MoveAction", nullptr, (EPropertyFlags)0x0020080000000015, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, MoveAction), Z_Construct_UClass_UInputAction_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_MoveAction_MetaData), NewProp_MoveAction_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_LookAction = { "LookAction", nullptr, (EPropertyFlags)0x0020080000000015, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, LookAction), Z_Construct_UClass_UInputAction_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_LookAction_MetaData), NewProp_LookAction_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_JumpAction = { "JumpAction", nullptr, (EPropertyFlags)0x0020080000000015, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, JumpAction), Z_Construct_UClass_UInputAction_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_JumpAction_MetaData), NewProp_JumpAction_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_AimAction = { "AimAction", nullptr, (EPropertyFlags)0x0020080000000015, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, AimAction), Z_Construct_UClass_UInputAction_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_AimAction_MetaData), NewProp_AimAction_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_ShootAction = { "ShootAction", nullptr, (EPropertyFlags)0x0020080000000015, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, ShootAction), Z_Construct_UClass_UInputAction_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ShootAction_MetaData), NewProp_ShootAction_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_FollowCamera = { "FollowCamera", nullptr, (EPropertyFlags)0x00200800000a001d, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, FollowCamera), Z_Construct_UClass_UCameraComponent_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_FollowCamera_MetaData), NewProp_FollowCamera_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_NormalFOV = { "NormalFOV", nullptr, (EPropertyFlags)0x0020080000000001, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, NormalFOV), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_NormalFOV_MetaData), NewProp_NormalFOV_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_AimFOV = { "AimFOV", nullptr, (EPropertyFlags)0x0020080000000001, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, AimFOV), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_AimFOV_MetaData), NewProp_AimFOV_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_AimInterpSpeed = { "AimInterpSpeed", nullptr, (EPropertyFlags)0x0020080000000001, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, AimInterpSpeed), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_AimInterpSpeed_MetaData), NewProp_AimInterpSpeed_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_MaxChargeTime = { "MaxChargeTime", nullptr, (EPropertyFlags)0x0020080000000001, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, MaxChargeTime), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_MaxChargeTime_MetaData), NewProp_MaxChargeTime_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_TiredThreshold = { "TiredThreshold", nullptr, (EPropertyFlags)0x0020080000000001, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, TiredThreshold), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_TiredThreshold_MetaData), NewProp_TiredThreshold_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_AutoReleaseTime = { "AutoReleaseTime", nullptr, (EPropertyFlags)0x0020080000000001, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, AutoReleaseTime), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_AutoReleaseTime_MetaData), NewProp_AutoReleaseTime_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_MinArrowSpeed = { "MinArrowSpeed", nullptr, (EPropertyFlags)0x0020080000000001, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, MinArrowSpeed), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_MinArrowSpeed_MetaData), NewProp_MinArrowSpeed_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AUserCharacter_Statics::NewProp_MaxArrowSpeed = { "MaxArrowSpeed", nullptr, (EPropertyFlags)0x0020080000000001, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AUserCharacter, MaxArrowSpeed), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_MaxArrowSpeed_MetaData), NewProp_MaxArrowSpeed_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AUserCharacter_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_DefaultMappingContext,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_MoveAction,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_LookAction,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_JumpAction,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_AimAction,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_ShootAction,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_FollowCamera,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_NormalFOV,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_AimFOV,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_AimInterpSpeed,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_MaxChargeTime,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_TiredThreshold,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_AutoReleaseTime,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_MinArrowSpeed,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AUserCharacter_Statics::NewProp_MaxArrowSpeed,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AUserCharacter_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_AUserCharacter_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AArrowCharacter,
	(UObject* (*)())Z_Construct_UPackage__Script_ArrowGame,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AUserCharacter_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AUserCharacter_Statics::ClassParams = {
	&AUserCharacter::StaticClass,
	"Game",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_AUserCharacter_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_AUserCharacter_Statics::PropPointers),
	0,
	0x009000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AUserCharacter_Statics::Class_MetaDataParams), Z_Construct_UClass_AUserCharacter_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AUserCharacter()
{
	if (!Z_Registration_Info_UClass_AUserCharacter.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AUserCharacter.OuterSingleton, Z_Construct_UClass_AUserCharacter_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AUserCharacter.OuterSingleton;
}
template<> ARROWGAME_API UClass* StaticClass<AUserCharacter>()
{
	return AUserCharacter::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(AUserCharacter);
AUserCharacter::~AUserCharacter() {}
// End Class AUserCharacter

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacter_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AUserCharacter, AUserCharacter::StaticClass, TEXT("AUserCharacter"), &Z_Registration_Info_UClass_AUserCharacter, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AUserCharacter), 2094391891U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacter_h_1788003554(TEXT("/Script/ArrowGame"),
	Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacter_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacter_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
