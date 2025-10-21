// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "ArrowGame/ArrowGameGameMode.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeArrowGameGameMode() {}

// Begin Cross Module References
ARROWGAME_API UClass* Z_Construct_UClass_AArrowGameGameMode();
ARROWGAME_API UClass* Z_Construct_UClass_AArrowGameGameMode_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_AGameModeBase();
UPackage* Z_Construct_UPackage__Script_ArrowGame();
// End Cross Module References

// Begin Class AArrowGameGameMode
void AArrowGameGameMode::StaticRegisterNativesAArrowGameGameMode()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AArrowGameGameMode);
UClass* Z_Construct_UClass_AArrowGameGameMode_NoRegister()
{
	return AArrowGameGameMode::StaticClass();
}
struct Z_Construct_UClass_AArrowGameGameMode_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "HideCategories", "Info Rendering MovementReplication Replication Actor Input Movement Collision Rendering HLOD WorldPartition DataLayers Transformation" },
		{ "IncludePath", "ArrowGameGameMode.h" },
		{ "ModuleRelativePath", "ArrowGameGameMode.h" },
		{ "ShowCategories", "Input|MouseInput Input|TouchInput" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AArrowGameGameMode>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_AArrowGameGameMode_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AGameModeBase,
	(UObject* (*)())Z_Construct_UPackage__Script_ArrowGame,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AArrowGameGameMode_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AArrowGameGameMode_Statics::ClassParams = {
	&AArrowGameGameMode::StaticClass,
	"Game",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x009002ACu,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AArrowGameGameMode_Statics::Class_MetaDataParams), Z_Construct_UClass_AArrowGameGameMode_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AArrowGameGameMode()
{
	if (!Z_Registration_Info_UClass_AArrowGameGameMode.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AArrowGameGameMode.OuterSingleton, Z_Construct_UClass_AArrowGameGameMode_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AArrowGameGameMode.OuterSingleton;
}
template<> ARROWGAME_API UClass* StaticClass<AArrowGameGameMode>()
{
	return AArrowGameGameMode::StaticClass();
}
AArrowGameGameMode::AArrowGameGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(AArrowGameGameMode);
AArrowGameGameMode::~AArrowGameGameMode() {}
// End Class AArrowGameGameMode

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowGameGameMode_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AArrowGameGameMode, AArrowGameGameMode::StaticClass, TEXT("AArrowGameGameMode"), &Z_Registration_Info_UClass_AArrowGameGameMode, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AArrowGameGameMode), 4096764053U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowGameGameMode_h_2506117592(TEXT("/Script/ArrowGame"),
	Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowGameGameMode_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowGameGameMode_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
