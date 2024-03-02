#include "MPlayer.h"

AMPlayer::AMPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(GetRootComponent());

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
}


void AMPlayer::Test(float MeshDeltaYaw)
{
	float CameraYaw = FMath::Fmod(SpringArmComponent->GetComponentRotation().Yaw, 360.0);
	float MeshYaw = FMath::Fmod(GetMesh()->GetComponentRotation().Yaw - MeshDeltaYaw, 360.0);

	CameraYaw = SpringArmComponent->GetComponentRotation().Yaw >= 0.f ? SpringArmComponent->GetComponentRotation().Yaw : SpringArmComponent->GetComponentRotation().Yaw + 360.f;
	MeshYaw = GetMesh()->GetComponentRotation().Yaw - MeshDeltaYaw >= 0.f ? GetMesh()->GetComponentRotation().Yaw - MeshDeltaYaw : GetMesh()->GetComponentRotation().Yaw - MeshDeltaYaw + 360.f;

	FVector CameraYawDir = FRotator(0.0, static_cast<double>(CameraYaw), 0.0).Vector();
	FVector MeshYawDir = FRotator(0.0, static_cast<double>(MeshYaw), 0.0).Vector();

	if (FMath::IsNearlyEqual(CameraYawDir.Dot(MeshYawDir), 1.0))
	{
		return;
	}

	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + CameraYawDir * 300.f, FColor::Red, true, -1.f, 0, 5);
	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + MeshYawDir * 300.f, FColor::Green, true, -1.f, 0, 5);

	FVector Cross = CameraYawDir.Cross(MeshYawDir);

	float a = MeshYaw + MeshDeltaYaw > 0.f ? MeshYaw + MeshDeltaYaw : MeshYaw + MeshDeltaYaw + 360.f;

	if (Cross.Z > 0.f)
	{
		if (a < CameraYaw)
		{
			a += 360.f;
		}

		GetMesh()->AddWorldRotation(FRotator(0.0, -1.0, 0.0));
		if (a < CameraYaw)
		{
			UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj A"));
			GetMesh()->SetWorldRotation(FRotator(0.0, CameraYaw + MeshDeltaYaw, 0.0));
		}
	}
	else
	{
		if (a > CameraYaw)
		{
			a -= 360.f;
		}

		GetMesh()->AddWorldRotation(FRotator(0.0, 1.0, 0.0));
		if (a > CameraYaw)
		{
			UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj B"));
			GetMesh()->SetWorldRotation(FRotator(0.0, CameraYaw + MeshDeltaYaw, 0.0));
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("ghoflvhxj Cross %f, %f, %f"), Cross.X, Cross.Y, Cross.Z);
}
