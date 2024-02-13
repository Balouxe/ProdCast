#include "VST/VSTPlugin.h"
#include "Engine.h"

#include <codecvt>
#include "public.sdk/source/common/memorystream.cpp"

#include <string>
#include <numeric>
#include <memory>

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
		m_plugFrameListener->OnResizePlugView(*newSize);
		view->onSize(newSize);
		return Steinberg::kResultOk;
	}
	
	// VSTProcessor

	VSTProcessor::VSTProcessor(VSTPlugin* plugin) : m_plugin(plugin) {};

	std::string VSTProcessor::getName() {
		return m_plugin->GetEffectName();
	}

	void VSTProcessor::OnStartProcessing(AudioSettings* settings) {
		m_plugin->SetSettings(settings);
	}

	void VSTProcessor::Process(ProcessInfo& processInfo) {
		m_plugin->Process(processInfo);
	}

	void VSTProcessor::OnStopProcessing() {

	}

	VSTPlugin* VSTProcessor::getPlugin() {
		return m_plugin;
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

	uint32_t AudioBusesInfo::GetNumChannels()
	{
		return std::accumulate(m_busInfos.begin(),
			m_busInfos.end(),
			0,
			[](int sum, auto const& info) {
				return sum + info.channelCount;
			});
	}

	uint32_t AudioBusesInfo::GetNumActiveChannels()
	{
		return std::accumulate(m_busInfos.begin(),
			m_busInfos.end(),
			0,
			[](int sum, auto const& info) {
				return sum + (info.isActive ? info.channelCount : 0);
			});
	}

	bool AudioBusesInfo::IsActive(uint32_t busIndex)
	{
		return GetBusInfo(busIndex)->isActive;
	}

	void AudioBusesInfo::SetActive(uint32_t busIndex, bool state)
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

		m_cid = std::array<Steinberg::int8, 16>();
		std::copy(info.cid, info.cid + 16, m_cid.begin());
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

		m_cid = std::array<Steinberg::int8, 16>();
		std::copy(info.cid, info.cid + 16, m_cid.begin());
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

		m_cid = std::array<Steinberg::int8, 16>();
		std::copy(info.cid, info.cid + 16, m_cid.begin());
	}

	VSTPlugin::ClassInfo::~ClassInfo() {
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

	VSTPlugin::VSTPlugin(VSTPluginImplementation* impl,
		VSTHostContext* hostContext,
		std::function<void(VSTPlugin* p)> onDestruction) : m_implementation(impl), m_hostContext(hostContext) {
		hostContext->SetVST3Plugin(this);
		m_onDestruction = onDestruction;
	}

	VSTPlugin::~VSTPlugin() {
		if (m_implementation->IsEditorOpened()) {
			m_implementation->CloseEditor();
		}
		delete m_implementation; // ?
		delete m_hostContext; // ?
		// might be the VSTPluginImplementation responsability to delete these
		m_onDestruction(this);
	}

	VSTPlugin::FactoryInfo& VSTPlugin::GetFactoryInfo() {
		return m_implementation->GetFactoryInfo();
	}
	VSTPlugin::ClassInfo& VSTPlugin::GetComponentInfo() {
		return m_implementation->GetComponentInfo();
	}
	std::string VSTPlugin::GetEffectName() {
		return m_implementation->GetEffectName();
	}
	uint32_t VSTPlugin::GetNumInputs() {
		return m_implementation->GetBusesInfo(Steinberg::Vst::BusDirections::kInput).GetNumActiveChannels();
	}
	uint32_t VSTPlugin::GetNumOutputs() {
		return m_implementation->GetBusesInfo(Steinberg::Vst::BusDirections::kOutput).GetNumActiveChannels();
	}

	size_t VSTPlugin::GetNumParams() {
		return m_implementation->GetParameterInfoList().size();
	}
	VSTPlugin::ParameterInfo& VSTPlugin::GetParameterInfoByIndex(uint32_t index) {
		return m_implementation->GetParameterInfoList()[index];
	}
	VSTPlugin::ParameterInfo& VSTPlugin::GetParameterInfoByID(Steinberg::Vst::ParamID id) {
		std::vector<VSTPlugin::ParameterInfo>& infoList = m_implementation->GetParameterInfoList();

		for (int i = 0; i < infoList.size(); i++) {
			if (id == infoList[i].ID) {
				return infoList[i];
			}
		}
		PC_ERROR("VST: Didn't find parameter info!");
		VSTPlugin::ParameterInfo empty = {};
		return empty;
	}

	uint32_t VSTPlugin::GetNumUnitInfo() {
		return m_implementation->GetUnitInfoList().size();
	}
	VSTPlugin::UnitInfo& VSTPlugin::GetUnitInfoByIndex(uint32_t index) {
		return m_implementation->GetUnitInfoList()[index];
	}
	VSTPlugin::UnitInfo& VSTPlugin::GetUnitInfoByID(Steinberg::Vst::UnitID id) {
		std::vector<VSTPlugin::UnitInfo>& infoList = m_implementation->GetUnitInfoList();

		for (int i = 0; i < infoList.size(); i++) {
			if (id == infoList[i].ID) {
				return infoList[i];
			}
		}
		PC_ERROR("VST: Didn't find unit info!");
		VSTPlugin::UnitInfo empty = {};
		return empty;
	}

	uint32_t VSTPlugin::GetNumBuses(Steinberg::Vst::BusDirection dir) {
		return m_implementation->GetBusesInfo(dir).GetNumBuses();
	}
	BusInfo& VSTPlugin::GetBusInfoByIndex(Steinberg::Vst::BusDirection dir, uint32_t index) {
		return *m_implementation->GetBusesInfo(dir).GetBusInfo(index);
	}

	Steinberg::Vst::ParamValue VSTPlugin::GetParameterValueByIndex(uint32_t index) {
		return m_implementation->GetParameterValueByIndex(index);
	}
	Steinberg::Vst::ParamValue VSTPlugin::GetParameterValueByID(Steinberg::Vst::ParamID id) {
		return m_implementation->GetParameterValueByID(id);
	}

	std::string VSTPlugin::ValueToStringByIndex(uint32_t index, Steinberg::Vst::ParamValue value) {
		return m_implementation->ValueToStringByIndex(index, value);
	}
	Steinberg::Vst::ParamValue VSTPlugin::StringToValueTByIndex(uint32_t index, std::string string) {
		return m_implementation->StringToValueTByIndex(index, string);
	}

	std::string VSTPlugin::ValueToStringByID(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value) {
		return m_implementation->ValueToStringByID(id, value);
	}
	Steinberg::Vst::ParamValue VSTPlugin::StringToValueByID(Steinberg::Vst::ParamID id, std::string string) {
		return m_implementation->StringToValueByID(id, string);
	}

	bool VSTPlugin::IsBusActive(Steinberg::Vst::BusDirection dir, uint32_t index) {
		return m_implementation->GetBusesInfo(dir).GetBusInfo(index)->isActive;
	}
	void VSTPlugin::SetBusActive(Steinberg::Vst::BusDirection dir, uint32_t index, bool state) {
		m_implementation->GetBusesInfo(dir).SetActive(index, state);
	}
	Steinberg::Vst::SpeakerArrangement VSTPlugin::GetSpeakerArrangementForBus(Steinberg::Vst::BusDirection dir, uint32_t index) {
		return m_implementation->GetBusesInfo(dir).GetBusInfo(index)->speaker;
	}
	bool VSTPlugin::SetSpeakerArrangement(Steinberg::Vst::BusDirection dir, uint32_t index, Steinberg::Vst::SpeakerArrangement arr) {
		return m_implementation->GetBusesInfo(dir).SetSpeakerArrangement(index, arr);
	}

	void VSTPlugin::Resume() {
		m_implementation->Resume();
	}
	void VSTPlugin::Suspend() {
		m_implementation->Suspend();
	}
	bool VSTPlugin::IsResumed() {
		return m_implementation->IsResumed();
	}

	void VSTPlugin::SetSettings(AudioSettings* settings) {
		m_implementation->SetAudioSettings(settings);
	}

	bool VSTPlugin::HasEditor() {
		return m_implementation->HasEditor();
	}
	bool VSTPlugin::OpenEditor(WindowHandle wnd, PlugFrameListener* listener) {
		CloseEditor();

		if (!listener) {
			PC_ERROR("VST: Tried to open editor without listener!");
			return false;
		}
		if (m_hostContext->m_plugFrameListener != nullptr) {
			PC_ERROR("VST: Plug Frame listener not nullptr!");
			return false;
		}

		m_hostContext->m_plugFrameListener = listener;
		return m_implementation->OpenEditor(wnd, m_hostContext);
	}
	void VSTPlugin::CloseEditor() {
		if (m_implementation) {
			m_implementation->CloseEditor();
			m_hostContext->m_plugFrameListener = nullptr;
		}
	}
	bool VSTPlugin::IsEditorOpened() {
		return m_implementation->IsEditorOpened();
	}
	Steinberg::ViewRect VSTPlugin::GetPreferredRect() {
		return m_implementation->GetPreferredRect();
	}

	uint32_t VSTPlugin::GetProgramIndex(Steinberg::Vst::UnitID unitID) {
		return m_implementation->GetProgramIndex(unitID);
	}
	void VSTPlugin::SetProgramIndex(uint32_t index, Steinberg::Vst::UnitID unitID) {
		return m_implementation->SetProgramIndex(index, unitID);
	}

	void VSTPlugin::EnqueueParameterChange(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value) {
		m_implementation->PushBackParameterChange(id, value);
	}

	void VSTPlugin::RestartComponent(Steinberg::int32 flags) {
		m_implementation->RestartComponent(flags);
	}

	void VSTPlugin::Process(ProcessInfo& pi) {
		m_implementation->Process(pi);
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

	VSTPluginImplementation::VSTPluginImplementation(Steinberg::IPluginFactory* factory,
		VSTPlugin::FactoryInfo& factoryInfo,
		VSTPlugin::ClassInfo& classInfo,
		Steinberg::FUnknown* hostContext)
	{
		m_factoryInfo = factoryInfo;
		m_editControllerIsCreatedNew = false;
		m_isEditorOpened = false;
		m_isProcessingStarted = false;
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
		Steinberg::FUID fuid = Steinberg::FUID::fromTUID(classInfo.CID().data());
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

		// Only used for instruments
	
		res = component->setIoMode(Steinberg::Vst::IoModes::kAdvanced);
		if (!(res == Steinberg::kResultOk || res == Steinberg::kNotImplemented)) {
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

		ResizeBuffers(m_inputBusesInfo.GetNumChannels(), m_outputBusesInfo.GetNumChannels(), m_settings->bufferSize);
		// prepare input buffer
		{
			auto data = m_inputBufferHeads.data();
			auto* busBuffers = m_inputBusesInfo.GetBusBuffers();
			for (int i = 0; i < m_inputBusesInfo.GetNumBuses(); i++) {
				auto& buffer = busBuffers[i];
				buffer.channelBuffers32 = data;
				buffer.silenceFlags = (m_inputBusesInfo.IsActive(i) ? 0 : -1);
				data += buffer.numChannels;
			}
		} 
		// prepare output buffer
		{
			auto data = m_outputBufferHeads.data();
			auto* busBuffers = m_outputBusesInfo.GetBusBuffers();
			for (int i = 0; i < m_outputBusesInfo.GetNumBuses(); i++) {
				auto& buffer = busBuffers[i];
				buffer.channelBuffers32 = data;
				buffer.silenceFlags = (m_outputBusesInfo.IsActive(i) ? 0 : -1);
				data += buffer.numChannels;
			}
		}

		/*
		delete m_inputBuffer;
		m_inputBuffer = new float[m_settings->bufferSize* m_inputBusesInfo.GetNumChannels()];
		float** data = &m_inputBuffer;
		Steinberg::Vst::AudioBusBuffers* busBuffers = m_inputBusesInfo.GetBusBuffers();
		for (int i = 0; i < m_inputBusesInfo.GetNumBuses(); ++i) {
			auto& busBuffer = busBuffers[i];
			busBuffer.channelBuffers32 = data;
			busBuffer.silenceFlags = (m_inputBusesInfo.IsActive(i) ? 0 : -1);
			*data += busBuffer.numChannels * m_settings->bufferSize;
		}

		delete m_outputBuffer;
		m_outputBuffer = new float[m_settings->bufferSize * m_outputBusesInfo.GetNumChannels()];
		data = &m_outputBuffer;
		busBuffers = m_outputBusesInfo.GetBusBuffers();
		for (int i = 0; i < m_outputBusesInfo.GetNumBuses(); ++i) {
			auto& busBuffer = busBuffers[i];
			busBuffer.channelBuffers32 = data;
			busBuffer.silenceFlags = (m_outputBusesInfo.IsActive(i) ? 0 : -1);
			*data += busBuffer.numChannels;
		}
		*/

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
			return;
		}
		if (!m_inputBuffer.data() || !m_outputBuffer.data()) {
			PC_WARN("VST: buffers uninitialized, cannot process!");
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
			Steinberg::Vst::Event e;
			e.busIndex = 0;
			e.sampleOffset = m.offset_;
			e.ppqPosition = process_context.projectTimeMusic;
			e.flags = Steinberg::Vst::Event::kIsLive;

			using namespace MidiDataType;

			auto midi_map = [this](int channel, int offset, int cc, Steinberg::Vst::ParamValue value) {
				if (!midi_mapping_) { return; }
				Steinberg::Vst::ParamID param_id = 0;
				auto result = midi_mapping_->getMidiControllerAssignment(0, channel, cc, param_id);
				if (result == kResultOk) {
					PushBackParameterChange(param_id, value, offset);
					edit_controller_->setParamNormalized(param_id, value);
				}
			};

			if (auto note_on = m.As<NoteOn>()) {
				e.type = Steinberg::Vst::Event::kNoteOnEvent;
				e.noteOn.channel = m.channel_;
				e.noteOn.pitch = note_on->pitch_;
				e.noteOn.velocity = note_on->velocity_ / 127.0;
				e.noteOn.length = 0;
				e.noteOn.tuning = 0;
				e.noteOn.noteId = -1;
				input_events_.addEvent(e);
			}
			else if (auto note_off = m.As<NoteOff>()) {
				e.type = Steinberg::Vst::Event::kNoteOffEvent;
				e.noteOff.channel = m.channel_;
				e.noteOff.pitch = note_off->pitch_;
				e.noteOff.velocity = note_off->off_velocity_ / 127.0;
				e.noteOff.tuning = 0;
				e.noteOff.noteId = -1;
				input_events_.addEvent(e);
			}
			else if (auto poly_press = m.As<PolyphonicKeyPressure>()) {
				e.type = Steinberg::Vst::Event::kPolyPressureEvent;
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
				midi_map(m.channel_, m.offset_, Steinberg::Vst::ControllerNumbers::kAfterTouch, cp->value_ / 128.0);
			}
			else if (auto pb = m.As<PitchBendChange>()) {
				midi_map(m.channel_, m.offset_, Steinberg::Vst::ControllerNumbers::kPitchBend,
					((pb->value_msb_ << 7) | pb->value_lsb_) / 16384.0);
			}
		}*/

		{
			size_t const min_ch = std::min(processInfo.numChannels, m_inputBusesInfo.GetNumChannels());
			if (min_ch == 0) { return; }
			for (int i = 0; i < processInfo.timeInfo->GetSmpDuration() * min_ch; i++) {
				m_inputBuffer[i] = processInfo.inputBuffer[i];
			}
		};

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

		Steinberg::tresult res = GetAudioProcessor()->process(processData);

		static bool kOutputParameter = false;

		if (res != Steinberg::kResultOk) {
			PC_ERROR("VST: Couldn't process!");
		}

		{
			size_t const min_ch = std::min(processInfo.numChannels, m_outputBusesInfo.GetNumChannels());
			if (min_ch == 0) { return; }
			for (int i = 0; i < min_ch * processInfo.timeInfo->GetSmpDuration(); i++) {
				processInfo.outputBuffer[i] = m_outputBuffer[i];
			}
		};
	}

	void VSTPluginImplementation::PushBackParameterChange(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value, int64_t offset) {
		std::unique_lock lock = std::unique_lock(m_ParameterChangesMutex);
		Steinberg::int32 parameter_index = 0;
		auto* queue = m_ParameterChangesQueue.addParameterData(id, parameter_index);
		Steinberg::int32 ref_point_index = 0;
		queue->addPoint(offset, value, ref_point_index);
	}

	void VSTPluginImplementation::PopFrontParameterChanges(Steinberg::Vst::ParameterChanges& dest) {
		std::unique_lock lock = std::unique_lock(m_ParameterChangesMutex);

		for (uint32_t i = 0; i < m_ParameterChangesQueue.getParameterCount(); i++) {
			Steinberg::Vst::IParamValueQueue *srcQueue = m_ParameterChangesQueue.getParameterData(i);
			Steinberg::Vst::ParamID const srcID = srcQueue->getParameterId();

			if (srcID == Steinberg::Vst::kNoParamId || srcQueue->getPointCount() == 0) {
				continue;
			}

			Steinberg::int32 ref_queue_index;
			auto dest_queue = dest.addParameterData(srcQueue->getParameterId(), ref_queue_index);

			for (uint32_t v = 0; v < srcQueue->getPointCount(); v++) {
				Steinberg::int32 ref_offset;
				Steinberg::Vst::ParamValue ref_value;
				Steinberg::tresult result = srcQueue->getPoint(v, ref_offset, ref_value);
				if (result != Steinberg::kResultTrue) { continue; }

				Steinberg::int32 ref_point_index;
				dest_queue->addPoint(ref_offset, ref_value, ref_point_index);
			}
		}

		m_ParameterChangesQueue.clearQueue();
	}

	void VSTPluginImplementation::SetAudioSettings(AudioSettings* settings) {
		Suspend();
		m_settings = settings;
		Resume();
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

	void VSTPluginImplementation::ResizeBuffers(uint32_t inputNumChannels, uint32_t outputNumChannels, uint32_t numSamples) {
		{
			std::vector<float> tmp(inputNumChannels * numSamples);
			std::vector<float*> tmp_heads(inputNumChannels);

			m_inputBuffer.swap(tmp);
			m_inputBufferHeads.swap(tmp_heads);
			for (size_t i = 0; i < inputNumChannels; i++) {
				m_inputBufferHeads[i] = m_inputBuffer.data() + (i * numSamples);
			}
		}
		{
			std::vector<float> tmp(outputNumChannels * numSamples);
			std::vector<float*> tmp_heads(outputNumChannels);

			m_outputBuffer.swap(tmp);
			m_outputBufferHeads.swap(tmp_heads);
			for (size_t i = 0; i < outputNumChannels; i++) {
				m_outputBufferHeads[i] = m_outputBuffer.data() + (i * numSamples);
			}
		}
	}

	// Useful for VSTPluginFactory

	VSTPlugin* CreatePlugin(Steinberg::IPluginFactory* factory,
		VSTPlugin::FactoryInfo& factoryInfo,
		VSTPlugin::ClassInfo& classInfo,
		std::function<void(VSTPlugin* p)> onDestruction) {

		VSTHostContext* hostContext = new VSTHostContext("Prodcast VST Handler");
		VSTPluginImplementation* impl = new VSTPluginImplementation(factory, factoryInfo, classInfo, hostContext->unknownCast());
		VSTPlugin* plugin = new VSTPlugin(impl, hostContext, onDestruction);

		return plugin;
	}

	// VSTPluginFactory
	
	VSTPluginFactory::VSTPluginFactory(std::string modulePath) {
		Module mod(modulePath.c_str());
		if (!mod.GetModule()) {
			PC_ERROR("VST: Error loading library!");
			return;
		}

		auto initDLL = reinterpret_cast<SetupProc>(mod.getProcAddress("InitDll"));
		if (initDLL) {
			initDLL();
		}

		GetFactoryProc getFactory = (GetFactoryProc)mod.getProcAddress("GetPluginFactory");
		if (!getFactory) {
			PC_ERROR("VST: Not a VST3 library!");
			return;
		}

		Steinberg::IPluginFactory* factory = getFactory();
		if (!factory) {
			PC_ERROR("VST: Failed to get VST factory!");
			return;
		}

		Steinberg::PFactoryInfo factoryInfo;
		factory->getFactoryInfo(&factoryInfo);

		std::vector<VSTPlugin::ClassInfo> classInfoList;

		for (int i = 0; i < factory->countClasses(); i++) {
			Steinberg::IPluginFactory3* factory3;
			Steinberg::tresult res = factory->queryInterface(Steinberg::IPluginFactory3::iid, (void**)&factory3);
			if (factory3) {
				Steinberg::PClassInfoW info;
				factory3->getClassInfoUnicode(i, &info);
				classInfoList.emplace_back(info);
			}
			else {
				Steinberg::IPluginFactory2* factory2;
				res = factory->queryInterface(Steinberg::IPluginFactory2::iid, (void**)&factory2);
				if (factory2) {
					Steinberg::PClassInfo2 info;
					factory2->getClassInfo2(i, &info);
					classInfoList.emplace_back(info);
				}
				else {
					Steinberg::PClassInfo info;
					factory->getClassInfo(i, &info);
					classInfoList.emplace_back(info);
				}
			}
		}

		m_module = std::move(mod);
		m_factory = std::move(factory);
		m_factoryInfo = VSTPlugin::FactoryInfo(factoryInfo);
		m_classInfoList = std::move(classInfoList);
	}

	VSTPluginFactory::~VSTPluginFactory() {
		if (!m_loadedPlugins.empty()) {
			PC_WARN("VST: loaded plugins list is not empty!");
			if (m_module.GetModule()) {
				SetupProc exitDLL = (SetupProc)m_module.getProcAddress("ExitDll");
				if (exitDLL) {
					exitDLL();
				}
			}
		}
	}

	VSTPlugin::FactoryInfo& VSTPluginFactory::GetFactoryInfo() {
		return m_factoryInfo;
	}

	VSTPlugin::ClassInfo& VSTPluginFactory::GetComponentInfo(uint32_t index) {
		return m_classInfoList[index];
	}

	size_t VSTPluginFactory::GetComponentCount() {
		return m_classInfoList.size();
	}

	VSTPlugin* VSTPluginFactory::CreateByIndex(uint32_t index) {
		VSTPlugin* plugin = CreatePlugin(m_factory, GetFactoryInfo(), GetComponentInfo(index), [this](VSTPlugin* p) { OnVSTPluginIsDestructed(p); });
		OnVSTPluginIsCreated(plugin);
		return plugin;
	}

	VSTPlugin* VSTPluginFactory::CreateByID(void* componentID) {
		for (uint32_t i = 0; i < GetComponentCount(); i++) {
			if (componentID == GetComponentInfo(i).CID().data()) {
				return CreateByIndex(i);
			}
		}
		PC_ERROR("VST: No VST found with this CID!");
	}

	size_t VSTPluginFactory::GetNumLoadedPlugins() {
		return m_loadedPlugins.size();
	}

	void VSTPluginFactory::OnVSTPluginIsCreated(VSTPlugin* plugin) {
		m_loadedPlugins.push_back(plugin);
		PC_TRACE("VST: Loaded plugin");
	}

	void VSTPluginFactory::OnVSTPluginIsDestructed(VSTPlugin* plugin){
		auto found = std::find(m_loadedPlugins.begin(), m_loadedPlugins.end(), plugin);
		if (found == m_loadedPlugins.end()) {
			PC_ERROR("VST: Plugin not found in loaded plugins list!");
		}
		m_loadedPlugins.erase(found);
	}

	// VSTHandler

	std::mutex VSTHandler::m_mutex = std::mutex();
	std::map<std::string, VSTPluginFactory*> VSTHandler::m_map = std::map<std::string, VSTPluginFactory*>();

	VSTPluginFactory* VSTHandler::FindOrCreateFactory(std::string modulePath) {
		std::unique_lock lock = std::unique_lock(m_mutex);

		auto found = m_map.find(modulePath);

		if (found == m_map.end()) {
			VSTPluginFactory* factory = new VSTPluginFactory(modulePath);
			found = m_map.emplace(modulePath, factory).first;
		}

		return found->second;
	}

	void VSTHandler::ShrinkList() {
		std::map<std::string, VSTPluginFactory*> tmp;

		for (auto it = m_map.begin(), end = m_map.end(); it != end;)
		{
			if (it->second->GetNumLoadedPlugins() == 0) {
				it = m_map.erase(it);
			}
			else {
				++it;
			}
		}
	}

	// VSTEffect

	VSTEffect::VSTEffect(ProdCast::ProdCastEngine* engine, const char* path) {
		m_engine = engine;
		m_settings = engine->GetAudioSettings();
		m_interleavedBuffer = new float[m_settings->outputChannels * m_settings->bufferSize];
		Init();
		LoadEffect(path);
	}

	void VSTEffect::LoadEffect(const char* path) {
		if (m_initialized) {
			PC_ERROR("VST: Effect already loaded!");
			return;
		}
		m_path = path;

		VSTPluginFactory* factory = ProdCast::VST::VSTHandler::FindOrCreateFactory(path);
		if (!factory) {
			PC_ERROR("VST: Couldn't find or create factory!");
			m_initialized = false;
			return;
		}
		else if (factory->GetComponentCount() == 0) {
			PC_ERROR("VST: no plugin!");
			return;
		}
		VSTPlugin* plugin = factory->CreateByIndex(0);

		if (!plugin) {
			PC_ERROR("VST: Error loading VST Effect!");
			m_initialized = false;
			return;
		}

		m_plugin = plugin;
		m_plugin->SetSpeakerArrangement(Steinberg::Vst::BusDirections::kInput, 0, Steinberg::Vst::SpeakerArr::kStereo);
		m_plugin->SetSpeakerArrangement(Steinberg::Vst::BusDirections::kOutput, 0, Steinberg::Vst::SpeakerArr::kStereo);
		m_plugin->SetSettings(m_settings);
		m_TransportInfo.playing = true;
		m_TransportInfo.sampleRate = m_settings->sampleRate;
		m_TransportInfo.tempo = 120;
		m_TransportInfo.smpBeginPos = 0;
		m_TransportInfo.smpEndPos = m_settings->bufferSize;
		m_initialized = true;
	}

	VSTEffect::~VSTEffect() {
		m_plugin->Suspend();
		delete m_plugin;
	}

				
	void VSTEffect::ProcessBuffer(float* buffer, unsigned int bufferSize, unsigned int numChannels) {
		if (!m_initialized) {
			return;
		}

		for (int i = 0; i < bufferSize; i++) {
			m_interleavedBuffer[i] = buffer[i * 2];
			m_interleavedBuffer[i + bufferSize] = buffer[i * 2 + 1];
		}

		ProcessInfo info;
		info.numChannels = numChannels;
		info.inputBuffer = m_interleavedBuffer;
		info.outputBuffer = m_interleavedBuffer;
		info.timeInfo = &m_TransportInfo;
		m_plugin->Process(info);

		for (int i = 0; i < bufferSize; i++) {
			buffer[i * 2] = info.outputBuffer[i];
			buffer[i * 2 + 1] = info.outputBuffer[bufferSize + i];
		}
	}

	VSTPlugin* VSTEffect::getPlugin() {
		return m_plugin;
	}
}
}