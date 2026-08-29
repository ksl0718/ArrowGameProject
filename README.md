# 행궁 (ArrowGame)

> 조선 화성 행궁을 배경으로 한 1 vs N 비대칭 멀티플레이 활 전투 게임

**Unreal Engine 5 · C++ · Steam Multiplayer · Server Authority Combat**

`Unreal Engine 5.4` `C++` `Replication` `Server RPC` `Multicast RPC` `OnlineSubsystemSteam` `UMG` `Niagara`

## 프로젝트 요약

**행궁**은 한 명의 도깨비와 다수의 궁수가 대결하는 비대칭 멀티플레이 게임입니다. 궁수는 활과 특수 화살로 도깨비를 추적하고, 도깨비는 분신, 은신, 저주, 투시 능력으로 궁수를 교란하며 제한 시간 동안 생존합니다.

| 항목 | 내용 |
| --- | --- |
| 개발 기간 | 2025.09 ~ 2026.06 |
| 개발 형태 | 캡스톤 프로젝트 / 1인 개발 |
| 엔진 | Unreal Engine 5.4 |
| 언어 | C++ |
| 플랫폼 | Windows |
| 네트워크 | Unreal Replication, RPC, OnlineSubsystemSteam |
| 담당 | 기획, 게임플레이, 멀티플레이, UI, 맵 구성, 최적화 |

## 한 줄 어필

Unreal Engine 5와 C++로 비대칭 멀티플레이 전투 시스템을 1인 개발하며, 서버 권위 기반 전투 판정과 역할별 스킬 동기화를 직접 구현했습니다.

## 플레이 구조

```text
메인 메뉴
→ Steam 세션 생성/검색/참가
→ 로비 Ready
→ 전투 맵 이동
→ 도깨비 1명 랜덤 배정
→ 카운트다운
→ 전투
→ 라운드 결과
→ 로비 복귀
```

## 주요 기능

| 역할 | 기능 |
| --- | --- |
| 궁수 | 활 조준, 차징, 발사, 구르기, 일반/화염/폭발 화살 전환 |
| 도깨비 | 분신, 은신, 저주 투사체, 투시 |
| 공통 | 체력, 피격, 사망, 스킬 쿨다운 HUD, 라운드 결과 |
| 멀티플레이 | Steam 세션, 로비 Ready, 역할 배정, ServerTravel, 서버 권위 판정 |

## 기술 하이라이트

### 1. 비대칭 멀티플레이 라운드 플로우

- `OnlineSubsystemSteam` 기반 세션 생성, 검색, 참가 구현
- 로비 Ready 이후 전투 맵으로 이동
- Seamless Travel 이후 모든 플레이어의 `PlayerState` 준비를 기다린 뒤 매치 시작
- 서버에서 도깨비 1명을 랜덤 배정하고 역할별 Pawn 스폰
- 카운트다운 중 입력 잠금, 라운드 시작 시 입력 활성화
- 도깨비 사망 또는 제한 시간 종료에 따른 승패 처리

관련 코드:

- `Source/ArrowGame/ArrowGameInstance.cpp`
- `Source/ArrowGame/Core/GameModes/ArrowGameGameMode.cpp`
- `Source/ArrowGame/Core/ArrowPlayerState.cpp`

### 2. 서버 권위 기반 활/화살 전투

- 활 상태를 조준, 장전, 차징, 재장전으로 분리
- 클라이언트에서는 조작 반응을 즉시 보여주고, 실제 발사와 데미지는 서버에서 확정
- 차징 비율에 따라 화살 속도 보간
- 발사체 Replication과 `ReplicateMovement` 적용
- 발사 직후 한 틱 뒤 충돌을 활성화해 자기 캐릭터와의 즉시 충돌 방지
- 일반/화염/폭발 화살을 상속 기반으로 확장

관련 코드:

- `Source/ArrowGame/Weapon/Bow.cpp`
- `Source/ArrowGame/Weapon/ArrowProjectile.cpp`
- `Source/ArrowGame/Weapon/Arrow/FireArrow.cpp`
- `Source/ArrowGame/Weapon/Arrow/ExplosiveArrow.cpp`
- `Source/ArrowGame/Component/HealthComponent.cpp`

