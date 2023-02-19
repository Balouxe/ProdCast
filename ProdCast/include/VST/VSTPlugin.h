/*
Basic implementation of VST handling. 
Recommended to use for basic audio applications, but has its limits, so consider making your own if you're making a DAW, for instance.
Main structure and some code taken from @hotwatermorning/Vst3HostDemo

Freak you Steinberg :D
*/
#pragma once

#if defined(_MSC_VER)
using WindowHandle = void*;
#else
using WindowHandle = NSView*;
#endif

#include "Core.h" 
#include "Effect.h"

#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/eventlist.cpp"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst/ivstunits.h"
#include <pluginterfaces/vst/vstpresetkeys.h>
#include "pluginterfaces/vst/vsttypes.h"


#include <string>

namespace ProdCast {
	namespace VST {

		class VSTPlugin;
		class VSTPluginImplementation;

		struct VSTNote {
			// ...
		};


		class VSTHostContext : public Steinberg::Vst::IHostApplication, 
			public Steinberg::Vst::IComponentHandler,
			public Steinberg::Vst::IComponentHandler2,
			public Steinberg::IPlugFrame
		{
		public:
			using SResult = Steinberg::tresult;

			VSTHostContext(std::string hostName);
			~VSTHostContext();

			// Getters/Setters
			void SetVST3Plugin(VSTPlugin* plugin);
			VSTPlugin* GetVST3Plugin();

			// IHostApplication virtual functions
			SResult PLUGIN_API getName(Steinberg::Vst::String128 name);

			SResult PLUGIN_API createInstance(Steinberg::TUID cid, Steinberg::TUID iid, void** obj);

			// IComponentHandler virtual functions
			SResult PLUGIN_API beginEdit(Steinberg::Vst::ParamID id);

			SResult PLUGIN_API performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue normalizedValue);

			SResult PLUGIN_API endEdit(Steinberg::Vst::ParamID id);

			SResult PLUGIN_API restartComponent(Steinberg::int32 flags);

			// IComponentHandler2 virtual functions

			SResult PLUGIN_API setDirty(Steinberg::TBool state);

			SResult PLUGIN_API requestOpenEditor(Steinberg::FIDString name = Steinberg::Vst::ViewType::kEditor);

			SResult PLUGIN_API startGroupEdit();

			SResult PLUGIN_API finishGroupEdit();

			// PlugFrame virtual functions

			SResult resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* newSize);

