#include "VST/VSTPlugin.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "pluginterfaces/base/fplatform.h"
#include "public.sdk/source/common/memorystream.cpp"

#include <string>
#include <codecvt>
#include <numeric>
#include <memory>

#include "Logger.h"

namespace ProdCast {
namespace VST {

	// Useful functions (why do you have your own data types steinberg!!!!! >:( seriously, utf-16 :skull: )

	std::string S128to_string(Steinberg::Vst::String128& string) {
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
		std::string a = convert.to_bytes(string);
		return a;
	}

	std::string C16to_string(Steinberg::char16* string) {
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
		std::string a = convert.to_bytes(string);
		return a;
	}

	std::u16string to_u16string(std::string& string) {
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
		std::u16string a = convert.from_bytes(string);
		return a;
	}

	// VSTHostContext

	VSTHostContext::VSTHostContext(std::string hostName) : m_hostName(hostName) {

	}

	VSTHostContext::~VSTHostContext() {

	}

	void VSTHostContext::SetVST3Plugin(VSTPlugin* plugin) {
		m_plugin = plugin;
	}

	VSTPlugin* VSTHostContext::GetVST3Plugin() {
		return m_plugin;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::getName(Steinberg::Vst::String128 name) {
		const char16_t* s16 = to_u16string(m_hostName).c_str();
		memcpy(&name, s16, 128 * sizeof(char16_t));

		// name = to_u16string(m_hostName).c_str();
		return Steinberg::kResultOk;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::createInstance(Steinberg::TUID cid, Steinberg::TUID iid, void** obj) {
		Steinberg::FUID const classID = Steinberg::FUID::fromTUID(cid);
		Steinberg::FUID const instanceID = Steinberg::FUID::fromTUID(iid);
		if (classID == Steinberg::Vst::IMessage::iid && instanceID == Steinberg::Vst::IMessage::iid) {
			*obj = new Steinberg::Vst::HostMessage;
			return Steinberg::kResultTrue;
		} else if (classID == Steinberg::Vst::IAttributeList::iid && instanceID == Steinberg::Vst::IAttributeList::iid) {
			// *obj = new Steinberg::Vst::HostAttributeList::make()
			return Steinberg::kResultTrue;
		}
		*obj = nullptr;
		return Steinberg::kResultFalse;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::beginEdit(Steinberg::Vst::ParamID id) {
		return Steinberg::kResultOk;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue normalizedValue) {
		PC_INFO("Editing parameter {0} as {1}", id, normalizedValue);
		// m_plugin->ChangeParameter(id, normalizedValue);
		return Steinberg::kResultOk;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::endEdit(Steinberg::Vst::ParamID id) {
		return Steinberg::kResultOk;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::restartComponent(Steinberg::int32 flags) {
		if (m_plugin)
			m_plugin->RestartComponent(flags);
		return Steinberg::kResultOk;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::setDirty(Steinberg::TBool state) {
		// ?
		return Steinberg::kResultOk;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::requestOpenEditor(Steinberg::FIDString name) {
		// ?
		return Steinberg::kResultOk;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::startGroupEdit() {
		return Steinberg::kResultOk;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::finishGroupEdit() {
		return Steinberg::kResultOk;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* newSize) {
		Steinberg::ViewRect current;
		view->getSize(&current);

		view->onSize(newSize);
		return Steinberg::kResultOk;
	}
	
	// VSTProcessor

	VSTProcessor::VSTProcessor(VSTPlugin* plugin) : m_plugin(plugin) {};

	std::string VSTProcessor::getName() {
		return m_plugin->getName();
	}

	void VSTProcessor::OnStartProcessing(uint32_t sampleRate, uint32_t blockSize) {
		m_plugin->setSampleRate(sampleRate);
	}

	void VSTProcessor::Process(ProcessInfo& processInfo) {
		m_plugin->Process(processInfo);
	}

	void VSTProcessor::OnStopProcessing() {

	}

	// AudioBusesInfo

	void AudioBusesInfo::Initialize(VSTPluginImplementation* owner, Steinberg::Vst::BusDirection dir)
	{
		m_owner = owner;
		m_direction = dir;

		Steinberg::Vst::IComponent* comp = m_owner->m_component;
		auto& proc = m_owner->m_audioProcessor;

		Steinberg::Vst::MediaTypes const media = Steinberg::Vst::MediaTypes::kAudio;

		size_t const numBuses = comp->getBusCount(media, dir);
		m_busInfos.resize(numBuses);

		Steinberg::tresult ret = Steinberg::kResultTrue;
		for (size_t i = 0; i < numBuses; ++i) {
			Steinberg::Vst::BusInfo vBusInfo;
			ret = comp->getBusInfo(media, dir, i, vBusInfo);
			if (ret != Steinberg::kResultTrue) { PC_ERROR("VST: Failed to get BusInfo"); }

			BusInfo busInfo;
			busInfo.busType = vBusInfo.busType;
			busInfo.channelCount = vBusInfo.channelCount;
			busInfo.direction = vBusInfo.direction;
			busInfo.isDefaultActive = (vBusInfo.flags & Steinberg::Vst::BusInfo::kDefaultActive) != 0;
			busInfo.mediaType = vBusInfo.mediaType;
			busInfo.name = S128to_string(vBusInfo.name);
			busInfo.isActive = busInfo.isDefaultActive;

			Steinberg::Vst::SpeakerArrangement arr;
			auto ret = proc->getBusArrangement(dir, i, arr);
			if (ret != Steinberg::kResultTrue) { PC_ERROR("VST: Failed to get SpeakerArrangement!"); }
			busInfo.speaker = arr;

			m_busInfos[i] = busInfo;
			
		}

		UpdateBusBuffers();
	}

	size_t AudioBusesInfo::GetNumBuses() {
		return m_busInfos.size();
	}

	BusInfo* AudioBusesInfo::GetBusInfo(uint32_t busIndex)
	{
		if (busIndex < GetNumBuses())
			return &m_busInfos[busIndex];
		else
			return nullptr;
	}

	size_t AudioBusesInfo::GetNumChannels()
	{
		return std::accumulate(m_busInfos.begin(),
			m_busInfos.end(),
			0,
			[](int sum, auto const& info) {
				return sum + info.channelCount;
			});
	}

	size_t AudioBusesInfo::GetNumActiveChannels()
	{
		return std::accumulate(m_busInfos.begin(),
			m_busInfos.end(),
			0,
			[](int sum, auto const& info) {
				return sum + (info.isActive ? info.channelCount : 0);
			});
	}

	bool AudioBusesInfo::IsActive(size_t busIndex)
	{
		return GetBusInfo(busIndex)->isActive;
	}

	void AudioBusesInfo::SetActive(size_t busIndex, bool state)
	{
		if (busIndex >= GetNumBuses())
			return;

		auto& comp = m_owner->m_component;
		auto const result = comp->activateBus(Steinberg::Vst::MediaTypes::kAudio, m_direction, busIndex, state);
		if (result != Steinberg::kResultTrue) {
			PC_ERROR("VST: Failed to activate bus {}!", busIndex);
		}

		m_busInfos[busIndex].isActive = state;
	}

	bool AudioBusesInfo::SetSpeakerArrangement(size_t busIndex, Steinberg::Vst::SpeakerArrangement arr)
	{
		if (busIndex >= GetNumBuses()) {
			return false;
		}

		auto inputArrs = m_owner->m_inputBusesInfo.GetSpeakers();
		auto outputArrs = m_owner->m_outputBusesInfo.GetSpeakers();

		auto& arrsDest = (m_direction == Steinberg::Vst::BusDirections::kInput ? inputArrs : outputArrs);
		arrsDest[busIndex] = arr;

		auto& processor = m_owner->m_audioProcessor;
		auto const result = processor->setBusArrangements(inputArrs.data(), inputArrs.size(),
			outputArrs.data(), outputArrs.size());

		if (result != Steinberg::kResultTrue) {
			PC_ERROR("VST: Failed setting speaker arrangement!");
			return false;
		}

		m_busInfos[busIndex].speaker = arr;
		m_busInfos[busIndex].channelCount = Steinberg::Vst::SpeakerArr::getChannelCount(arr);
		UpdateBusBuffers();
		return true;
	}

	std::vector<Steinberg::Vst::SpeakerArrangement> AudioBusesInfo::GetSpeakers()
	{
		std::vector<Steinberg::Vst::SpeakerArrangement> arrs;
		size_t num = GetNumBuses();
		for (size_t i = 0; i < num; ++i) {
			arrs.push_back(GetBusInfo(i)->speaker);
		}

		return arrs;
	}

	Steinberg::Vst::AudioBusBuffers* AudioBusesInfo::GetBusBuffers()
	{
		return m_busBuffers.data();
	}

	void AudioBusesInfo::UpdateBusBuffers()
	{
		m_busBuffers.clear();

		for (auto const& busInfo : m_busInfos) {
			Steinberg::Vst::AudioBusBuffers tmp = {};
			tmp.numChannels = busInfo.channelCount;
			m_busBuffers.push_back(tmp);
		}
	}

	// ClassInfo

	VSTPlugin::ClassInfo::ClassInfo(Steinberg::PClassInfo& info) {
		m_name = info.name;
		m_cardinality = info.cardinality;
		m_category = info.category;

		m_tuid = malloc(16 * sizeof(char));
		memcpy(m_tuid, &info.cid, 16 * sizeof(char));
	}

	VSTPlugin::ClassInfo::ClassInfo(Steinberg::PClassInfo2& info) {
		m_name = info.name;
		m_cardinality = info.cardinality;
		m_category = info.category;
		m_subCategories = info.subCategories;
		m_version = info.version;
		m_sdkVersion = info.sdkVersion;
		m_vendor = info.vendor;
		m_classInfo2Data = true;

		m_tuid = malloc(16 * sizeof(char));
		memcpy(m_tuid, &info.cid, 16 * sizeof(char));
	}

	VSTPlugin::ClassInfo::ClassInfo(Steinberg::PClassInfoW& info) {
		m_name = C16to_string(&info.name[0]);
		m_cardinality = info.cardinality;
		m_category = info.category;
		m_subCategories = info.subCategories;
		m_version = C16to_string(&info.version[0]);
		m_sdkVersion = C16to_string(&info.sdkVersion[0]);
		m_vendor = C16to_string(&info.vendor[0]);
		m_classInfo2Data = true;

		m_tuid = malloc(16 * sizeof(char));
		memcpy(m_tuid, &info.cid, 16 * sizeof(char));
	}

	VSTPlugin::ClassInfo::~ClassInfo() {
		free(m_tuid);
	}

	// FactoryInfo

	VSTPlugin::FactoryInfo::FactoryInfo(Steinberg::PFactoryInfo& info) : m_flags(info.flags), m_email(info.email), m_url(info.url), m_vendor(info.vendor) {}

	std::string VSTPlugin::FactoryInfo::Email() {
		return m_email;
	}

	std::string VSTPlugin::FactoryInfo::Vendor() {
		return m_vendor;
	}

	std::string VSTPlugin::FactoryInfo::URL() {
		return m_url;
	}

	bool VSTPlugin::FactoryInfo::Discardable()
	{
		return (m_flags & Steinberg::PFactoryInfo::FactoryFlags::kClassesDiscardable) != 0;
	}

	bool VSTPlugin::FactoryInfo::LicenseCheck()
	{
		return (m_flags & Steinberg::PFactoryInfo::FactoryFlags::kLicenseCheck) != 0;
	}

	bool VSTPlugin::FactoryInfo::ComponentNonDiscardable()
	{
		return (m_flags & Steinberg::PFactoryInfo::FactoryFlags::kComponentNonDiscardable) != 0;
	}

	bool VSTPlugin::FactoryInfo::Unicode()
	{
		return (m_flags & Steinberg::PFactoryInfo::FactoryFlags::kUnicode) != 0;
	}

	// VSTPlugin

	VSTPlugin* VSTProcessor::getPlugin() {
		return m_plugin;
	}

	VSTPlugin::VSTPlugin() {

	}

	VSTPlugin::~VSTPlugin() {

	}

	std::string VSTPlugin::getName() {
		return "";
	}

	void VSTPlugin::setSampleRate(uint32_t sampleRate) {

	}

	// Useful for VSTPluginImplementation

	VSTPlugin::ProgramList CreateProgramList(Steinberg::Vst::IUnitInfo* unitHandler, Steinberg::Vst::ProgramListID programListID) {
		VSTPlugin::ProgramList programList;
		Steinberg::int32 num = unitHandler->getProgramListCount();
		for (int i = 0; i < num; ++i) {
			Steinberg::Vst::ProgramListInfo plInfo;
			unitHandler->getProgramListInfo(i, plInfo);
			if (plInfo.id != programListID) { continue; }

			programList.ID = programListID;
			programList.name = S128to_string(plInfo.name);
			for (int pn = 0; pn < plInfo.programCount; ++pn) {
				VSTPlugin::ProgramInfo info;

				Steinberg::Vst::String128 buf = {};
				unitHandler->getProgramName(programListID, pn, buf);
				info.name = S128to_string(buf);

				auto get_attr = [&](auto key) {
					Steinberg::Vst::String128 buf = {};
					unitHandler->getProgramInfo(programListID, pn, key, buf);
					return S128to_string(buf);
				};

				namespace PA = Steinberg::Vst::PresetAttributes;
				info.pluginName = get_attr(PA::kPlugInName);
				info.pluginCategory = get_attr(PA::kPlugInCategory);
				info.instrument = get_attr(PA::kInstrument);
				info.style = get_attr(PA::kStyle);
				info.character = get_attr(PA::kCharacter);
				info.stateType = get_attr(PA::kStateType);
				info.filePathStringType = get_attr(PA::kFilePathStringType);
				info.fileName = get_attr(PA::kFileName);
				programList.programs.push_back(info);
			}
			break;
		}

		return programList;
	};

	Steinberg::Vst::ParamID FindProgramChangeParam(std::vector<VSTPlugin::ParameterInfo>& list, Steinberg::Vst::UnitID unitID)
	{
		for (auto& entry : list) {
			if (entry.unitID == unitID && entry.isProgramChange) {
				return entry.ID;
			}
		}

		return Steinberg::Vst::kNoParamId;
	}

	// VSTPluginImplementation

	VSTPluginImplementation::VSTPluginImplementation(AudioSettings* settings,
		Steinberg::IPluginFactory* factory,
		VSTPlugin::FactoryInfo& factoryInfo,
		VSTPlugin::ClassInfo& classInfo,
		Steinberg::FUnknown* hostContext)
	{
		m_factoryInfo = factoryInfo;
		m_editControllerIsCreatedNew = false;
		m_isEditorOpened = false;
		m_isProcessingStarted = false;
		m_settings = settings;
		m_hasEditor = false;
		m_status = Status::Invalid;

		if (!hostContext) {
			PC_ERROR("VST: Missing host context!");
			return;
		}

		LoadPlugin(factory, classInfo, hostContext);
		m_inputEvents.setMaxSize(128);
		m_outputEvents.setMaxSize(128);
	}

	VSTPluginImplementation::~VSTPluginImplementation() {
		UnloadPlugin();

		if (m_component) {
			m_component->terminate();
		}

		if (m_editControllerIsCreatedNew) {
			m_editController->terminate();
		}
	}

	void VSTPluginImplementation::LoadPlugin(Steinberg::IPluginFactory* factory, VSTPlugin::ClassInfo& classInfo, Steinberg::FUnknown* hostContext) {
		LoadInterfaces(factory, classInfo, hostContext);

		Steinberg::Vst::IComponentHandler* handler = nullptr;

		Steinberg::tresult res = hostContext->queryInterface(Steinberg::Vst::IComponentHandler::iid, (void**)&handler);

		if (res == Steinberg::kResultOk && handler) {
			Initialize(handler);
		}
		else {
			PC_ERROR("VST: Couldn't initialize or something!");
			return;
		}
	}

	void VSTPluginImplementation::UnloadPlugin() {
		if (m_status == Status::Activated || m_status == Status::Processing) {
			Suspend();
		}

		Steinberg::Vst::IConnectionPoint* componentConnectionPoint = nullptr;
		Steinberg::tresult res = m_component->queryInterface(Steinberg::Vst::IConnectionPoint::iid, (void**)&componentConnectionPoint);

		Steinberg::Vst::IConnectionPoint* controllerConnectionPoint = nullptr;
		Steinberg::tresult res2 = m_editController->queryInterface(Steinberg::Vst::IConnectionPoint::iid, (void**)&controllerConnectionPoint);

		if (componentConnectionPoint && controllerConnectionPoint) {
			componentConnectionPoint->disconnect(controllerConnectionPoint);
			controllerConnectionPoint->disconnect(componentConnectionPoint);
		}

		m_editController->setComponentHandler(nullptr);

		// ??????? might just be m_unitHandler = nullptr;

		delete m_unitHandler;
		delete m_plugView;

		if (m_editController && m_editControllerIsCreatedNew) {
			m_editController->terminate();
		}

		delete m_editController2;
		delete m_editController;

		delete m_audioProcessor;

		if (m_component) {
			m_component->terminate();
		}

		delete m_component;
	}

	void VSTPluginImplementation::LoadInterfaces(Steinberg::IPluginFactory* factory, VSTPlugin::ClassInfo& classInfo, Steinberg::FUnknown* hostContext) {
		Steinberg::FUID fuid = Steinberg::FUID::fromTUID((const char*)classInfo.CID());
		if (!factory) {
			PC_ERROR("VST: Plugin Factory is null!");
			return;
		}

		Steinberg::Vst::IComponent* component = nullptr;
		Steinberg::tresult res = factory->createInstance(fuid, Steinberg::Vst::IComponent::iid, (void**)&component);
		if (!(res == Steinberg::kResultTrue && component)) {
			PC_ERROR("VST: Failed creating instance of IPluginFactory!");
			return;
		}
		res = component->setIoMode(Steinberg::Vst::IoModes::kAdvanced);
		if (!(res == Steinberg::kResultOk && res == Steinberg::kNotImplemented)) {
			PC_ERROR("VST: Couldn't set VST IO mode!");
			return;
		}

		m_status = Status::Created;

		res = component->initialize(hostContext);
		if (res != Steinberg::kResultOk) {
			PC_ERROR("VST: Couldn't initialize VST!");
			return;
		}

		m_status = Status::Initialized;

		Steinberg::Vst::IAudioProcessor* audioProcessor = nullptr;
		res = component->queryInterface(Steinberg::Vst::IAudioProcessor::iid, (void**)&audioProcessor);
		if (!(res == Steinberg::kResultTrue && audioProcessor)) {
			PC_ERROR("VST: Failed getting VST audio processor!");
			return;
		}

		res = audioProcessor->canProcessSampleSize(Steinberg::Vst::SymbolicSampleSizes::kSample32);
		if (res != Steinberg::kResultOk) {
			PC_ERROR("VST: VST cannot process 32 bits floats!");
			return;
		}

		Steinberg::Vst::IEditController* editController = nullptr;
		bool editControllerIsCreatedNew = false;
		res = component->queryInterface(Steinberg::Vst::IEditController::iid, (void**)&editController);
		if (!editController) {
			Steinberg::TUID controllerID;
			res = component->getControllerClassId(controllerID);
			if (res == Steinberg::kResultOk) {
				res = factory->createInstance(Steinberg::FUID::fromTUID(controllerID), Steinberg::Vst::IEditController::iid, (void**)&editController);
				if (editController) {
					editControllerIsCreatedNew = true;
				}
				else {
					PC_INFO("VST: VST doesn't have edit controller");
				}
			}
		}

		if (!editController) {
			PC_ERROR("VST: Couldn't get VST edit controller!");
			return;
		}

		if (editControllerIsCreatedNew) {
			res = editController->initialize(hostContext);
			if (res != Steinberg::kResultOk) {
				PC_ERROR("VST: Couldn't initialize edit controller!");
				return;
			}
		}

		Steinberg::Vst::IEditController2* editController2 = nullptr;
		if (editController) {
			res = editController->queryInterface(Steinberg::Vst::IEditController2::iid, (void**)&editController2);
			if (!res) {
				PC_TRACE("VST: VST doesn't have edit controller 2");
			}
		}

		Steinberg::Vst::IMidiMapping* midiMapping = nullptr;
		res = editController->queryInterface(Steinberg::Vst::IMidiMapping::iid, (void**)&midiMapping);
		if (!midiMapping) {
			PC_INFO("VST has MIDI Mapping");
		}

		m_classInfo = classInfo;
		m_component = component;
		m_audioProcessor = audioProcessor;
		m_editController = editController;
		m_editController2 = editController2;
		m_editControllerIsCreatedNew = editControllerIsCreatedNew;
		if (midiMapping) {
			m_midiMapping = midiMapping;
		}
	}

	void VSTPluginImplementation::Initialize(Steinberg::Vst::IComponentHandler* handler) {
		Steinberg::tresult res;
		Steinberg::tresult res2;

		if (!m_editController) {
			return;
		}

		if (handler) {
			res = m_editController->setComponentHandler(handler);
			if (res != Steinberg::kResultOk) {
				PC_ERROR("VST: Couldn't set component handler to edit controller!");
				return;
			}
		}

		Steinberg::Vst::IConnectionPoint* componentConnectionPoint = nullptr;
		res = m_component->queryInterface(Steinberg::Vst::IConnectionPoint::iid, (void**)&componentConnectionPoint);

		Steinberg::Vst::IConnectionPoint* controllerConnectionPoint = nullptr;
		res2 = m_editController->queryInterface(Steinberg::Vst::IConnectionPoint::iid, (void**)&controllerConnectionPoint);

		if (componentConnectionPoint && controllerConnectionPoint) {	
			componentConnectionPoint->connect(controllerConnectionPoint);
			controllerConnectionPoint->connect(componentConnectionPoint);
		}

		Steinberg::MemoryStream stream;
		if (m_component->getState(&stream) == Steinberg::kResultOk) {
			stream.seek(0, Steinberg::IBStream::IStreamSeekMode::kIBSeekSet, 0);
			m_editController->setComponentState(&stream);
		}

		Steinberg::Vst::IUnitInfo* unitInfo = nullptr;
		res = m_editController->queryInterface(Steinberg::Vst::IUnitInfo::iid, (void**)&unitInfo);
		if (unitInfo) {
			m_unitHandler = unitInfo;
			if (m_unitHandler->getUnitCount() == 0) {
				PC_WARN("VST: VST doesn't have any unit info");
				m_unitHandler = nullptr;
			}
		}

		if (!m_unitHandler) {
			PC_WARN("VST: No IUnitInfo interface!");
		}

		m_inputBusesInfo.Initialize(this, Steinberg::Vst::BusDirections::kInput);
		m_outputBusesInfo.Initialize(this, Steinberg::Vst::BusDirections::kOutput);

		for (int i = 0; i < m_inputBusesInfo.GetNumBuses(); i++) {
			m_inputBusesInfo.SetActive(i);
		}

		for (int i = 0; i < m_outputBusesInfo.GetNumBuses(); i++) {
			m_outputBusesInfo.SetActive(i);
		}

		std::vector<Steinberg::Vst::SpeakerArrangement> inputSpeakers = m_inputBusesInfo.GetSpeakers();
		std::vector<Steinberg::Vst::SpeakerArrangement> outputSpeakers = m_outputBusesInfo.GetSpeakers();

		res = m_audioProcessor->setBusArrangements(inputSpeakers.data(), inputSpeakers.size(), outputSpeakers.data(), outputSpeakers.size());
		
		if (res != Steinberg::kResultOk) {
			PC_ERROR("VST: Failed to set bus arrangement!");
		}

		res = CreatePlugView();
		m_hasEditor = (res == Steinberg::kResultOk);
		PrepareParameters();
		PrepareUnitInfo();

		m_inParameterChanges.setMaxParameters(m_parameterInfoList.size());
		m_outParameterChanges.setMaxParameters(m_parameterInfoList.size());
	}

	Steinberg::tresult VSTPluginImplementation::CreatePlugView() {
		if (!m_editController) {
			PC_ERROR("VST: Tried to create plug view without edit controller!");
			return Steinberg::kResultFalse;
		}

		m_plugView = m_editController->createView(Steinberg::Vst::ViewType::kEditor);

		if (!m_plugView) {
			Steinberg::IPlugView* plugView = nullptr;
			Steinberg::tresult res = m_editController->queryInterface(Steinberg::IPlugView::iid, (void**)&plugView);
			if (plugView) {
				m_plugView = plugView;
			}
			else {
				return res;
			}
		}

#if defined(_MSC_VER)
		if (m_plugView->isPlatformTypeSupported(Steinberg::kPlatformTypeHWND) == Steinberg::kResultOk) {
			PC_TRACE("VST: Plugin editor supports HWND");
		}
		else {
			return Steinberg::kNotImplemented;
		}
#else
		if (m_plugView->isPlatformTypeSupported(Steinberg::kPlatformTypeNSView) == Steinberg::kResultOk) {
			PC_TRACE("VST: Plugin editor supports NS View");
		}
		else if (m_plugView->isPlatformTypeSupported(Steinberg::kPlatformTypeHIView) == Steinberg::kResultOk) {
			PC_TRACE("VST: Plugin editor supports HI View");
		}
		else {
			return Steinberg::kNotImplemented;
		}
#endif

		return Steinberg::kResultOk;
	}

	void VSTPluginImplementation::DeletePlugView() {
		if (IsEditorOpened() == true) {
			PC_ERROR("Cannot delete plug view whilst editor is opened!");
			return;
		}

		// ??
		delete m_plugView;
	}

	void VSTPluginImplementation::PrepareParameters() {
		for (Steinberg::int32 i = 0; i < m_editController->getParameterCount(); i++) {
			Steinberg::Vst::ParameterInfo VSTparameterInfo = {};
			m_editController->getParameterInfo(i, VSTparameterInfo);
			VSTPlugin::ParameterInfo parameterInfo;
			parameterInfo.ID = VSTparameterInfo.id;
			parameterInfo.title = S128to_string(VSTparameterInfo.title);
			parameterInfo.shortTitle = S128to_string(VSTparameterInfo.shortTitle);
			parameterInfo.units = S128to_string(VSTparameterInfo.units);
			parameterInfo.steps = VSTparameterInfo.stepCount;
			parameterInfo.defaultNormalizedValue = VSTparameterInfo.defaultNormalizedValue;
			parameterInfo.unitID = VSTparameterInfo.unitId;
			parameterInfo.canAutomate = (VSTparameterInfo.flags & Steinberg::Vst::ParameterInfo::kCanAutomate) != 0;
			parameterInfo.isReadOnly = (VSTparameterInfo.flags & Steinberg::Vst::ParameterInfo::kIsReadOnly) != 0;
			parameterInfo.isWrapAround = (VSTparameterInfo.flags & Steinberg::Vst::ParameterInfo::kIsWrapAround) != 0;
			parameterInfo.isList = (VSTparameterInfo.flags & Steinberg::Vst::ParameterInfo::kIsList) != 0;
			parameterInfo.isProgramChange = (VSTparameterInfo.flags & Steinberg::Vst::ParameterInfo::kIsProgramChange) != 0;
			parameterInfo.isBypassed= (VSTparameterInfo.flags & Steinberg::Vst::ParameterInfo::kIsBypass) != 0;

			m_parameterInfoList.push_back(parameterInfo);
		}
	}

	void VSTPluginImplementation::PrepareUnitInfo() {
		if (!m_unitHandler) {
			PC_ERROR("VST: Tried to prepare unit info without unit handler!");
			return;
		}

		int num = m_unitHandler->getUnitCount();
		if (num == 0) {
			PC_WARN("VST: Unit handler doesn't contain any unit");
		}

		for (int i = 0; i < num; i++) {
			Steinberg::Vst::UnitInfo info;
			m_unitHandler->getUnitInfo(i, info);
			VSTPlugin::UnitInfo myInfo;
			myInfo.ID = info.id;
			myInfo.name = S128to_string(info.name);
			myInfo.parentID = info.parentUnitId;
			if (info.programListId != Steinberg::Vst::kNoProgramListId) {
				myInfo.programList = CreateProgramList(m_unitHandler, info.programListId);
				myInfo.programChangeParam = FindProgramChangeParam(m_parameterInfoList, info.id);
			}

			for (int j = 0; j < m_unitInfoList.size(); j++) {
				if (myInfo.ID == m_unitInfoList[j].ID) {
					PC_ERROR("VST: Found units with identical IDs!");
					return;
				}
			}
			m_unitInfoList.push_back(myInfo);
		}
	}

	bool VSTPluginImplementation::HasEditor() {
		if (m_component) {
			return m_hasEditor;
		}
		else {
			PC_ERROR("VST: Component doesn't exist!");
			return false;
		}
	}

	bool VSTPluginImplementation::OpenEditor(WindowHandle parent, Steinberg::IPlugFrame* plugFrame) {
		if (!HasEditor()) {
			PC_ERROR("VST: Cannot open editor (component doesn't have editor) !");
			return false;
		}

		Steinberg::tresult res;

		if (!plugFrame) {
			PC_ERROR("VST: Cannot open editor (plug frame doesn't exist) !");
			return false;
		}

		m_plugView->setFrame(plugFrame);

#if defined(_MSC_VER)
		res = m_plugView->attached((void*)parent, Steinberg::kPlatformTypeHWND);
#else
		if (m_plugView->isPlatformTypeSupported(kPlatformTypeNSView) == Steinberg::kResultOk) {
			res = m_plugView->attached((void*)parent, Steinberg::kPlatformTypeNSView);
		}
		else if (m_plugView->isPlatformTypeSupported(kPlatformTypeHIView) == Steinberg::kResultOk) {
			res = m_plugView->attached(GetWindowRef(parent), Steinberg::kPlatformTypeHIView);
		}
		else {
			PC_ERROR("VST: Cannot open editor (not supported) !");
			return false;
		}
#endif

		m_isEditorOpened = (res == Steinberg::kResultOk);
		return m_isEditorOpened;
	}

	void VSTPluginImplementation::CloseEditor() {
		if (m_isEditorOpened) {
			m_plugView->removed();
			m_isEditorOpened = false;
		}
	}

	bool VSTPluginImplementation::IsEditorOpened() {
		return m_isEditorOpened;
	}

	Steinberg::ViewRect VSTPluginImplementation::GetPreferredRect() {
		Steinberg::ViewRect rect;
		m_plugView->getSize(&rect);
		return rect;
	}

	bool operator==(Steinberg::Vst::ProcessSetup& x, Steinberg::Vst::ProcessSetup& y)
	{
		auto to_tuple = [](auto& s) {
			return std::tie(s.maxSamplesPerBlock,
				s.sampleRate,
				s.symbolicSampleSize,
				s.processMode);
		};

		return to_tuple(x) == to_tuple(y);
	}

	bool operator!=(Steinberg::Vst::ProcessSetup& x, Steinberg::Vst::ProcessSetup& y)
	{
		return !(x == y);
	}

	void VSTPluginImplementation::Resume() {
		if (!(m_status == Status::Initialized || m_status == Status::SetupDone)) {
			PC_ERROR("VST: VST is already resumed!");
			return;
		}

		Steinberg::tresult res;

		Steinberg::Vst::ProcessSetup setup = {};
		setup.processMode = Steinberg::Vst::ProcessModes::kRealtime;
		setup.sampleRate = m_settings->sampleRate;
		setup.symbolicSampleSize = Steinberg::Vst::SymbolicSampleSizes::kSample32;
		setup.maxSamplesPerBlock = m_settings->bufferSize;

		if (setup != m_currentProcessSetup) {
			res = m_audioProcessor->setupProcessing(setup);
			if (res != Steinberg::kResultOk && res != Steinberg::kNotImplemented) {
				PC_ERROR("VST: Couldn't set processing setup!");
				return;
			}
			else {
				m_currentProcessSetup = setup;
			}
		}

		m_status = Status::SetupDone;

		auto PrepareBusBuffers = [&](AudioBusesInfo& buses, uint32_t block_size, float* buffer) {
			delete buffer;
			buffer = new float[buses.GetNumChannels(), block_size];

			float* data = buffer;
			Steinberg::Vst::AudioBusBuffers* busBuffers = buses.GetBusBuffers();
			for (int i = 0; i < buses.GetNumBuses(); ++i) {
				auto& buffer = busBuffers[i];
				buffer.channelBuffers32 = &data;
				buffer.silenceFlags = (buses.IsActive(i) ? 0 : -1);
				data += buffer.numChannels;
			}
		};

		PrepareBusBuffers(m_inputBusesInfo, m_settings->bufferSize, m_inputBuffer);
		PrepareBusBuffers(m_outputBusesInfo, m_settings->bufferSize, m_outputBuffer);

		res = GetComponent()->setActive(true);
		if (res != Steinberg::kResultOk && res != Steinberg::kNotImplemented) {
			PC_ERROR("VST: Couldn't set active!");
			return;
		}

		m_status = Status::Activated;

		PC_INFO("VST: Latency samples : {0}", GetAudioProcessor()->getLatencySamples());
		res = GetAudioProcessor()->setProcessing(true);
		if (res != Steinberg::kResultOk && res != Steinberg::kNotImplemented) {
			PC_ERROR("VST: Couldn't enable processing!");
			return;
		}

		m_status = Status::Processing;
		m_isProcessingStarted = true;
	}

	void VSTPluginImplementation::Suspend() {
		if (m_status == Status::Processing) {
			GetAudioProcessor()->setProcessing(false);
			m_status = Status::Activated;
		}

		GetComponent()->setActive(false);
		m_status = Status::SetupDone;
	}

	bool VSTPluginImplementation::IsResumed() {
		return m_status > Status::SetupDone;
	}

	void VSTPluginImplementation::RestartComponent(Steinberg::int32 flags) {
		if (flags & Steinberg::Vst::RestartFlags::kParamValuesChanged) {

		}
		else if (flags & Steinberg::Vst::RestartFlags::kReloadComponent) {
			Steinberg::tresult res;
			Steinberg::MemoryStream stream;
			res = m_component->getState(&stream);

			bool isResumed = IsResumed();
			if (isResumed) {
				Suspend();
				Resume();
			}

			res = m_component->setState(&stream);
		}
	}

	void VSTPluginImplementation::Process(ProcessInfo processInfo) {
		if (!processInfo.timeInfo) {
			PC_ERROR("VST: Process Information missing time info!");
		}
		ProcessInfo::TransportInfo& timeInfo = *processInfo.timeInfo;
		Steinberg::Vst::ProcessContext processContext = {};
		processContext.sampleRate = m_settings->sampleRate;
		processContext.projectTimeSamples = timeInfo.smpBeginPos;
		processContext.projectTimeMusic = timeInfo.ppqBeginPos;
		processContext.tempo = timeInfo.tempo;
		processContext.timeSigDenominator = timeInfo.timeSigDenom;
		processContext.timeSigNumerator = timeInfo.timeSigNumer;

		using Flags = Steinberg::Vst::ProcessContext::StatesAndFlags;

		processContext.state
			= (timeInfo.playing ? Steinberg::Vst::ProcessContext::StatesAndFlags::kPlaying : 0)
			| Steinberg::Vst::ProcessContext::StatesAndFlags::kProjectTimeMusicValid
			| Steinberg::Vst::ProcessContext::StatesAndFlags::kTempoValid
			| Steinberg::Vst::ProcessContext::StatesAndFlags::kTimeSigValid
			;

		m_inputEvents.clear();
		m_outputEvents.clear();
		m_inParameterChanges.clearQueue();
		m_outParameterChanges.clearQueue();

		for (int i = 0; i < m_settings->outputChannels * m_settings->bufferSize; i++) {
			m_inputBuffer[i] = 0.0f;
			m_outputBuffer[i] = 0.0f;
		}

		// MIDI Stuff - Won't care for now
		/*
		for (auto& m : processInfo.input_midi_buffer_.buffer_) {
			Vst::Event e;
			e.busIndex = 0;
			e.sampleOffset = m.offset_;
			e.ppqPosition = process_context.projectTimeMusic;
			e.flags = Vst::Event::kIsLive;

			using namespace MidiDataType;

			auto midi_map = [this](int channel, int offset, int cc, Vst::ParamValue value) {
				if (!midi_mapping_) { return; }
				Vst::ParamID param_id = 0;
				auto result = midi_mapping_->getMidiControllerAssignment(0, channel, cc, param_id);
				if (result == kResultOk) {
					PushBackParameterChange(param_id, value, offset);
					edit_controller_->setParamNormalized(param_id, value);
				}
			};

			if (auto note_on = m.As<NoteOn>()) {
				e.type = Vst::Event::kNoteOnEvent;
				e.noteOn.channel = m.channel_;
				e.noteOn.pitch = note_on->pitch_;
				e.noteOn.velocity = note_on->velocity_ / 127.0;
				e.noteOn.length = 0;
				e.noteOn.tuning = 0;
				e.noteOn.noteId = -1;
				input_events_.addEvent(e);
			}
			else if (auto note_off = m.As<NoteOff>()) {
				e.type = Vst::Event::kNoteOffEvent;
				e.noteOff.channel = m.channel_;
				e.noteOff.pitch = note_off->pitch_;
				e.noteOff.velocity = note_off->off_velocity_ / 127.0;
				e.noteOff.tuning = 0;
				e.noteOff.noteId = -1;
				input_events_.addEvent(e);
			}
			else if (auto poly_press = m.As<PolyphonicKeyPressure>()) {
				e.type = Vst::Event::kPolyPressureEvent;
				e.polyPressure.channel = m.channel_;
				e.polyPressure.pitch = poly_press->pitch_;
				e.polyPressure.pressure = poly_press->value_ / 127.0;
				e.polyPressure.noteId = -1;
				input_events_.addEvent(e);
			}
			else if (auto cc = m.As<ControlChange>()) {
				midi_map(m.channel_, m.offset_, cc->control_number_, cc->data_ / 128.0);
			}
			else if (auto cp = m.As<ChannelPressure>()) {
				midi_map(m.channel_, m.offset_, Vst::ControllerNumbers::kAfterTouch, cp->value_ / 128.0);
			}
			else if (auto pb = m.As<PitchBendChange>()) {
				midi_map(m.channel_, m.offset_, Vst::ControllerNumbers::kPitchBend,
					((pb->value_msb_ << 7) | pb->value_lsb_) / 16384.0);
			}
		} */

		auto copy_buffer = [&](float* src, float* dest,
			uint32_t lengthToCopy)
		{
			int channels = std::min(m_inputBusesInfo.GetNumChannels(), m_outputBusesInfo.GetNumChannels());
			if (channels == 0) { return; }

			for (int i = 0; i < lengthToCopy * channels; i++) {
				dest[i] = src[i];
			}
		};

		copy_buffer(processInfo.inputBuffer, m_inputBuffer, processInfo.timeInfo->GetSmpDuration());

		PopFrontParameterChanges(m_inParameterChanges);

		Steinberg::Vst::ProcessData processData;
		processData.processContext = &processContext;
		processData.processMode = Steinberg::Vst::ProcessModes::kRealtime;
		processData.symbolicSampleSize = Steinberg::Vst::SymbolicSampleSizes::kSample32;
		processData.numSamples = timeInfo.GetSmpDuration();
		processData.numInputs = m_inputBusesInfo.GetNumBuses();
		processData.numOutputs = m_outputBusesInfo.GetNumBuses();
		processData.inputs = m_inputBusesInfo.GetBusBuffers();
		processData.outputs = m_outputBusesInfo.GetBusBuffers();
		processData.inputEvents = &m_inputEvents;
		processData.outputEvents = &m_outputEvents;
		processData.inputParameterChanges = &m_inParameterChanges;
		processData.outputParameterChanges = &m_outParameterChanges;

		auto const res = GetAudioProcessor()->process(processData);

		static bool kOutputParameter = false;

		if (res != Steinberg::kResultOk) {
			PC_ERROR("VST: Couldn't process!");
		}

		copy_buffer(m_outputBuffer, processInfo.outputBuffer,
			processInfo.timeInfo->GetSmpDuration());
	}

	void VSTPluginImplementation::PushBackParameterChange(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value, int64_t offset) {
		std::unique_lock lock = std::unique_lock(m_ParameterChangesMutex);
		Steinberg::int32 parameter_index = 0;
		auto* queue = m_ParameterChangesQueue.addParameterData(id, parameter_index);
		Steinberg::int32 ref_point_index = 0;
		queue->addPoint(offset, value, ref_point_index);
	}

	// Getters / Setters

	void VSTPluginImplementation::SetBlockSize(int blockSize) {
		// useless because i use AudioSettings
	}

	void VSTPluginImplementation::SetSamplingRate(int samplingRate) {
		// useless because i use AudioSettings
	}

	VSTPlugin::FactoryInfo& VSTPluginImplementation::GetFactoryInfo() {
		return m_factoryInfo;
	}
	VSTPlugin::ClassInfo& VSTPluginImplementation::GetComponentInfo() {
		return m_classInfo;
	}

	std::string VSTPluginImplementation::GetEffectName() {
		return m_classInfo.Name();
	}

	std::vector<VSTPlugin::ParameterInfo>& VSTPluginImplementation::GetParameterInfoList() {
		return m_parameterInfoList;
	}

	std::vector<VSTPlugin::UnitInfo>& VSTPluginImplementation::GetUnitInfoList() {
		return m_unitInfoList;
	}

	AudioBusesInfo& VSTPluginImplementation::GetBusesInfo(Steinberg::Vst::BusDirection dir) {
		if (dir == Steinberg::Vst::BusDirections::kInput) {
			return m_inputBusesInfo;
		}
		else {
			return m_outputBusesInfo;
		}
	}

	uint32_t VSTPluginImplementation::GetNumParameters() {
		return m_editController->getParameterCount();
	}
	Steinberg::Vst::ParamValue VSTPluginImplementation::GetParameterValueByIndex(uint32_t index) {
		auto id = GetParameterInfoList()[index].ID;
		return GetParameterValueByID(id);
	}
	Steinberg::Vst::ParamValue VSTPluginImplementation::GetParameterValueByID(Steinberg::Vst::ParamID id) {
		return m_editController->getParamNormalized(id);
	}

	std::string VSTPluginImplementation::ValueToStringByIndex(uint32_t index, Steinberg::Vst::ParamValue value) {
		return ValueToStringByID(GetParameterInfoList()[index].ID, value);
	}
	Steinberg::Vst::ParamValue VSTPluginImplementation::StringToValueTByIndex(uint32_t index, std::string string) {
		return StringToValueByID(GetParameterInfoList()[index].ID, string);
	}

	std::string VSTPluginImplementation::ValueToStringByID(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value) {
		Steinberg::Vst::String128 str = {};
		auto result = m_editController->getParamStringByValue(id, value, str);
		if (result != Steinberg::kResultOk) {
			return "";
		}

		return S128to_string(str);
	}
	Steinberg::Vst::ParamValue VSTPluginImplementation::StringToValueByID(Steinberg::Vst::ParamID id, std::string string) {
		Steinberg::Vst::ParamValue value = 0;
		auto result = m_editController->getParamValueByString(id, to_u16string(string).data(), value);
		if (result != Steinberg::kResultOk) {
			return -1;
		}

		return value;
	}

	uint32_t VSTPluginImplementation::GetProgramIndex(Steinberg::Vst::UnitID unitID) {
		VSTPlugin::UnitInfo unitInfo;
		auto& unitInfoList = GetUnitInfoList();
		for(int i = 0; i < unitInfoList.size(); i++) {
			if (unitInfoList[i].ID == unitID) {
				unitInfo = unitInfoList[i];
			}
		}

		uint32_t size = unitInfo.programList.programs.size();
		Steinberg::Vst::ParamID paramID = unitInfo.programChangeParam;

		if (paramID == Steinberg::Vst::kNoParamId || size == 0) {
			return -1;
		}

		Steinberg::Vst::ParamValue normalizedValue = GetEditController()->getParamNormalized(paramID);
		uint32_t plain = (uint32_t)std::round(normalizedValue * (size - 1));
		if (plain >= size) {
			PC_ERROR("VST: program index is bigger than list size!");
		}

		return plain;
	}
	void VSTPluginImplementation::SetProgramIndex(uint32_t index, Steinberg::Vst::UnitID unitID) {
		VSTPlugin::UnitInfo unitInfo;
		auto& unitInfoList = GetUnitInfoList();
		for (int i = 0; i < unitInfoList.size(); i++) {
			if (unitInfoList[i].ID == unitID) {
				unitInfo = unitInfoList[i];
			}
		}

		uint32_t size = unitInfo.programList.programs.size();
		Steinberg::Vst::ParamID paramID = unitInfo.programChangeParam;

		if (index >= size) {
			PC_ERROR("VST: program index is bigger than list size!");
		}

		if (paramID == Steinberg::Vst::kNoParamId || size == 0) {
			return;
		}
		double normalizedValue = index / (double)size;

		GetEditController()->setParamNormalized(unitInfo.programChangeParam, normalizedValue);
		PushBackParameterChange(unitInfo.programChangeParam, normalizedValue);
	}

	bool VSTPluginImplementation::HasEditController() { return m_editController != nullptr; }
	bool VSTPluginImplementation::HasEditController2() { return m_editController2 != nullptr; }

	Steinberg::Vst::IComponent* VSTPluginImplementation::GetComponent() { return m_component; }
	Steinberg::Vst::IAudioProcessor* VSTPluginImplementation::GetAudioProcessor() { return m_audioProcessor; }
	Steinberg::Vst::IEditController* VSTPluginImplementation::GetEditController() { return m_editController; }
	Steinberg::Vst::IEditController2* VSTPluginImplementation::GetEditController2() { return m_editController2; }

}
}