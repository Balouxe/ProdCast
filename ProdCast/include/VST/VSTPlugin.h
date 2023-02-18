/*
Basic implementation of VST handling. 
Recommended to use for basic audio applications, but has its limits, so consider making your own if you're making a DAW, for instance.
Main structure and some code taken from @hotwatermorning/Vst3HostDemo

Freak you Steinberg :D
*/


#pragma once
#include "Core.h" 
#include "Effect.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/gui/iplugview.h"

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
			Steinberg::PlugFrame
		};

		class VSTProcessInfo {
			float* inputBuffer;
			float* outputBuffer;
		};
			
		class VSTProcessor {
		public:
			VSTProcessor(VSTPlugin* plugin);

			std::string getName();

			void OnStartProcessing(uint32_t sampleRate, uint32_t blockSize);
			void Process(VSTProcessInfo& processInfo);
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
			uint32_t channel_count;
			bool is_active = false;
			bool is_default_active = false;
		};

		struct AudioBusesInfo
		{
			void Initialize(VSTPluginImplementation* owner, Steinerg::Vst::BusDirection dir);
			size_t GetNumBuses();

			BusInfo& GetBusInfo(UInt32 busIndex);

			size_t GetNumChannels();

			size_t GetNumActiveChannels();

			bool IsActive(size_t busIndex);
			void SetActive(size_t busIndex, bool state = true);

			bool SetSpeakerArrangement(size_t busIndex, Vst::SpeakerArrangement arr);

			std::vector<Vst::SpeakerArrangement> GetSpeakers();
			Vst::AudioBusBuffers* GetBusBuffers();

		private:
			VSTPluginImplementation* m_owner = nullptr;
			std::vector<BusInfo> m_busInfos;
			Vst::BusDirection m_direction;
			std::vector<Steinberg::Vst::AudioBusBuffers> m_busBuffers;

			void UpdateBusBuffers();
		};

		class VSTPluginImplementation {
		public:
			enum class Status {
				Invalid,
				Created,
				Initialized,
				SetupDone,
				Activated,
				Processing
			};

		private:

		};

		class PC_API VSTPlugin {
		public:
			VSTPlugin();
			~VSTPlugin();

			struct ParameterInfo {
				// stuff
			};

			struct VSTInfo {
				// stuff


			};

			struct FactoryInfo {
				// stuff

			};

			std::string getName();

			void Process(VSTProcessInfo& processInfo);

			void setSampleRate(uint32_t sampleRate);

		private:
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