### 3. 도깨비 스킬 시스템

- `FSkillSpec`으로 스킬 ID, 쿨다운, 입력 잠금, 아이콘, 키 텍스트를 데이터화
- `FSkillRuntimeState`로 서버 시간 기준 쿨다운 관리
- 스킬 상태와 투시 종료 시간은 OwnerOnly 복제로 동기화 범위 축소
- 은신은 본인에게 반투명, 타인에게 메시 숨김으로 다르게 표현
- 저주 투사체 피격 시 궁수의 이동/공격 행동을 강제 제어
- 투시는 소유 클라이언트에서만 마커와 오버레이 갱신

관련 코드:

- `Source/ArrowGame/Character/DokkaebiCharacter.cpp`
- `Source/ArrowGame/Character/DokkaebiCurseProjectile.cpp`
- `Source/ArrowGame/Character/UserArcherCharacter.cpp`

### 4. UI 추상화와 위젯 풀링

- 궁수와 도깨비가 서로 다른 스킬 구성을 가져도 하나의 HUD를 재사용하도록 `ISkillCooldownProvider` 인터페이스 적용
- 현재 Pawn이 제공하는 스킬 개수, 아이콘, 키 텍스트, 쿨다운 값을 기반으로 HUD 슬롯 동적 생성
- 투시 마커는 매 프레임 생성하지 않고 필요한 수만큼 위젯을 풀링한 뒤 위치/스케일만 갱신

관련 코드:

- `Source/ArrowGame/Character/SkillCooldownProvider.h`
- `Source/ArrowGame/UI/SkillCooldownHUDWidget.cpp`
- `Source/ArrowGame/UI/SpiritSightMarkerWidget.cpp`

## AI 활용

1인 개발 과정에서 도깨비 캐릭터 리소스 제작 병목을 줄이기 위해 생성형 AI를 제작 파이프라인에 활용했습니다.

```text
레퍼런스 수집
→ Gemini Nano Banana로 도깨비 디자인 방향 추출
→ 정면/측면 이미지 생성
→ Meshy AI로 3D 메시 초안 생성
→ MetaHuman 기반 캐릭터화
→ Unreal Engine 5.6 환경으로 이주
→ 호환되지 않는 머티리얼 제거 후 게임 적용
```

AI 결과물을 그대로 사용하기보다, 게임 톤, 3D화 가능성, 엔진 호환성, 실제 플레이 적용 가능성을 기준으로 단계별 검증을 거쳤습니다.

## 성능/완성도 개선

- 행궁 맵 정적 환경에 Nanite 적용
- 캐릭터에는 LOD 적용
- 투시 마커 위젯 풀링
- 로컬 플레이어에게만 필요한 UI/Tick 처리 분리
- 호환되지 않는 머티리얼 제거로 실행 안정성 우선

## 프로젝트 구조

```text
Source/ArrowGame/
├── Actor/              # 활/화살 픽업 등 인게임 아이템
├── AI/                 # AI 캐릭터
├── Character/          # 궁수, 도깨비, 애니메이션, 스킬
├── Component/          # 체력/피격/DoT 컴포넌트
├── Core/               # GameMode, GameState, PlayerState, Controller
├── UI/                 # UMG 위젯
└── Weapon/             # 활, 투사체, 특수 화살
```

## 추가 문서

- [넥토리얼 포트폴리오 초안](docs/NEXON_Portfolio_Draft.md)
- [Notion 포트폴리오 원문](docs/HAENGGUNG_Portfolio_Notion.md)

## 실행 환경

- Unreal Engine 5.4
- Visual Studio 2022 또는 Rider
- Windows 10/11
- Steam 실행 필요

## 실행 방법

1. `ArrowGame.uproject`를 Unreal Editor에서 엽니다.
2. Development Editor 빌드로 컴파일합니다.
3. 멀티플레이 테스트는 PIE에서 플레이어 수를 2명 이상으로 설정하거나 Steam 세션으로 실행합니다.

## 라이선스

개인 포트폴리오 및 학습 목적으로 제작된 프로젝트입니다.
