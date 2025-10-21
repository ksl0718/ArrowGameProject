// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "ArrowGame/ArrowGamePlayerController.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeArrowGamePlayerController() {}

// Begin Cross Module References
ARROWGAME_API UClass* Z_Construct_UClass_AArrowGamePlayerController();
ARROWGAME_API UClass* Z_Construct_UClass_AArrowGamePlayerController_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_APlayerController();
UPackage* Z_Construct_UPackage__Script_ArrowGame();
// End Cross Module References

// Begin Class AArrowGamePlayerController
void AArrowGamePlayerController::StaticRegisterNativesAArrowGamePlayerController()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AArrowGamePlayerController);
UClass* Z_Construct_UClass_AArrowGamePlayerController_NoRegister()
{
	return AArrowGamePlayerController::StaticClass();
}
struct Z_Construct_UClass_AArrowGamePlayerController_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "HideCategories", "Collision Rendering Transformation" },
		{ "IncludePath", "ArrowGamePlayerController.h" },
		{ "ModuleRelativePath", "ArrowGamePlayerController.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AArrowGamePlayerController>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_AArrowGamePlayerController_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_APlayerController,
	(UObject* (*)())Z_Construct_UPackage__Script_ArrowGame,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AArrowGamePlayerController_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AArrowGamePlayerController_Statics::ClassParams = {
	&AArrowGamePlayerController::StaticClass,
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
	0x009002A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AArrowGamePlayerController_Statics::Class_MetaDataParams), Z_Construct_UClass_AArrowGamePlayerController_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AArrowGamePlayerController()
{
	if (!Z_Registration_Info_UClass_AArrowGamePlayerController.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AArrowGamePlayerController.OuterSingleton, Z_Construct_UClass_AArrowGamePlayerController_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AArrowGamePlayerController.OuterSingleton;
}
template<> ARROWGAME_API UClass* StaticClass<AArrowGamePlayerController>()
{
	return AArrowGamePlayerController::StaticClass();
}
AArrowGamePlayerController::AArrowGamePlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(AArrowGamePlayerController);
AArrowGamePlayerController::~AArrowGamePlayerController() {}
// End Class AArrowGamePlayerController

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowGamePlayerController_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AArrowGamePlayerController, AArrowGamePlayerController::StaticClass, TEXT("AArrowGamePlayerController"), &Z_Registration_Info_UClass_AArrowGamePlayerController, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AArrowGamePlayerController), 1374302576U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowGamePlayerController_h_38653720(TEXT("/Script/ArrowGame"),
	Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowGamePlayerController_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_ArrowGamePlayerController_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
