#include "OffScreenIndicator.h"
#include "AbilitySystemComponent.h"
#include "TestGame/MAttribute/MAttribute.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UOffScreenIndicateWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bOffScreen == false)
	{
		return;
	}

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

	double ClampedDistance = FMath::Min(Distance, FMath::Sqrt(static_cast<double>(Width * Width + Height * Height)) + 100.f);
	FVector2D TargetViewportLocation = FVector2D::ZeroVector;
	// 기울기가 기준보다 크면 가로축에 부딪히고 작으면 세로축에 부딪힘
	if (PlayerController->ProjectWorldLocationToScreen(ViewTarget->GetActorLocation() + DirToTarget * ClampedDistance, TargetViewportLocation))
	{
		Angle = FMath::Atan2(TargetViewportLocation.X - ViewportHalfSize.X, -TargetViewportLocation.Y + ViewportHalfSize.Y);
		double Slope = (-TargetViewportLocation.Y + ViewportHalfSize.Y) / (TargetViewportLocation.X - ViewportHalfSize.X);
		double SlopeRatio = Slope / PivotSlope;

		FVector2D PositionInViewport = ViewportHalfSize;

		if (FMath::Abs(SlopeRatio) > 1.0)
		{
			double Dir = DirToTarget.X > 0.0 ? 1.0 : -1.0;
			PositionInViewport.X += Dir * ViewportHalfSize.Y * (ViewportHalfSize.X / (ViewportHalfSize.Y * SlopeRatio));
			PositionInViewport.Y = Dir > 0.0 ? 40.0 : Height - 40.0;
		}
		else
		{
			double Dir = DirToTarget.Y > 0.0 ? 1.0 : -1.0;
			PositionInViewport.X = Dir > 0.0 ? Width - 40.0 : 40.0;
			PositionInViewport.Y += -Dir * ViewportHalfSize.Y * SlopeRatio;
		}

		PositionInViewport = FVector2D::Clamp(PositionInViewport, FVector2D::ZeroVector + 40.f, ViewportHalfSize * 2.f - 40.f);

		if (UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(Slot))
		{
			//FVector2D Test = CanvasPanelSlot->Parent->GetCachedGeometry().AbsoluteToLocal();
			CanvasPanelSlot->SetPosition(PositionInViewport / UWidgetLayoutLibrary::GetViewportScale(this));
		}
		else
		{
			SetPositionInViewport(PositionInViewport);
		}
	}
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

		IndicateData.Target->GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]() {
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
			FVector2D TargetViewportLocation = FVector2D::ZeroVector;
			Distance = FVector::Distance(ViewTarget->GetActorLocation(), TargetLocation);
			double ClampedDistance = FMath::Min(Distance, FMath::Sqrt(static_cast<double>(Width * Width + Height * Height)) + 100.f);
			if (PlayerController->ProjectWorldLocationToScreen(ViewTarget->GetActorLocation() + DirToTarget * ClampedDistance, TargetViewportLocation))
			{
				bOffScreen = TargetViewportLocation.X < 0 || TargetViewportLocation.X > Width || TargetViewportLocation.Y < 0 || TargetViewportLocation.Y > Height;
			}

			SetVisibility(bOffScreen ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}), 1.f, true);
	}
}
