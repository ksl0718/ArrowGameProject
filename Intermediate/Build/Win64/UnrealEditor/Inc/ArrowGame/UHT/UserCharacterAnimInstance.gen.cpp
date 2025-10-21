// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "ArrowGame/UserCharacterAnimInstance.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeUserCharacterAnimInstance() {}

// Begin Cross Module References
ARROWGAME_API UClass* Z_Construct_UClass_UUserCharacterAnimInstance();
ARROWGAME_API UClass* Z_Construct_UClass_UUserCharacterAnimInstance_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_UAnimInstance();
UPackage* Z_Construct_UPackage__Script_ArrowGame();
// End Cross Module References

// Begin Class UUserCharacterAnimInstance
void UUserCharacterAnimInstance::StaticRegisterNativesUUserCharacterAnimInstance()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UUserCharacterAnimInstance);
UClass* Z_Construct_UClass_UUserCharacterAnimInstance_NoRegister()
{
	return UUserCharacterAnimInstance::StaticClass();
}
struct Z_Construct_UClass_UUserCharacterAnimInstance_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "HideCategories", "AnimInstance" },
		{ "IncludePath", "UserCharacterAnimInstance.h" },
		{ "ModuleRelativePath", "UserCharacterAnimInstance.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_IsAiming_MetaData[] = {
		{ "Category", "State" },
		{ "ModuleRelativePath", "UserCharacterAnimInstance.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_IsCharging_MetaData[] = {
		{ "Category", "State" },
		{ "ModuleRelativePath", "UserCharacterAnimInstance.h" },
	};
#endif // WITH_METADATA
	static void NewProp_IsAiming_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_IsAiming;
	static void NewProp_IsCharging_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_IsCharging;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UUserCharacterAnimInstance>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
void Z_Construct_UClass_UUserCharacterAnimInstance_Statics::NewProp_IsAiming_SetBit(void* Obj)
{
	((UUserCharacterAnimInstance*)Obj)->IsAiming = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UUserCharacterAnimInstance_Statics::NewProp_IsAiming = { "IsAiming", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UUserCharacterAnimInstance), &Z_Construct_UClass_UUserCharacterAnimInstance_Statics::NewProp_IsAiming_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_IsAiming_MetaData), NewProp_IsAiming_MetaData) };
void Z_Construct_UClass_UUserCharacterAnimInstance_Statics::NewProp_IsCharging_SetBit(void* Obj)
{
	((UUserCharacterAnimInstance*)Obj)->IsCharging = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UUserCharacterAnimInstance_Statics::NewProp_IsCharging = { "IsCharging", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UUserCharacterAnimInstance), &Z_Construct_UClass_UUserCharacterAnimInstance_Statics::NewProp_IsCharging_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_IsCharging_MetaData), NewProp_IsCharging_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UUserCharacterAnimInstance_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UUserCharacterAnimInstance_Statics::NewProp_IsAiming,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UUserCharacterAnimInstance_Statics::NewProp_IsCharging,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UUserCharacterAnimInstance_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UUserCharacterAnimInstance_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UAnimInstance,
	(UObject* (*)())Z_Construct_UPackage__Script_ArrowGame,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UUserCharacterAnimInstance_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UUserCharacterAnimInstance_Statics::ClassParams = {
	&UUserCharacterAnimInstance::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_UUserCharacterAnimInstance_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UUserCharacterAnimInstance_Statics::PropPointers),
	0,
	0x009000A8u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UUserCharacterAnimInstance_Statics::Class_MetaDataParams), Z_Construct_UClass_UUserCharacterAnimInstance_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UUserCharacterAnimInstance()
{
	if (!Z_Registration_Info_UClass_UUserCharacterAnimInstance.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UUserCharacterAnimInstance.OuterSingleton, Z_Construct_UClass_UUserCharacterAnimInstance_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UUserCharacterAnimInstance.OuterSingleton;
}
template<> ARROWGAME_API UClass* StaticClass<UUserCharacterAnimInstance>()
{
	return UUserCharacterAnimInstance::StaticClass();
}
UUserCharacterAnimInstance::UUserCharacterAnimInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UUserCharacterAnimInstance);
UUserCharacterAnimInstance::~UUserCharacterAnimInstance() {}
// End Class UUserCharacterAnimInstance

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacterAnimInstance_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UUserCharacterAnimInstance, UUserCharacterAnimInstance::StaticClass, TEXT("UUserCharacterAnimInstance"), &Z_Registration_Info_UClass_UUserCharacterAnimInstance, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UUserCharacterAnimInstance), 3944687191U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacterAnimInstance_h_1634760491(TEXT("/Script/ArrowGame"),
	Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacterAnimInstance_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_UserCharacterAnimInstance_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
