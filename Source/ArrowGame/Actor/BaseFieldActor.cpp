#include "BaseFieldActor.h"
#include "Components/StaticMeshComponent.h"

ABaseFieldActor::ABaseFieldActor()
{
    // 1. 메쉬 생성 및 루트 설정
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FieldMesh"));
    RootComponent = Mesh;

    // [핵심] 2. 네트워킹 동기화 설정
    // 이걸 켜야 서버의 위치가 클라이언트로 강제 전송됩니다.
    bReplicates = true;
    SetReplicateMovement(true);

    // 3. 콜리전 설정 (캐릭터가 밟고 올라가야 하므로 BlockAll)
    Mesh->SetCollisionProfileName(TEXT("BlockAll"));
    
    // 이동하지 않는 고정 구조물이라면 이 설정을 통해 최적화 가능
    Mesh->SetMobility(EComponentMobility::Movable);
}