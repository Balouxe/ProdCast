/*
Basic implementation of VST handling. 
Recommended to use for basic audio applications, but has its limits, so consider making your own if you're making a DAW, for instance.
Main structure and some code taken from @hotwatermorning/Vst3HostDemo

Freak you Steinberg :D
*/
#pragma once

#include "Logger.h" // spdlog includes windows.h already

#if defined(_MSC_VER)
#ifdef PC_NLOG
#include <windows.h>
#endif
using WindowHandle = HWND;
#else
using WindowHandle = NSView*;
#endif

#include "Core.h" 
#include "Effect.h"

#include "pluginterfaces/base/fplatform.h"
#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst/ivstunits.h"
#include "pluginterfaces/vst/vstpresetkeys.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "base/source/fobject.h"
#include "public.sdk/source/vst/vstinitiids.cpp"
#include "public.sdk/source/common/commoniids.cpp"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/eventlist.cpp"
#include "public.sdk/source/vst/hosting/parameterchanges.h"

#include <string>

namespace ProdCast {
	class ProdCastEngine;

	namespace VST {

		class VSTPlugin;
		class VSTPluginImplementation;

		struct VSTNote {
			// ...
		};

		class PC_API PlugFrameListener
		{
		protected:
			PlugFrameListener() {}
		public:
			virtual ~PlugFrameListener() {}
			virtual void OnResizePlugView(Steinberg::ViewRect const& newSize) = 0;
		};

		class PC_API VSTHostContext : public Steinberg::FObject, 
			public Steinberg::Vst::IHostApplication,
			public Steinberg::Vst::IComponentHandler,
			public Steinberg::Vst::IComponentHandler2,
			public Steinberg::IPlugFrame
		{
		public:
			OBJ_METHODS(VSTHostContext, Steinberg::FObject)
			REFCOUNT_METHODS(Steinberg::FObject)
			DEFINE_INTERFACES
			DEF_INTERFACE(Steinberg::Vst::IHostApplication)
			DEF_INTERFACE(Steinberg::Vst::IComponentHandler)
			DEF_INTERFACE(Steinberg::Vst::IComponentHandler2)
			DEF_INTERFACE(Steinberg::IPlugFrame)
			END_DEFINE_INTERFACES(Steinberg::FObject)

			VSTHostContext(std::string hostName);
			~VSTHostContext();

			// Getters/Setters
			void SetVST3Plugin(VSTPlugin* plugin);
			VSTPlugin* GetVST3Plugin();

			std::string m_hostName;
			VSTPlugin* m_plugin = nullptr;
			PlugFrameListener* m_plugFrameListener;
		protected:

			// IHostApplication virtual functions
			Steinberg::tresult PLUGIN_API getName(Steinberg::Vst::String128 name);

			Steinberg::tresult PLUGIN_API createInstance(Steinberg::TUID cid, Steinberg::TUID iid, void** obj);

			// IComponentHandler virtual functions
			Steinberg::tresult PLUGIN_API beginEdit(Steinberg::Vst::ParamID id);

			Steinberg::tresult PLUGIN_API performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue normalizedValue);

			Steinberg::tresult PLUGIN_API endEdit(Steinberg::Vst::ParamID id);

			Steinberg::tresult PLUGIN_API restartComponent(Steinberg::int32 flags);

			// IComponentHandler2 virtual functions

			Steinberg::tresult PLUGIN_API setDirty(Steinberg::TBool state);

			Steinberg::tresult PLUGIN_API requestOpenEditor(Steinberg::FIDString name = Steinberg::Vst::ViewType::kEditor);

			Steinberg::tresult PLUGIN_API startGroupEdit();

			Steinberg::tresult PLUGIN_API finishGroupEdit();

			// PlugFrame virtual functions

