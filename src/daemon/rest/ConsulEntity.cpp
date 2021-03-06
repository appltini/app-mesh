#include <algorithm>
#include <thread>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include "../application/Application.h"
#include "../Label.h"
#include "../rest/ConsulConnection.h"
#include "../Configuration.h"
#include "../ResourceCollection.h"
#include "../../common/Utility.h"

std::shared_ptr<ConsulStatus> ConsulStatus::FromJson(const web::json::value &json)
{
	auto consul = std::make_shared<ConsulStatus>();
	for (const auto &app : json.as_object())
	{
		consul->m_apps[GET_STD_STRING(app.first)] = app.second;
	}
	return consul;
}

web::json::value ConsulStatus::AsJson() const
{
	auto result = web::json::value::object();
	for (const auto &app : m_apps)
	{
		result[app.first] = app.second;
	}
	return result;
}

ConsulTask::ConsulTask()
	: m_replication(0), m_priority(0), m_consulServicePort(0)
{
	m_condition = std::make_shared<Label>();
}

std::shared_ptr<ConsulTask> ConsulTask::FromJson(const web::json::value &jsonObj)
{
	auto consul = std::make_shared<ConsulTask>();
	if (HAS_JSON_FIELD(jsonObj, "content") && HAS_JSON_FIELD(jsonObj, "replication") &&
		jsonObj.at("replication").is_integer() &&
		jsonObj.at("content").is_object())
	{
		auto appJson = jsonObj.at("content");
		// mark consul application flag
		appJson[JSON_KEY_APP_metadata] = web::json::value::string(JSON_KEY_APP_CLOUD_APP);
		consul->m_app = Configuration::instance()->parseApp(appJson);
		SET_JSON_INT_VALUE(jsonObj, "replication", consul->m_replication);
		SET_JSON_INT_VALUE(jsonObj, "priority", consul->m_priority);
		SET_JSON_INT_VALUE(jsonObj, "port", consul->m_consulServicePort);
		if (HAS_JSON_FIELD(jsonObj, "condition"))
		{
			consul->m_condition = Label::FromJson(jsonObj.at("condition"));
		}
		for (std::size_t i = 1; i <= consul->m_replication; i++)
		{
			consul->m_taskIndexDic.insert(i);
		}
	}
	return consul;
}

web::json::value ConsulTask::AsJson() const
{
	auto result = web::json::value::object();
	result["replication"] = web::json::value::number(m_replication);
	result["priority"] = web::json::value::number(m_priority);
	result["port"] = web::json::value::number(m_consulServicePort);
	result["content"] = m_app->AsJson(false);
	if (m_condition != nullptr)
		result["condition"] = m_condition->AsJson();
	return result;
}

void ConsulTask::dump()
{
	const static char fname[] = "ConsulTask::dump() ";
	LOG_DBG << fname << "m_app=" << m_app->getName();
	LOG_DBG << fname << "m_priority=" << m_priority;
	LOG_DBG << fname << "m_replication=" << m_replication;
	m_app->dump();
}

bool ConsulTask::operator==(const std::shared_ptr<ConsulTask> &task)
{
	if (!task)
		return false;
	return m_replication == task->m_replication &&
		   m_priority == task->m_priority &&
		   m_consulServicePort == task->m_consulServicePort &&
		   m_app->operator==(task->m_app) &&
		   m_condition->operator==(task->m_condition);
}

std::shared_ptr<ConsulTopology> ConsulTopology::FromJson(const web::json::value &jsonObj, const std::string &hostName)
{
	auto topology = std::make_shared<ConsulTopology>();
	topology->m_hostName = hostName;
	if (jsonObj.is_array())
	{
		for (const auto &app : jsonObj.as_array())
		{
			auto appName = GET_JSON_STR_VALUE(app, "app");
			auto appIndex = GET_JSON_INT_VALUE(app, "index");
			topology->m_scheduleApps[appName] = appIndex;
		}
	}
	return topology;
}

web::json::value ConsulTopology::AsJson() const
{
	auto result = web::json::value::array(m_scheduleApps.size());
	std::size_t appIndex = 0;
	for (const auto &app : m_scheduleApps)
	{
		auto appJson = web::json::value::object();
		appJson["app"] = web::json::value::string(app.first);
		appJson["index"] = web::json::value::number(app.second);
		result[appIndex++] = appJson;
	}
	return result;
}

bool ConsulTopology::operator==(const std::shared_ptr<ConsulTopology> &topology)
{
	if (!topology)
		return false;
	if (m_scheduleApps.size() != topology->m_scheduleApps.size())
		return false;

	for (const auto &app : m_scheduleApps)
	{
		if (topology->m_scheduleApps.count(app.first) == 0)
			return false;
	}
	return true;
}

void ConsulTopology::dump()
{
	const static char fname[] = "ConsulTopology::dump() ";
	for (const auto &app : m_scheduleApps)
	{
		LOG_DBG << fname << "app:" << app.first << " host:" << m_hostName << " with index:" << app.second;
	}
}

ConsulNode::ConsulNode()
	: m_label(std::make_shared<Label>()), m_cores(0), m_total_bytes(0), m_free_bytes(0)
{
}

std::shared_ptr<ConsulNode> ConsulNode::FromJson(const web::json::value &jsonObj, const std::string &hostName)
{
	auto node = std::make_shared<ConsulNode>();
	node->m_hostName = hostName;
	if (HAS_JSON_FIELD(jsonObj, "label"))
	{
		node->m_label = Label::FromJson(jsonObj.at("label"));
	}
	if (HAS_JSON_FIELD(jsonObj, "appmesh"))
	{
		node->m_appmeshProxyUrl = GET_JSON_STR_VALUE(jsonObj, "appmesh");
	}
	if (HAS_JSON_FIELD(jsonObj, "resource"))
	{
		auto resourceJson = jsonObj.at("resource");
		if (HAS_JSON_FIELD(resourceJson, "cpu_cores"))
		{
			node->m_cores = GET_JSON_INT_VALUE(resourceJson, "cpu_cores");
		}
		if (HAS_JSON_FIELD(resourceJson, "mem_free_bytes"))
		{
			node->m_free_bytes = GET_JSON_NUMBER_VALUE(resourceJson, "mem_free_bytes");
		}
		if (HAS_JSON_FIELD(resourceJson, "mem_total_bytes"))
		{
			node->m_total_bytes = GET_JSON_NUMBER_VALUE(resourceJson, "mem_total_bytes");
		}
	}

	return node;
}

web::json::value ConsulNode::AsJson() const
{
	auto result = web::json::value::object();
	result["appmesh"] = web::json::value::string(m_appmeshProxyUrl);
	result["label"] = m_label->AsJson();
	auto resource = web::json::value::object();
	resource["cpu_cores"] = web::json::value::number(m_cores);
	resource["mem_total_bytes"] = web::json::value::number(m_total_bytes);
	resource["mem_free_bytes"] = web::json::value::number(m_free_bytes);
	result["resource"] = resource;
	return result;
}

void ConsulNode::assignApp(const std::shared_ptr<Application> &app)
{
	m_assignedApps[app->getName()] = app;
}

uint64_t ConsulNode::getAssignedAppMem() const
{
	return uint64_t(0);
}