		private:
			std::string m_hostName;
			VSTPlugin* m_plugin = nullptr;
			// Steinberg::PlugFrame m_plugFrame;
		};

		struct ProcessInfo {
		public:
			struct TransportInfo {
				double sampleRate = 44100.0;

				uint32_t smpBeginPos = 0;
				uint32_t smpEndPos = 0;
				double ppqBeginPos = 0;
				double ppqEndPos = 0;
				bool playing = 0;
				bool loopEnabled = false;
				uint32_t loopBegin = 0;
				uint32_t loopEnd = 0;
				double tempo = 120.0;
				uint8_t timeSigNumer = 4;
				uint8_t timeSigDenom = 4;

				uint32_t GetSmpDuration() const {
					return smpEndPos - smpBeginPos;
				}

				bool IsLooping() const {
					return loopEnabled && (loopBegin < loopEnd);
				}
			};

			TransportInfo* timeInfo = nullptr;
			float* inputBuffer;
			float* outputBuffer;
		};
			
		class VSTProcessor {
		public:
			VSTProcessor(VSTPlugin* plugin);

			std::string getName();

			void OnStartProcessing(uint32_t sampleRate, uint32_t blockSize);
			void Process(ProcessInfo& processInfo);
			void OnStopProcessing();

			VSTPlugin* getPlugin();

		private:
			VSTPlugin* m_plugin;
		};

		struct BusInfo
		{
			std::string name;
			Steinberg::Vst::MediaType mediaType;
			Steinberg::Vst::BusType busType;
			Steinberg::Vst::BusDirection direction;
			Steinberg::Vst::SpeakerArrangement speaker = Steinberg::Vst::SpeakerArr::kEmpty;
			uint32_t channelCount;
			bool isActive = false;
			bool isDefaultActive = false;
		};

		struct AudioBusesInfo
		{
			void Initialize(VSTPluginImplementation* owner, Steinberg::Vst::BusDirection dir);
			size_t GetNumBuses();

			BusInfo* GetBusInfo(uint32_t busIndex);

			size_t GetNumChannels();

			size_t GetNumActiveChannels();

			bool IsActive(size_t busIndex);
			void SetActive(size_t busIndex, bool state = true);

			bool SetSpeakerArrangement(size_t busIndex, Steinberg::Vst::SpeakerArrangement arr);

			std::vector<Steinberg::Vst::SpeakerArrangement> GetSpeakers();
			Steinberg::Vst::AudioBusBuffers* GetBusBuffers();

		private:
			VSTPluginImplementation* m_owner = nullptr;
			std::vector<BusInfo> m_busInfos;
			Steinberg::Vst::BusDirection m_direction;
			std::vector<Steinberg::Vst::AudioBusBuffers> m_busBuffers;

			void UpdateBusBuffers();
		};

		class PC_API VSTPlugin {
		public:
			VSTPlugin();
			~VSTPlugin();

			struct ParameterInfo {
				Steinberg::Vst::ParamID ID;
				std::string title;
				std::string shortTitle;
				std::string units;
				int32_t steps;
				Steinberg::Vst::ParamValue defaultNormalizedValue;
				Steinberg::Vst::UnitID unitID;

				bool canAutomate = false;
				bool isReadOnly = false;
				bool isWrapAround = false;
				bool isList = false;
				bool isProgramChange = false;
				bool isBypassed = false;
			};

			struct ProgramInfo
			{
				std::string name;
				std::string pluginName;
				std::string pluginCategory;
				std::string instrument;
				std::string style;
				std::string character;
				std::string stateType;
				std::string filePathStringType;
				std::string fileName;
			};

			struct ProgramList
			{
				std::string name;
				Steinberg::Vst::ProgramListID ID = Steinberg::Vst::kNoProgramListId;
				std::vector<ProgramInfo> programs;
			};

			struct UnitInfo
			{
				Steinberg::Vst::UnitID ID = Steinberg::Vst::kRootUnitId;
				Steinberg::Vst::UnitID parentID = Steinberg::Vst::kNoParentUnitId;
				std::string name;
				ProgramList programList;
				Steinberg::Vst::ParamID programChangeParam = Steinberg::Vst::kNoParamId;
			};

			struct VSTInfo {
				// stuff


			};

			struct ClassInfo {
			public:
				ClassInfo() = default;
				ClassInfo(Steinberg::PClassInfo& classInfo);
				ClassInfo(Steinberg::PClassInfo2& classInfo2);
				ClassInfo(Steinberg::PClassInfoW& classInfoW);

				~ClassInfo();
				
				inline void* CID() { return m_tuid; }
				inline std::string Name() { return m_name; }
				inline std::string Category() { return m_category; }
				inline Steinberg::int32 Cardinality() { return m_cardinality; }
				inline bool hasClassInfo2Data() { return m_classInfo2Data; }
				inline std::string Vendor() { return m_vendor; }
				inline std::string Version() { return m_version; }
				inline std::string SubCategories() { return m_subCategories; }
				inline std::string SDKVersion() { return m_sdkVersion; }

			private:
				void* m_tuid;
				std::string m_name;
				std::string m_category;
				std::string m_subCategories;
				std::string m_vendor;
				std::string m_version;
				std::string m_sdkVersion;
				Steinberg::int32 m_cardinality = -1;
				bool m_classInfo2Data = false;
			};

			struct FactoryInfo {
			public:
				FactoryInfo() = default;
				FactoryInfo(Steinberg::PFactoryInfo& info);

				bool Discardable();
				bool LicenseCheck();
				bool ComponentNonDiscardable();
				bool Unicode() ;

				std::string Vendor();
				std::string URL();
				std::string	Email();

			private:
				std::string m_vendor;
				std::string m_url;
				std::string m_email;
				Steinberg::int32 m_flags;
			};

			std::string getName();

			void Process(ProcessInfo& processInfo);

			void setSampleRate(uint32_t sampleRate);

			void RestartComponent(Steinberg::int32 flags);
		private:
		};

		class VSTPluginImplementation {
		public:
			VSTPluginImplementation(AudioSettings* settings,
				Steinberg::IPluginFactory* factory,
				VSTPlugin::FactoryInfo& factoryInfo,
				VSTPlugin::ClassInfo& classInfo,
				Steinberg::FUnknown* hostContext);

			~VSTPluginImplementation();

			bool HasEditor();
			bool OpenEditor(WindowHandle parent, Steinberg::IPlugFrame* plugFrame);
			void CloseEditor();
			bool IsEditorOpened();

			Steinberg::ViewRect GetPreferredRect();

			void Resume();
			void Suspend();
			bool IsResumed();

			void RestartComponent(Steinberg::int32 flags);

			void Process(ProcessInfo processInfo);

			void PushBackParameterChange(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value, int64_t offset = 0);

			// Getters / Setters

			void SetBlockSize(int blockSize);
			void SetSamplingRate(int samplingRate);

			VSTPlugin::FactoryInfo& GetFactoryInfo();
			VSTPlugin::ClassInfo& GetComponentInfo();

			bool HasEditController();
			bool HasEditController2();

			inline Steinberg::Vst::IComponent* GetComponent();
			inline Steinberg::Vst::IAudioProcessor* GetAudioProcessor();
			inline Steinberg::Vst::IEditController* GetEditController();
			inline Steinberg::Vst::IEditController2* GetEditController2();

			std::string GetEffectName();

			std::vector<VSTPlugin::ParameterInfo>& GetParameterInfoList();

			std::vector<VSTPlugin::UnitInfo>& GetUnitInfoList();

			AudioBusesInfo& GetBusesInfo(Steinberg::Vst::BusDirection dir);

			uint32_t GetNumParameters();
			Steinberg::Vst::ParamValue GetParameterValueByIndex(uint32_t index);
			Steinberg::Vst::ParamValue GetParameterValueByID(Steinberg::Vst::ParamID id);

			std::string ValueToStringByIndex(uint32_t index, Steinberg::Vst::ParamValue value);
			Steinberg::Vst::ParamValue StringToValueTByIndex(uint32_t index, std::string string);

			std::string ValueToStringByID(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value);
			Steinberg::Vst::ParamValue StringToValueByID(Steinberg::Vst::ParamID id, std::string string);

			uint32_t GetProgramIndex(Steinberg::Vst::UnitID unitID = 0);
			void SetProgramIndex(uint32_t index, Steinberg::Vst::UnitID unitID = 0);

			enum class Status {
				Invalid,
				Created,
				Initialized,
				SetupDone,
				Activated,
				Processing
			};
		
			friend struct AudioBusesInfo;
		private:
			void LoadPlugin(Steinberg::IPluginFactory* factory, VSTPlugin::ClassInfo& classInfo, Steinberg::FUnknown* hostContext);
			void Initialize(Steinberg::Vst::IComponentHandler* handler);
			void LoadInterfaces(Steinberg::IPluginFactory* factory, VSTPlugin::ClassInfo& classInfo, Steinberg::FUnknown* hostContext);
			void UnloadPlugin();

			Steinberg::tresult CreatePlugView();
			void DeletePlugView();

			void PrepareParameters();
			void PrepareUnitInfo();

			void UpdateBusBuffers();

			void PopFrontParameterChanges(Steinberg::Vst::ParameterChanges& dest);

			AudioSettings* m_settings;
	
			VSTPlugin::ClassInfo m_classInfo;
			VSTPlugin::FactoryInfo m_factoryInfo;
			Steinberg::Vst::IComponent* m_component;
			Steinberg::Vst::IAudioProcessor* m_audioProcessor;
			Steinberg::Vst::IEditController* m_editController;
			Steinberg::Vst::IEditController2* m_editController2;
			Steinberg::IPlugView* m_plugView;
			Steinberg::Vst::IUnitInfo* m_unitHandler;
			Steinberg::Vst::IMidiMapping* m_midiMapping;
			std::vector<VSTPlugin::ParameterInfo> m_parameterInfoList;
			std::vector<VSTPlugin::UnitInfo> m_unitInfoList;
			
			Steinberg::Vst::ProcessSetup m_currentProcessSetup = {};
			AudioBusesInfo m_inputBusesInfo;
			AudioBusesInfo m_outputBusesInfo;

			// Parameter Changes
			std::mutex m_ParameterChangesMutex;
			Steinberg::Vst::ParameterChanges   m_ParameterChangesQueue;
			Steinberg::Vst::ParameterChanges m_inParameterChanges;
			Steinberg::Vst::ParameterChanges m_outParameterChanges;
			Steinberg::Vst::EventList m_inputEvents;
			Steinberg::Vst::EventList m_outputEvents;

			float* m_inputBuffer;
			float* m_outputBuffer;

			// Flags
			bool m_editControllerIsCreatedNew;
			bool m_isEditorOpened;
			bool m_isProcessingStarted;
			bool m_hasEditor;
			Status m_status;
		};

		/*
		class VSTInstrument {



		};

		class VSTEffect : public Effect {
		public:

		private:


		};
		*/

	}
}