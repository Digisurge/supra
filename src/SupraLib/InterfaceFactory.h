// ================================================================================================
// 
// If not explicitly stated: Copyright (C) 2016, all rights reserved,
//      Rüdiger Göbl 
//		Email r.goebl@tum.de
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// ================================================================================================

#ifndef __INTERFACEFACTORY_H__
#define __INTERFACEFACTORY_H__

#include "AbstractNode.h"
#include "AbstractInput.h"
#include "AbstractOutput.h"

#include <tbb/flow_graph.h>

#include <functional>
#include <memory>

namespace supra
{
	class InterfaceFactory {
	public:
		static std::shared_ptr<tbb::flow::graph> createGraph();
		static std::shared_ptr<AbstractInput> createInputDevice(std::shared_ptr<tbb::flow::graph> pG, const std::string& nodeID, std::string deviceType, size_t numPorts);
		static std::shared_ptr<AbstractOutput> createOutputDevice(std::shared_ptr<tbb::flow::graph> pG, const std::string & nodeID, std::string deviceType, bool queueing);
		static std::shared_ptr<AbstractNode> createNode(std::shared_ptr<tbb::flow::graph> pG, const std::string & nodeID, std::string nodeType, bool queueing);
		static std::vector<std::string> getNodeTypes();

		// [TEE patch] Generic extension point so out-of-tree input devices
		// (that only need (graph, nodeID) to construct -- i.e. everything
		// else comes from their own parameters, see AbstractNode's
		// ValueRangeDictionary/ConfigurationDictionary) can register
		// themselves with createInputDevice() without editing this file, the
		// way createNode()'s m_nodeCreators map already allows for
		// processing nodes. Safe to call from another translation unit's own
		// static initializer (see inputCreators() below) -- unlike a plain
		// static map member, there is no ordering dependency on this class's
		// own static-initialization having already run first.
		typedef std::function<std::shared_ptr<AbstractInput>(tbb::flow::graph&, std::string)> inputCreationFunctionType;
		static void registerInputDeviceCreator(const std::string& deviceType, inputCreationFunctionType creator);

	private:
		typedef std::function<std::shared_ptr<AbstractNode>(tbb::flow::graph&, std::string, bool)> nodeCreationFunctionType;

		static std::map<std::string, nodeCreationFunctionType> m_nodeCreators;

		// [TEE patch] Construct-on-first-use instead of a plain static
		// member: registerInputDeviceCreator() is meant to be called from
		// another translation unit's global-object constructor (see
		// tee_supra_registration.cpp), and C++ gives no ordering guarantee
		// between that TU's static initializers and this class's own -- a
		// plain "static std::map<...> m_inputCreators;" could still be
		// zero-initialized memory (not yet placement-constructed) when that
		// call arrives, corrupting it. A function-local static is
		// guaranteed constructed on its first use (C++11 "magic statics"),
		// regardless of static-init order across TUs.
		static std::map<std::string, inputCreationFunctionType>& inputCreators();
	};
}

#endif //!__INTERFACEFACTORY_H__
