#include "VST/VSTPlugin.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "pluginterfaces/base/fplatform.h"

#include "Logger.h"

namespace ProdCast {
namespace VST {

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
		std::copy_n(m_hostName, m_hostName.length(), name);
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
		Steinberg::kResultOk;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue normalizedValue) {
		PC_INFO("Editing parameter {0} as {1}", id, normalizedValue);
		// m_plugin->ChangeParameter(id, normalizedValue);
		Steinberg::kResultOk;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::endEdit(Steinberg::Vst::ParamID id) {
		Steinberg::kResultOk;
	}

	Steinberg::tresult PLUGIN_API VSTHostContext::restartComponent(Steinberg::int32 flags) {
		if (m_plugin)
			m_plugin->restartComponent(flags);
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
		m_plugin->setSampleRate()
	}

	void VSTProcessor::Process(VSTProcessInfo& processInfo) {
		m_plugin->Process(processInfo);
	}

	void VSTProcessor::OnStopProcessing() {

	}

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

}
}