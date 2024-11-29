#include "OffScreenIndicator.h"
#include "AbilitySystemComponent.h"
#include "TestGame/MAttribute/MAttribute.h"

void UOffScreenIndicateWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	AActor* ViewTarget = PlayerController->GetViewTarget();
	if (IsValid(ViewTarget) == false)
	{
		return;
	}

	// 가리킬 대상의 위치와 방향을 구함
	FVector TargetLocation = IsValid(IndicateData.Target) ? IndicateData.Target->GetActorLocation() : IndicateData.TargetLocation;
	FVector DirToTarget = (TargetLocation - ViewTarget->GetActorLocation()).GetSafeNormal2D();

	// 뷰포트 사이즈와 기울기 
	int32 Width = 0, Height = 0;
	PlayerController->GetViewportSize(Width, Height);
	FVector2D ViewportHalfSize = { Width / 2.0, Height / 2.0 };
	double PivotSlope = static_cast<double>(Height) / Width;

	FVector2D ViewportPos = ViewportHalfSize;
	FVector2D TargetScreenLocation = FVector2D::ZeroVector;
	Distance = FVector::Distance(ViewTarget->GetActorLocation(), TargetLocation);
	double ClampedDistance = FMath::Min(Distance, FMath::Sqrt(static_cast<double>(Width * Width + Height * Height)) + 100.f);
	if (PlayerController->ProjectWorldLocationToScreen(ViewTarget->GetActorLocation() + DirToTarget * ClampedDistance, TargetScreenLocation))
	{
		if (bool bOffScreen = TargetScreenLocation.X < 0 || TargetScreenLocation.X > Width || TargetScreenLocation.Y < 0 || TargetScreenLocation.Y > Height)
		{
			if (IsVisible() == false)
			{
				ShowIndicateWidget();
			}

			Angle = FMath::Atan2(TargetScreenLocation.X - ViewportHalfSize.X, -TargetScreenLocation.Y + ViewportHalfSize.Y);
			double Slope = (-TargetScreenLocation.Y + ViewportHalfSize.Y) / (TargetScreenLocation.X - ViewportHalfSize.X);
			double SlopeRatio = Slope / PivotSlope;
			//UE_LOG(LogTemp, Warning, TEXT("Angle:%f, Slope:%f, PivotSlope:%f"), FMath::RadiansToDegrees(Angle), Slope, PivotSlope);
			// 기울기가 기준보다 크면 가로축에 부딪히고 작으면 세로축에 부딪힘
			if (FMath::Abs(SlopeRatio) > 1.0)
			{
				double Dir = DirToTarget.X > 0.0 ? 1.0 : -1.0;
				ViewportPos.X += Dir * ViewportHalfSize.Y * (ViewportHalfSize.X / (ViewportHalfSize.Y * SlopeRatio));
				ViewportPos.Y = Dir > 0.0 ? 40.0 : Height - 40.0;
			}
			else
			{
				double Dir = DirToTarget.Y > 0.0 ? 1.0 : -1.0;
				ViewportPos.X = Dir > 0.0 ? Width - 40.0 : 40.0;
				ViewportPos.Y += -Dir * ViewportHalfSize.Y * SlopeRatio;
			}
		}
	}
	
	ViewportPos = FVector2D::Clamp(ViewportPos, FVector2D::ZeroVector + 40.f, ViewportHalfSize * 2.f - 40.f);
	SetPositionInViewport(ViewportPos);
}

void UOffScreenIndicateWidget::ShowIndicateWidget_Implementation()
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UOffScreenIndicateWidget::HideIndicateWidget_Implementation(bool bRemove)
{
	SetVisibility(ESlateVisibility::Collapsed);

	if (bRemove)
	{
		RemoveFromParent();
	}
}

void UOffScreenIndicateWidget::SetIndicateTarget(const FIndicatorData& InData)
{
	if (IndicateData == InData)
	{
		return;
	}

	if (IsValid(IndicateData.Target))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = IndicateData.Target->GetComponentByClass<UAbilitySystemComponent>())
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMAttributeSet::GetHealthAttribute()).Remove(Handle);
		}
	}

	IndicateData = InData;
	if (IsValid(IndicateData.Target))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = IndicateData.Target->GetComponentByClass<UAbilitySystemComponent>())
		{
			Handle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMAttributeSet::GetHealthAttribute()).AddWeakLambda(this, [this](const FOnAttributeChangeData& AttributeChangeData) {
				if (AttributeChangeData.NewValue <= 0.f)
				{
					HideIndicateWidget(true);
				}
			});
		}
	}
}
