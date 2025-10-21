// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "ArrowGame/HealthComponent.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeHealthComponent() {}

// Begin Cross Module References
ARROWGAME_API UClass* Z_Construct_UClass_UHealthComponent();
ARROWGAME_API UClass* Z_Construct_UClass_UHealthComponent_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_AActor_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_AController_NoRegister();
ENGINE_API UClass* Z_Construct_UClass_UActorComponent();
ENGINE_API UClass* Z_Construct_UClass_UDamageType_NoRegister();
UPackage* Z_Construct_UPackage__Script_ArrowGame();
// End Cross Module References

// Begin Class UHealthComponent Function DamagerTaken
struct Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics
{
	struct HealthComponent_eventDamagerTaken_Parms
	{
		AActor* DamageActor;
		float Damage;
		const UDamageType* DamageType;
		AController* Instigator;
		AActor* DamageCause;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "HealthComponent.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DamageType_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_DamageActor;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_Damage;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_DamageType;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_Instigator;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_DamageCause;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::NewProp_DamageActor = { "DamageActor", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HealthComponent_eventDamagerTaken_Parms, DamageActor), Z_Construct_UClass_AActor_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::NewProp_Damage = { "Damage", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HealthComponent_eventDamagerTaken_Parms, Damage), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::NewProp_DamageType = { "DamageType", nullptr, (EPropertyFlags)0x0010000000000082, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HealthComponent_eventDamagerTaken_Parms, DamageType), Z_Construct_UClass_UDamageType_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DamageType_MetaData), NewProp_DamageType_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::NewProp_Instigator = { "Instigator", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HealthComponent_eventDamagerTaken_Parms, Instigator), Z_Construct_UClass_AController_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::NewProp_DamageCause = { "DamageCause", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HealthComponent_eventDamagerTaken_Parms, DamageCause), Z_Construct_UClass_AActor_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::NewProp_DamageActor,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::NewProp_Damage,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::NewProp_DamageType,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::NewProp_Instigator,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::NewProp_DamageCause,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UHealthComponent, nullptr, "DamagerTaken", nullptr, nullptr, Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::PropPointers), sizeof(Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::HealthComponent_eventDamagerTaken_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00040401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::Function_MetaDataParams), Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::HealthComponent_eventDamagerTaken_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UHealthComponent_DamagerTaken()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UHealthComponent_DamagerTaken_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UHealthComponent::execDamagerTaken)
{
	P_GET_OBJECT(AActor,Z_Param_DamageActor);
	P_GET_PROPERTY(FFloatProperty,Z_Param_Damage);
	P_GET_OBJECT(UDamageType,Z_Param_DamageType);
	P_GET_OBJECT(AController,Z_Param_Instigator);
	P_GET_OBJECT(AActor,Z_Param_DamageCause);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->DamagerTaken(Z_Param_DamageActor,Z_Param_Damage,Z_Param_DamageType,Z_Param_Instigator,Z_Param_DamageCause);
	P_NATIVE_END;
}
// End Class UHealthComponent Function DamagerTaken

// Begin Class UHealthComponent
void UHealthComponent::StaticRegisterNativesUHealthComponent()
{
	UClass* Class = UHealthComponent::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "DamagerTaken", &UHealthComponent::execDamagerTaken },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UHealthComponent);
UClass* Z_Construct_UClass_UHealthComponent_NoRegister()
{
	return UHealthComponent::StaticClass();
}
struct Z_Construct_UClass_UHealthComponent_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintSpawnableComponent", "" },
		{ "ClassGroupNames", "Custom" },
		{ "IncludePath", "HealthComponent.h" },
		{ "ModuleRelativePath", "HealthComponent.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_MaxHealth_MetaData[] = {
		{ "Category", "HealthComponent" },
		{ "ModuleRelativePath", "HealthComponent.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFloatPropertyParams NewProp_MaxHealth;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_UHealthComponent_DamagerTaken, "DamagerTaken" }, // 4162198756
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UHealthComponent>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UHealthComponent_Statics::NewProp_MaxHealth = { "MaxHealth", nullptr, (EPropertyFlags)0x0040000000000001, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UHealthComponent, MaxHealth), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_MaxHealth_MetaData), NewProp_MaxHealth_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UHealthComponent_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UHealthComponent_Statics::NewProp_MaxHealth,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UHealthComponent_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UHealthComponent_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UActorComponent,
	(UObject* (*)())Z_Construct_UPackage__Script_ArrowGame,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UHealthComponent_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UHealthComponent_Statics::ClassParams = {
	&UHealthComponent::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	Z_Construct_UClass_UHealthComponent_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	UE_ARRAY_COUNT(Z_Construct_UClass_UHealthComponent_Statics::PropPointers),
	0,
	0x00B000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UHealthComponent_Statics::Class_MetaDataParams), Z_Construct_UClass_UHealthComponent_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UHealthComponent()
{
	if (!Z_Registration_Info_UClass_UHealthComponent.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UHealthComponent.OuterSingleton, Z_Construct_UClass_UHealthComponent_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UHealthComponent.OuterSingleton;
}
template<> ARROWGAME_API UClass* StaticClass<UHealthComponent>()
{
	return UHealthComponent::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UHealthComponent);
UHealthComponent::~UHealthComponent() {}
// End Class UHealthComponent

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_HealthComponent_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UHealthComponent, UHealthComponent::StaticClass, TEXT("UHealthComponent"), &Z_Registration_Info_UClass_UHealthComponent, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UHealthComponent), 1963182377U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_HealthComponent_h_157708362(TEXT("/Script/ArrowGame"),
	Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_HealthComponent_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_ksl0718_Documents_Unreal_Projects_ArrowGame_Source_ArrowGame_HealthComponent_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