			Steinberg::tresult resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* newSize);
		};

		struct PC_API ProcessInfo {
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
			uint32_t numChannels;
		};
			
		class PC_API VSTProcessor {
		public:
			VSTProcessor(VSTPlugin* plugin);

			std::string getName();

			void OnStartProcessing(AudioSettings* settings);
			void Process(ProcessInfo& processInfo);
			void OnStopProcessing();

			VSTPlugin* getPlugin();

		private:
			VSTPlugin* m_plugin;
		};

		struct PC_API BusInfo
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

		struct PC_API AudioBusesInfo
		{
			void Initialize(VSTPluginImplementation* owner, Steinberg::Vst::BusDirection dir);
			uint32_t GetNumBuses();

			BusInfo* GetBusInfo(uint32_t busIndex);

			uint32_t GetNumChannels();

			uint32_t GetNumActiveChannels();

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
			VSTPlugin(VSTPluginImplementation* impl,
				VSTHostContext* hostContext,
				std::function<void(VSTPlugin* p)> onDestruction);

			virtual ~VSTPlugin();

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

			class ClassInfo {
			public:
				ClassInfo() = default;
				ClassInfo(Steinberg::PClassInfo& classInfo);
				ClassInfo(Steinberg::PClassInfo2& classInfo2);
				ClassInfo(Steinberg::PClassInfoW& classInfoW);

				~ClassInfo();
				
				std::array<Steinberg::int8, 16>& CID() { return m_cid; }
				inline std::string Name() { return m_name; }
				inline std::string Category() { return m_category; }
				inline Steinberg::int32 Cardinality() { return m_cardinality; }
				inline bool hasClassInfo2Data() { return m_classInfo2Data; }
				inline std::string Vendor() { return m_vendor; }
				inline std::string Version() { return m_version; }
				inline std::string SubCategories() { return m_subCategories; }
				inline std::string SDKVersion() { return m_sdkVersion; }

			private:
				std::array<Steinberg::int8, 16> m_cid = {{}};
				std::string m_name;
				std::string m_category;
				std::string m_subCategories;
				std::string m_vendor;
				std::string m_version;
				std::string m_sdkVersion;
				Steinberg::int32 m_cardinality = -1;
				bool m_classInfo2Data = false;
			};

			struct PC_API FactoryInfo {
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

			FactoryInfo& GetFactoryInfo();
			ClassInfo & GetComponentInfo();

			std::string GetEffectName();
			uint32_t GetNumInputs();
			uint32_t GetNumOutputs();

			uint32_t GetNumParams();
			ParameterInfo& GetParameterInfoByIndex(uint32_t index);
			ParameterInfo& GetParameterInfoByID(Steinberg::Vst::ParamID id);

			uint32_t GetNumUnitInfo();
			UnitInfo& GetUnitInfoByIndex(uint32_t index);
			UnitInfo& GetUnitInfoByID(Steinberg::Vst::UnitID id);

			uint32_t  GetNumBuses(Steinberg::Vst::BusDirection dir);
			BusInfo& GetBusInfoByIndex(Steinberg::Vst::BusDirection dir, uint32_t index);

			Steinberg::Vst::ParamValue GetParameterValueByIndex(uint32_t index);
			Steinberg::Vst::ParamValue GetParameterValueByID(Steinberg::Vst::ParamID id);

			std::string ValueToStringByIndex(uint32_t index, Steinberg::Vst::ParamValue value);
			Steinberg::Vst::ParamValue StringToValueTByIndex(uint32_t index, std::string string);

			std::string ValueToStringByID(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value);
			Steinberg::Vst::ParamValue StringToValueByID(Steinberg::Vst::ParamID id, std::string string);

			bool IsBusActive(Steinberg::Vst::BusDirection dir, uint32_t index);
			void SetBusActive(Steinberg::Vst::BusDirection dir, uint32_t index, bool state = true);
			Steinberg::Vst::SpeakerArrangement GetSpeakerArrangementForBus(Steinberg::Vst::BusDirection dir, uint32_t index);
			bool SetSpeakerArrangement(Steinberg::Vst::BusDirection dir, uint32_t index, Steinberg::Vst::SpeakerArrangement arr);

			void Resume();
			void Suspend();
			bool IsResumed() ;
			
			void SetSettings(AudioSettings* settings);

			bool HasEditor() ;
			bool OpenEditor(WindowHandle wnd, PlugFrameListener* listener);
			void CloseEditor();
			bool IsEditorOpened() ;
			Steinberg::ViewRect GetPreferredRect();

			uint32_t GetProgramIndex(Steinberg::Vst::UnitID unitID = 0);
			void SetProgramIndex(uint32_t index, Steinberg::Vst::UnitID unitID = 0);

			void EnqueueParameterChange(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value);

			void RestartComponent(Steinberg::int32 flags);

			void Process(ProcessInfo& pi);
		private:
			VSTPluginImplementation* m_implementation;
			VSTHostContext* m_hostContext;
			std::function<void(VSTPlugin* p)> m_onDestruction;
		};

		class PC_API VSTPluginImplementation {
		public:
			VSTPluginImplementation(Steinberg::IPluginFactory* factory,
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

			void SetAudioSettings(AudioSettings* settings);

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

			void ResizeBuffers(uint32_t inputNumChannels, uint32_t outputNumChannels, uint32_t numSamples);

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

			std::vector<float> m_inputBuffer;
			std::vector<float> m_outputBuffer;
			std::vector<float*> m_inputBufferHeads;
			std::vector<float*> m_outputBufferHeads;

			// Flags
			bool m_editControllerIsCreatedNew;
			bool m_isEditorOpened;
			bool m_isProcessingStarted;
			bool m_hasEditor;
			Status m_status;
		};

		class Module {
		public:
			// Per platform module loading
#if defined (_MSC_VER)
			typedef HMODULE platformModule;

			static platformModule loadImpl(const char* path) { return LoadLibraryA(path); }
			static platformModule loadImpl(const wchar_t* path) { return LoadLibraryW(path); }
			static void unloadImpl(platformModule handle) { assert(handle); FreeLibrary(handle); }
			static void* getProcAddressImpl(platformModule module, char const* function_name)
			{
				return GetProcAddress(module, function_name);
			}
#else
			// not supported
#error "Only supported on windows!"
#endif

			Module() = default;
			explicit Module(char const* path) : m_module(loadImpl(path)) {}
			explicit Module(wchar_t const* path) : m_module(loadImpl(path)) {}
			Module(platformModule mod) : m_module(mod){}
			Module(Module&& mod) : m_module(std::exchange(mod.m_module, platformModule())) { mod.m_module = nullptr; }
			Module& operator=(Module&& r){
				m_module = std::exchange(r.m_module, platformModule());
				return *this;
			}
			void* getProcAddress(char const* function_name)
			{
				return getProcAddressImpl(m_module, function_name);
			}
			~Module()
			{
				if (m_module) { unloadImpl(m_module); }
			}
			
			platformModule& GetModule() { return m_module; }

		private:
			platformModule m_module = platformModule();

		};

#undef GetClassInfo

		class PC_API VSTPluginFactory {
		public:
			VSTPluginFactory(std::string modulePath);
			~VSTPluginFactory();

			VSTPlugin::FactoryInfo& GetFactoryInfo();
			uint32_t GetComponentCount();
			VSTPlugin::ClassInfo& GetComponentInfo(uint32_t index);
			VSTPlugin* CreateByIndex(uint32_t index);
			VSTPlugin* CreateByID(void* componentID);
			uint32_t GetNumLoadedPlugins();

			void OnVSTPluginIsCreated(VSTPlugin* plugin);

			void OnVSTPluginIsDestructed(VSTPlugin* plugin); /* {
				auto found = std::find(loaded_plugins_.begin(), loaded_plugins_.end(), p);
				assert(found != loaded_plugins_.end());
				loaded_plugins_.erase(found);
			}*/
		private:
			Module m_module;
			typedef void (PLUGIN_API* SetupProc)(); // Function pointer
			Steinberg::IPluginFactory* m_factory;
			VSTPlugin::FactoryInfo m_factoryInfo;
			std::vector<VSTPlugin::ClassInfo>	m_classInfoList;
			std::vector<VSTPlugin*> m_loadedPlugins;
		};

		class PC_API VSTHandler final {
		public:
			static VSTPluginFactory* FindOrCreateFactory(std::string modulePath);

			static void ShrinkList();
		private:
			static std::mutex m_mutex;
			static std::map<std::string, VSTPluginFactory*> m_map;
		};


		class VSTInstrument {



		};

		class PC_API VSTEffect : public Effect {
		public:
			VSTEffect(ProdCast::ProdCastEngine* engine, const char* path);
			VSTEffect(ProdCast::ProdCastEngine* engine);
			~VSTEffect();

			void LoadEffect(const char* path);

			void ProcessBuffer(float* buffer, unsigned int bufferSize, unsigned int numChannels);

			VSTPlugin* getPlugin();
		private:
			ProdCastEngine* m_engine;
			AudioSettings* m_settings;

			VSTPlugin* m_plugin;
			ProcessInfo::TransportInfo m_TransportInfo;

			bool m_initialized = false;
			const char* m_path;
		};

	}
}