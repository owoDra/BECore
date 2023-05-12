// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright Eigi Chin

#include "BEGameSettingRegistry.h"

#include "Player/BELocalPlayer.h"
#include "GameSetting/SettingValue/BESettingValueDiscrete_PerfStat.h"
#include "Performance/BEPerformanceStatTypes.h"

#include "GameSettingCollection.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "Internationalization/Internationalization.h"
#include "UObject/UObjectGlobals.h"
#include "HAL/Platform.h"


#define LOCTEXT_NAMESPACE "BE"

void UBEGameSettingRegistry::AddPrefStatSettingsSection(UGameSettingCollection* Screen, UBELocalPlayer* InLocalPlayer)
{
	static_assert((int32)EBEDisplayablePerformanceStat::Count == 15, "Consider updating this function to deal with new performance stats");

	UGameSettingCollection* Information = NewObject<UGameSettingCollection>();
	Information->SetDevName(TEXT("InformationSection"));
	Information->SetDisplayName(LOCTEXT("InformationSection_Name", "Information"));
	Screen->AddSetting(Information);

	////////////////////////////////////////
	//	パフォーマンス表示設定ページ
	{
		UGameSettingCollectionPage* StatsPage = NewObject<UGameSettingCollectionPage>();
		StatsPage->SetDevName(TEXT("PerfStatsPage"));
		StatsPage->SetDisplayName(LOCTEXT("PerfStatsPage_Name", "Performance Stats"));
		StatsPage->SetDescriptionRichText(LOCTEXT("PerfStatsPage_Description", "Configure the display of performance statistics."));
		StatsPage->SetNavigationText(LOCTEXT("PerfStatsPage_Navigation", "Edit"));

		StatsPage->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

		Information->AddSetting(StatsPage);

		////////////////////////////////////////
		//	パフォーマンス表示設定
		{
			UGameSettingCollection* StatCategory_Performance = NewObject<UGameSettingCollection>();
			StatCategory_Performance->SetDevName(TEXT("PerfStatSection"));
			StatCategory_Performance->SetDisplayName(LOCTEXT("PerfStatSection_Name", "Performance"));
			StatsPage->AddSetting(StatCategory_Performance);

			//======================================
			//	クライアントFPS表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::ClientFPS);
				Setting->SetDisplayName(LOCTEXT("PerfStat_ClientFPS", "Client FPS"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_ClientFPS", "Client frame rate (higher is better)"));
				StatCategory_Performance->AddSetting(Setting);
			}

			//======================================
			//	サーバーFPS表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::ServerFPS);
				Setting->SetDisplayName(LOCTEXT("PerfStat_ServerFPS", "Server FPS"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_ServerFPS", "Server frame rate"));
				StatCategory_Performance->AddSetting(Setting);
			}

			//======================================
			//	フレーム時間表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::FrameTime);
				Setting->SetDisplayName(LOCTEXT("PerfStat_FrameTime", "Frame Time"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_FrameTime", "The total frame time."));
				StatCategory_Performance->AddSetting(Setting);
			}

			//======================================
			//	遅延時間表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::IdleTime);
				Setting->SetDisplayName(LOCTEXT("PerfStat_IdleTime", "Idle Time"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_IdleTime", "The amount of time spent waiting idle for frame pacing."));
				StatCategory_Performance->AddSetting(Setting);
			}

			//======================================
			//	CPUゲーム時間表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::FrameTime_GameThread);
				Setting->SetDisplayName(LOCTEXT("PerfStat_FrameTime_GameThread", "CPU Game Time"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_FrameTime_GameThread", "The amount of time spent on the main game thread."));
				StatCategory_Performance->AddSetting(Setting);
			}

			//======================================
			//	CPU描画時間表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::FrameTime_RenderThread);
				Setting->SetDisplayName(LOCTEXT("PerfStat_FrameTime_RenderThread", "CPU Render Time"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_FrameTime_RenderThread", "The amount of time spent on the rendering thread."));
				StatCategory_Performance->AddSetting(Setting);
			}

			//======================================
			//	CPU(RHI)時間表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::FrameTime_RHIThread);
				Setting->SetDisplayName(LOCTEXT("PerfStat_FrameTime_RHIThread", "CPU RHI Time"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_FrameTime_RHIThread", "The amount of time spent on the Render Hardware Interface thread."));
				StatCategory_Performance->AddSetting(Setting);
			}

			//======================================
			//	GPU描画時間表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::FrameTime_GPU);
				Setting->SetDisplayName(LOCTEXT("PerfStat_FrameTime_GPU", "GPU Render Time"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_FrameTime_GPU", "The amount of time spent on the GPU."));
				StatCategory_Performance->AddSetting(Setting);
			}
		}

		////////////////////////////////////////
		//	ネットワーク表示設定
		{
			UGameSettingCollection* StatCategory_Network = NewObject<UGameSettingCollection>();
			StatCategory_Network->SetDevName(TEXT("NetworkStatSection"));
			StatCategory_Network->SetDisplayName(LOCTEXT("NetworkStatSection_Name", "Network"));
			StatsPage->AddSetting(StatCategory_Network);

			//======================================
			//	PING表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::Ping);
				Setting->SetDisplayName(LOCTEXT("PerfStat_Ping", "Ping"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_Ping", "The roundtrip latency of your connection to the server."));
				StatCategory_Network->AddSetting(Setting);
			}

			//======================================
			//	受信パケットロス表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::PacketLoss_Incoming);
				Setting->SetDisplayName(LOCTEXT("PerfStat_PacketLoss_Incoming", "Incoming Packet Loss"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_PacketLoss_Incoming", "The percentage of incoming packets lost."));
				StatCategory_Network->AddSetting(Setting);
			}

			//======================================
			//	送信パケットロス表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::PacketLoss_Outgoing);
				Setting->SetDisplayName(LOCTEXT("PerfStat_PacketLoss_Outgoing", "Outgoing Packet Loss"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_PacketLoss_Outgoing", "The percentage of outgoing packets lost."));
				StatCategory_Network->AddSetting(Setting);
			}

			//======================================
			//	受信パケット速度表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::PacketRate_Incoming);
				Setting->SetDisplayName(LOCTEXT("PerfStat_PacketRate_Incoming", "Incoming Packet Rate"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_PacketRate_Incoming", "Rate of incoming packets (per second)"));
				StatCategory_Network->AddSetting(Setting);
			}

			//======================================
			//	送信パケット速度表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::PacketRate_Outgoing);
				Setting->SetDisplayName(LOCTEXT("PerfStat_PacketRate_Outgoing", "Outgoing Packet Rate"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_PacketRate_Outgoing", "Rate of outgoing packets (per second)"));
				StatCategory_Network->AddSetting(Setting);
			}

			//======================================
			//	受信パケットサイズ表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::PacketSize_Incoming);
				Setting->SetDisplayName(LOCTEXT("PerfStat_PacketSize_Incoming", "Incoming Packet Size"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_PacketSize_Incoming", "The average size (in bytes) of packets recieved in the last second."));
				StatCategory_Network->AddSetting(Setting);
			}

			//======================================
			//	送信パケットサイズ表示設定
			//======================================
			{
				UBESettingValueDiscrete_PerfStat* Setting = NewObject<UBESettingValueDiscrete_PerfStat>();
				Setting->SetStat(EBEDisplayablePerformanceStat::PacketSize_Outgoing);
				Setting->SetDisplayName(LOCTEXT("PerfStat_PacketSize_Outgoing", "Outgoing Packet Size"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_PacketSize_Outgoing", "The average size (in bytes) of packets sent in the last second."));
				StatCategory_Network->AddSetting(Setting);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
