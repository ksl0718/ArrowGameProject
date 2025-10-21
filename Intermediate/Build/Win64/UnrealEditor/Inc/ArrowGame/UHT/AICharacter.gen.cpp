// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "ArrowGame/AICharacter.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeAICharacter() {}

// Begin Cross Module References
ARROWGAME_API UClass* Z_Construct_UClass_AAICharacter();
ARROWGAME_API UClass* Z_Construct_UClass_AAICharacter_NoRegister();
ARROWGAME_API UClass* Z_Construct_UClass_AArrowCharacter();
ENGINE_API UClass* Z_Construct_UClass_AActor_NoRegister();
UPackage* Z_Construct_UPackage__Script_ArrowGame();
// End Cross Module References

// Begin Class AAICharacter
void AAICharacter::StaticRegisterNativesAAICharacter()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AAICharacter);
UClass* Z_Construct_UClass_AAICharacter_NoRegister()
{
	return AAICharacter::StaticClass();
}
struct Z_Construct_UClass_AAICharacter_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "HideCategories", "Navigation" },
		{ "IncludePath", "AICharacter.h" },
		{ "ModuleRelativePath", "AICharacter.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_AttackInterval_MetaData[] = {
		{ "Category", "AI" },
		{ "ModuleRelativePath", "AICharacter.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CurrentTarget_MetaData[] = {
		{ "Category", "AI" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "//\xc5\xb8\xef\xbf\xbd\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "AICharacter.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xc5\xb8\xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_AttackRange_MetaData[] = {
		{ "Category", "AI" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "//\xef\xbf\xbd\xef\xbf\xbd\xc5\xb8\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "AICharacter.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xef\xbf\xbd\xef\xbf\xbd\xc5\xb8\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_TurnInterpSpeed_MetaData[] = {
		{ "Category", "AI" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "//\xc8\xb8\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xd3\xb5\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "AICharacter.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xc8\xb8\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xd3\xb5\xef\xbf\xbd" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFloatPropertyParams NewProp_AttackInterval;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_CurrentTarget;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_AttackRange;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_TurnInterpSpeed;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AAICharacter>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AAICharacter_Statics::NewProp_AttackInterval = { "AttackInterval", nullptr, (EPropertyFlags)0x0020080000000001, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AAICharacter, AttackInterval), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_AttackInterval_MetaData), NewProp_AttackInterval_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AAICharacter_Statics::NewProp_CurrentTarget = { "CurrentTarget", nullptr, (EPropertyFlags)0x0020080000020015, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AAICharacter, CurrentTarget), Z_Construct_UClass_AActor_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CurrentTarget_MetaData), NewProp_CurrentTarget_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AAICharacter_Statics::NewProp_AttackRange = { "AttackRange", nullptr, (EPropertyFlags)0x0020080000000001, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AAICharacter, AttackRange), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_AttackRange_MetaData), NewProp_AttackRange_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AAICharacter_Statics::NewProp_TurnInterpSpeed = { "TurnInterpSpeed", nullptr, (EPropertyFlags)0x0020080000000001, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AAICharacter, TurnInterpSpeed), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_TurnInterpSpeed_MetaData), NewProp_TurnInterpSpeed_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AAICharacter_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AAICharacter_Statics::NewProp_AttackInterval,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AAICharacter_Statics::NewProp_CurrentTarget,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AAICharacter_Statics::NewProp_AttackRange,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AAICharacter_Statics::NewProp_TurnInterpSpeed,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AAICharacter_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_AAICharacter_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AArrowCharacter,
	(UObject* (*)())Z_Construct_UPackage__Script_ArrowGame,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AAICharacter_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AAICharacter_Statics::ClassParams = {
	&AAICharacter::StaticClass,
	"Game",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_AAICharacter_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_AAICharacter_Statics::PropPointers),
	0,
	0x009000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AAICharacter_Statics::Class_MetaDataParams), Z_Construct_UClass_AAICharacter_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AAICharacter()
{
	if (!Z_Registration_Info_UClass_AAICharacter.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AAICharacter.OuterSingleton, Z_Construct_UClass_AAICharacter_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AAICharacter.OuterSingleton;
}
template<> ARROWGAME_API UClass* StaticClass<AAICharacter>()
{
	return AAICharacter::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(AAICharacter);
AAICharacter::~AAICharacter() {}
// End Class AAICharacter

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_AICharacter_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AAICharacter, AAICharacter::StaticClass, TEXT("AAICharacter"), &Z_Registration_Info_UClass_AAICharacter, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AAICharacter), 891642633U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_AICharacter_h_2577641696(TEXT("/Script/ArrowGame"),
	Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_AICharacter_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_AICharacter_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
